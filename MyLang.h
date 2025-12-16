#ifndef MYLANG_H
#define MYLANG_H

#include <stdlib.h>
#include <string.h>

typedef const char * const string_t;

string_t dump_site_name             = "dump.html";
string_t AllValueTypesTxt[]         = {"OPER", "VAR", "NUM", "CNOP"};
string_t dump_graph_txt_file_name   = "graph/graph.txt";

const size_t MAX_STR_SIZE       = 500;
const size_t START_INIT_SIZE    = 100;
const size_t MAX_STR_VAR        = 5;

#define LANG_OPER_TYPES(n) \
    n(IF,               "если")\
    n(THEN,             "то")\
    n(PAR_OPEN,         "(")\
    n(PAR_CLOSE,        ")")\
    n(EQ,               "==")\
    n(LEQ,              "<=")\
    n(MEQ,              ">=")\
    n(LESS,             "<")\
    n(MORE,             ">")\
    n(NEQ,              "!=")\
    n(ASSIGN,           "равно", "=")\
    n(INIT,             "условимся", "положим", "пускай", "введем обозначение")\
    n(WRITE,            "напишем")\
    n(COLON,            ":")\
    n(COMMA,            ",")\
    n(ARTHM_MUL,        "*"     )\
    n(ARTHM_ADD,        "+"     )\
    n(ARTHM_SUB,        "-"     )\
    n(ARTHM_DIV,        "/"     )\
    n(ARTHM_SIN,        "sin"   )\
    n(ARTHM_COS,        "cos"   )\
    n(ARTHM_TG,         "tg"    )\
    n(ARTHM_CTG,        "ctg"   )\
    n(ARTHM_POW,        "pow"   )\
    n(ARTHM_SQRT,       "sqrt"   )\
    n(ARTHM_LN,         "ln"    )\
    n(ARTHM_LOG,        "log"   )\
    n(ARTHM_SH,         "sh"    )\
    n(ARTHM_CH,         "ch"    )\
    n(ARTHM_CTH,        "cth"   )\
    n(ARTHM_TH,         "th"    )\
    n(ARTHM_ARCSIN,     "arcsin")\
    n(ARTHM_ARCCOS,     "arccos")\
    n(ARTHM_ARCTG,      "arctg" )\
    n(ARTHM_ARCCTG,     "arcctg")
#define LANG_OPER_COMPILE_NEEDED_TYPES(n) \
    n(PROGRAM_START,    "□")\
    n(PROGRAM_END,      "▨")\
    n(FUNC_DECL_NAME,   "Формула")\
    n(NEW_PARAM,        "\0")\
    n(NEW_FUNC,         "\0")\
    n(NEW_OP,           "\0")\
    n(NEW_CMP,          "\0")\
    n(USING_FORMULAS,   "Используемые формулы:")\
    n(PATH_OF_SOLVING,  "Ход решения:")

#define INIT_OPER_STR(name, ...) \
    {__VA_ARGS__,}, 
#define INIT_OPER_STRDUMP(name, str, ...) \
    #name,
#define INIT_OPER_ENUM(name, ...) \
    name,


enum dirType {
    RIGHT_DIRECTION = 0,
    LEFT_DIRECTION  = 1,
    NO_DIRECTION    = 2
};

enum LangOperType_e {
    PLCHLD_OP = -2,
    NOP = -1,
    LANG_OPER_TYPES(INIT_OPER_ENUM)
};

enum LangCNOPType_e {
    NCNOP = -1,
    LANG_OPER_COMPILE_NEEDED_TYPES(INIT_OPER_ENUM)
};

enum LangErr_e {
    OK           = 0,
    ERR_PTR_NULL = 1,
    ERR_CMD_INVALID,
};

enum LangType_e {
    NTYPE           =  -1,
    TYPE_OP         =   0,
    TYPE_VAR        =   1,
    TYPE_NUM        =   2,
    TYPE_CNOP       =   3,
};

union LangElem_u {
    LangOperType_e oper;
    LangCNOPType_e CNop;
    char *var_name;
    int num;
};

typedef struct Node_t {
    LangType_e type;
    LangElem_u value;
    Node_t * left;
    Node_t * right;
} Node_t;

typedef struct LexArr_t {
    Node_t * NodeArr;
    char **VarArr;
    size_t VarArrSize;
    size_t NodeArrSize;
} LexArr_t;

const char * const AllOperStr[][MAX_STR_VAR]        = {LANG_OPER_TYPES(INIT_OPER_STR)};
const char * const AllOperCNOPStr[][MAX_STR_VAR]    = {LANG_OPER_COMPILE_NEEDED_TYPES(INIT_OPER_STR)};
const char * const AllOperCNOPDumpStr[]             = {LANG_OPER_COMPILE_NEEDED_TYPES(INIT_OPER_STRDUMP)};
const char * const AllOperDumpStr[]                 = {LANG_OPER_TYPES(INIT_OPER_STRDUMP)};
const size_t NumOfOpers                             = sizeof(AllOperStr)     / sizeof(AllOperStr[0]);
const size_t NumOfCNOPOpers                         = sizeof(AllOperCNOPStr) / sizeof(AllOperCNOPStr[0]);
#undef INIT_OPER_ENUM
#undef INIT_OPER_STR
#undef LANG_OPER_TYPES

#define CALLOC_WITH_TYPE(num_of_elements, type) \
    (type *) calloc(num_of_elements, sizeof(type))


void ArrayOfLexemsDtor_internal(LexArr_t * LexArr);
void NodeDtor(Node_t **node);

#endif // MYLANG_H