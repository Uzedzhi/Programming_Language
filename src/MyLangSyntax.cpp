#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include "../MyLibs/sassert.hpp"
#include "../includes/MyLangFront.hpp"
#include "../includes/MyLangDump.hpp"
#include "../Smart_Stack/stack.hpp"
#include "../MyLibs/helper_funcs.hpp"
#include "../includes/MyLangHelpers.hpp"

void syntax_error(int lin, LexArr_t *LexArr, Debug_Node_t **NodeArr, const char *format, ...) {
    fprintf(stderr, MAGENTA "line: %d, Вид в дереве:\n" RESET, lin);

    for (int i = ((**NodeArr).line > RANGE + 1) ? -RANGE : 0; i <= RANGE; i++) {
        Debug_Node_t CurNode = (*NodeArr)[i];
        if (i == 0)
            fprintf(stderr, GREEN);
        fprintf(stderr, "(%d) type: %s, ", i, AllValueTypesTxt[CurNode.type]);
        switch(CurNode.type) {
            case TYPE_NUM:   fprintf(stderr, "num: %d\n",  CurNode.value.num);                break;
            case TYPE_VAR:   fprintf(stderr, "var: %s\n",  CurNode.value.str);                break;
            case TYPE_STR:   fprintf(stderr, "str: %s\n",  CurNode.value.str);                break;
            case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOper[CurNode.value.oper].Dump); break;
            default:         fprintf(stderr, "not an op");                                    break;
        }

        if (i == 0)
            fprintf(stderr, RESET);
    }

    fprintf(stderr, MAGENTA "\nВид в коде:\n" RESET);

    size_t ErrPos = (*NodeArr)->pos;
    int    line   = (*NodeArr)->line;
    va_list args;
    va_start(args, format);

    const char *CurPos = NULL;

    if (line >= RANGE)
        CurPos = GetNLinesDown(LexArr->FileBuf, LexArr->FileBuf + ErrPos, RANGE) + 1;
    else
        CurPos = LexArr->FileBuf;
    
    for (int i = -RANGE; i <= RANGE && *CurPos; i++) {
        int len = strcspn(CurPos, "\n\0");
        fprintf(stderr, RESET "%d: %.*s\n", line + i, len, CurPos);

        if (CurPos >= ErrPos + LexArr->FileBuf - len - 1 && CurPos <= ErrPos + LexArr->FileBuf) {
            RepeatChar(' ', ErrPos - (CurPos - LexArr->FileBuf) - 2, CurPos);
            fprintf(stderr, RED "/\\\n");
            RepeatChar(' ', ErrPos - (CurPos - LexArr->FileBuf) - 2, CurPos);
            fprintf(stderr, "||\n");
            
            vfprintf(stderr, format, args);
            fprintf(stderr, " \n");
        }

        CurPos += len + 1;
    }

    va_end(args);
    exit(NOK);
}

void PrintStrArr(char **str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (str[i] == NULL)
            fprintf(stderr, "%zu (nil)", i);
        else
            fprintf(stderr, "%zu %s", i, str[i]);
        fprintf(stderr, "\n");
    }
}


Node_t * GetVAR(Debug_Node_t **NodeArr, LexArr_t *LexArr, bool IsInit) {
    CHECK_ORSYNTAXERROR(ISVAR, "там где ожидалось название переменной его нету(");

    char *name = (**NodeArr).value.str;
    Node_t *node = NewNode(TYPE_VAR, (LangElem_u){.str = name}, NULL, NULL);

    int VarIndex = InArrayStr(LexArr->Scope, name, LexArr->ScopeSize);
    if (IsInit) {
        CHECK_ORSYNTAXERROR(VarIndex == -1, "переменная <%s> уже существует в этом scope!", name);

        CheckArrayAndReallocate((void **) &(LexArr->Scope), LexArr->ScopeSize, &(LexArr->ScopeCapacity), sizeof(char *));
        LexArr->Scope[LexArr->ScopeSize] = name;
        LexArr->ScopeSize++;
    } else {
        CHECK_ORSYNTAXERROR(VarIndex != -1, "не существует переменной <%s> (в видимых scope)", name);
    }
    (*NodeArr)++;
    return node;
}

