#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../MyLibs/sassert.hpp"
#include "../includes/MyLangDump.hpp"
#include "../MyLibs/helper_funcs.hpp"
#include "../includes/MyLangHelpers.hpp"
#include "../includes/MyLangBack.hpp"

static char lang_last_error[ERROR_BUF_MAX_SIZE] = {};

void back_perror() {
    fprintf(stderr, RED "[ОШИБКА БЕКЭНДА]: " RESET "%s\n", lang_last_error);
}

void backend_error(int lin, Node_t *Tree, const char *format, ...) {
    fprintf(stderr, MAGENTA "line: %d, вид узла в дереве:\n" RESET, lin);

    if (Tree) {
        fprintf(stderr, GREEN "type: %s, " RESET, AllValueTypesTxt[Tree->type]);
        switch(Tree->type) {
            case TYPE_NUM:  fprintf(stderr, "num: %d\n",  Tree->value.num);                 break;
            case TYPE_VAR:  fprintf(stderr, "var: %s\n",  Tree->value.str);                 break;
            case TYPE_STR:  fprintf(stderr, "str: %s\n",  Tree->value.str);                 break;
            case TYPE_OP:   fprintf(stderr, "op: %s\n",   AllOper[Tree->value.oper].Dump);  break;
            default:        fprintf(stderr, "not an op\n");                                 break;
        }
    } else {
        fprintf(stderr, "(nil) - узла вообще нет, а он нужен\n");
    }

    va_list args;
    va_start(args, format);
    fprintf(stderr, RED "[ОШИБКА БЕКЭНДА]: " RESET);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    exit(NOK);
}

//=============================================================================
// Таблицы имен
//=============================================================================
int GetVarIndex(BackData_t *Back, const char *Name) {
    sassert(Back, ERR_PTR_NULL);
    sassert(Name, ERR_PTR_NULL);

    return InArrayStr(Back->Locals.Names, (char *) Name, Back->Locals.Size);
}

int DeclareVar(Node_t *Tree, BackData_t *Back, char *Name) {
    sassert(Back, ERR_PTR_NULL);
    sassert(Name, ERR_PTR_NULL);

    int index = GetVarIndex(Back, Name);
    if (index != -1)
        return index;   // переменная уже есть в этом scope - просто переиспользуем ячейку

    CHECK_ORBACKENDERROR(Back->Locals.Size < MAX_LOCAL_VARS,
                         "переменных в одном scope больше чем %zu, стек не резиновый", MAX_LOCAL_VARS);

    Back->Locals.Names[Back->Locals.Size] = Name;
    return (int) (Back->Locals.Size)++;
}

void GetVarAddr(BackData_t *Back, int index, char *buf) {
    sassert(Back, ERR_PTR_NULL);
    sassert(buf,  ERR_PTR_NULL);

    if ((size_t) index < Back->Locals.NParams) {
        // параметры лежат выше rbp, последний параметр - ближе всех
        size_t offset = FIRST_PARAM_OFFSET + WORD_SIZE * (Back->Locals.NParams - 1 - (size_t) index);
        snprintf(buf, MAX_ASM_ADDR_SIZE, "qword [rbp + %zu]", offset);
    } else {
        size_t offset = WORD_SIZE * ((size_t) index - Back->Locals.NParams + 1);
        snprintf(buf, MAX_ASM_ADDR_SIZE, "qword [rbp - %zu]", offset);
    }
}

FuncInfo_t * FindFunc(BackData_t *Back, const char *Name) {
    sassert(Back, ERR_PTR_NULL);
    sassert(Name, ERR_PTR_NULL);

    for (size_t i = 0; i < Back->FuncsSize; i++) {
        if (strcmp(Back->Funcs[i].Name, Name) == 0)
            return &(Back->Funcs[i]);
    }
    return NULL;
}

static size_t CountParams(Node_t *Params) {
    size_t count = 0;
    while (Params && Params->left) {
        count++;
        Params = Params->right;
    }
    return count;
}

