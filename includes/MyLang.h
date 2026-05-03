#ifndef MYLANG_H
#define MYLANG_H

#include <stdlib.h>
#include <string.h>

typedef const char * const string_t;

string_t dump_site_name             = "dump.html";
string_t AllValueTypesTxt[]         = {"OPER", "VAR", "NUM", "CNOP", "STR"};
string_t dump_graph_txt_file_name   = "graph/graph.txt";

const size_t MAX_STR_SIZE       = 500;
const size_t START_INIT_SIZE    = 100;
const size_t MAX_STR_VAR        = 5;
const size_t MAX_SIZE_T         = 0xffffffffffull;

#define ASM_OPER_TYPES(n) \
    n(IF,               "IF"    )\
    n(ELSE,             "\0")\
    n(THEN,             "\0")\
    n(PAR_OPEN,         "\0")\
    n(PAR_CLOSE,        "\0")\
    n(EQ,               "=="    )\
    n(LEQ,              "<="    )\
    n(MEQ,              ">="    )\
    n(LESS,             "<"     )\
    n(MORE,             ">"     )\
    n(NEQ,              "!="    )\
    n(ASSIGN,           "="     )\
    n(VAR_DECLARATION,  "VAR_DECLARATION")\
    n(OUT,              "\0")\
    n(RETURN,           "\0")\
    n(IN_POINT,         "\0")\
    n(WHILE,            "WHILE")\
    n(FUNC_CALL,        "\0")\
    n(FINDING,          "\0")\
    n(COLON,            "\0")\
    n(COMMA,            "\0")\
    n(QUOTE,            "\0")\
    n(ARTHM_MUL,        "*"     )\
    n(ARTHM_ADD,        "+"     )\
    n(ARTHM_SUB,        "-"     )\
    n(ARTHM_DIV,        "/"     )\
    n(ARTHM_SIN,        "sin"   )\
    n(ARTHM_COS,        "cos"   )\
    n(ARTHM_TG,         "tg"    )\
    n(ARTHM_CTG,        "ctg"   )\
    n(ARTHM_POW,        "pow"   )\
    n(ARTHM_SQRT,       "sqrt"  )\
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
#define LANG_OPER_IN_FILE_TYPES(n) \
    n(IF,               "IF")\
    n(ELSE,             "\0")\
    n(THEN,             "\0")\
    n(PAR_OPEN,         "(")\
    n(PAR_CLOSE,        ")")\
    n(IN,               "IN")\
    n(EQ,               "==")\
    n(LEQ,              "<=")\
    n(MEQ,              ">=")\
    n(LESS,             "<")\
    n(MORE,             ">")\
    n(NEQ,              "!=")\
    n(ASSIGN,           "=")\
    n(VAR_DECLARATION,  "VAR_DECLARATION")\
    n(OUT,              "OUT")\
    n(RETURN,           "RETURN")\
    n(IN_POINT,         "\0")\
    n(WHILE,            "WHILE")\
    n(FUNC_CALL,        "FUNC_CALL")\
    n(FINDING,          "\0")\
    n(COLON,            ":")\
    n(COMMA,            ",")\
    n(QUOTE,            "\"")\
    n(ARTHM_MUL,        "*"     )\
    n(ARTHM_ADD,        "+"     )\
    n(ARTHM_SUB,        "-"     )\
    n(ARTHM_DIV,        "/"     )\
    n(ARTHM_SIN,        "sin"   )\
    n(ARTHM_COS,        "cos"   )\
    n(ARTHM_TG,         "tg"    )\
    n(ARTHM_CTG,        "ctg"   )\
    n(ARTHM_POW,        "pow"   )\
    n(ARTHM_SQRT,       "sqrt"  )\
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
#define LANG_OPER_TYPES(n) \
    n(IF,               "если")\
    n(ELSE,             "в противном случае", "иначе")\
    n(THEN,             "то")\
    n(PAR_OPEN,         "(")\
    n(PAR_CLOSE,        ")")\
    n(IN,               "пускай читатель сам предложит значение для ", "спросим у зала значение ")\
    n(EQ,               "==", "в Uᵦ", "равно")\
    n(LEQ,              "<=", "меньше или равно")\
    n(MEQ,              ">=", "больше или равно")\
    n(LESS,             "<",  "меньше")\
    n(MORE,             ">",  "больше")\
    n(NEQ,              "≠",  "не равно")\
    n(ASSIGN,           "=")\
    n(VAR_DECLARATION,  "условимся", "положим", "пускай", "введем обозначение")\
    n(OUT,              "напишем")\
    n(RETURN,           "⇒")\
    n(IN_POINT,         "в точке")\
    n(WHILE,            "для ∀", "↳")\
    n(FUNC_CALL,         "из формулы")\
    n(FINDING,          "находим")\
    n(COLON,            ":")\
    n(COMMA,            ",")\
    n(QUOTE,            "\"")\
    n(ARTHM_MUL,        "*"     )\
    n(ARTHM_ADD,        "+"     )\
    n(ARTHM_SUB,        "-"     )\
    n(ARTHM_DIV,        "/"     )\
    n(ARTHM_SIN,        "sin"   )\
    n(ARTHM_COS,        "cos"   )\
    n(ARTHM_TG,         "tg"    )\
    n(ARTHM_CTG,        "ctg"   )\
    n(ARTHM_POW,        "pow"   )\
    n(ARTHM_SQRT,       "sqrt"  )\
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
    n(FUNC_DECL_NAME,   "формула")\
    n(NEW_PARAM,        "\0")\
    n(IF_ELSE,          "\0")\
    n(FUNC,             "\0")\
    n(NEW_OP,           "\0")\
    n(NEW_CMP,          "\0")\
    n(NEW_INIT,         "\0")\
    n(FUNC_BODY,        "\0")\
    n(USING_FORMULAS,   "используемые формулы:")\
    n(AI_REFERENCE,     "AI reference, for generation only")\
    n(PATH_OF_SOLVING,  "ход решения:")

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
    TYPE_STR        =   4
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
    int ScopeDeep;
} Node_t;

