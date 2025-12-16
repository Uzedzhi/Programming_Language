#ifndef MYLANGSYNTAX_H
#define MYLANGSYNTAX_H

#include <string.h>
#include "MyLang.h"

#define PRINT_CUR_TYPE_AND_OP(NodeArr, str) \
    fprintf(stderr, "%s: type: %s ", str, AllValueTypesTxt[(**NodeArr).type]);\
    switch((**NodeArr).type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  (**NodeArr).value.num);                      break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  (**NodeArr).value.var_name);                 break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[(**NodeArr).value.oper]);     break;\
        case TYPE_CNOP:  fprintf(stderr, "cnop: %s\n", AllOperCNOPDumpStr[(**NodeArr).value.CNop]); break;\
    }
#define DUMP_LANGNODE(node, str) {\
    create_tree_graph(node);\
    print_to_html(node, false, str);\
    PRINT_CUR_TYPE_AND_OP(NodeArr, str);\
    }

Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr);
Node_t * GetOPS(Node_t **NodeArr);
Node_t * GetE(Node_t **NodeArr);
Node_t * GetT(Node_t **NodeArr);
Node_t * GetM(Node_t **NodeArr);
Node_t * GetP(Node_t **NodeArr);
Node_t * GetExpr(Node_t **NodeArr);
LangOperType_e GetArthmOper(Node_t **NodeArr);
Node_t * GetG(Node_t ** NodeArr);
Node_t * GetSECT(Node_t ** NodeArr);
Node_t * GetOPS(Node_t **NodeArr);
Node_t * GetFUNC(Node_t **NodeArr);
Node_t * GetVAR(Node_t **NodeArr);
Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right);
bool CheckIfOperNextAndInc(Node_t **NodeArr, int oper, LangType_e type);
bool CheckIfOperNext(Node_t **NodeArr, int oper, LangType_e type);
Node_t * create_node();
Node_t * GetFUNCS(Node_t **NodeArr);
Node_t *GetOP(Node_t **NodeArr);

#endif // MYLANGSYNTAX_H