#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "MyLangBackEnd.h"
#include "better_output.h"
#include "sassert.h"
#include "MyLang.h"
#include "MyLangDump.h"

#define ISVAR   tree->type == TYPE_VAR
#define ISSTR   tree->type == TYPE_STR
#define ISNUM   tree->type == TYPE_NUM
#define ISOPER  tree->type == TYPE_OP
#define ISCNOP  tree->type == TYPE_CNOP
#define SYNTAX_ERROR(NodeArr, ...) {\
    fprintf(stderr, "NOW: type: %s ", AllValueTypesTxt[tree->type]);\
    switch(tree->type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  tree->value.num);                              break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  tree->value.var_name);                         break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[tree->value.oper]);             break;\
        case TYPE_CNOP:  fprintf(stderr, "cnop: %s\n", AllOperCNOPDumpStr[tree->value.CNop]);         break;\
    }\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); sassert(0, ERR_PTR_NULL);}

bool CheckIfOperNext(Node_t *tree, int oper, LangType_e type) {
    return (ISOPER && tree->value.oper == (LangOperType_e) oper && type == TYPE_OP) || (ISCNOP && tree->value.CNop == (LangCNOPType_e) oper && type == TYPE_CNOP);
}

#define MATCHOPER_ORSYNTAXERR(type, oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper, type)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}

void PrintHelp() {
    printf(MAGENTA "type 1) file you want to compile\n"
                   "     2) file as output\n" WHITE);
}

size_t get_file_size(FILE * fp) {
    sassert(fp, ERR_PTR_NULL);

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t) ftell(fp);
    rewind(fp);

    return file_size;
}

void nullify_anything_extra(char * buffer, size_t file_size, size_t actually_read) {
    sassert(buffer != NULL, ERR_PTR_NULL);

    while (actually_read < file_size) {
        buffer[++actually_read] = '\0';
    }
}

char * get_buffer_from_file(FILE * fp, size_t file_size) {
    sassert(fp, ERR_PTR_NULL);

    char * compile_buffer = (char *) calloc(file_size + 1, sizeof(char));
    sassert(compile_buffer, ERR_PTR_NULL);

    size_t actually_read = fread(compile_buffer, sizeof(char), file_size, fp);
    nullify_anything_extra(compile_buffer, file_size, actually_read);

    return compile_buffer;
}



void SkipSpaces(size_t * pos, const char * buffer) {
    while (isspace((unsigned char) buffer[*pos]))
        (*pos)++;
}

 char * ReadQuotedString(size_t *pos, const char *buffer) {
    (*pos)++; // skip first "
    size_t start = *pos;
    while (buffer[*pos] != '\"')
        (*pos)++;

    size_t len = *pos - start;
    char *str = CALLOC_WITH_TYPE(len + 1, char);
    strncpy(str, buffer + start, len);

    (*pos)++; // skip last "
    return str;
}

Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = CALLOC_WITH_TYPE(1, Node_t);
    sassert(node, ERR_PTR_NULL);

    node->type  = type;
    node->value = value;
    node->left  = left;
    node->right = right;
    return node;
}

bool IsIntStr(const char *s) {
    if (*s == '-') s++;
    if (!isdigit((unsigned char) *s)) return false;
    while (*s) {
        if (!isdigit((unsigned char) *s)) return false;
        s++;
    }
    return true;
}

LangOperType_e CheckOperType(const char * const FileBuf, int *str_len) {
    sassert(FileBuf, ERR_PTR_NULL);

    for (size_t i = 0; i < NumOfOpers; i++) {
        int CurStrLen = strlen(AllOperInFileStr[i][0]);
        if (strncmp(FileBuf, AllOperInFileStr[i][0], (size_t)CurStrLen) == 0) {
            char next = FileBuf[CurStrLen];
            if (next == '\0' || next == '(' || next == ')' || isspace((unsigned char)next)) {
                *str_len = CurStrLen;
                return (LangOperType_e) i;
            }
        }
    }

    return NOP;
}

LangCNOPType_e CheckOperCNOPType(const char * const FileBuf, int *str_len) {
    sassert(FileBuf, ERR_PTR_NULL);

    for (size_t i = 0; i < NumOfCNOPOpers; i++) {
        int CurStrLen = strlen(AllOperCNOPDumpStr[i]);
        if (strncmp(FileBuf, AllOperCNOPDumpStr[i], (size_t)CurStrLen) == 0) {
            char next = FileBuf[CurStrLen];
            if (next == '\0' || next == '(' || next == ')' || isspace((unsigned char)next)) {
                *str_len = CurStrLen;
                return (LangCNOPType_e)i;
            }
        }
    }

    return NCNOP;
}

