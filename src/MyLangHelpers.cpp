#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "../includes/MyLangHelpers.hpp"
#include "../MyLibs/sassert.hpp"

void SkipSpaces(size_t * pos, const char * buffer) {
    while (isspace(buffer[*pos]))
        (*pos)++;
}

 char * ReadQuotedString(size_t *pos, const char *buffer) {
    (*pos)++; // skip first "

    size_t len  = strcspn(buffer + *pos, "\"\'\0");
    char *str   = strndup(buffer + *pos, len);


    (*pos) += len + 1; // skip last "
    return str;
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

void NodeDtor(Node_t *node) {
    sassert(node, ERR_PTR_NULL);

    if (node != NULL && node->left != NULL)
        NodeDtor(node->left);
    if (node != NULL && node->right != NULL)
        NodeDtor(node->right);
}

void SSkipLine(const char *str, size_t *Pos) {
    while (str[*Pos] != '\n' && str[*Pos] != '\0') {
        (*Pos)++;
    }
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

void UTF8_tolower(const char *buf) {
    if (!buf) return;

    size_t i = 0;
    while (buf[i] != '\0') {
        if (buf[i + 1] != '\0')
            i += UTF8_tolower_char((unsigned char *) buf + i);
        else
            break;
    }
}

void PrintTabulations(FILE *fp, size_t n) {
    for (size_t i = 0; i < n; ++i)
        fprintf(fp, "\t");
}

Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = (Node_t *) calloc(1, sizeof(Node_t));
    sassert(node, ERR_PTR_NULL);

    node->type  = type;
    node->value = value;
    node->left  = left;
    node->right = right;
    return node;
}

const char *GetNLinesDown(const char *StartStr, const char *str, int n) {
    sassert(str,        ERR_PTR_NULL);
    sassert(StartStr,   ERR_PTR_NULL);

    if (n < 0)
        return NULL;

    while (n && str > StartStr) {
        str--;

        if (*str == '\n' || *str == '\r')
            n--;  
    }

    return str;
}

void RepeatChar(char ch, int n, const char *line_start) {
    sassert(line_start, ERR_PTR_NULL);

    bool rus_prev = false;
    for (int j = 0; j < n; j++) {
        if (IsRussianNext(line_start))
            rus_prev = true;
        else
            rus_prev = false;
        if (rus_prev == false)
            putc(ch, stderr);
        line_start++;
    }
}