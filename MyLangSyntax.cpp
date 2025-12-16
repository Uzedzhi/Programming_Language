#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "../my_libs/sassert.h"
#include "MyLangSyntax.h"
#include "MyLang.h"
#include "MyLangDump.h"

Node_t * node = NULL;

#define ISVAR   (**NodeArr).type == TYPE_VAR
#define ISNUM   (**NodeArr).type == TYPE_NUM
#define ISOPER  (**NodeArr).type == TYPE_OP
#define ISCNOP  (**NodeArr).type == TYPE_CNOP
#define SYNTAX_ERROR(NodeArr, ...) {\
    fprintf(stderr, "NOW: type: %s ", AllValueTypesTxt[(**NodeArr).type]);\
    switch((**NodeArr).type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  (**NodeArr).value.num);                      break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  (**NodeArr).value.var_name);                 break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[(**NodeArr).value.oper]);     break;\
        case TYPE_CNOP:  fprintf(stderr, "cnop: %s\n", AllOperCNOPDumpStr[(**NodeArr).value.CNop]); break;\
    }\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); sassert(0, ERR_PTR_NULL);}

#define MATCHOPER_ORSYNTAXERR(type, oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper, type)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}


bool CheckIfOperNextAndInc(Node_t **NodeArr, int oper, LangType_e type) {
    bool is_oper = (ISOPER && (**NodeArr).value.oper == (LangOperType_e) oper && type == TYPE_OP) || (ISCNOP && (**NodeArr).value.CNop == (LangCNOPType_e) oper && type == TYPE_CNOP);
    if (is_oper)
        (*NodeArr)++;
    return is_oper;
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

Node_t * GetVAR(Node_t **NodeArr) {
    if (!ISVAR) return NULL;
    Node_t * node = NewNode(TYPE_VAR, {.var_name = (**NodeArr).value.var_name}, NULL, NULL);
    (*NodeArr)++;
    return node;
}

Node_t * GetFUNC(Node_t **NodeArr) {
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "in func decl ( is missing");
    Node_t * func = GetVAR(NodeArr);

    DUMP_LANGNODE(func, "after in getfunc");
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE, "in func decl ) is missing");
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_OPEN, "in func params ( is missing");

    Node_t * AllParams = NewNode(TYPE_CNOP, {.CNop = NEW_PARAM}, NULL, NULL);
    Node_t * CurNode   = AllParams;
    while ((CurNode->left = GetVAR(NodeArr)) != NULL) {
        if (!CheckIfOperNextAndInc(NodeArr, COMMA, TYPE_OP))
            break;
        CurNode->right = NewNode(TYPE_CNOP, {.CNop = NEW_PARAM}, NULL, NULL);
        CurNode = CurNode->right;
        DUMP_LANGNODE(AllParams, "allparams in getfunc");

    }
    func->left = AllParams;
    
    MATCHOPER_ORSYNTAXERR(TYPE_OP, PAR_CLOSE,    "in func params ) is missing");
    MATCHOPER_ORSYNTAXERR(TYPE_OP, COLON,        "in func decl : is missing");
    DUMP_LANGNODE(func, "func in getfunc");

    func->right = GetOPS(NodeArr);
    
    DUMP_LANGNODE(func, "after func in getfunc");
    return func;
}

Node_t * GetFUNCS(Node_t **NodeArr) {
    Node_t * AllFuncs = NewNode(TYPE_CNOP, {.CNop = NEW_FUNC}, NULL, NULL);
    Node_t * cur_node = AllFuncs;
    DUMP_LANGNODE(AllFuncs, "before getfuncs");

    while (CheckIfOperNextAndInc(NodeArr, FUNC_DECL_NAME, TYPE_CNOP)) {
        cur_node->left  = GetFUNC(NodeArr);
        cur_node->right = NewNode(TYPE_CNOP, {.CNop = NEW_FUNC}, NULL, NULL);
        cur_node        = cur_node->right;
        DUMP_LANGNODE(AllFuncs, "after getfunc");
    }
    DUMP_LANGNODE(AllFuncs, "after getfuncs");
    
    return AllFuncs;  
}

