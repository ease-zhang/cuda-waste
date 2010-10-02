
#include <stdio.h>
#include <iostream>
#include "PtxLexer.h"
#include "PtxParser.h"
#include "../emulator/tree.h"
#include "../emulator/emulator.h"
#include "../emulator/string-table.h"

static STRING_TABLE * string_table;

TREE * convert(pANTLR3_BASE_TREE node)
{
    TREE * result = new TREE();
    char * text = string_table->Entry((char*)node->getText(node)->chars);
    result->SetText(text);
    int type = node->getType(node);
    result->SetType(type);
    for (int i = 0; i < (int)node->getChildCount(node); ++i)
    {
        pANTLR3_BASE_TREE child = (pANTLR3_BASE_TREE)node->getChild(node, i);
        TREE * c = convert(child);
        result->AddChild(c);
        c->SetParent(result);
    }
    return result;
}

TREE * parse(char * ptx_module)
{
    pANTLR3_INPUT_STREAM       input;
    pPtxLexer             lxr;
    pANTLR3_COMMON_TOKEN_STREAM        tstream;
    pPtxParser                psr;
    PtxParser_prog_return     langAST;
    pANTLR3_COMMON_TREE_NODE_STREAM    nodes;

    input  = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)ptx_module, strlen(ptx_module), 0);
    if ( input == NULL )
    {
        ANTLR3_FPRINTF(stderr, "Unable to read from string PTX module.\n");
    }
    else {
    }
    lxr = PtxLexerNew(input);
    if ( lxr == NULL )
    {
        ANTLR3_FPRINTF(stderr, "Unable to create the lexer due to malloc() failure1\n");
        return 0;
    }
    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    if (tstream == NULL)
    {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate token stream\n");
        return 0;
    }
    psr = PtxParserNew(tstream);  // CParserNew is generated by ANTLR3
    if (psr == NULL)
    {
        ANTLR3_FPRINTF(stderr, "Out of memory trying to allocate parser\n");
        return 0;
    }
    langAST = psr->prog(psr);
    if (psr->pParser->rec->getNumberOfSyntaxErrors(psr->pParser->rec) > 0)
    {
        ANTLR3_FPRINTF(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->getNumberOfSyntaxErrors);
    }
    else
    {
		string_table = new STRING_TABLE();

        // Convert Antlr tree into AST tree.
        TREE * result = convert(langAST.tree);

		// Free up Antlr data structures.
        psr->free  (psr);
        tstream->free(tstream);
        lxr->free  (lxr);
        input->close (input);

		// Return the AST.
        return result;
    }
    return 0;
}
