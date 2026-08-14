#ifndef MYLANG_H
#define MYLANG_H

#include <stdlib.h>
#include <string.h>

#include "MyLangVars.hpp"
#include "../Smart_Stack/stack.hpp"

#define PRINT_CUR_TYPE_AND_OP(NodeArr, str, ...) \
    fprintf(stderr, "type: %s", AllValueTypesTxt[(NodeArr).type]);\
    fprintf(stderr, str, ##__VA_ARGS__);\
    switch((NodeArr).type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  (NodeArr).value.num);                                  break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  (NodeArr).value.var_name);                             break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOper[(NodeArr).value.oper].Dump);                   break;\
        case TYPE_STR:   fprintf(stderr, "not an op");                                                        break;\
        default:         fprintf(stderr, "not an op");                                                        break;\
    }

#define ISVAR   ((**NodeArr).type == TYPE_VAR)
#define ISSTR   ((**NodeArr).type == TYPE_STR)
#define ISNUM   ((**NodeArr).type == TYPE_NUM)
#define ISARTHMOPER ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper <= ARTHM_ARCCTG)
#define ISOPER  ((**NodeArr).type == TYPE_OP)
#define SYNTAX_ERROR(NodeArr, ...) {\
    for (int i = -RANGE; i <= RANGE; i++) {\
        PRINT_CUR_TYPE_AND_OP((*NodeArr)[i], "%d ветка", i);\
    }\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); exit(ERR_PTR_NULL);}

#define MATCHOPER_ORSYNTAXERR(oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}

#define IN_PERSONAL_NAMETABLE(lines_of_code) \
    push(LexArr->ScopeBorders, (int) LexArr->ScopeSize);\
    lines_of_code\
    pop(LexArr->ScopeBorders, (int *) &(LexArr->ScopeSize));


typedef struct LexArr_t {
    Node_t *NodeArr;
    size_t NodeArrCapacity;
    size_t NodeArrSize;

    char **Scope;
    size_t ScopeSize;
    size_t ScopeCapacity;
    stack_t *ScopeBorders;
} LexArr_t;

LangErr_t ArrayOfLexemsCtor(LexArr_t * Arr);
void ArrayOfLexemsDtor(LexArr_t * LexArr);
int GetVarIndexInArr(char ** Arr, const char * Var, size_t ArrSize);
LangErr_t FillArrayOfLexems(LexArr_t *LexArr, const char * file_name);
LangErr_t PrintProgramToFile(const char *FileName, LexArr_t *LexArr);
void NodeDtor(Node_t **node);

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

#endif // MYLANG_H