// первый проход: чтобы функция могла вызвать функцию, объявленную ниже
LangErr_t RegisterAllFuncs(Node_t *Tree, BackData_t *Back) {
    sassert(Back, ERR_PTR_NULL);

    while (Tree && Tree->left) {
        Node_t *func = Tree->left;
        CHECK_ORBACKENDERROR(func->type == TYPE_VAR, "у функции потерялось имя");
        CHECK_ORBACKENDERROR(func->left,             "у функции <%s> нету тела", func->value.str);
        CHECK_ORBACKENDERROR(Back->FuncsSize < MAX_FUNCS,
                             "функций больше чем %zu, вы слишком продуктивны", MAX_FUNCS);

        Back->Funcs[Back->FuncsSize] = {func->value.str, CountParams(func->left->left), Back->FuncsSize};
        Back->FuncsSize++;

        Tree = Tree->right;
    }

    return OK;
}

//=============================================================================
// Выражения. Стековая машина: результат всегда лежит на вершине стека
//=============================================================================
LangErr_t TranslateE(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp,   ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree, "пустое выражение, а должно быть не пустое");

    if (ISNUM) {
        EMIT("mov  rax, %d", Tree->value.num);
        EMIT("push rax");
        return OK;
    }

    if (ISVAR) {
        int index = GetVarIndex(Back, Tree->value.str);
        CHECK_ORBACKENDERROR(index != -1, "переменной <%s> не существует в этом scope", Tree->value.str);

        char addr[MAX_ASM_ADDR_SIZE] = {};
        GetVarAddr(Back, index, addr);
        EMIT("mov  rax, %s%*s; %s", addr, (int) (24 - strlen(addr)), "", Tree->value.str);
        EMIT("push rax");
        return OK;
    }

    CHECK_ORBACKENDERROR(ISOPER, "в выражении что-то, что не является ни числом, ни переменной, ни оператором");

    // унарные
    if (Tree->value.oper == OPER_ARTHM_SQRT) {
        TranslateE(fp, Tree->left, Back);
        EMIT("pop  rax");
        EMIT("call IntSqrt");
        EMIT("push rax");
        return OK;
    }

    CHECK_ORBACKENDERROR(Tree->left && Tree->right,
                         "у бинарного оператора <%s> не хватает операнда", AllOper[Tree->value.oper].Dump);

    TranslateE(fp, Tree->left,  Back);
    TranslateE(fp, Tree->right, Back);
    EMIT("pop  rbx");   // правый
    EMIT("pop  rax");   // левый

    switch (Tree->value.oper) {
        case OPER_ARTHM_ADD:    EMIT("add  rax, rbx");                          break;
        case OPER_ARTHM_SUB:    EMIT("sub  rax, rbx");                          break;
        case OPER_ARTHM_MUL:    EMIT("imul rax, rbx");                          break;
        case OPER_ARTHM_DIV:    EMIT("cqo");
                                EMIT("idiv rbx");                               break;
        case OPER_ARTHM_POW:    EMIT("call IntPow");                            break;
        default:
            CHECK_ORBACKENDERROR(0, "оператор <%s> бекэнд пока не умеет (целочисленная арифметика, сорри)",
                                 AllOper[Tree->value.oper].Dump);
            break;
    }

    EMIT("push rax");
    return OK;
}

//=============================================================================
// Условия: на вершине стека остается 0 или 1
//=============================================================================
LangErr_t TranslateCOND(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree, "в if/пока нету сравнения");

    if (ISLOGICOPER) {
        TranslateCOND(fp, Tree->left,  Back);
        TranslateCOND(fp, Tree->right, Back);
        EMIT("pop  rbx");
        EMIT("pop  rax");
        if (Tree->value.oper == OPER_LOGIC_AND)
            EMIT("and  rax, rbx");
        else
            EMIT("or   rax, rbx");
        EMIT("push rax");
        return OK;
    }

    CHECK_ORBACKENDERROR(ISOPER,               "в сравнении нету знака сравнения");
    CHECK_ORBACKENDERROR(Tree->left,           "у сравнения нету левой части");
    CHECK_ORBACKENDERROR(Tree->right,          "у сравнения нету правой части");

    TranslateE(fp, Tree->left,  Back);
    TranslateE(fp, Tree->right, Back);
    EMIT("pop  rbx");
    EMIT("pop  rax");
    EMIT("cmp  rax, rbx");

    switch (Tree->value.oper) {
        case OPER_MORE: EMIT("setg  al");   break;
        case OPER_LESS: EMIT("setl  al");   break;
        case OPER_MEQ:  EMIT("setge al");   break;
        case OPER_LEQ:  EMIT("setle al");   break;
        case OPER_EQ:   EMIT("sete  al");   break;
        case OPER_NEQ:  EMIT("setne al");   break;
        default:
            CHECK_ORBACKENDERROR(0, "<%s> это не знак сравнения", AllOper[Tree->value.oper].Dump);
            break;
    }

    EMIT("movzx rax, al");
    EMIT("push rax");
    return OK;
}

