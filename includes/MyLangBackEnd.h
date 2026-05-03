#ifndef PROGRAMTOASM_H
#define PROGRAMTOASM_H

#include "MyLang.h"

bool            CheckIfOperNext(Node_t *tree, int oper, LangType_e type);
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
LangCNOPType_e  CheckOperCNOPType(const char *FileBuf, int *str_len);
Node_t         *BuildAsmTreeInternal(const char *buffer, size_t *pos, char *NameTable[]);
Node_t         *BuildAsmTree(const char *buffer, char *NameTable[], size_t NameTableSize);
void            NodeDtor(Node_t **node);
int             GetVarIndexByName(char *NameTable[], size_t NameTableSize, const char *name);
void            PrintTab(FILE *fp, size_t tab);
int             GetVarInNameTable(char *Var, char *NameTable[]);
void            PrintLabel(FILE *fp, const char *prefix, size_t id);
void            GenPushVar(FILE *fp, Node_t * node,
                          char *NameTable[], size_t NameTableSize, bool IsFuncOp);
void            GenPopVar(FILE *fp, Node_t * node,
                          char *NameTable[], size_t NameTableSize, bool IsFuncOp);
void            GenCmpJumpFalse(FILE *fp, LangOperType_e cmp_op,
                               const char *label_prefix, size_t label_id, bool IsInverse);
void            GenCmp(FILE *fp, Node_t *cmp_node, const char *label_prefix, size_t label_id,
                       char *NameTable[], size_t NameTableSize,
                       bool IsFuncOp, bool IsInverse);
void            GenExpr(FILE *fp, Node_t *node,
                        char *NameTable[], size_t NameTableSize,
                        bool IsFuncOp);
void            GenInitList(FILE *fp, Node_t *tree,
                            char *NameTable[], size_t NameTableSize,
                            bool IsFuncOp);
void            GenIf(FILE *fp, Node_t *tree,
                      char *NameTable[], size_t NameTableSize,
                      bool IsFuncOp);
void            GenWhile(FILE *fp, Node_t *while_node,
                         char *NameTable[], size_t NameTableSize,
                         bool IsFuncOp);
void            GenFuncCall(FILE *fp, Node_t *call_node,
                            char *NameTable[], size_t NameTableSize,
                            bool IsFuncOp);
void            GenOut(FILE *fp, Node_t *out_node,
                       char *NameTable[], size_t NameTableSize,
                       bool IsFuncOp);
void            GenAssign(FILE *fp, Node_t *assign_node,
                          char *NameTable[], size_t NameTableSize,
                          bool IsFuncOp);
void            GenOp(FILE *fp, Node_t *tree,
                      char *NameTable[], size_t NameTableSize,
                      bool IsFuncOp);
void            GenOpList(FILE *fp, Node_t *tree,
                          char *NameTable[], size_t NameTableSize,
                          bool IsFuncOp);
void            GenFuncDef(FILE *fp, Node_t *tree,
                           char *NameTable[], size_t NameTableSize);
void            GenFuncList(FILE *fp, Node_t *tree,
                            char *NameTable[], size_t NameTableSize);
void            PrintProgramAssemblyFromTree(FILE *fp, Node_t *tree,
                                            char *NameTable[], size_t NameTableSize);
void            CompileFile(const char *InputFile, const char *OutputFile);

#endif // PROGRAMTOASM_H