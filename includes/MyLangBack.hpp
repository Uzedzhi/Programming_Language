#ifndef MYLANG_BACK_HPP
#define MYLANG_BACK_HPP

#include <stdio.h>
#include <stdlib.h>

#include "MyLangHelpers.hpp"

#define ISVAR           (Tree->type == TYPE_VAR)
#define ISVAR_X(Tree)   (Tree->type == TYPE_VAR)

#define ISSTR           (Tree->type == TYPE_STR)
#define ISSTR_X(Tree)   (Tree->type == TYPE_STR)

#define ISNUM           (Tree->type == TYPE_NUM)
#define ISNUM_X(Tree)   (Tree->type == TYPE_NUM)

#define ISARTHMOPER (Tree->type == TYPE_OP && Tree->value.oper >= ARTHM_MUL && Tree->value.oper <= ARTHM_ARCCTG)

#define ISOPER          (Tree->type == TYPE_OP)
#define ISOPER_X(Tree, op)  (Tree->type == TYPE_OP && Tree->value.oper == op)

const size_t MAX_LOCAL_VARS     = 64;   
const size_t MAX_FUNCS          = 64;
const size_t LOCALS_RESERVE     = MAX_LOCAL_VARS * 8;
const size_t MAX_ASM_ADDR_SIZE  = 64;
const size_t WORD_SIZE          = 8;
const size_t FIRST_PARAM_OFFSET = 16;

struct FuncInfo_t {
    char   *Name;
    size_t  NParams;
    size_t  Label;
};

struct NameTable_t {
    char   *Names[MAX_LOCAL_VARS];
    size_t  Size;
    size_t  NParams;
};

struct BackData_t {
    NameTable_t Locals;
    FuncInfo_t  Funcs[MAX_FUNCS];
    size_t      FuncsSize;
    size_t      LabelCount;
    const char *CurFuncName;
};

#define CHECK_ORBACKENDERROR(condition, str, ...)           \
    if (!(condition)) {                                     \
        backend_error(__LINE__, Tree, str, ##__VA_ARGS__);  \
    }

#define EMIT(str, ...)          fprintf(fp, "    " str "\n", ##__VA_ARGS__)
#define EMIT_LABEL(str, ...)    fprintf(fp, str ":\n",       ##__VA_ARGS__)
#define EMIT_COMMENT(str, ...)  fprintf(fp, "; " str "\n",   ##__VA_ARGS__)
#define EMIT_RAW(str, ...)      fprintf(fp, str "\n",        ##__VA_ARGS__)

#define ISNEWOP     (Tree->type == TYPE_OP && Tree->value.oper == OPER_NEW_OP)
#define ISLOGICOPER (Tree->type == TYPE_OP && (Tree->value.oper == OPER_LOGIC_AND || \
                                               Tree->value.oper == OPER_LOGIC_OR))

//-----------------------------------------------------------------------------
// MyLangTreeRead.cpp
//-----------------------------------------------------------------------------
LangErr_t  ReadTreeFromFile     (const char *FileName, Node_t **Tree);
Node_t *   ReadNodeFromBuffer   (const char *buffer, size_t *pos);
LangErr_t  ReadValueToNode      (const char *buffer, size_t *pos, Node_t *node);

//-----------------------------------------------------------------------------
// MyLangBack.cpp
//-----------------------------------------------------------------------------
void       backend_error        (int lin, Node_t *Tree, const char *format, ...);
void       back_perror          ();

LangErr_t  TranslateProgram     (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateFuncs       (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateFunc        (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateSteps       (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateOPS         (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateOP          (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateIF          (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateWHILE       (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateINITS       (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateASSIGN      (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateIN          (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateOUT         (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateFUNC_CALL   (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateCOND        (FILE *fp, Node_t *Tree, BackData_t *Back);
LangErr_t  TranslateE           (FILE *fp, Node_t *Tree, BackData_t *Back);

LangErr_t  RegisterAllFuncs     (Node_t *Tree, BackData_t *Back);
FuncInfo_t*FindFunc             (BackData_t *Back, const char *Name);
int        GetVarIndex          (BackData_t *Back, const char *Name);
int        DeclareVar           (Node_t *Tree, BackData_t *Back, char *Name);
void       GetVarAddr           (BackData_t *Back, int index, char *buf);

void       PrintAsmHeader       (FILE *fp);
void       PrintAsmRuntime      (FILE *fp);

#endif // MYLANG_BACK_HPP