Node_t * BuildAsmTreeInternal(const char *buffer, size_t *pos, char *NameTable[]) {
    SkipSpaces(pos, buffer);

    (*pos)++; // skip '('
    SkipSpaces(pos, buffer);

    Node_t *node = NULL;

    if (buffer[*pos] == '\"') {
        char *str = ReadQuotedString(pos, buffer);
        SkipSpaces(pos, buffer);

        if (IsIntStr(str)) {
            node = NewNode(TYPE_NUM, (LangElem_u){.num = atoi(str)}, NULL, NULL);
            free(str);
        }
        else {
            if (isdigit(buffer[*pos])) {
                int Index = atoi(buffer + *pos);
                if (NameTable[Index] == NULL) {
                    node = NewNode(TYPE_VAR, (LangElem_u){.var_name = str}, NULL, NULL);
                    NameTable[Index] = str;
                }
                else {
                    node = NewNode(TYPE_VAR, (LangElem_u){.var_name = NameTable[Index]}, NULL, NULL);
                    free(str);
                }
                if (Index == 0)
                    (*pos)++;
                else
                    (*pos) += (1 + (int)log10(Index));

            }
            else
                sassert(0, ERR_PTR_NULL, "unexpected error");
            
            (*pos)++;
            if (isdigit(buffer[*pos])) {
                int Scope = atoi(buffer + *pos);
                node->ScopeDeep = Scope;
                if (Scope == 0)
                    (*pos)++;
                else
                    (*pos) += (1 + (int)log10(Scope));

            }
        }
    }
    else {
        int str_len = 0;
        LangOperType_e OperType     = CheckOperType(buffer + *pos, &str_len);
        LangCNOPType_e OperCNOPType = CheckOperCNOPType(buffer + *pos, &str_len);

        if (OperCNOPType != NCNOP)
            node = NewNode(TYPE_CNOP, (LangElem_u){.CNop = OperCNOPType}, NULL, NULL);
        else if (OperType != NOP)
            node = NewNode(TYPE_OP, (LangElem_u){.oper = OperType}, NULL, NULL);

        (*pos) += (size_t)str_len;
    }
    SkipSpaces(pos, buffer);

    if (buffer[*pos] == '(')
        node->left = BuildAsmTreeInternal(buffer, pos, NameTable);

    SkipSpaces(pos, buffer);

    if (buffer[*pos] == '(')
        node->right = BuildAsmTreeInternal(buffer, pos, NameTable);

    SkipSpaces(pos, buffer);
    (*pos)++; // skip ')'
    return node;
}

Node_t * BuildAsmTree(const char *buffer, char *NameTable[], size_t NameTableSize) {
    size_t pos = 0;
    return BuildAsmTreeInternal(buffer, &pos, NameTable);
}

void NodeDtor(Node_t **node) {
    sassert(node, ERR_PTR_NULL);

    if (*node != NULL && (*node)->left != NULL)
        NodeDtor(&((*node)->left));
    if (*node != NULL && (*node)->right != NULL)
        NodeDtor(&((*node)->right));
    free(*node);
    *node = NULL;
}

 int GetVarIndexByName(char *NameTable[], size_t NameTableSize, const char *name) {
    for (int i = 0; i < NameTableSize; i++) {
        if (NameTable[i] != NULL && strcmp(NameTable[i], name) == 0)
            return i;
    }
    return -1;
}

void PrintTab(FILE *fp, size_t tab) {
    for (size_t i = 0; i < tab; i++)
        fprintf(fp, "    ");
}

 size_t label_counter = 0;

 int GetVarInNameTable(char * Var, char *NameTable[]) {
    sassert(Var, ERR_PTR_NULL);


    if (NameTable == NULL)
        return -1;
    int i = 0;
    while (NameTable[i] != NULL) {

        if (strcmp(NameTable[i], Var) == 0)
            return i;
        i++;
    }
    return -1;
}

void PrintLabel(FILE *fp, const char *prefix, size_t id) {
    fprintf(fp, ":%s_%zu", prefix, id);
}