Node_t * GetNUM(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    CHECK_ORSYNTAXERROR(ISNUM, "там где ожидалось число его нету");
    Node_t *node = NewNode(TYPE_NUM, {.num = (**NodeArr).value.num}, NULL, NULL);
    (*NodeArr)++;
    return node;
}

Node_t * GetFUNCPARAMS(Debug_Node_t **NodeArr, bool AllowingNums, LexArr_t *LexArr) {
    Node_t * AllParams = NewNode(TYPE_OP, {.oper = OPER_NEW_PARAM}, NULL, NULL);
    Node_t * CurNode   = AllParams;
    while (ISVAR && (CurNode->left  = GetVAR(NodeArr, LexArr, !AllowingNums)) != NULL ||
           AllowingNums && ISNUM && (CurNode->left = GetNUM(NodeArr, LexArr)) != NULL) {
        if (!CheckIfOperNextAndInc(NodeArr, OPER_COMMA))
            break;
        CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_PARAM}, NULL, NULL);
        CurNode = CurNode->right;
    }
    return AllParams;
}

char *GetFuncName(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    char *FuncName = CALLOC_WITH_TYPE(MAX_STR_SIZE, char);
    
    int left = MAX_STR_SIZE - 1;
    while (ISVAR) {
        char *name = (**NodeArr).value.str;
        (*NodeArr)++;
        
        int len = strlen(name);
        if (left > len) {
            strcat(FuncName, name);
        } else {
            CHECK_ORSYNTAXERROR(0, "имя функции слишком длинное");
        }

        left -= len;
    }

    return FuncName;
}

char * IntToAscii(int num) {
    int numlen = (int) log10(num) + 2;
    char *buf = CALLOC_WITH_TYPE(numlen + 1, char);
    snprintf(buf, numlen, "%d", num);

    return buf;
}

Node_t *PlaceFuncNameInNameTable(Debug_Node_t **NodeArr, LexArr_t *LexArr, int num) {
    char *FuncName = GetFuncName(NodeArr, LexArr);

    int index = InArrayStr(LexArr->Scope, FuncName, LexArr->ScopeSize);
    CHECK_ORSYNTAXERROR(index == -1, "функция <%s> уже существует", FuncName);

    char *buf = IntToAscii(num);
    int numindex = InArrayStr(LexArr->Scope, buf, LexArr->ScopeSize);
    CHECK_ORSYNTAXERROR(numindex == -1, "функция с номером <%d> уже существует", num);
    
    CheckArrayAndReallocate((void **) &(LexArr->Scope), LexArr->ScopeSize - 1, &(LexArr->ScopeCapacity), sizeof(char *));
    LexArr->Scope[LexArr->ScopeSize++] = buf;
    LexArr->Scope[LexArr->ScopeSize++] = FuncName;

    return NewNode(TYPE_VAR, {.str = FuncName}, NULL, NULL);
}

