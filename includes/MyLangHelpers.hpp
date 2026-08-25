#ifndef MYLANGHELPERS_H
#define MYLANGHELPERS_H

#include <stdio.h>
#include "MyLangVars.hpp"

LangOperType_e CheckOperType(const char * const FileBuf, int *str_len);
 char * ReadQuotedString(size_t *pos, const char *buffer);
 char * ReadQuotedString(size_t *pos, const char *buffer);
void SkipSpaces(size_t * pos, const char * buffer);
void NodeDtor(Node_t *node);
void SSkipLine(const char *str, size_t *Pos);
bool IsEngAlph(char ch);
int IsRussianNext(const char *buf);
int UTF8_tolower_char(unsigned char *ch);
void UTF8_tolower(const char *buf);
void PrintTabulations(FILE *fp, size_t n);
Node_t * NewNode(LangType_e type, LangElem_u value, Node_t *left, Node_t *right);
void RepeatChar(char ch, int n, const char *line_start);
const char *GetNLinesDown(const char *StartStr, const char *str, int n);

#endif // MYLANGHELPERS_H