Node_t * GetOPS(Node_t **NodeArr) {
    Node_t *ops_list = NewNode(TYPE_CNOP, (LangElem_u){.CNop = NEW_OP}, NULL, NULL);
    Node_t *cur = ops_list;
    size_t iter = 0;

    while (!CheckIfOperNext(NodeArr, PROGRAM_END, TYPE_CNOP)    &&
           !CheckIfOperNext(NodeArr, FUNC_DECL_NAME, TYPE_CNOP) &&
           !CheckIfOperNext(NodeArr, PATH_OF_SOLVING, TYPE_CNOP)) {
        if (iter++ != 0) {
            cur->right = NewNode(TYPE_CNOP, {.CNop = NEW_OP}, NULL, NULL);
            cur = cur->right;
        }

        Node_t *op = GetOP(NodeArr);
        if (!op) {
            (*NodeArr)++;
            break;
        }

        cur->left = op;
    }

    return ops_list;
}

Node_t *GetCMP(Node_t **NodeArr) {
    Node_t * CmpNode = NewNode(TYPE_OP, {.oper = NOP}, NULL, NULL);
    CmpNode->left = GetE(NodeArr);
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

    CmpNode->right = GetE(NodeArr);
    return CmpNode;
}

Node_t *GetOP(Node_t **NodeArr) {
    // IF "если"
    if (CheckIfOperNextAndInc(NodeArr, IF, TYPE_OP)) {
        Node_t * IfStatement = NewNode(TYPE_OP, {.oper = IF}, NULL, NULL);
        IfStatement->left = NewNode(TYPE_CNOP, {.CNop = NEW_CMP}, NULL, NULL);
        Node_t * AllCompares = IfStatement->left;

        AllCompares->left = GetCMP(NodeArr);
        while (CheckIfOperNextAndInc(NodeArr, COMMA, TYPE_OP) && AllCompares) {
            AllCompares->right = NewNode(TYPE_CNOP, {.CNop = NEW_CMP}, NULL, NULL);
            AllCompares = AllCompares->right;
            AllCompares->left = GetCMP(NodeArr);
        }

        MATCHOPER_ORSYNTAXERR(TYPE_OP, THEN, "нету то после если");
        IfStatement->right = GetOPS(NodeArr);
        return IfStatement;
    }

    // 'условимся' -> INIT
    if (CheckIfOperNextAndInc(NodeArr, INIT, TYPE_OP)) {
        Node_t *InitValue = NewNode(TYPE_OP, {.oper = INIT}, NULL, NULL);
        InitValue->left = GetVAR(NodeArr);
        MATCHOPER_ORSYNTAXERR(TYPE_OP, ASSIGN, "нету равно после инициализации");
        InitValue->right = GetE(NodeArr);
        
        return NULL;
    }

    // 'Напишем' -> PRINT
    if (ISOPER && (**NodeArr).value.oper == WRITE) {
        return NULL;
    }

    Node_t * value = GetE(NodeArr);

    // равно, '=' -> ASSIGN
    if (ISOPER && ((**NodeArr).value.oper == ASSIGN)) {
        Node_t * AssignNode = NewNode(TYPE_OP, {.oper = ASSIGN}, NULL, NULL);
        AssignNode->left = value;
        MATCHOPER_ORSYNTAXERR(TYPE_OP, ASSIGN, "no = after lvalue");
        AssignNode->right = GetE(NodeArr);
        return AssignNode;
    }

    // Если ничего не подошло, считаем, что OPS закончился.
    return value;
}