Node_t * GetFUNC(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    MATCH_ORSYNTAXERR(OPER_PAR_OPEN,  "У формулы нет номера в начале. Пример: (4) Формула для ...");

    Node_t *value = GetNUM(NodeArr, LexArr);
    MATCH_ORSYNTAXERR(OPER_PAR_CLOSE, "У номера формулы нет закрывающей скобки");

    MATCH_ORSYNTAXERR(OPER_FUNC_DECL_NAME, "вы наверно хотели обьявить формулу, но у нет ключевого слова \"%s\"", AllOper[OPER_FUNC_DECL_NAME].Text[0]);
    Node_t *func = PlaceFuncNameInNameTable(NodeArr, LexArr, value->value.num);
    
    func->left = NewNode(TYPE_OP, {.oper = OPER_FUNC_BODY}, NULL, NULL);
    Node_t *func_body = func->left;

    MATCH_ORSYNTAXERR(OPER_IN_POINT, "у функции <%s> нету ключевого слова \"%s\" (да знаю что не хочется писать, но так красивее, напиши несколько букв, а то обижусь)", func->value.str, AllOper[OPER_IN_POINT].Text[0]);
    MATCH_ORSYNTAXERR(OPER_PAR_OPEN, "У функции с именем <%s> нет параметров (открывающейся скобки нет)", func->value.str);

    IN_PERSONAL_NAMETABLE(
        func_body->left = GetFUNCPARAMS(NodeArr, false, LexArr);

        MATCH_ORSYNTAXERR(OPER_PAR_CLOSE,   "У функции <%s> нет закрывающей скобки у параметров", func->value.str);
        MATCH_ORSYNTAXERR(OPER_COLON,       "в функции нет разделителя :");

        func_body->right = GetOPS(NodeArr, false, LexArr);

        MATCH_ORSYNTAXERR(OPER_RETURN, "нету возвращаемого значения!");
        func->right = GetE(NodeArr, LexArr);
        func->left  = func_body;
    )

    return func;
}

Node_t * GetFUNCS(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * AllFuncs = NewNode(TYPE_OP, {.oper = OPER_NEW_FUNC}, NULL, NULL);
    Node_t * CurNode = AllFuncs;

    while (CheckIfOperNext(NodeArr, OPER_PAR_OPEN)) {
        CurNode->left  = GetFUNC(NodeArr, LexArr);

        if (CheckIfOperNext(NodeArr, OPER_PAR_OPEN))
            CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_FUNC}, NULL, NULL);
        
        CurNode = CurNode->right;
    }

    return AllFuncs;
}

Node_t * GetOPS(Debug_Node_t **NodeArr, bool in_scope, LexArr_t *LexArr) {
    Node_t *Opers   = NewNode(TYPE_OP, (LangElem_u){.oper = OPER_NEW_OP}, NULL, NULL);
    Node_t *CurNode = Opers;

    while (CheckIfOperNext(NodeArr, OPER_IF)               ||
           CheckIfOperNext(NodeArr, OPER_VAR_DECLARATION)  ||
           CheckIfOperNext(NodeArr, OPER_WHILE)            ||
           CheckIfOperNext(NodeArr, OPER_OUT)              ||
           CheckIfOperNext(NodeArr, OPER_IN)               ||
           CheckIfOperNext(NodeArr, OPER_ASSIGN)           ||
           CheckIfOperNext(NodeArr, OPER_FUNC_CALL)        ||
           CheckIfOperNext(NodeArr, OPER_SCOPE_CLOSE)      ||
           CheckIfOperNext(NodeArr, OPER_SCOPE_OPEN)       ||
           ISVAR) {
        Node_t *op = NULL;

        if (CheckIfOperNextAndInc(NodeArr, OPER_SCOPE_CLOSE)) {
            CHECK_ORSYNTAXERROR(in_scope, "похоже вы открыли где то скобку и не закрыли ее");
        } else if (CheckIfOperNextAndInc(NodeArr, OPER_SCOPE_OPEN)) {
            IN_PERSONAL_NAMETABLE(
                op = GetOPS(NodeArr, true, LexArr);
            )
        } else {
            op = GetOP(NodeArr, LexArr);
        }
        if (!op)
            break;

        CurNode->left  = op;
        CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_OP}, NULL, NULL);
        CurNode        = CurNode->right;
    }
    return Opers;
}

Node_t * GetCMP(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * CmpNode = NewNode(TYPE_OP, {.oper = OPER_NOP}, NULL, NULL);
    CmpNode->left = GetE(NodeArr, LexArr);

    switch((**NodeArr).value.oper) {
        case OPER_MORE:
        case OPER_LESS:
        case OPER_MEQ:
        case OPER_LEQ:
        case OPER_EQ:
        case OPER_NEQ:
            CmpNode->value.oper = (**NodeArr).value.oper;
            (*NodeArr)++;
            break;
        default:
            CHECK_ORSYNTAXERROR(0, "нету знака сравнения в if");
            break;
    }

    CmpNode->right = GetE(NodeArr, LexArr);
    return CmpNode;
}

