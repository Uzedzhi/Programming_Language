#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>


#include "../MyLibs/sassert.hpp"
#include "../includes/MyLangFront.hpp"
#include "../includes/MyLangDump.hpp"
#include "../Smart_Stack/stack.hpp"
#include "../MyLibs/helper_funcs.hpp"

const char *GetNLinesDown(const char *StartStr, const char *str, int n) {
    sassert(str,        ERR_PTR_NULL);
    sassert(StartStr,   ERR_PTR_NULL);

    if (n < 0)
        return NULL;

    while (n && str > StartStr) {
        str--;

        if (*str == '\n' || *str == '\r')
            n--;  
    }

    return str;
}

void RepeatChar(char ch, int n, FILE *fp) {
    sassert(fp, ERR_PTR_NULL);

    for (size_t j = 0; j < n; j++)
        putc(ch, fp);
}

void syntax_error(LexArr_t *LexArr, Debug_Node_t **NodeArr, const char *format, ...) {
    fprintf(stderr, "Вид в дереве:\n");

    Debug_Node_t CurNode = **NodeArr;
    for (int i = -RANGE; i <= RANGE; i++) {
        fprintf(stderr, "(%d) type: %s, ", i, AllValueTypesTxt[CurNode.type]);
        switch(CurNode.type) {
            case TYPE_NUM:   fprintf(stderr, "num: %d\n",  CurNode.value.num);                break;
            case TYPE_VAR:   fprintf(stderr, "var: %s\n",  CurNode.value.str);                break;
            case TYPE_STR:   fprintf(stderr, "str: %s\n",  CurNode.value.str);                break;
            case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOper[CurNode.value.oper].Dump); break;
            default:         fprintf(stderr, "not an op");                                    break;
        }
    }

    fprintf(stderr, "\nВид в коде:\n");

    size_t pos  = CurNode.pos;
    int    line = CurNode.line;
    va_list args;
    va_start(args, format);

    const char *StartOfPrinting = GetNLinesDown(LexArr->FileBuf, LexArr->FileBuf + pos, RANGE);
    for (int i = -RANGE; i <= RANGE && *StartOfPrinting; i++) {
        int len = strcspn(StartOfPrinting, "\n\0");
        fprintf(stderr, RESET "%d: %.*s", line + i, len, StartOfPrinting);

        if (i == 0) {
            RepeatChar(' ', pos - (StartOfPrinting - LexArr->FileBuf), stderr);
            fprintf(stderr, "/\\\n");
            RepeatChar(' ', pos - (StartOfPrinting - LexArr->FileBuf), stderr);
            fprintf(stderr, " |\n" RED);
            
            vfprintf(stderr, format, args);
        }

        StartOfPrinting += len;
    }

    va_end(args);
}

