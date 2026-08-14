#ifndef MYLANGVARS_H
#define MYLANGVARS_H

#include <stdlib.h>

typedef const char * const string;

string AllValueTypesTxt[]         = {"OPER", "VAR", "NUM", "CNOP", "STR"};
string dump_site_name             = "dump.html";
string dump_graph_txt_file_name   = "graph/graph.txt";

const size_t START_INIT_SIZE    = 100;
const size_t MAX_STR_VAR        = 7;
const size_t MAX_STR_SIZE       = 300;
const size_t POISON             = 0xDEDDEAF;

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
    n(OPER_RETURN,           ""                 ,   "⇒")                                                                        \
    n(OPER_IN_POINT,         ""                 ,   "в точке")                                                                   \
    n(OPER_WHILE_BODY,       ""                 ,   "↳")                                                                         \
    n(OPER_FINDING,          ""                 ,   "находим")                                                                   \
    n(OPER_COLON,            ""                 ,   ":")                                                                         \
    n(OPER_LINE,             "LINE"             ,   "-")                                                                         \
    n(OPER_QUOTE,            ""                 ,   "\"")                                                                        \
    n(OPER_NEW_NAME,         "NEW_NAME"         ,   "")                                                                          \
    n(OPER_NEW_FUNC,         "NEW_FUNC"         ,   "")                                                                          \
    n(OPER_NEW_INIT,         "NEW_FUNC"         ,   "")                                                                          \
    n(OPER_NEW_OP,           "NEW_OP"           ,   "")                                                                          \
    n(OPER_NEW_PARAM,        "NEW_PARAM"        ,   "")                                                                          \
    n(OPER_IF_BODY,          "IF_BODY"          ,   "")                                                                          \
    n(OPER_FUNC_BODY,        "FUNC_BODY"        ,   "")                                                                          \
    n(OPER_VAR_DECLARATION,  "VAR_DECLARATION"  ,   "условимся", "положим", "пускай", "введем обозначение")                      \
    n(OPER_PROGRAM_START,    "PROGRAM_START"    ,   "□")                                                                         \
    n(OPER_PROGRAM_END,      "PROGRAM_END"      ,   "▨")                                                                        \
    n(OPER_FUNC_DECL_NAME,   "FUNC_NAME"        ,   "формула для")                                                               \
    n(OPER_USING_FORMULAS,   "ALL_FUNCS"        ,   "используемые формулы:")                                                     \
    n(OPER_AI_REFERENCE,     "ENDING_STR"       ,   "AI reference, for generation only")                                         \
    n(OPER_PATH_OF_SOLVING,  "ALL_OPERS"        ,   "ход решения:")                                                              \
    n(OPER_STEP,             "STEP"             ,   "шаг №")                                                                     \
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

#define INIT_OPER_ENUM(name, ...) \
    name,
#define MAKE_OPER_STRUCT(NAME, TreeName, ...) \
    (oper_t) {\
                .Dump = #NAME,\
                .Tree = TreeName,\
                .Text = (string []){__VA_ARGS__}\
            },

typedef struct {
    string Dump;
    string Tree;
    string *Text;
} oper_t;
const oper_t AllOper[] = {
    LANG_OPER_TYPES(MAKE_OPER_STRUCT)
};
const size_t NumOfOpers = sizeof(AllOper) / sizeof(AllOper[0]);

enum LangOperType_e {
    OPER_NOP = -1,
    LANG_OPER_TYPES(INIT_OPER_ENUM)
};

// enum dirType {
//     RIGHT_DIRECTION = 0,
//     LEFT_DIRECTION  = 1,
//     NO_DIRECTION    = 2
// };

enum LangErr_t {
    NOK                         = -1,
    OK                          = 0,
    ERR_STACK_NULL              = 1,
    ERR_CAPACITY_INVALID        = 2,
    ERR_SIZE_INVALID            = 3,
    ERR_DIFFERENT_TYPE          = 4,
    ERR_CANAREIKA_LEFT_CHANGE   = 5,
    ERR_CANAREIKA_RIGHT_CHANGE  = 6,
    ERR_OVERFLOW                = 7,
    ERR_BUFFER_SIZE_INVALID     = 8,
    ERR_HASH_CHANGED            = 9,
    ERR_UNDEFINED_CMD           = 10,
    ERR_FILE_DOES_NOT_EXIST     = 11,
    ERR_FILE_SIZE_INCORRECT     = 12,
    ERR_INCORRECT_SIGN          = 13,
    ERR_INCORRECT_VERSION       = 14,
    FATAL_ERROR                 = 15,
    NO_ERROR                    = 16,
    ERR_INCORRECT_ARGUMENT      = 17,
    ERR_INCORRECT_LABEL         = 18,
    ERR_CONFLICTING_TYPES       = 19,
    ERR_SYSTEM_FAILED           = 20,
    ERR_VIDEO_DIVIDE_FAIL       = 21,
    ERR_INCORRECT_VIDEO_HEADER  = 22,
    ERR_PTR_NULL                = 23,
    ERR_CALLOC_FAIL             = 24
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
    Node_t *left;
    Node_t *right;

    const char *pos;
    int line;
} Node_t;


#define ERROR_BUF_MAX_SIZE 256
#ifndef NDEBUG
    #define ADD_ERR(string, ...) \
        snprintf(lang_last_error, ERROR_BUF_MAX_SIZE - 1, string, ##__VA_ARGS__);
#else
    #define ADD_ERR(string, ...)
#endif


#define RETURN_ERR(condition, code, string, ...) \
    if (!(condition)) {\
        ADD_ERR(string, ##__VA_ARGS__)\
        return code;\
    }

#undef INIT_OPER_ENUM
#undef INIT_OPER_STR
#undef LANG_OPER_TYPES
#endif // MYLANG_H