typedef struct LexArr_t {
    Node_t * NodeArr;

    char **NameTable;
    size_t NameTableSize;

    size_t NodeArrCapacity;
    size_t NodeArrSize;

    int CurScopeDeep;

    char **NameTableLocal;
    size_t NameTableLocalSize;
    size_t NameTableLocalCapacity;

    size_t *NameTableLocalScope;

    size_t *ScopeStack;
    size_t ScopeStackSize;
    size_t ScopeStackCapacity;
} LexArr_t;

const char * const AllAsmOperStr[]                  = {ASM_OPER_TYPES(INIT_OPER_STR)};
const char * const AllOperStr[][MAX_STR_VAR]        = {LANG_OPER_TYPES(INIT_OPER_STR)};
const char * const AllOperCNOPStr[][MAX_STR_VAR]    = {LANG_OPER_COMPILE_NEEDED_TYPES(INIT_OPER_STR)};
const char * const AllOperCNOPDumpStr[]             = {LANG_OPER_COMPILE_NEEDED_TYPES(INIT_OPER_STRDUMP)};
const char * const AllOperDumpStr[]                 = {LANG_OPER_TYPES(INIT_OPER_STRDUMP)};
const char * const AllOperInFileStr[][MAX_STR_VAR]  = {LANG_OPER_IN_FILE_TYPES(INIT_OPER_STR)};
const size_t NumOfOpers                             = sizeof(AllOperStr)     / sizeof(AllOperStr[0]);
const size_t NumOfCNOPOpers                         = sizeof(AllOperCNOPStr) / sizeof(AllOperCNOPStr[0]);
const size_t NumOpers = sizeof(AllAsmOperStr) / sizeof(*AllAsmOperStr);
#undef INIT_OPER_ENUM
#undef INIT_OPER_STR
#undef LANG_OPER_TYPES

#define ArrayOfLexemsCtor(ArrName) \
    LexArr_t * ArrName = CALLOC_WITH_TYPE(1, LexArr_t);\
    sassert(ArrName, ERR_PTR_NULL);\
    ArrayOfLexemsCtor_internal(ArrName);\
    STARTTXTDUMPS()\

#define ArrayOfLexemsDtor(ArrName) \
    ArrayOfLexemsDtor_internal(ArrName);\
    FINISHTXTDUMPS()\

#define CALLOC_WITH_TYPE(num_of_elements, type) \
    (type *) calloc(num_of_elements, sizeof(type))


void ArrayOfLexemsDtor_internal(LexArr_t * LexArr);
void PrintProgramToFile(const char *FileName, LexArr_t *LexArr);
LangErr_e FillArrayOfLexems(LexArr_t *LexArr, const char * file_name);
void ArrayOfLexemsCtor_internal(LexArr_t * Arr);
void NodeDtor(Node_t **node);
void *reallocate_array(void ** array, size_t capacity, size_t new_bytes);
int GetVarIndexInArr(char ** Arr, char * Var, size_t ArrSize);

#endif // MYLANG_H