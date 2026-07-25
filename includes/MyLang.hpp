#ifndef MYLANG_H
#define MYLANG_H

#include <stdlib.h>
#include <string.h>

#include "../stack/stack.hpp"

typedef const char * const string_t;

string_t dump_site_name             = "dump.html";
string_t AllValueTypesTxt[]         = {"OPER", "VAR", "NUM", "CNOP", "STR"};
string_t dump_graph_txt_file_name   = "graph/graph.txt";

const size_t START_INIT_SIZE    = 100;
const size_t MAX_STR_VAR        = 7;
const size_t MAX_SIZE_T         = 0xffffffffffull;
const size_t POISON             = 0x2134256754321123;

#define ASM_OPER_TYPES(n) \
    n(ASM_IF,               "IF"    )\
    n(ASM_ELSE,             "\0")\
    n(ASM_THEN,             "\0")\
    n(ASM_PAR_OPEN,         "\0")\
    n(ASM_PAR_CLOSE,        "\0")\
    n(ASM_EQ,               "=="    )\
    n(ASM_LEQ,              "<="    )\
    n(ASM_MEQ,              ">="    )\
    n(ASM_LESS,             "<"     )\
    n(ASM_MORE,             ">"     )\
    n(ASM_NEQ,              "!="    )\
    n(ASM_ASSIGN,           "="     )\
    n(ASM_VAR_DECLARATION,  "VAR_DECLARATION")\
    n(ASM_OUT,              "\0")\
    n(ASM_RETURN,           "\0")\
    n(ASM_IN_POINT,         "\0")\
    n(ASM_WHILE,            "WHILE")\
    n(ASM_FUNC_CALL,        "\0")\
    n(ASM_FINDING,          "\0")\
    n(ASM_COLON,            "\0")\
    n(ASM_COMMA,            "\0")\
    n(ASM_QUOTE,            "\0")\
    n(ASM_ARTHM_MUL,        "*"     )\
    n(ASM_ARTHM_ADD,        "+"     )\
    n(ASM_ARTHM_SUB,        "-"     )\
    n(ASM_ARTHM_DIV,        "/"     )\
    n(ASM_ARTHM_SIN,        "sin"   )\
    n(ASM_ARTHM_COS,        "cos"   )\
    n(ASM_ARTHM_TG,         "tg"    )\
    n(ASM_ARTHM_CTG,        "ctg"   )\
    n(ASM_ARTHM_POW,        "pow"   )\
    n(ASM_ARTHM_SQRT,       "sqrt"  )\
    n(ASM_ARTHM_LN,         "ln"    )\
    n(ASM_ARTHM_LOG,        "log"   )\
    n(ASM_ARTHM_SH,         "sh"    )\
    n(ASM_ARTHM_CH,         "ch"    )\
    n(ASM_ARTHM_CTH,        "cth"   )\
    n(ASM_ARTHM_TH,         "th"    )\
    n(ASM_ARTHM_ARCSIN,     "arcsin")\
    n(ASM_ARTHM_ARCCOS,     "arccos")\
    n(ASM_ARTHM_ARCTG,      "arctg" )\
    n(ASM_ARTHM_ARCCTG,     "arcctg")