Node_t * GetIF(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * IfStatement    = NewNode(TYPE_OP, {.oper = OPER_IF}, NULL, NULL);
    IfStatement->left       = GetCMP(NodeArr, LexArr);
    Node_t * AllCompares    = IfStatement->left;

    while ((CheckIfOperNext(NodeArr, OPER_LOGIC_AND)    ||
            CheckIfOperNext(NodeArr, OPER_LOGIC_OR))    &&
            AllCompares) {
        AllCompares->right = NewNode(TYPE_OP, {.oper = (**NodeArr).value.oper}, NULL, NULL);
        AllCompares = AllCompares->right;
        AllCompares->left = GetCMP(NodeArr, LexArr);
    }

    CHECK_ORSYNTAXERROR(AllCompares && AllCompares->left, "отсутствует сравнение в if");
    MATCH_ORSYNTAXERR(OPER_THEN, "нету то после if");
    IN_PERSONAL_NAMETABLE(
        IfStatement->right = NewNode(TYPE_OP, {.oper = OPER_IF_BODY}, NULL, NULL);
        Node_t *IfBody = IfStatement->right;

        if (CheckIfOperNextAndInc(NodeArr, OPER_SCOPE_OPEN))
            IfBody->left  = GetOPS(NodeArr, 1, LexArr);
        else
            IfBody->left  = GetOP(NodeArr, LexArr);
    )

    IN_PERSONAL_NAMETABLE(
        if (CheckIfOperNextAndInc(NodeArr, OPER_ELSE)) {
            if (CheckIfOperNextAndInc(NodeArr, OPER_SCOPE_OPEN))
                IfBody->right = GetOPS(NodeArr, 1, LexArr);
            else
                IfBody->right = GetOP(NodeArr, LexArr);
        }
    )
    return IfStatement;
}

Node_t *GetINIT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *Init    = NewNode(TYPE_OP, {.oper = OPER_VAR_DECLARATION}, NULL, NULL);
    Init->left      = GetVAR(NodeArr, LexArr, true);
    CHECK_ORSYNTAXERROR(Init->left, "нету переменной для инициализации");
    MATCH_ORSYNTAXERR(OPER_ASSIGN, "нету равно после инициализации");
    Init->right = GetE(NodeArr, LexArr);
    return Init;
}

Node_t * GetINITS(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *InitValue   = NewNode(TYPE_OP, {.oper = OPER_NEW_INIT}, NULL, NULL);
    Node_t *CurNode     = InitValue;
    CurNode->left       = GetINIT(NodeArr, LexArr);
    
    while (CheckIfOperNextAndInc(NodeArr, OPER_COMMA)) {
        CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_INIT}, NULL, NULL);
        CurNode = CurNode->right;

        CurNode->left = GetINIT(NodeArr, LexArr);
    }
    return InitValue;
}

Node_t * GetPRINT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *InitValue = NewNode(TYPE_OP, {.oper = OPER_OUT}, NULL, NULL);
    InitValue->left = GetE(NodeArr, LexArr);
    return InitValue;
}

Node_t * GetASSIGN(Debug_Node_t **NodeArr, Node_t *value, LexArr_t *LexArr) {
    Node_t * AssignNode = NewNode(TYPE_OP, {.oper = OPER_ASSIGN}, NULL, NULL);
    AssignNode->left = value;
    MATCH_ORSYNTAXERR(OPER_ASSIGN, "no = after lvalue");

    AssignNode->right = GetE(NodeArr, LexArr);
    return AssignNode;
}

Node_t * GetWHILE(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *WhileNode = NewNode(TYPE_OP, {.oper = OPER_WHILE}, NULL, NULL);
    WhileNode->left = GetCMP(NodeArr, LexArr);

    MATCH_ORSYNTAXERR(OPER_WHILE_BODY, "отсутствует запись WHILE");
    IN_PERSONAL_NAMETABLE(
        if (CheckIfOperNextAndInc(NodeArr, OPER_SCOPE_OPEN))
            WhileNode->right = GetOPS(NodeArr, 1, LexArr);
        else
            WhileNode->right = GetOP(NodeArr, LexArr);
    )
    return WhileNode;
}

