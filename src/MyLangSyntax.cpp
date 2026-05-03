#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "../my_libs/sassert.h"
#include "MyLangSyntax.h"
#include "MyLang.h"
#include "MyLangDump.h"

#define ISVAR   (**NodeArr).type == TYPE_VAR
#define ISSTR   (**NodeArr).type == TYPE_STR
#define ISNUM   (**NodeArr).type == TYPE_NUM
#define ISARTHMOPER (**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper <= ARTHM_ARCCTG
#define ISOPER  (**NodeArr).type == TYPE_OP
#define ISCNOP  (**NodeArr).type == TYPE_CNOP
#define SYNTAX_ERROR(NodeArr, ...) {\
    fprintf(stderr, "NOW: type: %s ", AllValueTypesTxt[(**NodeArr).type]);\
    switch((**NodeArr).type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  (**NodeArr).value.num);                              break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  (**NodeArr).value.var_name);                         break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[(**NodeArr).value.oper]);             break;\
        case TYPE_CNOP:  fprintf(stderr, "cnop: %s\n", AllOperCNOPDumpStr[(**NodeArr).value.CNop]);         break;\
    }\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); sassert(0, ERR_PTR_NULL);}

#define MATCHOPER_ORSYNTAXERR(type, oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper, type)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}

#define IN_PERSONAL_NAMETABLE(lines_of_code) {\
    PushScope(LexArr);\
    lines_of_code\
    PopScope(LexArr);}