bool CheckIfOperNextAndInc(Debug_Node_t **NodeArr, LangOperType_e oper) {
    bool is_oper = (ISOPER &&  ((**NodeArr).value.oper == (LangOperType_e) oper ||
                               (**NodeArr).value.oper == OPER_NOP));
    if (is_oper)
        (*NodeArr)++;
    return is_oper;
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

int GetVarIndexInNameTable(LexArr_t *LexArr, char * var) {
    sassert(LexArr, ERR_PTR_NULL);
    sassert(var,    ERR_PTR_NULL);

    for (size_t i = 0; i < LexArr->ScopeSize; i++) {
        if (strcmp(var, LexArr->Scope[i]) == 0) {
            return i;
        }
    }

    return -1;
}

Node_t * GetVAR(Debug_Node_t **NodeArr, LexArr_t *LexArr, bool IsInit) {
    CHECK_ORSYNTAXERROR(ISVAR, "там где ожидалось название переменной его нету(");

    char *name = (**NodeArr).value.str;
    Node_t *node = NewNode(TYPE_VAR, (LangElem_u){.str = name}, NULL, NULL);

    int VarIndex = GetVarIndexInNameTable(LexArr, name);
    if (IsInit) {
        CHECK_ORSYNTAXERROR(VarIndex == -1, "переменная <%s> уже существует в этом scope!", name);

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

Node_t *GetFuncName(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    char *FuncName = CALLOC_WITH_TYPE(MAX_STR_SIZE, char);
    
    while (ISVAR) {
        char *name = (**NodeArr).value.str;
        (*NodeArr)++;
        
        strcat(FuncName, name);
        free(name);
    }

    int index = GetVarIndexInNameTable(LexArr, FuncName);
    CHECK_ORSYNTAXERROR(index == -1, "функция <%s> уже существует", FuncName);
    LexArr->Scope[LexArr->ScopeSize++] = FuncName;

    return NewNode(TYPE_VAR, {.str = FuncName}, NULL, NULL);
}

Node_t * GetFUNC(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    MATCH_ORSYNTAXERR(OPER_PAR_OPEN,  "У формулы нет номера в начале. Пример: (4) Формула для ...");
    Node_t *value = GetVAR(NodeArr, LexArr, true);
    MATCH_ORSYNTAXERR(OPER_PAR_CLOSE, "У номера формулы нет закрывающей скобки");

    MATCH_ORSYNTAXERR(OPER_FUNC_DECL_NAME, "вы наверно хотели обьявить формулу, но у нет ключевого слова \"%s\"", AllOper[OPER_FUNC_DECL_NAME].Text[0]);
    Node_t *func = GetFuncName(NodeArr, LexArr);
    Node_t *func_body = func->left;
    func->left = NewNode(TYPE_OP, {.oper = OPER_FUNC_BODY}, NULL, NULL);

    MATCH_ORSYNTAXERR(OPER_IN_POINT, "у функции <%s> нету ключевого слова \"%s\" (да знаю что не хочется писать, но так красивее, напиши несколько букв, а то обижусь)", func->value.str, AllOper[OPER_IN_POINT].Text[0]);
    MATCH_ORSYNTAXERR(OPER_PAR_OPEN, "У функции с именем <%s> нет параметров (открывающейся скобки нет)", func->value.str);

    IN_PERSONAL_NAMETABLE(
        func_body->left = GetFUNCPARAMS(NodeArr, false, LexArr);

        MATCH_ORSYNTAXERR(OPER_PAR_CLOSE,   "У функции <%s> нет закрывающей скобки у параметров", func->value.str);
        MATCH_ORSYNTAXERR(OPER_COLON,       "в функции нет разделителя :");

        func_body->right = GetOPS(NodeArr, OPER_NOP, LexArr);

        MATCH_ORSYNTAXERR(OPER_RETURN, "нету возвращаемого значения!");
        func->right = GetE(NodeArr, LexArr);
        func->left  = func_body;
    )

    return func;
}

Node_t * GetFUNCS(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * AllFuncs = NewNode(TYPE_OP, {.oper = OPER_NEW_FUNC}, NULL, NULL);
    Node_t * CurNode = AllFuncs;

    while (CheckIfOperNextAndInc(NodeArr, OPER_FUNC_DECL_NAME)) {
        CurNode->left  = GetFUNC(NodeArr, LexArr);

        if (CheckIfOperNext(NodeArr, OPER_FUNC_DECL_NAME))
            CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_FUNC}, NULL, NULL);
        
        CurNode = CurNode->right;
    }

    return AllFuncs;
}

Node_t * GetOPS(Debug_Node_t **NodeArr, LangOperType_e sep, LexArr_t *LexArr) {
    Node_t *Opers   = NewNode(TYPE_OP, (LangElem_u){.oper = OPER_NEW_OP}, NULL, NULL);
    Node_t *CurNode = Opers;
    size_t iter     = 0;

    while (!CheckIfOperNext(NodeArr, OPER_PROGRAM_END)      &&
           !CheckIfOperNext(NodeArr, OPER_FUNC_DECL_NAME)   &&
           !CheckIfOperNext(NodeArr, OPER_RETURN)           &&
           !CheckIfOperNext(NodeArr, OPER_ELSE)           &&
           !CheckIfOperNext(NodeArr, OPER_WHILE)           &&
           !CheckIfOperNext(NodeArr, OPER_PATH_OF_SOLVING)) {
        if (iter++ != 0) {
            CurNode->right = NewNode(TYPE_OP, {.oper = OPER_NEW_OP}, NULL, NULL);
            CurNode = CurNode->right;
        }

        Node_t *op = GetOP(NodeArr, LexArr);
        DUMP_LANGNODE(&op, "состояние дерева после обработки одного оператора в функции GetOPS");

        if (!op)
            break;
        CurNode->left = op;

        if (!CheckIfOperNextAndInc(NodeArr, sep) && sep != OPER_NOP) {
            break;
        }
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
            syntax_error(LexArr, NodeArr, "нету знака сравнения в if");
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
    if (!AllCompares || !(AllCompares->left))
        syntax_error(LexArr, NodeArr, "отсутствует сравнение в if");

    MATCH_ORSYNTAXERR(OPER_THEN, "нету то после if");
    IN_PERSONAL_NAMETABLE(
        IfStatement->right = NewNode(TYPE_OP, {.oper = OPER_IF_BODY}, NULL, NULL);
        Node_t *IfBody = IfStatement->right;
        IfBody->left  = GetOPS(NodeArr, OPER_NOP, LexArr);
        DUMP_LANGNODE(NodeArr, "if оператор");
    )

    IN_PERSONAL_NAMETABLE(
        if (CheckIfOperNextAndInc(NodeArr, OPER_ELSE)) {
            IfBody->right = GetOPS(NodeArr, OPER_NOP, LexArr);
        }
    )
    return IfStatement;
}

Node_t *GetINIT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *Init    = NewNode(TYPE_OP, {.oper = OPER_VAR_DECLARATION}, NULL, NULL);
    Init->left      = GetVAR(NodeArr, LexArr, true);
    if (!Init->left)
        syntax_error(LexArr, NodeArr, "нету переменной для инициализации");
    
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
        WhileNode->right = GetOPS(NodeArr, OPER_NOP, LexArr);
    )
    return WhileNode;
}
  