bool CheckIfOperNextAndInc(Debug_Node_t **NodeArr, LangOperType_e oper) {
    bool is_oper = (ISOPER &&  ((**NodeArr).value.oper == (LangOperType_e) oper ||
                               (**NodeArr).value.oper == OPER_NOP));
    if (is_oper)
        (*NodeArr)++;
    return is_oper;
}

char *FindMatchingFunc(Debug_Node_t **NodeArr, LexArr_t *LexArr, int num) {
    char *buf = IntToAscii(num);
    int index = InArrayStr(LexArr->Scope, buf, LexArr->ScopeSize);

    CHECK_ORSYNTAXERROR(index != -1, "функции которую вы пытаетесь запустить нету");
    free(buf);

    return LexArr->Scope[index + 1];
}
  
Node_t * GetFUNC_RUN(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    char *FuncName = NULL;
    if (CheckIfOperNextAndInc(NodeArr, OPER_PAR_OPEN)) {
        CHECK_ORSYNTAXERROR(ISNUM, "для вызова формулы с помощью скобок синтаксис такой: из формулы (<номер функции>) находим ...");
        Node_t *num = GetNUM(NodeArr, LexArr);

        FuncName = FindMatchingFunc(NodeArr, LexArr, num->value.num);
        MATCH_ORSYNTAXERR(OPER_PAR_CLOSE, "закрывающую скобку забыл");
    } else {
        MATCH_ORSYNTAXERR(OPER_DLYA, "если вы не вызываете функцию через обращение (2), то синтаксис должен быть такой: \"%s %s <name>\"", AllOper[OPER_FUNC_CALL].Text[0], AllOper[OPER_DLYA].Text[0]);
        FuncName = GetFuncName(NodeArr, LexArr);
    }

    Node_t *func      = NewNode(TYPE_OP,  {.oper = OPER_FUNC_CALL}, NULL, NULL);
    func->left        = NewNode(TYPE_OP,  {.oper = OPER_FUNC_VARS}, NULL, NULL);

    Node_t *func_vars = func->left;
    func_vars->left   = NewNode(TYPE_VAR, {.str = FuncName},        NULL, NULL);
    MATCH_ORSYNTAXERR(OPER_FINDING,     "ничего из функции не находишь хорош");

    CHECK_ORSYNTAXERROR(ISVAR, "из формулы ... находим что??? Напиши название переменной балбес");
    func_vars->right = GetVAR(NodeArr, LexArr, true);
    MATCH_ORSYNTAXERR(OPER_IN_POINT,    "из формулы ... находим ... иииии не написано в какой точке, маладесс");

    MATCH_ORSYNTAXERR(OPER_PAR_OPEN,    "отсутствует открывающаяся скобка в обозначении точки");
    func->right = GetFUNCPARAMS(NodeArr, true, LexArr);
    MATCH_ORSYNTAXERR(OPER_PAR_CLOSE,   "отсутствует закрывающаяся скобка в обозначении точки");
    return func;
}

Node_t *GetIN(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    CHECK_ORSYNTAXERROR(ISVAR, "нельзя дать значение не переменной!");
    
    Node_t * in = NewNode(TYPE_OP, {.oper = OPER_IN}, NULL, NULL);

    in->left = GetVAR(NodeArr, LexArr, true);
    return in;
}

Node_t * GetOP(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNextAndInc(NodeArr, OPER_NOP))
        return NULL;


    if (CheckIfOperNextAndInc(NodeArr, OPER_IF))
        return GetIF(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, OPER_VAR_DECLARATION))
        return GetINITS(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, OPER_OUT))
        return GetPRINT(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, OPER_WHILE))
        return GetWHILE(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, OPER_FUNC_CALL))
        return GetFUNC_RUN(NodeArr, LexArr);
    

    if (CheckIfOperNextAndInc(NodeArr, OPER_IN))
        return GetIN(NodeArr, LexArr);


    if (ISVAR) {
        Node_t * value = GetVAR(NodeArr, LexArr, false);
        if (CheckIfOperNext(NodeArr, OPER_ASSIGN))
            return GetASSIGN(NodeArr, value, LexArr);
        return value;
    }

    return GetE(NodeArr, LexArr);

}