//=============================================================================
// Операторы
//=============================================================================
LangErr_t TranslateIF(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->right, "у if нету тела");

    size_t label = (Back->LabelCount)++;

    EMIT_COMMENT("---------- if #%zu ----------", label);
    TranslateCOND(fp, Tree->left, Back);
    EMIT("pop  rax");
    EMIT("test rax, rax");
    EMIT("je   .IfElse_%zu", label);

    Node_t *IfBody = Tree->right;
    TranslateOPS(fp, IfBody->left, Back);
    EMIT("jmp  .IfEnd_%zu", label);

    EMIT_LABEL(".IfElse_%zu", label);
    if (IfBody->right)
        TranslateOPS(fp, IfBody->right, Back);

    EMIT_LABEL(".IfEnd_%zu", label);
    return OK;
}

LangErr_t TranslateWHILE(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->right, "у цикла нету тела");

    size_t label = (Back->LabelCount)++;

    EMIT_COMMENT("---------- while #%zu ----------", label);
    EMIT_LABEL(".WhileStart_%zu", label);
    TranslateCOND(fp, Tree->left, Back);
    EMIT("pop  rax");
    EMIT("test rax, rax");
    EMIT("je   .WhileEnd_%zu", label);

    TranslateOPS(fp, Tree->right, Back);
    EMIT("jmp  .WhileStart_%zu", label);
    EMIT_LABEL(".WhileEnd_%zu", label);

    return OK;
}

LangErr_t TranslateINITS(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);

    while (Tree && Tree->left) {
        Node_t *Init = Tree->left;
        CHECK_ORBACKENDERROR(Init->left && Init->right, "у объявления переменной нет имени или значения");

        TranslateE(fp, Init->right, Back);

        int index = DeclareVar(Init->left, Back, Init->left->value.str);
        char addr[MAX_ASM_ADDR_SIZE] = {};
        GetVarAddr(Back, index, addr);

        EMIT("pop  rax");
        EMIT("mov  %s, rax%*s; условимся %s = ...", addr, (int) (24 - strlen(addr)), "", Init->left->value.str);

        Tree = Tree->right;
    }

    return OK;
}

LangErr_t TranslateASSIGN(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->left && Tree->right, "у присваивания нет левой или правой части");
    CHECK_ORBACKENDERROR(Tree->left->type == TYPE_VAR, "присваивать можно только переменной");

    TranslateE(fp, Tree->right, Back);

    int index = GetVarIndex(Back, Tree->left->value.str);
    CHECK_ORBACKENDERROR(index != -1, "переменной <%s> не существует в этом scope", Tree->left->value.str);

    char addr[MAX_ASM_ADDR_SIZE] = {};
    GetVarAddr(Back, index, addr);

    EMIT("pop  rax");
    EMIT("mov  %s, rax%*s; %s = ...", addr, (int) (24 - strlen(addr)), "", Tree->left->value.str);
    return OK;
}

LangErr_t TranslateIN(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->left && Tree->left->type == TYPE_VAR, "спросить у зала можно только значение переменной");

    int index = DeclareVar(Tree->left, Back, Tree->left->value.str);
    char addr[MAX_ASM_ADDR_SIZE] = {};
    GetVarAddr(Back, index, addr);

    EMIT_COMMENT("спросим у зала значение %s", Tree->left->value.str);
    EMIT("mov  rbx, rsp");
    EMIT("and  rsp, -16");
    EMIT("lea  rsi, %s", addr + strlen("qword "));
    EMIT("lea  rdi, [rel FMT_IN]");
    EMIT("xor  eax, eax");
    EMIT("call scanf wrt ..plt");
    EMIT("mov  rsp, rbx");

    return OK;
}

