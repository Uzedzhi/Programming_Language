#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "MyLang.h"
#include "MyLangSyntax.h"
#include "MyLangDump.h"
#include "sassert.h"

void ArrayOfLexemsCtor_internal(LexArr_t * Arr) {
    Arr->NodeArr = CALLOC_WITH_TYPE(START_INIT_SIZE, Node_t);
    sassert(Arr->NodeArr, ERR_PTR_NULL);

    Arr->NameTable = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(Arr->NameTable, ERR_PTR_NULL);

    Arr->NameTableLocalScope = CALLOC_WITH_TYPE(START_INIT_SIZE, size_t);
    sassert(Arr->NameTableLocalScope, ERR_PTR_NULL);

    Arr->ScopeStack = CALLOC_WITH_TYPE(START_INIT_SIZE, size_t);
    sassert(Arr->ScopeStack, ERR_PTR_NULL);

    Arr->NameTableLocal = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(Arr->NameTableLocal, ERR_PTR_NULL);

    Arr->NodeArrSize        = 0;
    Arr->NodeArrCapacity    = START_INIT_SIZE;
    Arr->NameTableSize      = 0;
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

void ArrayOfLexemsDtor_internal(LexArr_t * LexArr) {
    if (LexArr != NULL) {
        for (size_t i = 0; i < LexArr->NameTableSize; ++i)
            free(LexArr->NameTable[i]);
        free(LexArr->NameTable);
        free(LexArr->NameTableLocal);
        free(LexArr);
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
    sassert(FileBuf, ERR_PTR_NULL);

    for (size_t i = 0; i < NumOfOpers; i++) {
        for (size_t j = 0; j < MAX_STR_VAR; j++) {
            if (!AllOperStr[i][j])
                continue;
            int CurStrLen = strlen(AllOperStr[i][j]);
            if (strncmp(FileBuf, AllOperStr[i][j], CurStrLen) == 0 && CurStrLen != 0) {
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
        for (size_t j = 0; j < MAX_STR_VAR; j++) {
            if (!AllOperCNOPStr[i][j])
                continue;
            int CurStrLen = strlen(AllOperCNOPStr[i][j]);
            if (strncmp(FileBuf, AllOperCNOPStr[i][j], CurStrLen) == 0 && CurStrLen != 0) {
                *str_len = CurStrLen;
                return (LangCNOPType_e) i;
            }
        }
    }

    return NCNOP;
}

void SSkipLine(char **str) {
    while (**str != '\n' && **str != '\0') {
        (*str)++;
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
    unsigned char b0 = (unsigned char)buf[0];
    unsigned char b1 = (unsigned char)buf[1];

    if (b0 == 0xD0) {
        if (b1 == 0x81) return 1;                 // Ё
        if (b1 >= 0x90 && b1 <= 0xAF) return 1;   // А-Я
        if (b1 >= 0xB0 && b1 <= 0xBF) return 1;   // а-п
        return 0;
    }

    if (b0 == 0xD1) {
        if (b1 >= 0x80 && b1 <= 0x8F) return 1;   // р-я
        if (b1 == 0x91) return 1;                 // ё
        return 0;
    }

    return 0;
}

void UTF8_tolower(char *buf)
{
    if (!buf) return;

    size_t i = 0;
    while (buf[i] != '\0') {
        unsigned char b0 = (unsigned char) buf[i];

        if (b0 < 0x80) {
            i++; 
            continue;
        }

        unsigned char b1 = (unsigned char) buf[i + 1];
        if (b1 == '\0')
            break;

        if (b0 == 0xD0 && b1 == 0x81) {
            buf[i]     = (char) 0xD1;
            buf[i + 1] = (char) 0x91;
            i += 2;
            continue;
        }

        if (b0 == 0xD0 && b1 >= 0x90 && b1 <= 0xAF) {
            if (b1 <= 0x9F) {
                buf[i + 1] = (char) (b1 + 0x20);
            } else {
                buf[i]     = (char) 0xD1;
                buf[i + 1] = (char) (b1 - 0x20);
            }
            i += 2;
            continue;
        }
        i += 2;
    }
}

void *reallocate_array(void ** array, size_t capacity, size_t new_bytes) {
    sassert(array != NULL, ERR_PTR_NULL);

    if ((double) MAX_SIZE_T / (double) capacity <= (double) new_bytes / (double) capacity)
        sassert(0, ERR_PTR_NULL, "buffer overflew, maybe you have too many elements in a stack?");
    
    void * new_array = realloc(*array, new_bytes);
    sassert(new_array != NULL, ERR_PTR_NULL);

    *array = new_array;
    return *array;
}

char * GetBufferFromFile(FILE *fp, size_t FileSize) {
    sassert(fp, ERR_PTR_NULL);

    char *FileBuffer = (char *) calloc(FileSize + 1, sizeof(char));
    sassert(FileBuffer, ERR_PTR_NULL);

    size_t CharsRead = fread(FileBuffer, sizeof(char), FileSize, fp);
    FileBuffer[CharsRead] = '\0';
    return FileBuffer;
}

LangErr_e FillArrayOfLexems(LexArr_t *LexArr, const char * file_name) {
    sassert(LexArr,     ERR_PTR_NULL);
    sassert(file_name,  ERR_PTR_NULL);

    FILE * fp = fopen(file_name, "r");
    sassert(fp, ERR_PTR_NULL, "Не удалось открыть файл %s", file_name);

    size_t FileSize = GetFileSize(fp);

    char * FileBuf = GetBufferFromFile(fp, FileSize);
    char * StartOfFileBuf = FileBuf;
    fclose(fp);
    UTF8_tolower(FileBuf);

    size_t NextIndex = 0;
    while(*FileBuf != '\0') {
        if (LexArr->NodeArrSize >= LexArr->NodeArrCapacity - 1) {
            reallocate_array((void**) &(LexArr->NodeArr), LexArr->NodeArrCapacity, LexArr->NodeArrCapacity * 2 * sizeof(Node_t));
            LexArr->NodeArrCapacity *= 2;
        }

        if (*FileBuf == '|') {
            SSkipLine(&FileBuf);
            continue;
        }

        bool is_negative_digit = false;
        if (*FileBuf == '-') {
            if (isdigit(FileBuf[1])) {
                is_negative_digit = true;
                FileBuf++;
            }
        }
        
        // OPER
        int str_len = 0;
        LangOperType_e OperType     = CheckOperType(FileBuf, &str_len);
        LangCNOPType_e OperCNOPType = CheckOperCNOPType(FileBuf, &str_len);
        if (OperCNOPType != NCNOP) {
            FileBuf += str_len;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_CNOP, {.CNop = OperCNOPType}, NULL, NULL};
            continue;
        }
        else if (OperType != NOP) {
            FileBuf += str_len;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_OP, {.oper = OperType}, NULL, NULL};
            continue;
        }

        // NUM
        if (isdigit(*FileBuf)) {
            int val = 0;
            while (isdigit(*FileBuf)) {
                val = val * 10 + *FileBuf - '0';
                FileBuf++;
            }
            if (is_negative_digit == true)
                val = -val;
            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_NUM, {.num = val}, NULL, NULL};
            continue;
        }

        // if (IsRussianNext(FileBuf)) {
        //     while (IsRussianNext(FileBuf) || *FileBuf == ' ') {
        //         if (*FileBuf == ' ')
        //             FileBuf++;
        //         else
        //             FileBuf += 2;
        //     }
        //     continue;
        // }

        // VAR
        if (isalnum(*FileBuf)) {
            const char * s2 = FileBuf;
            size_t len = 0;  
            
            while (isalnum(*FileBuf)) {
                FileBuf++;
                len++;
            }

            char * Id = CALLOC_WITH_TYPE(len + 1, char);
            strncpy(Id, s2, len);
            if (len != 1) {
                fprintf(stderr, CYAN "WARNING: " WHITE "your variable <%s>\n"
                                     "\t is more than 1 character long, it is inappropiate in math\n", Id);
            }

            int IndexOfVar = GetVarIndexInArr(LexArr->NameTable, Id, LexArr->NameTableSize);
            if (IndexOfVar == -1) {
                LexArr->NameTable[(LexArr->NameTableSize)++] = Id;
                LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.var_name = Id}};
            }
            else {
                LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.var_name = LexArr->NameTable[IndexOfVar]}, NULL, NULL};
                free(Id);
            }
            continue;
        }

        // SKIP SPACE
        if (isspace(*FileBuf)) {
            while (isspace(*FileBuf))
                FileBuf++;
            continue;
        }

        sassert(0, ERR_PTR_NULL, "syntax error at %s", FileBuf);
    }

    free(StartOfFileBuf);
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
        case TYPE_CNOP:
            fprintf(stderr, "CNOP %s", AllOperCNOPStr[node->value.CNop][0]);
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
            VarIndex = GetVarIndexInArr(LexArr->NameTable, Node->value.var_name, LexArr->NameTableSize);
            fprintf(fp, "\"%s\" %zu %d\n", Node->value.var_name, VarIndex, Node->ScopeDeep);
            break;
        case TYPE_NUM:   fprintf(fp, "\"%d\"\n",  Node->value.num);                                         break;
        case TYPE_OP:    fprintf(fp, "%s\n",   AllOperInFileStr[Node->value.oper][0]);                      break;
        case TYPE_CNOP:  fprintf(fp, "%s\n", AllOperCNOPDumpStr[Node->value.CNop]);                         break;
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
    sassert(FileName, ERR_PTR_NULL);

    FILE *fp = fopen(FileName, "w");
    sassert(fp, ERR_PTR_NULL, "не удалось открыть %s", FileName);

    size_t Tabulation = 1000-7-993;
    fprintf(fp, "%zu\n", LexArr->NameTableSize);
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
    FillArrayOfLexems(LexArr, file_name);

    Node_t *node = MakeTreeFromArrayOfLexems(LexArr);
    NodeDtor(&(LexArr->NodeArr));

    LexArr->NodeArr = node;
    PrintProgramToFile(argv[2], LexArr);
    NodeDtor(&(LexArr->NodeArr));
    ArrayOfLexemsDtor(LexArr);
    return 0;
}