void GetNOTES(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Debug_Node_t *NotesPtr = (LexArr->NodeArr + LexArr->Excepts_ptr + 1);

    while (CheckIfOperNextAndInc(&NotesPtr, OPER_NOTE)) {
        CheckArrayAndReallocate((void **) &(LexArr->Notes), LexArr->NotesSize, &(LexArr->NotesCapacity), sizeof(Node_t));
        CHECK_ORSYNTAXERROR_X(&NotesPtr, LexArr, NotesPtr->type == TYPE_NUM, 
                             "после примечания обяз должен быть номер примечания ты ПОНЕЛ?!?!");
        Node_t *num = GetNUM(&NotesPtr, LexArr);
        MATCH_ORSYNTAXERR_X(&NotesPtr, LexArr, OPER_COLON, 
                            "ну вот пишут примечание <число>:, так сложно написать : чтоли?")

        num->left = GetOPS(&NotesPtr, false, LexArr);
        LexArr->Notes[LexArr->NotesSize++] = *num;
    }
}

Node_t * GetNOTE(Debug_Node_t **NodeArr, LexArr_t *LexArr, int NoteNum) {
    for (int i = 0; i < LexArr->NotesSize; i++) {
        if (NoteNum == LexArr->Notes[i].value.num)
            return LexArr->Notes[i].left;
    }

    return NULL;
}

Node_t * GetSTEPS(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNext(NodeArr, OPER_PROGRAM_END))
        return NULL;

    Node_t *steps = NewNode(TYPE_OP, {.oper = OPER_NEW_STEP}, NULL, NULL);
    Node_t *next_step = steps;
    while (!CheckIfOperNext(NodeArr, OPER_PROGRAM_END) &&
            CheckIfOperNextAndInc(NodeArr, OPER_STEP)) {
        
        CHECK_ORSYNTAXERROR(ISNUM, "после слов \"%s\" должен стоять номер, помоему это логично", AllOper[OPER_STEP].Text[0]);
        Node_t *step_body = GetNUM(NodeArr, LexArr);
        MATCH_ORSYNTAXERR(OPER_COLON, "а после номера у шага должна стоять :, так большой папа сказал");

        step_body->left = GetOPS(NodeArr, false, LexArr);

        if (CheckIfOperNextAndInc(NodeArr, OPER_EXCEPT)) {
            CHECK_ORSYNTAXERROR(ISNUM, "после ссылки на примечание должен быть номер примечания балбес");

            Node_t *num = GetNUM(NodeArr, LexArr);
            step_body->right = GetNOTE(NodeArr, LexArr, num->value.num);
        }

        next_step->left  = step_body;
        next_step->right = NewNode(TYPE_OP, {.oper = OPER_NEW_STEP}, NULL, NULL);
        next_step = next_step->right;
    }

    return steps;
}

Node_t * GetSECT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = NewNode(TYPE_OP, {.oper = OPER_PROGRAM_START}, NULL, NULL);

    if (!CheckIfOperNextAndInc(NodeArr, OPER_USING_FORMULAS))
        fprintf(stderr, CYAN "WARNING:" RESET "не обнаружено участка с используемыми формулами, продолжаю без него\n");
    else
        value->left = GetFUNCS(NodeArr, LexArr);

    DUMP_LANGNODE(value, "состояние дерева после парсинга всех функций");
    MATCH_ORSYNTAXERR(OPER_PATH_OF_SOLVING, "без хода решений компилируешь негодяй");

    GetNOTES(NodeArr, LexArr);
    DUMP_LANGNODE(LexArr->Notes, "состояние дерева после парсинга всех примечаний");

    value->right = GetSTEPS(NodeArr, LexArr);

    CHECK_ORSYNTAXERROR(value->right, "все решение должно делиться на шаги. После хода решений не обнаружен первый шаг.")
    DUMP_LANGNODE(value, "финальное состояние дерева");

    return value;
}

