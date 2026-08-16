#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../includes/MyLangBackEnd.hpp"
#include "../my_libs/better_output.hpp"
#include "../my_libs/sassert.hpp"
#include "../includes/MyLang.hpp"
#include "../includes/MyLangDump.hpp"

#define ISVAR   Tree->type == TYPE_VAR
#define ISSTR   Tree->type == TYPE_STR
#define ISNUM   Tree->type == TYPE_NUM
#define ISOPER  Tree->type == TYPE_OP
#define ISCNOP  Tree->type == TYPE_CNOP
#define SYNTAX_ERROR(NodeArr, ...) {\
    fprintf(stderr, "NOW: type: %s ", AllValueTypesTxt[NodeArr->type]);\
    switch(NodeArr->type) {\
        case TYPE_NUM:   fprintf(stderr, "num: %d\n",  NodeArr->value.num);                              break;\
        case TYPE_VAR:   fprintf(stderr, "var: %s\n",  NodeArr->value.var_name);                         break;\
        case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[NodeArr->value.oper]);             break;\
    }\
    fprintf(stderr, RED "-->SYNTAX ERROR: " __VA_ARGS__); fprintf(stderr, "\n" WHITE); sassert(0, ERR_PTR_NULL_LANG);}

#define MATCH_ORSYNTAXERR(type, oper, ...) {\
    if (!CheckIfOperNextAndInc(NodeArr, oper, type)) {\
        SYNTAX_ERROR(NodeArr, __VA_ARGS__);}}

void PrintHelp() {
    printf(MAGENTA "type 1) file you want to compile\n"
                   "     2) file as output\n" WHITE);
}

size_t get_file_size(FILE * fp) {
    sassert(fp, ERR_PTR_NULL_LANG);

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t) ftell(fp);
    rewind(fp);

    return file_size;
}

void nullify_anything_extra(char * buffer, size_t file_size, size_t actually_read) {
    sassert(buffer != NULL, ERR_PTR_NULL_LANG);

    while (actually_read < file_size) {
        buffer[++actually_read] = '\0';
    }
}

char * get_buffer_from_file(FILE * fp, size_t file_size) {
    sassert(fp, ERR_PTR_NULL_LANG);

    char * compile_buffer = (char *) calloc(file_size + 1, sizeof(char));
    sassert(compile_buffer, ERR_PTR_NULL_LANG);

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

    size_t len  = strcspn(buffer + *pos, "\"\0");
    char *str   = strndup(buffer + *pos, len);


    (*pos) += len + 1; // skip last "
    return str;
}