bool CheckIfOperNextAndInc(Node_t **NodeArr, int oper, LangType_e type) {
    bool is_oper = (ISOPER && (**NodeArr).value.oper == (LangOperType_e) oper && type == TYPE_OP) || (ISCNOP && (**NodeArr).value.CNop == (LangCNOPType_e) oper && type == TYPE_CNOP);
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

int GetVarIndex(LexArr_t *LexArr, char *VarName) {
    int i = 0;
    while (i < LexArr->NameTableLocalSize) {
        if (strcmp(LexArr->NameTableLocal[i], VarName) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

size_t CurScopeBase(const LexArr_t *LexArr) {
    if (LexArr->ScopeStackSize == 0) return 0;
    return LexArr->ScopeStack[LexArr->ScopeStackSize - 1];
}

void PushScope(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    if (LexArr->ScopeStackSize >= LexArr->ScopeStackCapacity - 1) {
        reallocate_array((void **) &(LexArr->ScopeStack), LexArr->ScopeStackCapacity, LexArr->ScopeStackCapacity * 2 * sizeof(size_t));
        LexArr->ScopeStackCapacity *= 2;
    }

    LexArr->ScopeStack[LexArr->ScopeStackSize++] = LexArr->NameTableLocalSize;
    LexArr->CurScopeDeep++;
}

void PopScope(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    sassert(LexArr->ScopeStackSize > 0, ERR_PTR_NULL);

    LexArr->NameTableLocalSize = LexArr->ScopeStack[--LexArr->ScopeStackSize];
    LexArr->CurScopeDeep--;
}

int GetVarIndexInNameTable(const LexArr_t *LexArr, const char *name) {
    sassert(LexArr, ERR_PTR_NULL);
    sassert(name, ERR_PTR_NULL);

    for (int i = LexArr->NameTableLocalSize - 1; i >= 0; --i) {
        if (strcmp(LexArr->NameTableLocal[i], name) == 0)
            return i;
    }
    return -1;
}

int PlaceVarInNameTable(LexArr_t *LexArr, char *name) {
    sassert(LexArr, ERR_PTR_NULL);
    sassert(name, ERR_PTR_NULL);

    for (size_t i = CurScopeBase(LexArr); i < LexArr->NameTableLocalSize; ++i) {
        if (strcmp(LexArr->NameTableLocal[i], name) == 0)
            return -1;
    }

    if (LexArr->NameTableLocalSize >= LexArr->NameTableLocalCapacity - 1) {
        reallocate_array((void **)&LexArr->NameTableLocal,
                         LexArr->NameTableLocalCapacity,
                         LexArr->NameTableLocalCapacity * 2 * sizeof(char*));
        reallocate_array((void **)&LexArr->NameTableLocalScope,
                         LexArr->NameTableLocalCapacity,
                         LexArr->NameTableLocalCapacity * 2 * sizeof(size_t));

        LexArr->NameTableLocalCapacity *= 2;
    }

    int id = LexArr->NameTableLocalSize++;
    LexArr->NameTableLocal[id]      = name;
    LexArr->NameTableLocalScope[id] = LexArr->CurScopeDeep;
    return id;
}
Node_t * GetVAR(Node_t **NodeArr, LexArr_t *LexArr, bool IsInit) {
    if (!ISVAR)
        SYNTAX_ERROR(NodeArr, "там где ожидалось название переменной его нету(");

    char *name = (**NodeArr).value.var_name;
    Node_t *node = NewNode(TYPE_VAR, (LangElem_u){.var_name = name}, NULL, NULL);

    if (IsInit) {
        int VarIndex = PlaceVarInNameTable(LexArr, name);
        if (VarIndex < 0)
            SYNTAX_ERROR(NodeArr, "переменная уже существует в этом scope!");

        node->ScopeDeep = LexArr->CurScopeDeep;
    } else {
        int VarIndex = GetVarIndexInNameTable(LexArr, name);
        if (VarIndex < 0)
            SYNTAX_ERROR(NodeArr, "не существует такой переменной (в видимых scope)");

        node->ScopeDeep = LexArr->NameTableLocalScope[VarIndex];
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
    Node_t * AllParams = NewNode(TYPE_CNOP, {.CNop = NEW_PARAM}, NULL, NULL);
    Node_t * CurNode   = AllParams;
    while (ISVAR && (CurNode->left  = GetVAR(NodeArr, LexArr, !AllowingNums)) != NULL ||
           AllowingNums && ISNUM && (CurNode->left = GetNUM(NodeArr, LexArr)) != NULL) {
        if (!CheckIfOperNextAndInc(NodeArr, COMMA, TYPE_OP))
            break;
        CurNode->right = NewNode(TYPE_CNOP, {.CNop = NEW_PARAM}, NULL, NULL);
        CurNode = CurNode->right;
        DUMP_LANGNODE(AllParams, "allparams in getfunc");
    }
    return AllParams;
}

Node_t * GetFUNC(Node_t **NodeArr, LexArr_t *LexArr) {
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "in func decl ( is missing");
    Node_t * func = GetVAR(NodeArr, LexArr, true);
    func->left = NewNode(TYPE_CNOP, {.CNop = FUNC_BODY}, NULL, NULL);
    Node_t * func_body = func->left;

    DUMP_LANGNODE(func, "after in getfunc");
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE, "in func decl ) is missing");
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "in func params ( is missing");
    IN_PERSONAL_NAMETABLE(
        func_body->left = GetFUNCPARAMS(NodeArr, false, LexArr);
        MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE,     "in func params ) is missing");
        MATCHOPER_ORSYNTAXERR(TYPE_OP, COLON,         "in func decl : is missing");
        DUMP_LANGNODE(func, "func in getfunc");
        func_body->right = GetOPS(NodeArr, COMMA, LexArr);
        MATCHOPER_ORSYNTAXERR(TYPE_OP, RETURN,        "нету возвращаемого значения!");
        func->right = GetE(NodeArr, LexArr);
    )

    DUMP_LANGNODE(func, "after func in getfunc");
    return func;
}

Node_t * GetFUNCS(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * AllFuncs = NewNode(TYPE_CNOP, {.CNop = FUNC}, NULL, NULL);
    Node_t * cur_node = AllFuncs;
    DUMP_LANGNODE(AllFuncs, "before getfuncs");

    while (CheckIfOperNextAndInc(NodeArr, FUNC_DECL_NAME, TYPE_CNOP)) {
        cur_node->left  = GetFUNC(NodeArr, LexArr);
        if (CheckIfOperNext(NodeArr, FUNC_DECL_NAME, TYPE_CNOP))
            cur_node->right = NewNode(TYPE_CNOP, {.CNop = FUNC}, NULL, NULL);
        cur_node = cur_node->right;
        DUMP_LANGNODE(AllFuncs, "after getfunc");
    }
    DUMP_LANGNODE(AllFuncs, "after getfuncs");

    return AllFuncs;
}

Node_t * GetOPS(Node_t **NodeArr, LangOperType_e sep, LexArr_t *LexArr) {
    Node_t *ops_list = NewNode(TYPE_CNOP, (LangElem_u){.CNop = NEW_OP}, NULL, NULL);
    Node_t *cur = ops_list;
    size_t iter = 0;

    while (!CheckIfOperNext(NodeArr, PROGRAM_END, TYPE_CNOP)     &&
           !CheckIfOperNext(NodeArr, FUNC_DECL_NAME, TYPE_CNOP)  &&
           !CheckIfOperNext(NodeArr, PATH_OF_SOLVING, TYPE_CNOP)) {
        if (iter++ != 0) {
            cur->right = NewNode(TYPE_CNOP, {.CNop = NEW_OP}, NULL, NULL);
            cur = cur->right;
        }

        Node_t *op = GetOP(NodeArr, LexArr);
        if (!op) {
            (*NodeArr)++;
            break;
        }
        cur->left = op;

        if (!CheckIfOperNextAndInc(NodeArr, sep, TYPE_OP) && sep != NOP) {
            break;
        }
    }

    return ops_list;
}

Node_t * GetCMP(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * CmpNode = NewNode(TYPE_OP, {.oper = NOP}, NULL, NULL);
    CmpNode->left = GetE(NodeArr, LexArr);
    switch((**NodeArr).value.oper) {
        case MORE:
        case LESS:
        case MEQ:
        case LEQ:
        case EQ:
        case NEQ:
            CmpNode->value.oper = (**NodeArr).value.oper; (*NodeArr)++; break;
        default:
            MATCHOPER_ORSYNTAXERR(TYPE_OP, NOP, "нету знака сравнения в if"); break;
    }


    CmpNode->right = GetE(NodeArr, LexArr);
    return CmpNode;
}

Node_t * GetIF(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * IfStatement    = NewNode(TYPE_OP, {.oper = IF}, NULL, NULL);
    IfStatement->left       = NewNode(TYPE_CNOP, {.CNop = NEW_CMP}, NULL, NULL);
    Node_t * AllCompares = IfStatement->left;

    AllCompares->left = GetCMP(NodeArr, LexArr);
    while (CheckIfOperNextAndInc(NodeArr, COMMA, TYPE_OP) && AllCompares) {
        AllCompares->right = NewNode(TYPE_CNOP, {.CNop = NEW_CMP}, NULL, NULL);
        AllCompares = AllCompares->right;
        AllCompares->left = GetCMP(NodeArr, LexArr);
    }

    DUMP_LANGNODE(IfStatement, "ifstatement before");
    IfStatement->right = NewNode(TYPE_CNOP, {.CNop = IF_ELSE}, NULL, NULL);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, THEN, "нету то после если")
    IN_PERSONAL_NAMETABLE (
        IfStatement->right->left  = GetOPS(NodeArr, COMMA, LexArr);
    )
    DUMP_LANGNODE(IfStatement, "ifstatement middle");

    if (CheckIfOperNextAndInc(NodeArr, ELSE, TYPE_OP)) {
        IN_PERSONAL_NAMETABLE (
            IfStatement->right->right = GetOPS(NodeArr, COMMA, LexArr);
        )
    DUMP_LANGNODE(IfStatement, "ifstatement after");
    }
    
    return IfStatement;
}

