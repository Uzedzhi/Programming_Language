#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "my_libs/sassert.hpp"
#include "MyLangSyntax.hpp"
#include "MyLang.hpp"
#include "MyLangDump.hpp"

#define ISVAR   (**NodeArr).type == TYPE_VAR
#define ISSTR   (**NodeArr).type == TYPE_STR
#define ISNUM   (**NodeArr).type == TYPE_NUM
#define ISARTHMOPER (**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper <= ARTHM_ARCCTG
#define ISOPER  (**NodeArr).type == TYPE_OP
#define SYNTAX_ERROR(NodeArr, ...) {\
    PRINT_CUR_TYPE_AND_OP((*NodeArr)[-2], "ПредПредыдущая ветка");\
    PRINT_CUR_TYPE_AND_OP((*NodeArr)[-1], "Предыдущая ветка");\
    PRINT_CUR_TYPE_AND_OP(**NodeArr,      "Текущая ветка");\
    PRINT_CUR_TYPE_AND_OP((*NodeArr)[1],  "Будущая ветка");\
    PRINT_CUR_TYPE_AND_OP((*NodeArr)[2],  "БудБудущая ветка");\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); exit(ERR_PTR_NULL_LANG);}

#define MATCHOPER_ORSYNTAXERR(oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}

#define IN_PERSONAL_NAMETABLE(lines_of_code) \
    stackPush(LexArr->ScopeBorders, LexArr->ScopeSize);\
    lines_of_code\
    stackPop(LexArr->ScopeBorders, &(LexArr->ScopeSize));

bool CheckIfOperNextAndInc(Node_t **NodeArr, LangOperType_e oper) {
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
    sassert(LexArr, ERR_PTR_NULL_LANG);
    sassert(var,    ERR_PTR_NULL_LANG);

    for (size_t i = 0; i < LexArr->ScopeSize; i++) {
        if (strcmp(var, LexArr->Scope[i]) == 0) {
            return i;
        }
    }

    return -1;
}

Node_t * GetVAR(Node_t **NodeArr, LexArr_t *LexArr, bool IsInit) {
    if (!ISVAR)
        SYNTAX_ERROR(NodeArr, "там где ожидалось название переменной его нету(");

    char *name = (**NodeArr).value.var_name;
    Node_t *node = NewNode(TYPE_VAR, (LangElem_u){.var_name = name}, NULL, NULL);

    int VarIndex = GetVarIndexInNameTable(LexArr, name);
    if (IsInit) {
        if (VarIndex != -1)
            SYNTAX_ERROR(NodeArr, "переменная уже существует в этом scope!");

        LexArr->Scope[LexArr->ScopeSize] = name;
        LexArr->ScopeSize++;
    } else {
        if (VarIndex == -1)
            SYNTAX_ERROR(NodeArr, "не существует такой переменной (в видимых scope)");
    }
    (*NodeArr)++;
    return node;
}

Node_t * GetNUM(Node_t **NodeArr, LexArr_t *LexArr) {
    if (!ISNUM)
        SYNTAX_ERROR(NodeArr, "там где ожидалось число его нету");
    Node_t *node = NewNode(TYPE_NUM, {.num = (**NodeArr).value.num}, NULL, NULL);
    (*NodeArr)++;
    return node;
}

Node_t * GetFUNCPARAMS(Node_t **NodeArr, bool AllowingNums, LexArr_t *LexArr) {
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

Node_t * GetFUNC(Node_t **NodeArr, LexArr_t *LexArr) {
    MATCHOPER_ORSYNTAXERR(OPER_PAR_OPEN, "У формулы нет имени");

    Node_t * func = GetVAR(NodeArr, LexArr, true);
    Node_t * func_body = func->left;
    func_body = NewNode(TYPE_OP, {.oper = OPER_FUNC_BODY}, NULL, NULL);

    MATCHOPER_ORSYNTAXERR(OPER_PAR_CLOSE,   "У функции <%s> нет закрывающей скобки в имени", func->value.var_name);
    MATCHOPER_ORSYNTAXERR(OPER_PAR_OPEN,    "У функции с именем <%s> нет параметров", func->value.var_name);

    IN_PERSONAL_NAMETABLE(
        func_body->left = GetFUNCPARAMS(NodeArr, false, LexArr);

        MATCHOPER_ORSYNTAXERR(OPER_PAR_CLOSE,   "У функции <%s> в нет закрывающей скобки у параметров", func->value.var_name);
        MATCHOPER_ORSYNTAXERR(OPER_COLON,       "в функции нет разделителя :");

        func_body->right = GetOPS(NodeArr, OPER_NOP, LexArr);

        MATCHOPER_ORSYNTAXERR(OPER_RETURN, "нету возвращаемого значения!");
        func->right = GetE(NodeArr, LexArr);
        func->left  = func_body;
    )

    return func;
}

Node_t * GetFUNCS(Node_t **NodeArr, LexArr_t *LexArr) {
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

Node_t * GetOPS(Node_t **NodeArr, LangOperType_e sep, LexArr_t *LexArr) {
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
        DUMP_LANGNODE(&op, "состояние дерева после парсинга оператора");

        if (!op)
            break;
        CurNode->left = op;

        if (!CheckIfOperNextAndInc(NodeArr, sep) && sep != OPER_NOP) {
            break;
        }
    }

    return Opers;
}

Node_t * GetCMP(Node_t **NodeArr, LexArr_t *LexArr) {
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
            SYNTAX_ERROR(NodeArr, "нету знака сравнения в if");
            break;
    }

    CmpNode->right = GetE(NodeArr, LexArr);
    return CmpNode;
}

