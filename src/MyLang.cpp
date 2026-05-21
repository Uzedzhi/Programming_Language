#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>

#include "MyLang.h"
#include "MyLangSyntax.h"
#include "MyLangDump.h"
#include "sassert.h"
#include "../stack/stack.h"

void ArrayOfLexemsCtor_internal(LexArr_t * Arr) {
    Arr->NodeArr = CALLOC_WITH_TYPE(START_INIT_SIZE, Node_t);
    sassert(Arr->NodeArr, ERR_PTR_NULL_LANG);

    Arr->Scope = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(Arr->Scope, ERR_PTR_NULL_LANG);

    Arr->NameTable = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(Arr->NameTable, ERR_PTR_NULL_LANG);

    init_stack(stk, START_INIT_SIZE);
    Arr->ScopeBorders = stk;

    Arr->ScopeSize          = 0;
    Arr->ScopeCapacity      = START_INIT_SIZE;
    Arr->NameTableSize      = 0;
    Arr->NameTableCapacity  = START_INIT_SIZE;
    Arr->NodeArrSize        = 0;
    Arr->NodeArrCapacity    = START_INIT_SIZE;
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

#define SmartFree(Ptr) \
    if (Ptr)\
        free(Ptr)


void ArrayOfLexemsDtor_internal(LexArr_t * LexArr) {
    if (LexArr != NULL) {
        for (size_t i = 0; i < LexArr->NameTableSize; i++) {
            SmartFree(LexArr->NameTable[i]);
        }
        SmartFree(LexArr->Scope);
        SmartFree(LexArr->NameTable);
        stackDtor(LexArr->ScopeBorders);

        NodeDtor(&(LexArr->NodeArr));
        SmartFree(LexArr);
    }
}

void print_help() {
    printf("Please specify the file you want to compile\n");
}

size_t GetFileSize(FILE * fp) {
    fseek(fp, 0, SEEK_END);
    size_t FileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return FileSize;
}

LangOperType_e CheckOperType(const char * const FileBuf, int *str_len) {
    sassert(FileBuf, ERR_PTR_NULL_LANG);

    LangOperType_e  PrevFoundOperNum    = OPER_NOP;
    int             PrevFoundStrLen     = 0;
    for (size_t i = 0; i < NumOfOpers; i++) {
        for (size_t j = 0; j < MAX_STR_VAR; j++) {
            if (!AllOperStr[i][j])
                continue;
            int CurStrLen = strlen(AllOperStr[i][j]);

        if (CurStrLen > PrevFoundStrLen && strncmp(FileBuf, AllOperStr[i][j], CurStrLen) == 0) {
            PrevFoundStrLen = CurStrLen;
            PrevFoundOperNum = (LangOperType_e) i;
        }
        }
    }

    *str_len = PrevFoundStrLen;
    return PrevFoundOperNum;
}

void SSkipLine(char *str, size_t *Pos) {
    while (str[*Pos] != '\n' && str[*Pos] != '\0') {
        (*Pos)++;
    }
}

int GetVarIndexInArr(char ** Arr, char * Var, size_t ArrSize) {
    for (size_t i = 0; i < ArrSize; i++) {
        if (strcmp(Arr[i], Var) == 0)
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

void UTF8_tolower(char *buf)
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

void *reallocate_array(void ** array, size_t capacity, size_t new_bytes) {
    sassert(array != NULL, ERR_PTR_NULL_LANG);

    if ((double) MAX_SIZE_T / (double) capacity <= (double) new_bytes / (double) capacity) {
        fprintf(stderr, "buffer overflew, maybe you have too many elements in a stack?");
        exit(ERR_PTR_NULL_LANG);
    }
    
    void * new_array = realloc(*array, new_bytes);
    sassert(new_array != NULL, ERR_PTR_NULL_LANG);

    *array = new_array;
    return *array;
}

char * GetBufferFromFile(FILE *fp, size_t FileSize) {
    sassert(fp, ERR_PTR_NULL_LANG);

    char *FileBuffer = (char *) calloc(FileSize + 1, sizeof(char));
    sassert(FileBuffer, ERR_PTR_NULL_LANG);

    size_t CharsRead = fread(FileBuffer, sizeof(char), FileSize, fp);
    FileBuffer[CharsRead] = '\0';
    return FileBuffer;
}

LangErr_e FillArrayOfLexems(LexArr_t *LexArr, const char * file_name) {
    sassert(LexArr,     ERR_PTR_NULL_LANG);
    sassert(file_name,  ERR_PTR_NULL_LANG);

    FILE * fp = fopen(file_name, "r");
    sassert(fp, ERR_PTR_NULL_LANG, "Не удалось открыть файл %s", file_name);

    size_t FileSize = GetFileSize(fp);

    size_t Pos = 0;
    char * FileBuf = GetBufferFromFile(fp, FileSize);
    fclose(fp);

    char * FileBufCopy = strdup(FileBuf);
    UTF8_tolower(FileBufCopy);

    size_t NextIndex = 0;
    while(FileBuf[Pos] != '\0') {
        if (LexArr->NodeArrSize >= LexArr->NodeArrCapacity - 1) {
            reallocate_array((void**) &(LexArr->NodeArr), LexArr->NodeArrCapacity, LexArr->NodeArrCapacity * 2 * sizeof(Node_t));
            LexArr->NodeArrCapacity *= 2;
        }

        // SKIP SPACE
        while (isspace(FileBuf[Pos]))
            Pos++;

        // COMMENT
        if (FileBuf[Pos] == '|') {
            SSkipLine(FileBuf, &Pos);
            continue;
        }

        // NUM
        char * CurPos = FileBuf + Pos;
        int value = strtol(FileBuf + Pos, &CurPos, 10);
        if (CurPos != FileBuf + Pos) {
            Pos = CurPos - FileBuf;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_NUM, {.num = value}, NULL, NULL};
            continue;
        }

        // OPER
        int str_len = 0;
        LangOperType_e OperType = CheckOperType(FileBufCopy + Pos, &str_len);
        if (OperType != OPER_NOP) {
            Pos += str_len;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_OP, {.oper = OperType}, NULL, NULL};
            continue;
        }

        // VAR
        if (isalnum(FileBuf[Pos]) || IsRussianNext(FileBuf + Pos)) {
            const char * s2 = FileBuf + Pos;
            size_t len = 0;

            int IsRussian = false;
            while (isalnum(FileBuf[Pos]) || (IsRussian = IsRussianNext(FileBuf + Pos))) {
                if (IsRussian)
                    Pos += 2;
                else
                    Pos++;
                len++;
            }
            

            char * Id = CALLOC_WITH_TYPE(len + 1, char);
            strncpy(Id, s2, len);

            size_t VarInd = GetVarIndexInArr(LexArr->NameTable, Id, LexArr->NameTableSize);
            if (len > 1 && VarInd == -1)
                fprintf(stderr, CYAN "WARNING: " WHITE "your variable <%s>\n"
                                     "\t is more than 1 character long, it is inappropiate in math\n", Id);

            if (LexArr->NameTableSize >= LexArr->NameTableCapacity)
                reallocate_array((void **) LexArr->NameTable, LexArr->NameTableCapacity,
                                 LexArr->NameTableCapacity * 2 * sizeof(char *));

            if (VarInd == -1) { 
                LexArr->NameTable[LexArr->NameTableSize] = Id;
                VarInd = LexArr->NameTableSize++;
            } else {
                free(Id);
            }

            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.var_name = LexArr->NameTable[VarInd]}, NULL, NULL};
            continue;
        }

        int len = strcspn(FileBuf, "\n\0");
        fprintf(stderr, RED "Синтаксическая ошибка: " RESET "не смог определить ключевое слово на строке: \n %.*s \n", len, FileBuf + Pos);
        free(FileBuf);
        free(FileBufCopy);
        return NOK;
    }

    free(FileBuf);
    free(FileBufCopy);
    return OK;
}