void GenInVar(FILE *fp, Node_t * node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    int Index = GetVarInNameTable(node->left->value.var_name, NameTable);
    fprintf(fp ,"    IN\n");
    if (IsFuncOp)
        fprintf(fp, "    POPR [%zu]\n", Index + (node->ScopeDeep - 1) * NameTableSize);
    else
        fprintf(fp, "    POPMN [%zu]\n", Index + node->ScopeDeep * NameTableSize);
}
    
void GenPushVar(FILE *fp, Node_t * node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    int Index = GetVarInNameTable(node->value.var_name, NameTable);
    if (IsFuncOp)
        fprintf(fp, "    PUSHR [%zu]\n", Index + (node->ScopeDeep - 1) * NameTableSize);
    else
        fprintf(fp, "    PUSHMN [%zu]\n", Index + node->ScopeDeep * NameTableSize);
}

void GenPopVar(FILE *fp, Node_t * node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    int Index = GetVarInNameTable(node->value.var_name, NameTable);

    if (IsFuncOp)
        fprintf(fp, "    POPR [%zu]\n", Index + (node->ScopeDeep - 1) * NameTableSize);
    else
        fprintf(fp, "    POPMN [%zu]\n", Index + node->ScopeDeep * NameTableSize);
}

void GenCmpJumpFalse(FILE *fp, LangOperType_e cmp_op, const char *label_prefix, size_t label_id, bool IsInverse) {
    switch (cmp_op) {
        case EQ:    fprintf(fp, "    JNE "); break;  // (==)
        case NEQ:   fprintf(fp, "    JE ");  break;  // (!=)
        case LESS:  fprintf(fp, "    JAE "); break;  // (<)
        case MORE:  fprintf(fp, "    JBE "); break;  // (>)
        case LEQ:   fprintf(fp, "    JA ");  break;  // (<=)
        case MEQ:   fprintf(fp, "    JB ");  break;  // (>=)
        default:
            sassert(0, ERR_PTR_NULL, "unexpected cmp op");
    }
    PrintLabel(fp, label_prefix, label_id);
    fprintf(fp, "\n");
}

void GenCmp(FILE *fp, Node_t *cmp_node, const char *label_prefix, size_t label_id,
                   char *NameTable[], size_t NameTableSize, bool IsFuncOp, bool IsInverse) {
    GenExpr(fp, cmp_node->left,  NameTable, NameTableSize, IsFuncOp);
    GenExpr(fp, cmp_node->right, NameTable, NameTableSize, IsFuncOp);
    GenCmpJumpFalse(fp, cmp_node->value.oper, label_prefix, label_id, IsInverse);
}

void GenExpr(FILE *fp, Node_t *node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    if (node == NULL)
        return;

    if (node->type == TYPE_NUM) {
        fprintf(fp, "    PUSH %d\n", node->value.num);
        return;
    }
    if (node->type == TYPE_VAR) {
        GenPushVar(fp, node, NameTable, NameTableSize, IsFuncOp);
        return;
    }

    if (node->type == TYPE_OP) {
        LangOperType_e op = node->value.oper;

        // arithmetic
        if (op == ARTHM_ADD || op == ARTHM_SUB || op == ARTHM_MUL || op == ARTHM_DIV || op == ARTHM_POW || op == ARTHM_SQRT) {
            GenExpr(fp, node->left,  NameTable, NameTableSize, IsFuncOp);
            GenExpr(fp, node->right, NameTable, NameTableSize, IsFuncOp);

            switch(op) {
                case ARTHM_ADD:     fprintf(fp, "    ADD\n");   break;
                case ARTHM_SUB:     fprintf(fp, "    SUB\n");   break;
                case ARTHM_MUL:     fprintf(fp, "    MUL\n");   break;
                case ARTHM_DIV:     fprintf(fp, "    DIV\n");   break;
                case ARTHM_POW:     fprintf(fp, "    POW\n");   break;
                case ARTHM_SQRT:    fprintf(fp, "    ROOT\n");  break;
            }
            return;
        }
        sassert(0, ERR_PTR_NULL, "cmp as expr is not supported");
    }

    sassert(0, ERR_PTR_NULL, "штмфдшв тщву ензу");
}