Node_t *GetINIT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *Init = NewNode(TYPE_OP, {.oper = VAR_DECLARATION}, NULL, NULL);
    Init->left  = GetVAR(NodeArr, LexArr, true);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, ASSIGN, "нету равно после инициализации");
    Init->right = GetE(NodeArr, LexArr);
    return Init;
}

Node_t * GetINITS(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *InitValue   = NewNode(TYPE_CNOP, {.CNop = NEW_INIT}, NULL, NULL);
    Node_t *cur_node    = InitValue;
    cur_node->left = GetINIT(NodeArr, LexArr);
    while (CheckIfOperNextAndInc(NodeArr, COMMA, TYPE_OP)) {
        cur_node->right = NewNode(TYPE_CNOP, {.CNop = NEW_INIT}, NULL, NULL);
        cur_node = cur_node->right;
        cur_node->left = GetINIT(NodeArr, LexArr);
    }
    return InitValue;
}

Node_t * GetPRINT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *InitValue = NewNode(TYPE_OP, {.oper = OUT}, NULL, NULL);
    InitValue->left = GetE(NodeArr, LexArr);
    return InitValue;
}

Node_t * GetASSIGN(Node_t **NodeArr, Node_t *value, LexArr_t *LexArr) {
    Node_t * AssignNode = NewNode(TYPE_OP, {.oper = ASSIGN}, NULL, NULL);
    AssignNode->left = value;
    MATCHOPER_ORSYNTAXERR(TYPE_OP, ASSIGN, "no = after lvalue");
    AssignNode->right = GetE(NodeArr, LexArr);
    return AssignNode;
}