Node_t * NewNode(LangOperType_e type, LangElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = CALLOC_WITH_TYPE(1, Node_t);
    sassert(node, ERR_PTR_NULL_LANG);

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
    sassert(FileBuf, ERR_PTR_NULL_LANG);

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
    sassert(FileBuf, ERR_PTR_NULL_LANG);

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

Node_t * BuildAsmTreeInternal(char *buffer, size_t *pos, BackEnd_t *BackEndStruct) {
    SkipSpaces(pos, buffer);

    (*pos)++; // skip '('
    SkipSpaces(pos, buffer);

    Node_t *node = NULL;

    if (buffer[*pos] == '\"') {
        char *str = ReadQuotedString(pos, buffer);
        SkipSpaces(pos, buffer);

        char *BufferCopy = buffer + *pos;
        list_el_t value = strtol(buffer, &BufferCopy, 10);
        if (BufferCopy != buffer + *pos) {
            node = NewNode(TYPE_NUM, (LangElem_u){.num = value}, NULL, NULL);
            free(str);
        }
        else {
            size_t NameTableSize = BackEndStruct->NameTableSize;
            int VarIndex = GetVarIndexInArr(BackEndStruct->NameTable, str, NameTableSize);

            if (VarIndex == -1) {
                BackEndStruct->NameTable[NameTableSize] = str;
                BackEndStruct->NameTableSize++;
                VarIndex = NameTableSize;
            }
            else {
                free(str);
            }

            node = NewNode(TYPE_VAR, (LangElem_u){.var_name = str}, NULL, NULL);
        }
    } else {
        int str_len = 0;
        LangOperType_e OperType = CheckOperType(buffer + *pos, &str_len);

        if (OperType != OPER_NOP)
            node = NewNode(TYPE_OP, (LangElem_u){.oper = OperType}, NULL, NULL);

        (*pos) += (size_t) str_len;
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

Node_t * BuildAsmTree(const char *buffer, BackEnd_t *BackEndStruct) {
    size_t pos = 0;
    return BuildAsmTreeInternal(buffer, &pos, BackEndStruct);
}

void NodeDtor(Node_t **node) {
    sassert(node, ERR_PTR_NULL_LANG);

    if (*node != NULL && (*node)->left != NULL)
        NodeDtor(&((*node)->left));
    if (*node != NULL && (*node)->right != NULL)
        NodeDtor(&((*node)->right));
    free(*node);
    *node = NULL;
}

 int GetVarIndexByName(BackEnd_t *BackEndStruct, const char *name) {
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
    sassert(Var, ERR_PTR_NULL_LANG);


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

void GenInVar(FILE *fp, BackEnd_t *BackEndStruct, Node_t * node, bool IsFuncOp) {
    int Index = GetVarInNameTable(node->left->value.var_name, NameTable);
    fprintf(fp ,"    IN\n");
    if (IsFuncOp)
        fprintf(fp, "    POPR [%zu]\n", Index + (node->ScopeDeep - 1) * NameTableSize);
    else
        fprintf(fp, "    POPMN [%zu]\n", Index + node->ScopeDeep * NameTableSize);
}
    
void GenPushVar(FILE *fp, BackEnd_t *BackEndStruct, Node_t * node, bool IsFuncOp) {
    int Index = GetVarInNameTable(node->value.var_name, NameTable);
    if (IsFuncOp)
        fprintf(fp, "    PUSHR [%zu]\n", Index + (node->ScopeDeep - 1) * NameTableSize);
    else
        fprintf(fp, "    PUSHMN [%zu]\n", Index + node->ScopeDeep * NameTableSize);
}

void GenPopVar(FILE *fp, BackEnd_t *BackEndStruct, Node_t * node, bool IsFuncOp) {
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
            sassert(0, ERR_PTR_NULL_LANG, "unexpected cmp op");
    }
    PrintLabel(fp, label_prefix, label_id);
    fprintf(fp, "\n");
}

void GenCmp(FILE *fp, BackEnd_t *BackEndStruct, Node_t *cmp_node, const char *label_prefix, size_t label_id,
            bool IsFuncOp, bool IsInverse) {
    GenExpr(fp, cmp_node->left,  NameTable, NameTableSize, IsFuncOp);
    GenExpr(fp, cmp_node->right, NameTable, NameTableSize, IsFuncOp);
    GenCmpJumpFalse(fp, cmp_node->value.oper, label_prefix, label_id, IsInverse);
}

void GenExpr(FILE *fp, BackEnd_t *BackEndStruct, Node_t *node, bool IsFuncOp) {
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
        sassert(0, ERR_PTR_NULL_LANG, "cmp as expr is not supported");
    }

    sassert(0, ERR_PTR_NULL_LANG, "штмфдшв тщву ензу");
}



void GenInitList(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree, bool IsFuncOp) {
    Node_t *cur = Tree;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur,OPER_NEW_INIT))
            break;

        Node_t *decl = cur->left; // VAR_DECLARATION
        if (decl != NULL && CheckIfOperNext(decl,OPER_VAR_DECLARATION)) {
            GenExpr(fp, decl->right, NameTable, NameTableSize, IsFuncOp);
            GenPopVar(fp, decl->left, NameTable, NameTableSize, IsFuncOp);
        }

        cur = cur->right;
    }
}