void PrintTree(Node_t * node) {
    fprintf(stderr, "(");
    if (node->left != NULL)
        PrintTree(node->left);
    switch(node->type) {
        case TYPE_OP:
            fprintf(stderr, "OP %s", AllOperStr[node->value.oper][0]);
            break;
        case TYPE_VAR:
            fprintf(stderr, "VAR %s", node->value.var_name);
            break;
        case TYPE_NUM:
            fprintf(stderr, "NUM %d", node->value.num);
            break;
    }
    if (node->right != NULL)
        PrintTree(node->right);
    fprintf(stderr, ")");
} 

static void PrintTabulations(FILE *fp, size_t n) {
    for (size_t i = 0; i < n; ++i)
        fprintf(fp, "\t");
}

bool PrintNodeSexprToFile(FILE *fp, LexArr_t *LexArr, Node_t *Node, size_t *Tabulation) {
    PrintTabulations(fp, *Tabulation);
    fprintf(fp, "(\n");
    (*Tabulation)++;

    PrintTabulations(fp, *Tabulation);
    size_t VarIndex = -1;
    switch(Node->type) {
        case TYPE_VAR:
        case TYPE_STR:   
            fprintf(fp, "\"%s\"\n", Node->value.var_name);
            break;
        case TYPE_NUM:   fprintf(fp, "\"%d\"\n",  Node->value.num);                                         break;
        case TYPE_OP:    fprintf(fp, "%s\n",   AllOperTreeStr[Node->value.oper]);                      break;
        }

    if (Node->left != NULL)
        PrintNodeSexprToFile(fp, LexArr, Node->left, Tabulation);
    if (Node->right != NULL)
        PrintNodeSexprToFile(fp, LexArr, Node->right, Tabulation);
    
    (*Tabulation)--;
    PrintTabulations(fp, *Tabulation);
    fprintf(fp, ")\n");

    return true;
}

void PrintProgramToFile(const char *FileName, LexArr_t *LexArr) {
    sassert(FileName, ERR_PTR_NULL_LANG);

    FILE *fp = fopen(FileName, "w");
    sassert(fp, ERR_PTR_NULL_LANG, "не удалось открыть %s", FileName);

    size_t Tabulation = 1000-7-993;
    PrintNodeSexprToFile(fp, LexArr, LexArr->NodeArr, &Tabulation);
    fclose(fp);
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        print_help();
        return 0;
    }
    ArrayOfLexemsCtor(LexArr);

    char * file_name = argv[1];
    if (FillArrayOfLexems(LexArr, file_name) == NOK)
        return -1;
    
    for (size_t i = 0; i < LexArr->NodeArrSize; i++) {
        fprintf(stderr, "type: %s ", AllValueTypesTxt[LexArr->NodeArr[i].type]);\
        switch(LexArr->NodeArr[i].type) {\
            case TYPE_NUM:   fprintf(stderr, "num: %d\n",  LexArr->NodeArr[i].value.num);                                  break;\
            case TYPE_VAR:   fprintf(stderr, "var: %s\n",  LexArr->NodeArr[i].value.var_name);                             break;\
            case TYPE_OP:    fprintf(stderr, "op: %s\n",   AllOperDumpStr[LexArr->NodeArr[i].value.oper]);                 break;\
        }
    }

    Node_t *node = MakeTreeFromArrayOfLexems(LexArr);
    
    free(LexArr->NodeArr);
    LexArr->NodeArr = node;

    PrintProgramToFile(argv[2], LexArr);
    ArrayOfLexemsDtor(LexArr);
    return 0;
}