LangErr_t TranslateOUT(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->left, "напишем... а что напишем то?");

    EMIT_COMMENT("напишем ...");
    TranslateE(fp, Tree->left, Back);
    EMIT("pop  rsi");
    EMIT("mov  rbx, rsp");
    EMIT("and  rsp, -16");
    EMIT("lea  rdi, [rel FMT_OUT]");
    EMIT("xor  eax, eax");
    EMIT("call printf wrt ..plt");
    EMIT("mov  rsp, rbx");

    return OK;
}

LangErr_t TranslateFUNC_CALL(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);

    Node_t *FuncVars   = Tree->left;
    Node_t *FuncName   = FuncVars->left;
    Node_t *FuncReturn = FuncVars->right;
    CHECK_ORBACKENDERROR(ISOPER_X(FuncVars, OPER_FUNC_VARS), "у вызова функции нету названия");
    CHECK_ORBACKENDERROR(ISVAR_X(FuncName),            "у вызова функции нету имени функции");
    CHECK_ORBACKENDERROR(ISVAR_X(FuncReturn),         "у вызова функции <%s> нету переменной, в которую класть результат ", FuncReturn->value.str);

    FuncInfo_t *Func = FindFunc(Back, FuncName->value.str);
    CHECK_ORBACKENDERROR(Func, "функции <%s> нету среди используемых формул", FuncName->value.str);

    size_t NArgs = 0;
    Node_t *Params = Tree->right;
    EMIT_COMMENT("---------- из формулы <%s> находим %s ----------", Func->Name, FuncReturn->value.str);

    while (Params && Params->left) {
        TranslateE(fp, Params->left, Back);
        NArgs++;
        Params = Params->right;
    }

    CHECK_ORBACKENDERROR(NArgs == Func->NParams,
                         "функция <%s> хочет %zu параметров, а ей дали %zu",
                         Func->Name, Func->NParams, NArgs);

    EMIT("call Func_%zu", Func->Label);
    if (NArgs != 0)
        EMIT("add  rsp, %zu", NArgs * WORD_SIZE);

    int index = DeclareVar(FuncReturn, Back, FuncReturn->value.str);
    char addr[MAX_ASM_ADDR_SIZE] = {};
    GetVarAddr(Back, index, addr);
    EMIT("mov  %s, rax%*s; %s", addr, (int) (24 - strlen(addr)), "", FuncReturn->value.str);

    return OK;
}

LangErr_t TranslateOP(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    if (!Tree)
        return OK;

    if (!ISOPER)
        return TranslateE(fp, Tree, Back);       // выражение ради выражения, ну ладно

    switch (Tree->value.oper) {
        case OPER_NEW_OP:           return TranslateOPS       (fp, Tree, Back);
        case OPER_IF:               return TranslateIF        (fp, Tree, Back);
        case OPER_WHILE:            return TranslateWHILE     (fp, Tree, Back);
        case OPER_NEW_INIT:         return TranslateINITS     (fp, Tree, Back);
        case OPER_VAR_DECLARATION:  return TranslateINITS     (fp, NewNode(TYPE_OP, {.oper = OPER_NEW_INIT}, Tree, NULL), Back);
        case OPER_ASSIGN:           return TranslateASSIGN    (fp, Tree, Back);
        case OPER_IN:               return TranslateIN        (fp, Tree, Back);
        case OPER_OUT:              return TranslateOUT       (fp, Tree, Back);
        case OPER_FUNC_CALL:        return TranslateFUNC_CALL (fp, Tree, Back);
        case OPER_NOP:              return OK;
        default:                    return TranslateE         (fp, Tree, Back);
    }
}

LangErr_t TranslateOPS(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    if (!Tree)
        return OK;

    if (!ISNEWOP)
        return TranslateOP(fp, Tree, Back);      // тело if-а может быть одним оператором без NEW_OP

    while (Tree) {
        if (Tree->left)
            TranslateOP(fp, Tree->left, Back);
        Tree = Tree->right;
    }

    return OK;
}