Node_t * GetIF(Node_t **NodeArr, LexArr_t *LexArr) {
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
        SYNTAX_ERROR(NodeArr, "отсутствует сравнение в if");

    MATCHOPER_ORSYNTAXERR(OPER_THEN, "нету то после if");
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

Node_t *GetINIT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *Init    = NewNode(TYPE_OP, {.oper = OPER_VAR_DECLARATION}, NULL, NULL);
    Init->left      = GetVAR(NodeArr, LexArr, true);
    if (!Init->left)
        SYNTAX_ERROR(NodeArr, "нету переменной для инициализации");
    
    MATCHOPER_ORSYNTAXERR(OPER_ASSIGN, "нету равно после инициализации");
    Init->right = GetE(NodeArr, LexArr);
    return Init;
}

Node_t * GetINITS(Node_t **NodeArr, LexArr_t *LexArr) {
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

Node_t * GetPRINT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *InitValue = NewNode(TYPE_OP, {.oper = OPER_OUT}, NULL, NULL);
    InitValue->left = GetE(NodeArr, LexArr);
    return InitValue;
}

Node_t * GetASSIGN(Node_t **NodeArr, Node_t *value, LexArr_t *LexArr) {
    Node_t * AssignNode = NewNode(TYPE_OP, {.oper = OPER_ASSIGN}, NULL, NULL);
    AssignNode->left = value;
    MATCHOPER_ORSYNTAXERR(OPER_ASSIGN, "no = after lvalue");

    AssignNode->right = GetE(NodeArr, LexArr);
    return AssignNode;
}

Node_t * GetWHILE(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *WhileNode = NewNode(TYPE_OP, {.oper = OPER_WHILE}, NULL, NULL);
    WhileNode->left = GetCMP(NodeArr, LexArr);

    MATCHOPER_ORSYNTAXERR(OPER_WHILE_BODY, "отсутствует запись WHILE");
    IN_PERSONAL_NAMETABLE(
        WhileNode->right = GetOPS(NodeArr, OPER_NOP, LexArr);
    )
    return WhileNode;
}
  
Node_t * GetFUNC_RUN(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *FUNC_RUNNode = NewNode(TYPE_OP, {.oper = OPER_FUNC_CALL}, NULL, NULL);
    MATCHOPER_ORSYNTAXERR(OPER_PAR_OPEN,    "отсутствует открывающаяся скобка в названии функции");

    Node_t *func = NewNode(TYPE_OP, {.oper = OPER_FUNC_CALL}, NULL, NULL);
    func->left = GetVAR(NodeArr, LexArr, false);

    MATCHOPER_ORSYNTAXERR(OPER_PAR_CLOSE,   "отсутствует закрывающаяся скобка в названии функции");
    MATCHOPER_ORSYNTAXERR(OPER_FINDING,     "ничего из функции не находишь хорош");

    FUNC_RUNNode->left = GetVAR(NodeArr, LexArr, true);
    MATCHOPER_ORSYNTAXERR(OPER_IN_POINT,    "не написано в какой точке");

    MATCHOPER_ORSYNTAXERR(OPER_PAR_OPEN,    "отсутствует открывающаяся скобка в обозначении точки");
    func->right = GetFUNCPARAMS(NodeArr, true, LexArr);

    MATCHOPER_ORSYNTAXERR(OPER_PAR_CLOSE,   "отсутствует закрывающаяся скобка в обозначении точки");
    FUNC_RUNNode->right = func;
    return FUNC_RUNNode;
}