Node_t * GetWHILE(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *WhileNode = NewNode(TYPE_OP, {.oper = WHILE}, NULL, NULL);
    WhileNode->left = GetCMP(NodeArr, LexArr);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, WHILE, "отсутствует запись WHILE");
    IN_PERSONAL_NAMETABLE (
        WhileNode->right = GetOPS(NodeArr, COMMA, LexArr);
    )


    return WhileNode;
}

Node_t * GetFUNC_RUN(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *FUNC_RUNNode = NewNode(TYPE_OP, {.oper = FUNC_CALL}, NULL, NULL);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "отсутствует открывающаяся скобка в названии функции");
    Node_t *func = NewNode(TYPE_CNOP, {.CNop = FUNC}, NULL, NULL);
    func->left = GetVAR(NodeArr, LexArr, false);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE, "отсутствует закрывающаяся скобка в названии функции");

    MATCHOPER_ORSYNTAXERR(TYPE_OP, FINDING, "ничего из функции не находишь хорош");
    DUMP_LANGNODE(func, "funcrun_node");
    FUNC_RUNNode->left = GetVAR(NodeArr, LexArr, true);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, IN_POINT, "не написано в какой точке");
    DUMP_LANGNODE(FUNC_RUNNode, "funcrun_node after");

    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "отсутствует открывающаяся скобка в обозначении точки");
    func->right = GetFUNCPARAMS(NodeArr, true, LexArr);
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE, "отсутствует закрывающаяся скобка в обозначении точки");
    FUNC_RUNNode->right = func;
    return FUNC_RUNNode;
}

Node_t *GetIN(Node_t **NodeArr, LexArr_t *LexArr) {
    if (!ISVAR)
        SYNTAX_ERROR(NodeArr, "нельзя дать значение не переменной!");
    
    Node_t * in = NewNode(TYPE_OP, {.oper = IN}, NULL, NULL);
    in->left = GetVAR(NodeArr, LexArr, true);
    return in;
}

Node_t * GetOP(Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNextAndInc(NodeArr, IF, TYPE_OP))
        return GetIF(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, VAR_DECLARATION, TYPE_OP))
        return GetINITS(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, OUT, TYPE_OP))
        return GetPRINT(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, WHILE, TYPE_OP))
        return GetWHILE(NodeArr, LexArr);


    if (CheckIfOperNextAndInc(NodeArr, FUNC_CALL, TYPE_OP))
        return GetFUNC_RUN(NodeArr, LexArr);
    
    if (CheckIfOperNextAndInc(NodeArr, IN, TYPE_OP))
        return GetIN(NodeArr, LexArr);


    Node_t * value = GetE(NodeArr, LexArr);


    if (ISOPER && ((**NodeArr).value.oper == ASSIGN))
        return GetASSIGN(NodeArr, value, LexArr);


    return value;
}

