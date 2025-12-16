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

    Arr->VarArr  = CALLOC_WITH_TYPE(START_INIT_SIZE, char *);
    sassert(Arr->VarArr, ERR_PTR_NULL);

    Arr->NodeArrSize = 0;
    Arr->VarArrSize  = 0;
}

// todo add realloc

#define ArrayOfLexemsCtor(ArrName) \
    LexArr_t * ArrName = CALLOC_WITH_TYPE(1, LexArr_t);\
    sassert(ArrName, ERR_PTR_NULL);\
    ArrayOfLexemsCtor_internal(ArrName);\
    STARTTXTDUMPS()\

#define ArrayOfLexemsDtor(ArrName) \
    ArrayOfLexemsDtor_internal(ArrName);\
    FINISHTXTDUMPS()\

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
        if (LexArr->NodeArr != NULL) {
            NodeDtor(&(LexArr->NodeArr));
        }
        for (size_t i = 0; i < LexArr->VarArrSize; ++i)
            free(LexArr->VarArr[i]);
        free(LexArr->VarArr);
        free(LexArr);
    }
}

void print_help() {
    printf("Please specify the file you want to compile");
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

int GetVarIndexInArr(char * Arr[], char * Var, size_t ArrSize) {
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

int IsRussianNext(const char *buf)
{

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



LangErr_e FillArrayOfLexems(LexArr_t *LexArr, const char * file_name) {
    sassert(LexArr,     ERR_PTR_NULL);
    sassert(file_name,  ERR_PTR_NULL);

    FILE * fp = fopen(file_name, "r");
    sassert(fp, ERR_PTR_NULL);

    size_t FileSize = GetFileSize(fp);
    char * FileBuf = CALLOC_WITH_TYPE(FileSize + 1, char);
    char * StartOfFileBuf = FileBuf;
    fread(FileBuf, sizeof(char), FileSize, fp);
    fclose(fp);

    size_t NextIndex = 0;
    while(*FileBuf != '\0') {
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

            int IndexOfVar = GetVarIndexInArr(LexArr->VarArr, Id, LexArr->VarArrSize);
            if (IndexOfVar == -1) {
                LexArr->VarArr[(LexArr->VarArrSize)++] = Id;
                LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.var_name = Id}};
            }
            else {
                LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.var_name = LexArr->VarArr[IndexOfVar]}, NULL, NULL};
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
            fprintf(stderr, "OP %s", AllOperStr[node->value.oper]);
            break;
        case TYPE_CNOP:
            fprintf(stderr, "CNOP %s", AllOperCNOPStr[node->value.CNop]);
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

int main(int argc, char * argv[]) {
    if (argc != 2) {
        print_help();
        return 0;
    }
    ArrayOfLexemsCtor(LexArr);

    char * file_name = argv[1];
    FillArrayOfLexems(LexArr, file_name);

    for (size_t i = 0; i < LexArr->NodeArrSize; i++) {
        fprintf(stderr, "type_id: %s\n", AllValueTypesTxt[LexArr->NodeArr[i].type]);
        if ((LexArr->NodeArr[i]).type == TYPE_NUM)
            fprintf(stderr, "value: %d | ", LexArr->NodeArr[i].value.num);
        else if (LexArr->NodeArr[i].type == TYPE_OP)
            fprintf(stderr, "value: <%s> | ", AllOperDumpStr[LexArr->NodeArr[i].value.oper]);
        else if (LexArr->NodeArr[i].type == TYPE_CNOP)
            fprintf(stderr, "value: <%s> | ", AllOperCNOPDumpStr[LexArr->NodeArr[i].value.oper]);
        else if (LexArr->NodeArr[i].type == TYPE_VAR)
            fprintf(stderr, "value: <%s>, ptr: %p | ", LexArr->NodeArr[i].value.var_name, LexArr->NodeArr[i].value.var_name);
        fprintf(stderr, "\n\n");
    }
    fprintf(stderr, "\n");

    LexArr->NodeArr = MakeTreeFromArrayOfLexems(LexArr);
    ArrayOfLexemsDtor(LexArr);
    return 0;
}