Node_t *GetIN(Node_t **NodeArr, LexArr_t *LexArr) {
    if (!ISVAR)
        SYNTAX_ERROR(NodeArr, "нельзя дать значение не переменной!");
    
    Node_t * in = NewNode(TYPE_OP, {.oper = OPER_IN}, NULL, NULL);

    in->left = GetVAR(NodeArr, LexArr, true);
    return in;
}

Node_t * GetOP(Node_t **NodeArr, LexArr_t *LexArr) {
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

Node_t * GetSECT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = NewNode(TYPE_OP, {.oper = OPER_PROGRAM_START}, NULL, NULL);

    if (!CheckIfOperNextAndInc(NodeArr, OPER_USING_FORMULAS))
        fprintf(stderr, CYAN "WARNING:" RESET "proceeding without formulas section\n");
    else
        value->left = GetFUNCS(NodeArr, LexArr);

    DUMP_LANGNODE(&value, "состояние дерева после парсинга всех функций");
    MATCHOPER_ORSYNTAXERR(OPER_PATH_OF_SOLVING, "без хода решений компилируешь негодяй");

    value->right = GetOPS(NodeArr, OPER_NOP, LexArr);
    DUMP_LANGNODE(&value, "финальное состояние дерева после всех операций");
    return value;
}

Node_t * GetG(Node_t **NodeArr, LexArr_t *LexArr) {
    MATCHOPER_ORSYNTAXERR(OPER_PROGRAM_START, "no starting program symbol!");

    Node_t * ProgramTree = GetSECT(NodeArr, LexArr);

    MATCHOPER_ORSYNTAXERR(OPER_PROGRAM_END, "no ending program symbol!");
    MATCHOPER_ORSYNTAXERR(OPER_AI_REFERENCE, "Not AI reference, Not for generation only");

    return ProgramTree;
}

Node_t * GetExpr(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *value = NULL;
    LangOperType_e type = GetArthmOper(NodeArr);

    if (ISNUM)
        value = GetNUM(NodeArr, LexArr);
    else if (ISVAR)
        value = GetVAR(NodeArr, LexArr, false);
    else if (type != OPER_NOP) {
        value = NewNode(TYPE_OP,  {.oper = type}, NULL, NULL);
        (*NodeArr)++;
        MATCHOPER_ORSYNTAXERR(OPER_PAR_OPEN, "no ( in oper");
        value->left = GetE(NodeArr, LexArr);
        MATCHOPER_ORSYNTAXERR(OPER_PAR_CLOSE, "no ) in oper");
    }
    else
        return NULL;
    return value;
}

Node_t * GetP(Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNextAndInc(NodeArr, OPER_PAR_OPEN)) {
        Node_t * value = GetE(NodeArr, LexArr);

        if (!CheckIfOperNextAndInc(NodeArr, OPER_PAR_CLOSE)) {
            SYNTAX_ERROR(NodeArr, "no closing par");
        }
        return value;
    }
    return GetExpr(NodeArr, LexArr);
}

Node_t * GetM(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetP(NodeArr, LexArr);

    while (CheckIfOperNext(NodeArr, OPER_ARTHM_POW)) {
        (*NodeArr)++;

        Node_t * value2 = GetP(NodeArr, LexArr);
        value = NewNode(TYPE_OP, {.oper = OPER_ARTHM_POW}, value, value2);
    }
    return value;
}

Node_t * GetT(Node_t **NodeArr, LexArr_t *LexArr) {
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

Node_t * GetE(Node_t **NodeArr, LexArr_t *LexArr) {
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

bool CheckIfOperNext(Node_t **NodeArr, LangOperType_e oper) {
    return (ISOPER && ( (**NodeArr).value.oper == (LangOperType_e) oper ||
                        (**NodeArr).value.oper == OPER_NOP));
}

Node_t * create_node() {
    return CALLOC_WITH_TYPE(1, Node_t);
}

Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL_LANG);


    node->type  = type;
    node->value = value;
    node->left  = left;
    node->right = right;
    return node;
}

LangOperType_e GetArthmOper(Node_t **NodeArr) {
    if ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= OPER_ARTHM_MUL && (**NodeArr).value.oper < NumOfOpers)
        return (**NodeArr).value.oper;
    return OPER_NOP;
}


Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL_LANG);
    
    Node_t * NodeArr = LexArr->NodeArr;
    Node_t * Node = GetG(&NodeArr, LexArr);
    return Node;
}