Node_t * GetSECT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = NewNode(TYPE_CNOP, {.CNop = PROGRAM_START}, NULL, NULL);
    if (!CheckIfOperNextAndInc(NodeArr, USING_FORMULAS, TYPE_CNOP))
        fprintf(stderr, CYAN "WARNING: proceeding without formulas\n" WHITE);
    else
        value->left = GetFUNCS(NodeArr, LexArr);
    DUMP_LANGNODE(value, "before getsectops");

    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PATH_OF_SOLVING, "без хода решений компилируешь негодяй");
    value->right = GetOPS(NodeArr, NOP, LexArr);

    DUMP_LANGNODE(value, "after getsectops");
    return value;
}

Node_t * GetG(Node_t **NodeArr, LexArr_t *LexArr) {
    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PROGRAM_START, "no starting program symbol!");

    Node_t * ProgramTree = GetSECT(NodeArr, LexArr);

    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PROGRAM_END, "no ending program symbol!");
    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, AI_REFERENCE, "Not AI reference, Not for generation only");

    return ProgramTree;
}

Node_t * GetExpr(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t *value       = NULL;
    LangOperType_e type = GetArthmOper(NodeArr);


    if (ISNUM)
        value = GetNUM(NodeArr, LexArr);
    else if (type != NOP) {
        value = NewNode(TYPE_OP,  {.oper = type}, NULL, NULL);
        (*NodeArr)++;
        MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "no ( in oper");
        value->left = GetE(NodeArr, LexArr);
        MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE, "no ) in oper");
    }
    else if (ISVAR)
        value = GetVAR(NodeArr, LexArr, false);
    else
        return NULL;
    return value;
}

Node_t * GetP(Node_t **NodeArr, LexArr_t *LexArr) {
    if (CheckIfOperNextAndInc(NodeArr, PAR_OPEN, TYPE_OP)) {
        Node_t * value = GetE(NodeArr, LexArr);
        if (!CheckIfOperNextAndInc(NodeArr, PAR_CLOSE, TYPE_OP)) {
            SYNTAX_ERROR(NodeArr, "no closing par");
        }


        return value;
    }
    return GetExpr(NodeArr, LexArr);
}

Node_t * GetM(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetP(NodeArr, LexArr);


    while (CheckIfOperNext(NodeArr, ARTHM_POW, TYPE_OP)) {
        (*NodeArr)++;


        Node_t * value2 = GetP(NodeArr, LexArr);
        value = NewNode(TYPE_OP, {.oper = ARTHM_POW}, value, value2);
    }
    return value;
}

Node_t * GetT(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetM(NodeArr, LexArr);


    while (CheckIfOperNext(NodeArr, ARTHM_MUL, TYPE_OP) || CheckIfOperNext(NodeArr, ARTHM_DIV, TYPE_OP)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;


        Node_t * value2 = GetM(NodeArr, LexArr);
        if (op == ARTHM_MUL)
            value = NewNode(TYPE_OP, {.oper = ARTHM_MUL}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = ARTHM_DIV}, value, value2);
    }
    return value;
}

Node_t * GetE(Node_t **NodeArr, LexArr_t *LexArr) {
    Node_t * value = GetT(NodeArr, LexArr);


    while (CheckIfOperNext(NodeArr, ARTHM_ADD, TYPE_OP) || CheckIfOperNext(NodeArr, ARTHM_SUB, TYPE_OP)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;


        Node_t * value2 = GetT(NodeArr, LexArr);
        if (op == ARTHM_ADD)
            value = NewNode(TYPE_OP, {.oper = ARTHM_ADD}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = ARTHM_SUB}, value, value2);
    }
    return value;
}

bool CheckIfOperNext(Node_t **NodeArr, int oper, LangType_e type) {
    return (ISOPER && (**NodeArr).value.oper == (LangOperType_e) oper && type == TYPE_OP) || (ISCNOP && (**NodeArr).value.CNop == (LangCNOPType_e) oper && type == TYPE_CNOP);
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

LangOperType_e GetArthmOper(Node_t **NodeArr) {
    if ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper < NumOfOpers)
        return (**NodeArr).value.oper;
    return NOP;
}


Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    
    Node_t * NodeArr = LexArr->NodeArr;
    Node_t * Node = GetG(&NodeArr, LexArr);
    return Node;
}