Node_t * GetFUNC_RUN(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *FUNC_RUNNode = NewNode(TYPE_OP, {.oper = OPER_FUNC_CALL}, NULL, NULL);
    MATCH_ORSYNTAXERR(OPER_PAR_OPEN,    "отсутствует открывающаяся скобка в названии функции");

    Node_t *func = NewNode(TYPE_OP, {.oper = OPER_FUNC_CALL}, NULL, NULL);
    func->left = GetVAR(NodeArr, LexArr, false);

    MATCH_ORSYNTAXERR(OPER_PAR_CLOSE,   "отсутствует закрывающаяся скобка в названии функции");
    MATCH_ORSYNTAXERR(OPER_FINDING,     "ничего из функции не находишь хорош");

    FUNC_RUNNode->left = GetVAR(NodeArr, LexArr, true);
    MATCH_ORSYNTAXERR(OPER_IN_POINT,    "не написано в какой точке");

    MATCH_ORSYNTAXERR(OPER_PAR_OPEN,    "отсутствует открывающаяся скобка в обозначении точки");
    func->right = GetFUNCPARAMS(NodeArr, true, LexArr);

    MATCH_ORSYNTAXERR(OPER_PAR_CLOSE,   "отсутствует закрывающаяся скобка в обозначении точки");
    FUNC_RUNNode->right = func;
    return FUNC_RUNNode;
}

Node_t *GetIN(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    if (!ISVAR)
        syntax_error(LexArr, NodeArr, "нельзя дать значение не переменной!");
    
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

Node_t * GetSECT(Debug_Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = NewNode(TYPE_OP, {.oper = OPER_PROGRAM_START}, NULL, NULL);

    if (!CheckIfOperNextAndInc(NodeArr, OPER_USING_FORMULAS))
        fprintf(stderr, CYAN "WARNING:" RESET "не обнаружено участка с используемыми формулами, продолжаю без него\n");
    else
        value->left = GetFUNCS(NodeArr, LexArr);

    DUMP_LANGNODE(&value, "состояние дерева после парсинга всех функций");
    MATCH_ORSYNTAXERR(OPER_PATH_OF_SOLVING, "без хода решений компилируешь негодяй");

    value->right = GetOPS(NodeArr, OPER_NOP, LexArr);
    DUMP_LANGNODE(&value, "финальное состояние дерева после всех операций");
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

        if (!CheckIfOperNextAndInc(NodeArr, OPER_PAR_CLOSE)) {
            syntax_error(LexArr, NodeArr, "no closing par");
        }
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

Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL);

    node->type  = type;
    node->value = value;
    node->left  = left;
    node->right = right;
    return node;
}

LangOperType_e GetArthmOper(Debug_Node_t **NodeArr) {
    if ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= OPER_ARTHM_MUL && (**NodeArr).value.oper < NumOfOpers)
        return (**NodeArr).value.oper;
    return OPER_NOP;
}


Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    
    Debug_Node_t *NodeArr = LexArr->NodeArr;

    LexArr->ScopeSize = 0;
    Node_t *Node = GetG(&NodeArr, LexArr);
    return Node;
}