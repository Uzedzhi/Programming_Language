
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>

#include "../includes/MyLangFront.hpp"
#include "../MyLibs/helper_funcs.hpp"
#include "../MyLibs/sassert.hpp"
#include "../Smart_Stack/stack.hpp"

char lang_last_error[ERROR_BUF_MAX_SIZE] = {};

void lex_perror_file(FILE* out) {
    if (out)
        fprintf(out, RED "[Лексическая ОШИБКА]: " RESET "%s\n", lang_last_error);
}
void lex_perror() {
    fprintf(stderr,  RED "[Лексическая ОШИБКА]: " RESET "%s\n", lang_last_error);
}

LangErr_t ArrayOfLexemsCtor(LexArr_t * Arr) {
    Arr->NodeArr = CALLOC_WITH_TYPE(START_INIT_SIZE, Debug_Node_t);
    RETURN_ERR(Arr->NodeArr,    ERR_CALLOC_FAIL, "не удалось создать массив звеньев размером %zu", START_INIT_SIZE);

    Arr->Scope = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    RETURN_ERR(Arr->Scope,      ERR_CALLOC_FAIL, "не удалось создать массив звеньев размером %zu", START_INIT_SIZE);

    Arr->ScopeSize          = 0;
    Arr->ScopeCapacity      = START_INIT_SIZE;

    Arr->NodeArrSize        = 0;
    Arr->NodeArrCapacity    = START_INIT_SIZE;

    Arr->ScopeBorders = StackCtor();
    #ifdef DEBUG
        STARTTXTDUMPS();
    #endif

    return OK;
}

void NodeDtor(Debug_Node_t **node) {
    sassert(node, ERR_PTR_NULL);

    if (*node != NULL && (*node)->left != NULL)
        NodeDtor(&((*node)->left));
    if (*node != NULL && (*node)->right != NULL)
        NodeDtor(&((*node)->right));
    SMART_FREE(*node);
    *node = NULL;
}


void ArrayOfLexemsDtor(LexArr_t * LexArr) {
    if (LexArr != NULL) {
        for (size_t i = 0; i < LexArr->ScopeSize; i++) {
            SMART_FREE(LexArr->Scope[i]);
        }
        SMART_FREE(LexArr->Scope);
        stackDtor(LexArr->ScopeBorders);
        NodeDtor(&(LexArr->NodeArr));
        
        SMART_FREE(LexArr);
    }

    #ifdef DEBUG
        FINISHTXTDUMPS();
    #endif
}

LangOperType_e CheckOperType(const char * const FileBuf, int *str_len) {
    sassert(FileBuf, ERR_PTR_NULL);

    LangOperType_e  PrevFoundOperNum    = OPER_NOP;
    int             PrevFoundStrLen     = 0;
    for (size_t i = 0; i < NumOfOpers; i++) {
        for (size_t j = 0; j < MAX_STR_VAR; j++) {
            string CurOperStr = AllOper[i].Text[j];
            if (!CurOperStr)
                continue;

            int CurStrLen = strlen(CurOperStr);
            if (CurStrLen > PrevFoundStrLen && strncmp(FileBuf, CurOperStr, CurStrLen) == 0) {
                PrevFoundStrLen = CurStrLen;
                PrevFoundOperNum = (LangOperType_e) i;
            }
        }
    }

    *str_len = PrevFoundStrLen;
    return PrevFoundOperNum;
}

void SSkipLine(const char *str, size_t *Pos) {
    while (str[*Pos] != '\n' && str[*Pos] != '\0') {
        (*Pos)++;
    }
}

int GetVarIndexInArr(scope_el_t *Arr, const char * Var, size_t ArrSize) {
    for (size_t i = 0; i < ArrSize; i++) {
        if (strcmp(Arr[i].VarName, Var) == 0)
            return i;
    }
    return -1;
}

bool IsEngAlph(char ch) {
    if (('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z'))
        return true;
    return false;
}

int IsRussianNext(const char *buf) {
    unsigned char b0 = (unsigned char) buf[0];
    unsigned char b1 = (unsigned char) buf[1];

    if (b0 == 0xD0) {
        if (b1 == 0x81) return 1;                 // Ё
        if (b1 >= 0x90 && b1 <= 0xBF) return 1;   // А-Я
        return 0;
    }

    if (b0 == 0xD1) {
        if (b1 >= 0x80 && b1 <= 0xAF) return 1;   // р-я
        if (b1 == 0x91) return 1;                 // ё
        return 0;
    }

    return 0;
}