void GenIf(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree, bool IsFuncOp) {
    size_t end_id = label_counter++;

    Node_t *cmp_list = Tree->left;
    while (cmp_list != NULL) {
        if (!CheckIfOperNext(cmp_list,OPER_NEW_CMP))
            break;

        Node_t *cmp = cmp_list->left;
        GenCmp(fp, cmp, "IF_MIDDLE", end_id, NameTable, NameTableSize, IsFuncOp, false);
        cmp_list = cmp_list->right;
    }

    GenOpList(fp, Tree->right->left, NameTable, NameTableSize, IsFuncOp);
    fprintf(fp, "JMP ");
    PrintLabel(fp, "IF_END", end_id);
    fprintf(fp, "\n");
    PrintLabel(fp, "IF_MIDDLE", end_id);
    fprintf(fp, "\n");    GenOpList(fp, Tree->right->right, NameTable, NameTableSize, IsFuncOp);
    PrintLabel(fp, "IF_END", end_id);
    fprintf(fp, "\n");
}

void GenWhile(FILE *fp, BackEnd_t *BackEndStruct, Node_t *while_node,  bool IsFuncOp) {
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

void GenFuncCall(FILE *fp, BackEnd_t *BackEndStruct, Node_t *call_node, bool IsFuncOp) {
    Node_t *dst = call_node->left;
    Node_t *funcwrap = call_node->right;

    Node_t *fname = funcwrap->left;
    Node_t *params = funcwrap->right;

    Node_t *cur = params;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur,OPER_NEW_PARAM))
            break;

        GenExpr(fp, cur->left, NameTable, NameTableSize, IsFuncOp);
        cur = cur->right;
    }

    fprintf(fp, "    CALL :%s\n", fname->value.var_name);
    GenPopVar(fp, dst, NameTable, NameTableSize, IsFuncOp);
}

void GenOut(FILE *fp, BackEnd_t *BackEndStruct, Node_t *out_node, bool IsFuncOp) {
    GenExpr(fp, out_node->left, NameTable, NameTableSize, IsFuncOp);
    fprintf(fp, "    OUT\n");
}

void GenAssign(FILE *fp, BackEnd_t *BackEndStruct, Node_t *assign_node, bool IsFuncOp) {
    GenExpr(fp, assign_node->right, NameTable, NameTableSize, IsFuncOp);
    GenPopVar(fp, assign_node->left, NameTable, NameTableSize, IsFuncOp);
}

void GenOp(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree, bool IsFuncOp) {
    if (Tree == NULL)
        return;

    if (CheckIfOperNext(Tree,OPER_NEW_INIT)) {
        GenInitList(fp, Tree, NameTable, NameTableSize, IsFuncOp);
        return;
    }

    if (Tree->type == TYPE_OP) {
        switch (Tree->value.oper) {
            case IF:            GenIf(fp, Tree, NameTable, NameTableSize   , IsFuncOp);     return;
            case WHILE:         GenWhile(fp, Tree, NameTable, NameTableSize, IsFuncOp);     return;
            case OUT:           GenOut(fp, Tree, NameTable, NameTableSize  , IsFuncOp);     return;
            case FUNC_CALL:     GenFuncCall(fp, Tree, NameTable, NameTableSize, IsFuncOp);  return;
            case ASSIGN:        GenAssign(fp, Tree, NameTable, NameTableSize, IsFuncOp);    return;
            case IN:            GenInVar(fp, Tree, NameTable, NameTableSize, IsFuncOp);     return;
            case VAR_DECLARATION: {
                GenExpr(fp, Tree->right, NameTable, NameTableSize, IsFuncOp);
                GenPopVar(fp, Tree->left, NameTable, NameTableSize, IsFuncOp);
                return;
            }
            default:
                break;
        }
    }
    else 
        SYNTAX_ERROR(Tree, "нету валидной операции");

    if (Tree->type == TYPE_OP || Tree->type == TYPE_NUM || Tree->type == TYPE_VAR) {
        GenExpr(fp, Tree, NameTable, NameTableSize, IsFuncOp);
        return;
    }
}

void GenOpList(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree, bool IsFuncOp) {
    Node_t *cur = Tree;
    while (cur != NULL) {
        if (!CheckIfOperNext(cur,OPER_NEW_OP))
            break;
        GenOp(fp, cur->left, NameTable, NameTableSize, IsFuncOp);
        cur = cur->right;
    }
}

