#ifndef MYLANGSYNTAX_H
#define MYLANGSYNTAX_H


#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "MyLang.hpp"


#define PRINT_CUR_TYPE_AND_OP(NodeArr, str) \
    fprintf(stderr, "%s: type: %s ", str, AllValueTypesTxt[(NodeArr).type]);\
    switch((NodeArr).type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  (NodeArr).value.num);                                  break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  (NodeArr).value.var_name);                             break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[(NodeArr).value.oper]);                 break;\
    }


Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr);
Node_t * GetG(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetSECT(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNCS(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNC(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetOPS(Node_t **NodeArr, LangOperType_e sep, LexArr_t *LexArr);
Node_t * GetOP(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetIF(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetCMP(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetWHILE(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetINIT(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetPRINT(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNC_RUN(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetASSIGN(Node_t **NodeArr, Node_t *value, LexArr_t *LexArr);
Node_t * GetFUNCPARAMS(Node_t **NodeArr, bool AllowingNums, LexArr_t *LexArr);
Node_t * GetE(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetT(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetM(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetP(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetExpr(Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetVAR(Node_t **NodeArr, LexArr_t *LexArr, bool IsInit);
Node_t * GetNUM(Node_t **NodeArr, LexArr_t *LexArr);


LangOperType_e GetArthmOper(Node_t **NodeArr);
bool CheckIfOperNextAndInc(Node_t **NodeArr, LangOperType_e oper);
bool CheckIfOperNext(Node_t **NodeArr, LangOperType_e oper);


int GetVarIndex(LexArr_t *LexArr, char *VarName);
void PlaceVarInNameTable(LexArr_t *LexArr, char **VarName);


Node_t * create_node(void);
Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right);


#endif // MYLANGSYNTAX_H