int UTF8_tolower_char(unsigned char *ch) {

    // А-П (0xD090 - 0xD09F) -> а-п (0xD0B0 - 0xD0BF)
    if (ch[0] == 0xD0 && ch[1] >= 0x90 && ch[1] <= 0x9F) {
        ch[1] += 0x20;
        return 2;
    }
    
    // Р-Я (0xD0A0 - 0xD0AF) -> р-я (0xD180 - 0xD18F)
    if (ch[0] == 0xD0 && ch[1] >= 0xA0 && ch[1] <= 0xAF) {
        ch[0]  = 0xD1;
        ch[1] -= 0x20;
        return 2;
    }
    
    // 'Ё' (0xD081) -> 'ё' (0xD191)
    if (ch[0] == 0xD0 && ch[1] == 0x81) {
        ch[0] = 0xD1;
        ch[1] = 0x91;
        return 2;
    }

    return 1;
}

void UTF8_tolower(const char *buf)
{
    if (!buf) return;

    size_t i = 0;
    while (buf[i] != '\0') {
        if (buf[i + 1] != '\0')
            i += UTF8_tolower_char((unsigned char *) buf + i);
        else
            break;
    }
}

LangErr_t FillArrayOfLexems(LexArr_t *LexArr, const char *file_name) {
    sassert(LexArr,     ERR_PTR_NULL);
    sassert(file_name,  ERR_PTR_NULL);

    FILE *fp = fopen(file_name, "r");
    RETURN_ERR(fp, ERR_FILE_DOES_NOT_EXIST, "Не удалось открыть файл <%s>", file_name);

    size_t FileSize  = GetFileSize(fp);
    RETURN_ERR(FileSize != -1, ERR_FILE_SIZE_INCORRECT, "Не удалось считать размер файла <%s>: может быть он удалился во время чтения?", file_name);

    LexArr->FileBuf = GetBufferFromFile(fp, FileSize);
    RETURN_ERR(LexArr->FileBuf, ERR_PTR_NULL, "Не удалось прочитать содержимое файла <%s>: может он слишком большой?", file_name);
    fclose(fp);

    const char *FileBuf = LexArr->FileBuf;
    size_t Pos  = 0;
    int    line = 0;
    UTF8_tolower(FileBuf);

    size_t NextIndex = 0;
    while(FileBuf[Pos] != '\0') {
        if (LexArr->NodeArrSize >= LexArr->NodeArrCapacity - 1) {
            LexArr->NodeArrCapacity *= 2;
            reallocate_array((void**) &(LexArr->NodeArr), LexArr->NodeArrCapacity, LexArr->NodeArrCapacity * sizeof(Node_t));
        }

        if (LexArr->ScopeSize >= LexArr->ScopeCapacity - 1) {
            LexArr->ScopeCapacity *= 2;
            reallocate_array((void**) &(LexArr->Scope), LexArr->ScopeCapacity, LexArr->ScopeCapacity * sizeof(Node_t));
        }

        // SKIP SPACE
        while (isspace(FileBuf[Pos])) {
            Pos++;
            if (FileBuf[Pos] == '\n')
                line++;
        }

        // COMMENT
        if (FileBuf[Pos] == '|') {
            SSkipLine(FileBuf, &Pos);
            continue;
        }

        // NUM
        char *CurPos = (char *) FileBuf + Pos;
        int value = strtol(FileBuf + Pos, &CurPos, 10);
        if (CurPos != FileBuf + Pos) {
            Pos = CurPos - FileBuf;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_NUM, {.num = value}, NULL, NULL, Pos, line};
            continue;
        }

        // OPER
        int str_len = 0;
        LangOperType_e OperType = CheckOperType(FileBuf + Pos, &str_len);
        if (OperType != OPER_NOP) {
            Pos += str_len;

            if (OperType == OPER_AI_REFERENCE)
                LexArr->Excepts_ptr = LexArr->NodeArrSize;

            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_OP, {.oper = OperType}, NULL, NULL, Pos, line};
            continue;
        }

        // VAR
        if ((FileBuf[Pos]) || IsRussianNext(FileBuf + Pos)) {
            const char * s2 = FileBuf + Pos;
            size_t len = 0;

            int IsRussian = false;

            while (isalnum(FileBuf[Pos]) || (IsRussian = IsRussianNext(FileBuf + Pos))) {
                if (IsRussian)
                    Pos += 2; // russian character is 2 bytes
                else
                    Pos++;
                len++;
            }

            char * Id = CALLOC_WITH_TYPE(len + 1, char);
            strncpy(Id, s2, len);

            int VarInd = GetVarIndexInArr(LexArr->Scope, Id, LexArr->ScopeSize);
            
            if (LexArr->ScopeSize >= LexArr->ScopeCapacity)
                reallocate_array((void **) LexArr->Scope, LexArr->ScopeCapacity,
                                 LexArr->ScopeCapacity * 2 * sizeof(char *));

            if (VarInd == -1) { 
                if (len > 1)
                    fprintf(stderr, CYAN "WARNING: " WHITE "your variable <%s>\n"
                                         "\t is more than 1 character long, it is inappropiate in math\n", Id);
                
                LexArr->Scope[LexArr->ScopeSize] = Id;
                VarInd = LexArr->ScopeSize++;
            } else {
                free(Id);
            }


            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.str = LexArr->Scope[VarInd]}, NULL, NULL, Pos, line};
            continue;
        }

        int len = strcspn(FileBuf, "\n\0");
        fprintf(stderr, RED "Синтаксическая ошибка: " RESET "не смог определить ключевое слово на строке: \n %.*s \n", len, FileBuf + Pos);
        
        return ERR_INCORRECT_LABEL;
    }

    LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_OP, {.oper = OPER_NULL}, NULL, NULL};

    return OK;
}