Node_t * GetSECT(Node_t ** NodeArr) {
    Node_t * value = NewNode(TYPE_CNOP, {.CNop = PROGRAM_START}, NULL, NULL);
    if (!CheckIfOperNextAndInc(NodeArr, USING_FORMULAS, TYPE_CNOP))
        fprintf(stderr, CYAN "WARNING: proceeding without formulas\n" WHITE);
    else
        value->left = GetFUNCS(NodeArr);
    DUMP_LANGNODE(value, "before getsectops");

    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PATH_OF_SOLVING, "без хода решений компилируешь негодяй");
    value->right = GetOPS(NodeArr);

    DUMP_LANGNODE(value, "after getsectops");
    return value;
}

Node_t * GetG(Node_t ** NodeArr) {
    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PROGRAM_START, "no starting program symbol!");
    Node_t * ProgramTree = GetSECT(NodeArr);
    node = *NodeArr;
    MATCHOPER_ORSYNTAXERR(TYPE_CNOP, PROGRAM_END, "no ending program symbol!");
    
    return ProgramTree;
}

LangOperType_e GetArthmOper(Node_t **NodeArr) {
    if ((**NodeArr).type == TYPE_OP && (**NodeArr).value.oper >= ARTHM_MUL && (**NodeArr).value.oper < NumOfOpers)
        return (**NodeArr).value.oper;
    return NOP;
}

Node_t * GetExpr(Node_t **NodeArr) {
    Node_t *value       = NULL;
    LangOperType_e type = GetArthmOper(NodeArr);

    if (ISNUM)
        value = NewNode(TYPE_NUM, {.num = (**NodeArr).value.num}, NULL, NULL);
    else if (type != NOP)
        value = NewNode(TYPE_OP,  {.oper = type}, NULL, NULL);
    else if (ISVAR)
        value = NewNode(TYPE_VAR, {.var_name = (**NodeArr).value.var_name}, NULL, NULL);
    else
        return NULL;
    (*NodeArr)++;
    return value;
}

Node_t * GetP(Node_t **NodeArr) {
    if (CheckIfOperNextAndInc(NodeArr, PAR_OPEN, TYPE_OP)) {
        Node_t * value = GetE(NodeArr);
        if (!CheckIfOperNextAndInc(NodeArr, PAR_CLOSE, TYPE_OP)) {
            SYNTAX_ERROR(NodeArr, "no closing par");
        }

        return value;
    }
    return GetExpr(NodeArr);
}

Node_t * GetM(Node_t **NodeArr) {
    Node_t * value = GetP(NodeArr);

    while (CheckIfOperNext(NodeArr, ARTHM_POW, TYPE_OP)) {
        (*NodeArr)++;

        Node_t * value2 = GetP(NodeArr);
        value = NewNode(TYPE_OP, {.oper = ARTHM_POW}, value, value2);
    }
    return value;
}

Node_t * GetT(Node_t **NodeArr) {
    Node_t * value = GetM(NodeArr);

    while (CheckIfOperNext(NodeArr, ARTHM_MUL, TYPE_OP) || CheckIfOperNext(NodeArr, ARTHM_DIV, TYPE_OP)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;

        Node_t * value2 = GetM(NodeArr);
        if (op == ARTHM_MUL)
            value = NewNode(TYPE_OP, {.oper = ARTHM_MUL}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = ARTHM_DIV}, value, value2);
    }
    return value;
}

Node_t * GetE(Node_t **NodeArr) {
    Node_t * value = GetT(NodeArr);

    while (CheckIfOperNext(NodeArr, ARTHM_ADD, TYPE_OP) || CheckIfOperNext(NodeArr, ARTHM_SUB, TYPE_OP)) {
        LangOperType_e op = (**NodeArr).value.oper;
        (*NodeArr)++;

        Node_t * value2 = GetT(NodeArr);
        if (op == ARTHM_ADD)
            value = NewNode(TYPE_OP, {.oper = ARTHM_ADD}, value, value2);
        else
            value = NewNode(TYPE_OP, {.oper = ARTHM_SUB}, value, value2);
    }
    return value;
}

Node_t * MakeTreeFromArrayOfLexems(LexArr_t *LexArr) {
    sassert(LexArr, ERR_PTR_NULL);
    
    Node_t * Node = GetG(&(LexArr->NodeArr));
    return Node;
}