void GenInitList(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    Node_t *cur = tree;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur, NEW_INIT, TYPE_CNOP))
            break;

        Node_t *decl = cur->left; // VAR_DECLARATION
        if (decl != NULL && CheckIfOperNext(decl, VAR_DECLARATION, TYPE_OP)) {
            GenExpr(fp, decl->right, NameTable, NameTableSize, IsFuncOp);
            GenPopVar(fp, decl->left, NameTable, NameTableSize, IsFuncOp);
        }

        cur = cur->right;
    }
}

void GenIf(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    size_t end_id = label_counter++;

    Node_t *cmp_list = tree->left;
    while (cmp_list != NULL) {
        if (!CheckIfOperNext(cmp_list, NEW_CMP, TYPE_CNOP))
            break;

        Node_t *cmp = cmp_list->left;
        GenCmp(fp, cmp, "IF_MIDDLE", end_id, NameTable, NameTableSize, IsFuncOp, false);
        cmp_list = cmp_list->right;
    }

    GenOpList(fp, tree->right->left, NameTable, NameTableSize, IsFuncOp);
    fprintf(fp, "JMP ");
    PrintLabel(fp, "IF_END", end_id);
    fprintf(fp, "\n");
    PrintLabel(fp, "IF_MIDDLE", end_id);
    fprintf(fp, "\n");    GenOpList(fp, tree->right->right, NameTable, NameTableSize, IsFuncOp);
    PrintLabel(fp, "IF_END", end_id);
    fprintf(fp, "\n");
}

void GenWhile(FILE *fp, Node_t *while_node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    size_t begin_id = label_counter;
    size_t end_id   = label_counter++;

    PrintLabel(fp, "WHILE_BEGIN", begin_id);
    fprintf(fp, "\n");

    GenCmp(fp, while_node->left, "WHILE_END", end_id, NameTable, NameTableSize, IsFuncOp, true);
    GenOpList(fp, while_node->right, NameTable, NameTableSize, IsFuncOp);

    fprintf(fp, "    JMP ");
    PrintLabel(fp, "WHILE_BEGIN", begin_id);
    fprintf(fp, "\n");

    PrintLabel(fp, "WHILE_END", end_id);
    fprintf(fp, "\n");
}

void GenFuncCall(FILE *fp, Node_t *call_node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    Node_t *dst = call_node->left;
    Node_t *funcwrap = call_node->right;

    Node_t *fname = funcwrap->left;
    Node_t *params = funcwrap->right;

    Node_t *cur = params;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur, NEW_PARAM, TYPE_CNOP))
            break;

        GenExpr(fp, cur->left, NameTable, NameTableSize, IsFuncOp);
        cur = cur->right;
    }

    fprintf(fp, "    CALL :%s\n", fname->value.var_name);
    GenPopVar(fp, dst, NameTable, NameTableSize, IsFuncOp);
}

void GenOut(FILE *fp, Node_t *out_node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    GenExpr(fp, out_node->left, NameTable, NameTableSize, IsFuncOp);
    fprintf(fp, "    OUT\n");
}

void GenAssign(FILE *fp, Node_t *assign_node, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    GenExpr(fp, assign_node->right, NameTable, NameTableSize, IsFuncOp);
    GenPopVar(fp, assign_node->left, NameTable, NameTableSize, IsFuncOp);
}

void GenOp(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    if (tree == NULL)
        return;

    if (CheckIfOperNext(tree, NEW_INIT, TYPE_CNOP)) {
        GenInitList(fp, tree, NameTable, NameTableSize, IsFuncOp);
        return;
    }

    if (tree->type == TYPE_OP) {
        switch (tree->value.oper) {
            case IF:            GenIf(fp, tree, NameTable, NameTableSize   , IsFuncOp);     return;
            case WHILE:         GenWhile(fp, tree, NameTable, NameTableSize, IsFuncOp);     return;
            case OUT:           GenOut(fp, tree, NameTable, NameTableSize  , IsFuncOp);     return;
            case FUNC_CALL:     GenFuncCall(fp, tree, NameTable, NameTableSize, IsFuncOp);  return;
            case ASSIGN:        GenAssign(fp, tree, NameTable, NameTableSize, IsFuncOp);    return;
            case IN:            GenInVar(fp, tree, NameTable, NameTableSize, IsFuncOp);     return;
            case VAR_DECLARATION: {
                GenExpr(fp, tree->right, NameTable, NameTableSize, IsFuncOp);
                GenPopVar(fp, tree->left, NameTable, NameTableSize, IsFuncOp);
                return;
            }
            default:
                break;
        }
    }
    else 
        SYNTAX_ERROR(tree, "нету валидной операции");

    if (tree->type == TYPE_OP || tree->type == TYPE_NUM || tree->type == TYPE_VAR) {
        GenExpr(fp, tree, NameTable, NameTableSize, IsFuncOp);
        return;
    }
}

