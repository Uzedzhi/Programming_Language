#ifndef PROGRAMTOASM_H
#define PROGRAMTOASM_H

#include <stdint.h>
#include "MyLang.hpp"

typedef struct BackEnd_t {
    Node_t  *Tree;
    char   **NameTable;
    size_t NameTableSize;
    size_t NameTableCapacity;

    char   **Scope;
    size_t ScopeSize;
    size_t ScopeCapacity;

    stack_t *ScopeBorders;

    int64_t *RegexTable;
} BackEnd_t;

#define ALL_REGEX(n) \
    n(RAX   ,   "rax") \
    n(RBX   ,   "rbx") \
    n(RCX   ,   "rcx") \
    n(RDX   ,   "rdx") \
    n(RSI   ,   "rsi") \
    n(RDI   ,   "rdi") \
    n(R8    ,   "r8") \
    n(R9    ,   "r9") \
    n(R10   ,   "r10") \
    n(R11   ,   "r11") \
    n(R12   ,   "r12") \
    n(R13   ,   "r13") \
    n(R14   ,   "r14") \
    n(R15   ,   "r15")

#define MAKE_ENUM(n1, n2) n1,
#define MAKE_STR(n1, n2) n2,  

enum AllRegex_e {
    ALL_REGEX(MAKE_ENUM)
};

string AllRegexStr[] = {ALL_REGEX(MAKE_STR)};
const size_t NumOfRegex = sizeof(AllRegexStr) / sizeof(AllRegexStr[0]);
#undef MAKE_ENUM
#undef MAKE_STR
#undef ALL_REGEX


void            PrintHelp(void);
size_t          get_file_size(FILE *fp);
void            nullify_anything_extra(char *buffer, size_t file_size, size_t actually_read);
char           *get_buffer_from_file(FILE *fp, size_t file_size);
void            SkipSpaces(size_t *pos, const char *buffer);
char           *ReadQuotedString(size_t *pos, const char *buffer);
Node_t         *create_node(void);
Node_t         *NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right);
bool            IsIntStr(const char *s);
LangOperType_e  CheckOperType(const char *FileBuf, int *str_len);
Node_t         *BuildAsmTreeInternal(const char *buffer, size_t *pos, BackEnd_t *BackEndStruct);
Node_t         *BuildAsmTree(const char *buffer, BackEnd_t *BackEndStruct);
void            NodeDtor(Node_t **node);
int             GetVarIndexByName(BackEnd_t *BackEndStruct, const char *name);
void            PrintTab(FILE *fp, size_t tab);
int             GetVarInNameTable(char *Var, char *NameTable[]);
void            PrintLabel(FILE *fp, const char *prefix, size_t id);
void            GenPushVar(FILE *fp, BackEnd_t *BackEndStruct, Node_t * node,
                           bool IsFuncOp);
void            GenPopVar(FILE *fp, BackEnd_t *BackEndStruct, Node_t * node,
                           bool IsFuncOp);
void            GenCmpJumpFalse(FILE *fp, LangOperType_e cmp_op,
                               const char *label_prefix, size_t label_id, bool IsInverse);
void            GenCmp(FILE *fp, BackEnd_t *BackEndStruct, Node_t *cmp_node, const char *label_prefix, size_t label_id,
                       bool IsFuncOp, bool IsInverse);
void            GenExpr(FILE *fp, BackEnd_t *BackEndStruct, Node_t *node,
                        bool IsFuncOp);
void            GenInitList(FILE *fp, BackEnd_t *BackEndStruct, Node_t *tree,
                            bool IsFuncOp);
void            GenIf(FILE *fp, BackEnd_t *BackEndStruct, Node_t *tree,
                      bool IsFuncOp);
void            GenWhile(FILE *fp, BackEnd_t *BackEndStruct, Node_t *while_node,
                         bool IsFuncOp);
void            GenFuncCall(FILE *fp, Node_t *call_node, bool IsFuncOp);
void            GenOut(FILE *fp, Node_t *out_node, bool IsFuncOp);
void            GenAssign(FILE *fp, Node_t *assign_node, bool IsFuncOp);
void            GenOp(FILE *fp, Node_t *tree, bool IsFuncOp);
void            GenOpList(FILE *fp, Node_t *tree, bool IsFuncOp);
void            GenFuncDef(FILE *fp, BackEnd_t *BackEndStruct, Node_t *tree);
void            GenFuncList(FILE *fp, BackEnd_t *BackEndStruct, Node_t *tree);
void            PrintProgramAssemblyFromTree(FILE *fp, BackEnd_t *BackEndStruct, Node_t *tree);
void            CompileFile(const char *InputFile, const char *OutputFile);

#endif // PROGRAMTOASM_H