//=============================================================================
// Функции и шаги
//=============================================================================
LangErr_t TranslateFunc(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);
    CHECK_ORBACKENDERROR(Tree->type == TYPE_VAR, "у функции потерялось имя");
    CHECK_ORBACKENDERROR(Tree->left,             "у функции <%s> нету тела", Tree->value.str);
    CHECK_ORBACKENDERROR(Tree->right,            "у функции <%s> нету возвращаемого значения", Tree->value.str);

    FuncInfo_t *Func = FindFunc(Back, Tree->value.str);
    CHECK_ORBACKENDERROR(Func, "функцию <%s> не зарегистрировали, как так вышло", Tree->value.str);

    NameTable_t OldLocals = Back->Locals;        // свой личный scope, как IN_PERSONAL_NAMETABLE во фронте
    Back->Locals = {};

    // параметры кладем в таблицу имен первыми - они лежат выше rbp
    Node_t *Params = Tree->left->left;
    while (Params && Params->left) {
        CHECK_ORBACKENDERROR(Params->left->type == TYPE_VAR, "параметр функции <%s> не переменная", Tree->value.str);
        DeclareVar(Params->left, Back, Params->left->value.str);
        Params = Params->right;
    }
    Back->Locals.NParams = Back->Locals.Size;

    EMIT_RAW("");
    EMIT_COMMENT("=============================================================");
    EMIT_COMMENT(" формула <%s>, параметров: %zu", Func->Name, Func->NParams);
    EMIT_COMMENT("=============================================================");
    EMIT_LABEL("Func_%zu", Func->Label);
    EMIT("push rbp");
    EMIT("mov  rbp, rsp");
    EMIT("sub  rsp, %zu", LOCALS_RESERVE);

    TranslateOPS(fp, Tree->left->right, Back);   // тело формулы (условимся ... = ...)

    EMIT_COMMENT("возвращаемое значение");
    TranslateE(fp, Tree->right, Back);
    EMIT("pop  rax");
    EMIT("leave");
    EMIT("ret");

    Back->Locals = OldLocals;
    return OK;
}

LangErr_t TranslateFuncs(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);

    while (Tree && Tree->left) {
        TranslateFunc(fp, Tree->left, Back);
        Tree = Tree->right;
    }

    return OK;
}

LangErr_t TranslateSteps(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp, ERR_PTR_NULL);

    while (Tree && Tree->left) {
        Node_t *Step = Tree->left;
        CHECK_ORBACKENDERROR(Step->type == TYPE_NUM, "у шага нету номера");

        EMIT_RAW("");
        EMIT_COMMENT("---------- Шаг №%d ----------", Step->value.num);
        TranslateOPS(fp, Step->left, Back);

        if (Step->right) {
            size_t label = (Back->LabelCount)++;
            EMIT_COMMENT("примечание к шагу №%d (обработчик, вызывается по jmp .Note_%zu)", Step->value.num, label);
            EMIT("jmp  .NoteEnd_%zu", label);
            EMIT_LABEL(".Note_%zu", label);
            TranslateOPS(fp, Step->right, Back);
            EMIT_LABEL(".NoteEnd_%zu", label);
        }

        Tree = Tree->right;
    }

    return OK;
}

//=============================================================================
// Обвязка асма
//=============================================================================
void PrintAsmHeader(FILE *fp) {
    EMIT_RAW("; ============================================================");
    EMIT_RAW("; Сгенерировано бекэндом MyLang. Руками не править, обидится.");
    EMIT_RAW("; ============================================================");
    EMIT_RAW("default rel");
    EMIT_RAW("");
    EMIT_RAW("extern printf");
    EMIT_RAW("extern scanf");
    EMIT_RAW("global main");
    EMIT_RAW("");
    EMIT_RAW("section .rodata");
    EMIT_RAW("FMT_IN:     db \"%%lld\", 0");
    EMIT_RAW("FMT_OUT:    db \"%%lld\", 10, 0");
    EMIT_RAW("");
    EMIT_RAW("section .text");
}