Node_t * GetG(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    MATCH_ORSYNTAXERR(OPER_PROGRAM_START, "нету символа □ в начале программы, это обязательно!");

    Node_t * ProgramTree = GetSECT(NodeArr, LexArr);

    MATCH_ORSYNTAXERR(OPER_PROGRAM_END,   "нету символа ▨ в конце программы, как ты мог это забыть");
    MATCH_ORSYNTAXERR(OPER_AI_REFERENCE,  "Нету надписи \"AI reference, for generation only\", без этого никак");

    return ProgramTree;
}

Node_t * GetExpr(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *value = NULL;
    LangOperType_e type = GetArthmOper(NodeArr);

    if (ISNUM)
        value = GetNUM(NodeArr, LexArr);
    else if (ISVAR)
        value = GetVAR(NodeArr, LexArr, false);
    else if (type != OPER_NOP) {
        value = NewNode(TYPE_OP,  {.oper = type}, NULL, NULL);
        (*NodeArr)++;
        MATCH_ORSYNTAXERR(OPER_PAR_OPEN, "no ( in oper");
        value->left = GetE(NodeArr, LexArr);
        MATCH_ORSYNTAXERR(OPER_PAR_CLOSE, "no ) in oper");
    }
    else
        return NULL;
    return value;
}

Node_t * GetP(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNextAndInc(NodeArr, OPER_PAR_OPEN)) {
        Node_t * value = GetE(NodeArr, LexArr);

        MATCH_ORSYNTAXERR(OPER_PAR_CLOSE, "no closing parenthesis");
        return value;
    }
    return GetExpr(NodeArr, LexArr);
}

Node_t * GetM(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetP(NodeArr, LexArr);

    while (CheckIfOperNext(NodeArr, OPER_ARTHM_POW)) {
        (*NodeArr)++;

        Node_t * value2 = GetP(NodeArr, LexArr);
        value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_POW}, value, value2);
    }
    return value;
}

Node_t * GetT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetM(NodeArr, LexArr);

    while (CheckIfOperNext(NodeArr, OPER_ARTHM_MUL) || CheckIfOperNext(NodeArr, OPER_ARTHM_DIV)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;


        Node_t * value2 = GetM(NodeArr, LexArr);
        if (op == OPER_ARTHM_MUL)
            value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_MUL}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_DIV}, value, value2);
    }
    return value;
}

Node_t * GetE(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetT(NodeArr, LexArr);

    while (CheckIfOperNext(NodeArr, OPER_ARTHM_ADD) || CheckIfOperNext(NodeArr, OPER_ARTHM_SUB)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;

        Node_t * value2 = GetT(NodeArr, LexArr);
        if (op == OPER_ARTHM_ADD)
            value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_ADD}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_SUB}, value, value2);
    }
    return value;
}

bool CheckIfOperNext(Debug_Node_t **NodeArr, LangOperType_e oper) {
    return (ISOPER && ( (**NodeArr).value.oper == (LangOperType_e) oper ||
                        (**NodeArr).value.oper == OPER_NOP));
}

Node_t * create_node() {
    return CALLOC_WITH_TYPE(1, Node_t);
}

LangOperType_e GetArthmOper(Debug_Node_t **NodeArr) {
    if ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= OPER_ARTHM_MUL && (**NodeArr).value.oper < NumOfOpers)
        return (**NodeArr).value.oper;
    return OPER_NOP;
}


Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    
    Debug_Node_t *NodeArr = LexArr->NodeArr;
    for (size_t i = 0; i < LexArr->ScopeSize; i++) {
        SMART_FREE(LexArr->Scope[i]);
    }
    LexArr->ScopeSize = 0;
    Node_t *Node = GetG(&NodeArr, LexArr);
    return Node;
}