
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
#include "../includes/MyLangDump.hpp"
#include "../includes/MyLangHelpers.hpp"

static char lang_last_error[ERROR_BUF_MAX_SIZE] = {};

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

    Arr->Notes = CALLOC_WITH_TYPE(START_INIT_SIZE, Node_t);
    RETURN_ERR(Arr->Notes,      ERR_CALLOC_FAIL, "не удалось создать массив звеньев размером %zu", START_INIT_SIZE);

    Arr->NotesSize          = 0;
    Arr->NotesCapacity      = START_INIT_SIZE;

    Arr->ScopeSize          = 0;
    Arr->ScopeCapacity      = START_INIT_SIZE;

    Arr->NodeArrSize        = 0;
    Arr->NodeArrCapacity    = START_INIT_SIZE;

    Arr->ScopeBorders = StackCtor();

    STARTTXTDUMPS();

    return OK;
}

void ArrayOfLexemsDtor(LexArr_t * LexArr) {
    if (LexArr != NULL) {
        for (size_t i = 0; i < LexArr->NodeArrCapacity; i++) {
            if (LexArr->NodeArr && (LexArr->NodeArr[i].type == TYPE_VAR || LexArr->NodeArr[i].type == TYPE_STR)) {
                SMART_FREE(LexArr->NodeArr[i].value.str);
            }
        }
        SMART_FREE(LexArr->Scope);
        stackDtor(LexArr->ScopeBorders);
    }

    FINISHTXTDUMPS();
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
    int    line = 2;
    UTF8_tolower(FileBuf);

    size_t NextIndex = 0;
    while(FileBuf[Pos] != '\0') {
        CheckArrayAndReallocate((void **) &(LexArr->Scope),   LexArr->ScopeSize,   &(LexArr->ScopeCapacity),   sizeof(char *));
        CheckArrayAndReallocate((void **) &(LexArr->NodeArr), LexArr->NodeArrSize, &(LexArr->NodeArrCapacity), sizeof(Debug_Node_t));

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
        if (isalnum(FileBuf[Pos]) || IsRussianNext(FileBuf + Pos)) {
            const char * s2 = FileBuf + Pos;
            size_t len = 0;

            int IsRussian = false;

            while (isalnum(FileBuf[Pos]) || (IsRussian = IsRussianNext(FileBuf + Pos))) {
                if (IsRussian) {
                    Pos += 2; // russian character is 2 bytes
                    len += 2;
                } else {
                    Pos++;
                    len++;
                }
            }

            char * Id = CALLOC_WITH_TYPE(len + 1, char);
            strncpy(Id, s2, len);

            CheckArrayAndReallocate((void **) &(LexArr->Scope), LexArr->ScopeSize, &(LexArr->ScopeCapacity), sizeof(char *));
            int VarInd = InArrayStr(LexArr->Scope, Id, LexArr->ScopeSize);
            if (VarInd == -1) {
                LexArr->Scope[LexArr->ScopeSize] = Id;
                VarInd = LexArr->ScopeSize++;
            } else {
                free(Id);
            }

            LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_VAR, {.str = strdup(LexArr->Scope[VarInd])}, NULL, NULL, Pos, line};
            continue;
        }

        int len = strcspn(FileBuf, "\n\0");
        fprintf(stderr, RED "Синтаксическая ошибка: " RESET "не смог определить ключевое слово на строке: \n %.*s \n", len, FileBuf + Pos);
        
        return ERR_INCORRECT_LABEL;
    }

    LexArr->NodeArr[(LexArr->NodeArrSize)++] = {TYPE_OP, {.oper = OPER_NULL}, NULL, NULL};

    return OK;
}

LangErr_t PrintNodeSexprToFile(FILE *fp, Node_t *Node) {
    static int Tabulation = 0;

    PrintTabulations(fp, Tabulation);
    fprintf(fp, "(\n");
    Tabulation++;

    PrintTabulations(fp, Tabulation);
    size_t VarIndex = -1;
    switch(Node->type) {
        case TYPE_VAR:  fprintf(fp, "\'%s\'\n", Node->value.str);               break;
        case TYPE_STR:  fprintf(fp, "\"%s\"\n", Node->value.str);               break;
        case TYPE_NUM:  fprintf(fp, "%d\n",     Node->value.num);               break;
        case TYPE_OP:   fprintf(fp, "%s\n",   AllOper[Node->value.oper].Tree);  break;
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

    fprintf(stdout, GREEN "Tree file was made successfully\n" RESET);
    return OK;
}