static void PrintTabulations(FILE *fp, size_t n) {
    for (size_t i = 0; i < n; ++i)
        fprintf(fp, "\t");
}

LangErr_t PrintNodeSexprToFile(FILE *fp, Node_t *Node) {
    static int Tabulation = 0;

    PrintTabulations(fp, Tabulation);
    fprintf(fp, "(\n");
    Tabulation++;

    PrintTabulations(fp, Tabulation);
    size_t VarIndex = -1;
    switch(Node->type) {
        case TYPE_VAR:
        case TYPE_STR:   
            fprintf(fp, "\"%s\"\n", Node->value.str);
            break;
        case TYPE_NUM:   fprintf(fp, "\"%d\"\n",  Node->value.num);             break;
        case TYPE_OP:    fprintf(fp, "%s\n",   AllOper[Node->value.oper].Tree); break;
        default:
            RETURN_ERR(0, NOK, "HOW TF");
        }

    if (Node->left != NULL)
        PrintNodeSexprToFile(fp, Node->left);
    if (Node->right != NULL)
        PrintNodeSexprToFile(fp, Node->right);
    
    Tabulation--;
    PrintTabulations(fp, Tabulation);
    fprintf(fp, ")\n");

    return OK;
}

LangErr_t PrintProgramToFile(const char *FileName, Node_t *NodeTree) {
    sassert(FileName, ERR_PTR_NULL);

    FILE *fp = fopen(FileName, "w");
    RETURN_ERR(fp, ERR_FILE_DOES_NOT_EXIST, "не удалось открыть файл <%s>", FileName);

    LangErr_t result = PrintNodeSexprToFile(fp, NodeTree);
    if (result != OK)
        return result;
    fclose(fp);

    return OK;
}

void print_help(char *argv[]) {
    printf("USAGE: %s <input_file> <output_tree_file>\n", argv[0]);
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        print_help(argv);
        return OK;
    }

    LexArr_t LexArr = {};
    if (ArrayOfLexemsCtor(&LexArr) != OK) {
        lex_perror();
        return NOK;
    }

    if (FillArrayOfLexems(&LexArr, argv[1]) != OK) {
        lex_perror();
        return NOK;
    }
    
    // for (size_t i = 0; i < LexArr->NodeArrSize; i++) {
    //     fprintf(stderr, "type: %s ", AllValueTypesTxt[LexArr->NodeArr[i].type]);\
    //     switch(LexArr->NodeArr[i].type) {\
    //         case TYPE_NUM:   fprintf(stderr, "num: %d\n",  LexArr->NodeArr[i].value.num);                                  break;\
    //         case TYPE_VAR:   fprintf(stderr, "var: %s\n",  LexArr->NodeArr[i].value.str);                             break;\
    //         case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[LexArr->NodeArr[i].value.oper]);                 break;\
    //     }
    // }

    Node_t *NodeTree = MakeTreeFromArrayOfLexems(&LexArr);
    if (!NodeTree) {
        lex_perror();
        return NOK;
    }
    PrintProgramToFile(argv[2], NodeTree);

    ArrayOfLexemsDtor(&LexArr);
    free(NodeTree);
    return OK;
}