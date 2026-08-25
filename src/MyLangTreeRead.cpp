#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../MyLibs/sassert.hpp"
#include "../MyLibs/helper_funcs.hpp"
#include "../includes/MyLangDump.hpp"
#include "../includes/MyLangHelpers.hpp"
#include "../includes/MyLangBack.hpp"

static char lang_last_error[ERROR_BUF_MAX_SIZE] = {};

//-----------------------------------------------------------------------------
// Читаем то, что напечатал PrintNodeSexprToFile:
//      ( <value> [<node>] [<node>] )
// где <value> это 'имя', "строка", число или Tree-имя оператора
//-----------------------------------------------------------------------------

static LangOperType_e FindOperByTreeName(const char *Name) {
    for (size_t i = 0; i < NumOfOpers; i++) {
        if (AllOper[i].Tree && strcmp(AllOper[i].Tree, Name) == 0)
            return (LangOperType_e) i;
    }
    return OPER_NOP;
}

static bool IsWholeNumber(const char *str, int *value) {
    sassert(str,    ERR_PTR_NULL);
    sassert(value,  ERR_PTR_NULL);

    if (*str == '\0')
        return false;

    char *EndPtr = NULL;
    long num = strtol(str, &EndPtr, 10);

    if (*EndPtr != '\0')
        return false;

    *value = (int) num;
    return true;
}

LangErr_t ReadValueToNode(const char *buffer, size_t *pos, Node_t *node) {
    sassert(buffer, ERR_PTR_NULL);
    sassert(pos,    ERR_PTR_NULL);
    sassert(node,   ERR_PTR_NULL);
    SkipSpaces(pos, buffer);

    if (buffer[*pos] == '\"' || buffer[*pos] == '\'') {
        bool WasDoubleQuote = (buffer[*pos] == '\"');
        char *str = ReadQuotedString(pos, buffer);
        RETURN_ERR(str, ERR_PTR_NULL, "не смог прочитать строку из файла дерева");

        node->type      = WasDoubleQuote ? TYPE_STR : TYPE_VAR;
        node->value.str = str;
        return OK;
    }

    if (isdigit(buffer[*pos]) || (buffer[*pos] == '-' && isdigit(buffer[*pos + 1]))) {
        char *EndPtr = NULL;
        node->type      = TYPE_NUM;
        node->value.num = (int) strtol(buffer + *pos, &EndPtr, 10);
        *pos = EndPtr - buffer;
        return OK;
    }

    size_t len = strcspn(buffer + *pos, " \t\n\r()");
    RETURN_ERR(len != 0, ERR_INCORRECT_LABEL, "пустое значение в узле дерева, позиция %zu", *pos);

    char *Name = strndup(buffer + *pos, len);
    RETURN_ERR(Name, ERR_CALLOC_FAIL, "не смог скопировать имя оператора");
    *pos += len;

    LangOperType_e oper = FindOperByTreeName(Name);
    RETURN_ERR(oper != OPER_NOP, ERR_INCORRECT_LABEL, "неизвестный оператор <%s> в файле дерева", Name);
    SMART_FREE(Name);

    node->type       = TYPE_OP;
    node->value.oper = oper;
    return OK;
}

Node_t * ReadNodeFromBuffer(const char *buffer) {
    sassert(buffer, ERR_PTR_NULL);

    static size_t pos = 0;
    SkipSpaces(&pos, buffer);
    if (buffer[pos] != '(')
        return NULL;
    if (buffer[pos] == ')') {
        pos++;
        return NULL;
    }
    pos++;

    Node_t *node = NewNode(TYPE_OP, {.oper = OPER_NOP}, NULL, NULL);
    if (ReadValueToNode(buffer, &pos, node) != OK) {
        SMART_FREE(node);
        back_perror();
        exit(NOK);
    }

    SkipSpaces(&pos, buffer);
    node->left  = ReadNodeFromBuffer(buffer);
    node->right = ReadNodeFromBuffer(buffer);

    SkipSpaces(&pos, buffer);
    if (buffer[pos] != ')') {
        fprintf(stderr, RED "[ОШИБКА ЧТЕНИЯ ДЕРЕВА]: " RESET "нету ) на позиции %zu\n", pos);
        exit(NOK);
    }
    pos++;

    return node;
}

LangErr_t ReadTreeFromFile(const char *FileName, Node_t **Tree) {
    sassert(FileName, ERR_PTR_NULL);

    STARTTXTDUMPS();
    FILE *fp = fopen(FileName, "r");
    RETURN_ERR(fp, ERR_FILE_DOES_NOT_EXIST, "не удалось открыть файл <%s>\n", FileName);

    size_t FileSize = GetFileSize(fp);
    RETURN_ERR(FileSize != -1, ERR_FILE_SIZE_INCORRECT, "не удалось узнать размер файла <%s>\n", FileName);

    char *FileBuf = GetBufferFromFile(fp, FileSize);
    RETURN_ERR(FileBuf, ERR_PTR_NULL, "не удалось прочитать файл <%s>\n", FileName);
    fclose(fp);

    *Tree = ReadNodeFromBuffer(FileBuf);

    SMART_FREE(FileBuf);
    return OK;
}