void PrintAsmRuntime(FILE *fp) {
    EMIT_RAW("");
    EMIT_COMMENT("=============================================================");
    EMIT_COMMENT(" рантайм: целочисленный корень (метод Ньютона) и степень");
    EMIT_COMMENT("=============================================================");
    EMIT_LABEL("IntSqrt");                  // rax = sqrt(rax)
    EMIT("push rbx");
    EMIT("push rcx");
    EMIT("push rdx");
    EMIT("mov  rbx, rax");
    EMIT("cmp  rbx, 1");
    EMIT("jle  .SqrtSmall");
    EMIT("mov  rax, rbx");
    EMIT_LABEL(".SqrtLoop");
    EMIT("mov  rcx, rax");
    EMIT("mov  rax, rbx");
    EMIT("cqo");
    EMIT("idiv rcx");
    EMIT("add  rax, rcx");
    EMIT("sar  rax, 1");
    EMIT("cmp  rax, rcx");
    EMIT("jl   .SqrtLoop");
    EMIT("mov  rax, rcx");
    EMIT_LABEL(".SqrtDone");
    EMIT("pop  rdx");
    EMIT("pop  rcx");
    EMIT("pop  rbx");
    EMIT("ret");
    EMIT_LABEL(".SqrtSmall");
    EMIT("mov  rax, rbx");
    EMIT("jmp  .SqrtDone");

    EMIT_RAW("");
    EMIT_LABEL("IntPow");                   // rax = rax ^ rbx
    EMIT("push rcx");
    EMIT("push rdx");
    EMIT("mov  rcx, rbx");
    EMIT("mov  rdx, rax");
    EMIT("mov  rax, 1");
    EMIT("cmp  rcx, 0");
    EMIT("jle  .PowDone");
    EMIT_LABEL(".PowLoop");
    EMIT("imul rax, rdx");
    EMIT("dec  rcx");
    EMIT("jnz  .PowLoop");
    EMIT_LABEL(".PowDone");
    EMIT("pop  rdx");
    EMIT("pop  rcx");
    EMIT("ret");

    EMIT_RAW("");
    EMIT_RAW("section .note.GNU-stack noalloc noexec nowrite progbits");
}

LangErr_t TranslateProgram(FILE *fp, Node_t *Tree, BackData_t *Back) {
    sassert(fp,   ERR_PTR_NULL);
    sassert(Back, ERR_PTR_NULL);
    sassert(Tree, ERR_PTR_NULL);
    DUMP_LANGNODE(Tree, "до транслирования дерево");

    CHECK_ORBACKENDERROR(Tree, "дерево пустое, компилировать нечего");
    CHECK_ORBACKENDERROR((ISOPER) && Tree->value.oper == OPER_PROGRAM_START,
                         "в корне дерева должен быть %s", AllOper[OPER_PROGRAM_START].Tree);

    RegisterAllFuncs(Tree->left, Back);
    PrintAsmHeader(fp);

    EMIT_RAW("");
    EMIT_COMMENT("=============================================================");
    EMIT_COMMENT(" ход решения");
    EMIT_COMMENT("=============================================================");
    EMIT_LABEL("main");
    EMIT("push rbp");
    EMIT("mov  rbp, rsp");
    EMIT("sub  rsp, %zu", LOCALS_RESERVE);

    Back->Locals         = {};
    Back->Locals.NParams = 0;
    TranslateSteps(fp, Tree->right, Back);

    EMIT_RAW("");
    EMIT("xor  eax, eax");
    EMIT("leave");
    EMIT("ret");

    TranslateFuncs(fp, Tree->left, Back);
    PrintAsmRuntime(fp);

    FINISHTXTDUMPS();

    return OK;
}

void print_help(char *argv[]) {
    printf("USAGE: %s <input_tree_file> <output_asm_file>\n", argv[0]);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_help(argv);
        return OK;
    }

    Node_t *Tree = NULL;
    LangErr_t res = ReadTreeFromFile(argv[1], &Tree);
    if (res != OK) {
        back_perror();
        return res;
    }

    FILE *fp = fopen(argv[2], "w");
    RETURN_ERR(fp, ERR_FILE_DOES_NOT_EXIST, "не удалось открыть файл <%s>", argv[2]);

    BackData_t Back = {};
    if (TranslateProgram(fp, Tree, &Back) != OK) {
        back_perror();
        fclose(fp);
        return NOK;
    }

    fclose(fp);
    NodeDtor(Tree);

    fprintf(stdout, GREEN "Asm file <%s> was made successfully\n" RESET, argv[2]);
    return OK;
}
