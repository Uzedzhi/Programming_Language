#ifndef MYLANG_H
#define MYLANG_H

#include <stdlib.h>
#include <string.h>

#include "MyLangVars.hpp"
#include "../Smart_Stack/stack.hpp"

#define LEXEMS_FILE_NAME "lib/lang_lexems"
const size_t DICT_TABLE_SIZE = 1000;
const int RANGE = 3;

#define ISVAR   ((**NodeArr).type == TYPE_VAR)
#define ISSTR   ((**NodeArr).type == TYPE_STR)
#define ISNUM   ((**NodeArr).type == TYPE_NUM)
#define ISARTHMOPER ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper <= ARTHM_ARCCTG)
#define ISOPER  ((**NodeArr).type == TYPE_OP)

#define MATCH_ORSYNTAXERR(oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper)) {\
        syntax_error(LexArr, NodeArr, __VA_ARGS__);}}
#define CHECK_ORSYNTAXERROR(condition, str, ...) \
    if (!(condition)) {\
        syntax_error(LexArr, NodeArr, str, ##__VA_ARGS__);\
    }

#define IN_PERSONAL_NAMETABLE(lines_of_code) \
    push(LexArr->ScopeBorders, (int) LexArr->ScopeSize);\
    lines_of_code\
    pop(LexArr->ScopeBorders, (int *) &(LexArr->ScopeSize));

typedef struct scope_el_t {
    char *VarName;
    LangVarType_e Vartype;
} scope_el_t;

typedef struct LexArr_t {
    Debug_Node_t *NodeArr;
    size_t NodeArrCapacity;
    size_t NodeArrSize;

    char **Scope;
    size_t ScopeSize;
    size_t ScopeCapacity;
    stack_t *ScopeBorders;

    Node_t *Notes;
    size_t NotesSize;
    size_t NotesCapacity;

    const char *FileBuf;
    size_t Excepts_ptr;
} LexArr_t;

LangErr_t ArrayOfLexemsCtor(LexArr_t * Arr);
void ArrayOfLexemsDtor(LexArr_t * LexArr);
int GetVarIndexInArr(char ** Arr, const char * Var, size_t ArrSize);
LangErr_t FillArrayOfLexems(LexArr_t *LexArr, const char * file_name, const char **buf);
LangErr_t PrintProgramToFile(const char *FileName, LexArr_t *LexArr);
void NodeDtor(Node_t **node);

Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr);
Node_t * GetG(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetSECT(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNCS(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNC(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetOPS(Debug_Node_t **NodeArr, bool sep, LexArr_t *LexArr);
Node_t * GetOP(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetIF(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetCMP(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetWHILE(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetINIT(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetPRINT(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetFUNC_RUN(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetASSIGN(Debug_Node_t **NodeArr, Node_t *value, LexArr_t *LexArr);
Node_t * GetFUNCPARAMS(Debug_Node_t **NodeArr, bool AllowingNums, LexArr_t *LexArr);
Node_t * GetE(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetT(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetM(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetP(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetExpr(Debug_Node_t **NodeArr, LexArr_t *LexArr);
Node_t * GetVAR(Debug_Node_t **NodeArr, LexArr_t *LexArr, bool IsInit);
Node_t * GetNUM(Debug_Node_t **NodeArr, LexArr_t *LexArr);

LangOperType_e GetArthmOper(Debug_Node_t **NodeArr);
bool CheckIfOperNextAndInc(Debug_Node_t **NodeArr, LangOperType_e oper);
bool CheckIfOperNext(Debug_Node_t **NodeArr, LangOperType_e oper);

int GetVarIndex(LexArr_t *LexArr, char *VarName);
void PlaceVarInNameTable(LexArr_t *LexArr, char **VarName);

Node_t * create_node(void);
Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right);

#endif // MYLANG_H