void GenFuncDef(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree) {
    fprintf(fp, ":%s {\n", Tree->value.var_name);

    Node_t *func_body = Tree->left;
    int param_cnt = 0;
    int ParamIndex[NameTableSize];

    Node_t *params = func_body->left;
    while (params != NULL) {
        if (!CheckIfOperNext(params,OPER_NEW_PARAM))
            break;

        Node_t *var = params->left;
        params = params->right;

        int idx = GetVarInNameTable(var->value.var_name, NameTable);
        sassert(idx >= 0, ERR_PTR_NULL_LANG);
        ParamIndex[param_cnt++] = idx;
    }

    for (int i = param_cnt - 1; i >= 0; i--) {
        fprintf(fp, "    POPR [%d]\n", ParamIndex[i]);
    }

    GenOpList(fp, func_body->right, NameTable, NameTableSize, true);

    GenExpr(fp, Tree->right, NameTable, NameTableSize, true);
    fprintf(fp, "    RET\n");
    fprintf(fp, "}\n\n");
}


void GenFuncList(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree) {
    while (Tree != NULL) {
        if (!CheckIfOperNext(Tree, OPER_FUNC_DECL_NAME))
            break;

        GenFuncDef(fp, BackEndStruct, Tree->left);
        Tree = Tree->right;
    }
}

bool CheckIfOperNext(Node_t *Tree, LangOperType_e oper) {
    return (ISOPER &&  (Tree->value.oper == (LangOperType_e) oper ||
                        Tree->value.oper == OPER_NOP));
}

void PrintProgramAssemblyFromTree(FILE *fp, BackEnd_t *BackEndStruct, Node_t *Tree) {
    sassert(fp,             ERR_PTR_NULL_LANG);
    sassert(BackEndStruct,  ERR_PTR_NULL_LANG);

    if (CheckIfOperNext(Tree, OPER_PROGRAM_START)) {
        fprintf(fp, "CALL :main\n\n");

        // funcs
        if (Tree->left != NULL)
            GenFuncList(fp, BackEndStruct, Tree->left);

        // main
        fprintf(fp, ":main {\n");
        GenOpList(fp, BackEndStruct, Tree->right, false);
        fprintf(fp, "    HLT\n");
        fprintf(fp, "}\n");
        return;
    }
    SYNTAX_ERROR(Tree, "нету начала программы!");
}

#define BackEndCtor(Name) \
    BackEnd_t *Name = CALLOC_WITH_TYPE(1, BackEnd_t);\
    BackEndCtor_internal(Name);

void BackEndCtor_internal(BackEnd_t *BackEndStruct) {
    init_stack(stk, START_INIT_SIZE);
    BackEndStruct->ScopeBorders     = stk;

    BackEndStruct->NameTableSize        = 0;
    BackEndStruct->NameTableCapacity    = START_INIT_SIZE;
    BackEndStruct->ScopeSize            = 0;
    BackEndStruct->ScopeCapacity        = START_INIT_SIZE;

    BackEndStruct->NameTable            = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(BackEndStruct->NameTable,   ERR_PTR_NULL_LANG);

    BackEndStruct->Scope                = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(BackEndStruct->Scope,       ERR_PTR_NULL_LANG);

    BackEndStruct->RegexTable           = CALLOC_WITH_TYPE(NumOfRegex, int64_t);
    sassert(BackEndStruct->RegexTable,  ERR_PTR_NULL_LANG);
}

void CompileFile(const char * InputFile, const char * OutputFile) {
    FILE *fp_input = fopen(InputFile, "r");
    sassert(fp_input, ERR_PTR_NULL_LANG);

    size_t FileSize = get_file_size(fp_input);
    char *buffer = get_buffer_from_file(fp_input, FileSize);
    fclose(fp_input);

    BackEndCtor(BackEndStruct);
    Node_t * Tree = BuildAsmTree(buffer, BackEndStruct);
    free(buffer);

    create_Tree_graph(Tree);
    FILE *fp_output = fopen(OutputFile, "w");
    sassert(fp_output, ERR_PTR_NULL_LANG, "не удалось открыть %s", OutputFile);

    PrintProgramAssemblyFromTree(fp_output, BackEndStruct, BackEndStruct->Tree);

    fclose(fp_output);
    NodeDtor(&Tree);
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