void GenOpList(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize, bool IsFuncOp) {
    Node_t *cur = tree;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur, NEW_OP, TYPE_CNOP))
            break;
        GenOp(fp, cur->left, NameTable, NameTableSize, IsFuncOp);
        cur = cur->right;
    }
}

void GenFuncDef(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize) {
    fprintf(fp, ":%s {\n", tree->value.var_name);

    Node_t *func_body = tree->left;
    int param_cnt = 0;
    int ParamIndex[NameTableSize];

    Node_t *params = func_body->left;
    while (params != NULL) {
        if (!CheckIfOperNext(params, NEW_PARAM, TYPE_CNOP))
            break;

        Node_t *var = params->left;
        params = params->right;

        int idx = GetVarInNameTable(var->value.var_name, NameTable);
        sassert(idx >= 0, ERR_PTR_NULL);
        ParamIndex[param_cnt++] = idx;
    }

    for (int i = param_cnt - 1; i >= 0; i--) {
        fprintf(fp, "    POPR [%d]\n", ParamIndex[i]);
    }

    GenOpList(fp, func_body->right, NameTable, NameTableSize, true);

    GenExpr(fp, tree->right, NameTable, NameTableSize, true);
    fprintf(fp, "    RET\n");
    fprintf(fp, "}\n\n");
}


void GenFuncList(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize) {
    while (tree != NULL) {
        if (!CheckIfOperNext(tree, FUNC, TYPE_CNOP))
            break;

        GenFuncDef(fp, tree->left, NameTable, NameTableSize);
        tree = tree->right;
    }
}

void PrintProgramAssemblyFromTree(FILE *fp, Node_t *tree, char *NameTable[], size_t NameTableSize) {
    sassert(fp, ERR_PTR_NULL);
    sassert(tree, ERR_PTR_NULL);

    if (CheckIfOperNext(tree, PROGRAM_START, TYPE_CNOP)) {
        fprintf(fp, "CALL :main\n\n");

        // funcs
        if (tree->left != NULL)
            GenFuncList(fp, tree->left, NameTable, NameTableSize);

        // main
        fprintf(fp, ":main {\n");
        GenOpList(fp, tree->right, NameTable, NameTableSize, false);
        fprintf(fp, "    HLT\n");
        fprintf(fp, "}\n");
        return;
    }
    SYNTAX_ERROR(Tree, "нету начала программы!");
}


void CompileFile(const char * InputFile, const char * OutputFile) {
    FILE *fp_input = fopen(InputFile, "r");
    sassert(fp_input, ERR_PTR_NULL);

    size_t FileSize = get_file_size(fp_input);
    int NameTableSize = 0;
    size_t ReadValNum = fscanf(fp_input, "%d\n", &NameTableSize);
    sassert(ReadValNum == 1, ERR_PTR_NULL,
            "неправильный формат файлы \"%s\"", InputFile);

    char *buffer = get_buffer_from_file(fp_input, FileSize);

    fclose(fp_input);

    char *NameTable[NameTableSize] = {};
    Node_t * Tree = BuildAsmTree(buffer, NameTable, NameTableSize);
    free(buffer);

    create_tree_graph(Tree);

    FILE *fp_output = fopen(OutputFile, "w");
    sassert(fp_output, ERR_PTR_NULL, "не удалось открыть %s", OutputFile);

    PrintProgramAssemblyFromTree(fp_output, Tree, NameTable, NameTableSize);

    NodeDtor(&Tree);
    fclose(fp_output);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        PrintHelp();
        return 0;
    }

    char output_file[MAX_STR_SIZE] = "out.txt";
    if (argc == 2)
        printf(MAGENTA "you did not type output file.\n"
                       "Compiling will be proceeded to <out.txt>\n" WHITE);
    else if (argc == 3)
        strcpy(output_file, argv[2]);
    
    CompileFile(argv[1], output_file);
    return 0;
}