#define LANG_OPER_TYPES(n) \
    n(OPER_IF,               "IF"               ,   "если")                                                                      \
    n(OPER_LOGIC_AND,        "LOGIC_AND"        ,   "и", "а также", "ещё", "в добавок к этому", "and", "&&")                     \
    n(OPER_LOGIC_OR,         "LOGIC_OR"         ,   "или", "or", "||")                                                           \
    n(OPER_AND,              "ARTHM_AND"        ,   "&")                                                                         \
    n(OPER_OR,               "ARTHM_OR"         ,   "|")                                                                         \
    n(OPER_XOR,              "XOR"              ,   "исключающее или", "xor", "^")                                               \
    n(OPER_NOT,              "NOT"              ,   "not", "~", "!", "не", "только не")                                          \
    n(OPER_ELSE,             "ELSE"             ,   "в противном случае", "иначе")                                               \
    n(OPER_PAR_OPEN,         "PAR_OPEN"         ,   "(")                                                                         \
    n(OPER_PAR_CLOSE,        "PAR_CLOSE"        ,   ")")                                                                         \
    n(OPER_IN,               "IN"               ,   "пускай читатель сам предложит значение для ", "спросим у зала значение ")   \
    n(OPER_EQ,               "EQ"               ,   "==", "в Uᵦ", "равно")                                                       \
    n(OPER_LEQ,              "LEQ"              ,   "<=", "меньше или равно")                                                    \
    n(OPER_MEQ,              "MEQ"              ,   ">=", "больше или равно")                                                    \
    n(OPER_LESS,             "LESS"             ,   "<",  "меньше")                                                              \
    n(OPER_MORE,             "MORE"             ,   ">",  "больше")                                                              \
    n(OPER_NEQ,              "NEQ"              ,   "≠",  "не равно")                                                            \
    n(OPER_ASSIGN,           "ASSIGN"           ,   "=")                                                                         \
    n(OPER_OUT,              "OUT"              ,   "напишем")                                                                   \
    n(OPER_WHILE,            "WHILE"            ,   "для ∀")                                                                     \
    n(OPER_FUNC_CALL,        "FUNC_CALL"        ,   "из формулы")                                                                \
    n(OPER_THEN,             ""                 ,   "то")                                                                        \
    n(OPER_COMMA,            ""                 ,   ",")                                                                         \
    n(OPER_RETURN,           ""                 ,   "⇒")                                                                         \
    n(OPER_IN_POINT,         ""                 ,   "в точке")                                                                   \
    n(OPER_WHILE_BODY,       ""                 ,   "↳")                                                                         \
    n(OPER_FINDING,          ""                 ,   "находим")                                                                   \
    n(OPER_COLON,            ""                 ,   ":")                                                                         \
    n(OPER_QUOTE,            ""                 ,   "\"")                                                                        \
    n(OPER_NEW_FUNC,         "NEW_FUNC"         ,   "")                                                                          \
    n(OPER_NEW_INIT,         "NEW_FUNC"         ,   "")                                                                          \
    n(OPER_NEW_OP,           "NEW_OP"           ,   "")                                                                          \
    n(OPER_NEW_PARAM,        "NEW_PARAM"        ,   "")                                                                          \
    n(OPER_IF_BODY,          "IF_BODY"          ,   "")                                                                          \
    n(OPER_FUNC_BODY,        "FUNC_BODY"        ,   "")                                                                          \
    n(OPER_VAR_DECLARATION,  "VAR_DECLARATION"  ,   "условимся", "положим", "пускай", "введем обозначение")                      \
    n(OPER_PROGRAM_START,    "PROGRAM_START"    ,   "□")                                                                         \
    n(OPER_PROGRAM_END,      "PROGRAM_END"      ,   "▨")                                                                         \
    n(OPER_FUNC_DECL_NAME,   "FUNC_NAME"        ,   "формула")                                                                   \
    n(OPER_USING_FORMULAS,   "ALL_FUNCS"        ,   "используемые формулы:")                                                     \
    n(OPER_AI_REFERENCE,     "ENDING_STR"       ,   "AI reference, for generation only")                                         \
    n(OPER_PATH_OF_SOLVING,  "ALL_OPERS"        ,   "ход решения:")                                                              \
    n(OPER_ARTHM_MUL,        "*"                ,   "*"     )                                                                    \
    n(OPER_ARTHM_ADD,        "+"                ,   "+"     )                                                                    \
    n(OPER_ARTHM_SUB,        "-"                ,   "-"     )                                                                    \
    n(OPER_ARTHM_DIV,        "/"                ,   "/"     )                                                                    \
    n(OPER_ARTHM_SIN,        "sin"              ,   "sin"   )                                                                    \
    n(OPER_ARTHM_COS,        "cos"              ,   "cos"   )                                                                    \
    n(OPER_ARTHM_TG,         "tg"               ,   "tg"    )                                                                    \
    n(OPER_ARTHM_CTG,        "ctg"              ,   "ctg"   )                                                                    \
    n(OPER_ARTHM_POW,        "pow"              ,   "pow"   )                                                                    \
    n(OPER_ARTHM_SQRT,       "sqrt"             ,   "sqrt"  )                                                                    \
    n(OPER_ARTHM_LN,         "ln"               ,   "ln"    )                                                                    \
    n(OPER_ARTHM_LOG,        "log"              ,   "log"   )                                                                    \
    n(OPER_ARTHM_SH,         "sh"               ,   "sh"    )                                                                    \
    n(OPER_ARTHM_CH,         "ch"               ,   "ch"    )                                                                    \
    n(OPER_ARTHM_CTH,        "cth"              ,   "cth"   )                                                                    \
    n(OPER_ARTHM_TH,         "th"               ,   "th"    )                                                                    \
    n(OPER_ARTHM_ARCSIN,     "arcsin"           ,   "arcsin")                                                                    \
    n(OPER_ARTHM_ARCCOS,     "arccos"           ,   "arccos")                                                                    \
    n(OPER_ARTHM_ARCTG,      "arctg"            ,   "arctg" )                                                                    \
    n(OPER_ARTHM_ARCCTG,     "arcctg"           ,   "arcctg")

#define INIT_OPER_STR(name, TreeOperStr, ...) \
    {__VA_ARGS__,}, 
#define INIT_OPER_OPER_STR(name, TreeOperStr, ...) \
    TreeOperStr,
#define INIT_OPER_DUMP_STR(name, ...) \
    #name,
#define INIT_OPER_ENUM(name, ...) \
    name,

enum dirType {
    RIGHT_DIRECTION = 0,
    LEFT_DIRECTION  = 1,
    NO_DIRECTION    = 2
};

enum LangOperType_e {
    OPER_NOP = -1,
    LANG_OPER_TYPES(INIT_OPER_ENUM)
};

enum LangErr_e {
    OK                      = 0,
    ERR_PTR_NULL_LANG       = 1,
    ERR_CMD_INVALID_LANG    = 2,
    NOK                     = 3
};

enum LangType_e {
    NTYPE           =  -1,
    TYPE_OP         =   0,
    TYPE_VAR        =   1,
    TYPE_NUM        =   2,
    TYPE_STR        =   3
};

union LangElem_u {
    LangOperType_e oper;
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

    size_t NodeArrCapacity;
    size_t NodeArrSize;

    char **Scope;
    size_t ScopeSize;
    size_t ScopeCapacity;

    char **NameTable;
    size_t NameTableSize;
    size_t NameTableCapacity;

    stack_t *ScopeBorders;
} LexArr_t;

const char * const AllOperStr[][MAX_STR_VAR]        = {LANG_OPER_TYPES(INIT_OPER_STR)};
const char * const AllOperTreeStr[]                 = {LANG_OPER_TYPES(INIT_OPER_OPER_STR)};
const char * const AllOperDumpStr[]                 = {LANG_OPER_TYPES(INIT_OPER_DUMP_STR)};
const size_t NumOfOpers                             = sizeof(AllOperTreeStr)     / sizeof(AllOperTreeStr[0]);
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