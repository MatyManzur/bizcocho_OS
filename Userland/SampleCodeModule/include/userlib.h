#ifndef USERLIB_H
#define USERLIB_H

#include <syslib.h>
#include <stdarg.h>


int strToNum(const char *str);

char strLength(const char *str);

uint8_t strPrefix(const char *prefix, const char *str, char **afterPrefix);

int strCmp(const char *str1, const char *str2);

void setColor(color_t backgroundColor, color_t characterColor);

void printString(char *str);

void printStringColor(char *str, color_t backgroundColor, color_t characterColor);

void putChar(char c);

void putCharColor(char c, color_t backgroundColor, color_t characterColor);

void printWithFormat(char *format, ...);

char *convert(unsigned int num, int base, unsigned int minDigitCount);

uint8_t ulongToStr(unsigned long num, char *ans);

uint8_t strCopy(char *source, char *dest);

int parser(char *string, char **buffer, char separator, int maxTokenCount, int maxTokenLenght);

int xtou64(const char *str, uint64_t *ans);

void newLine();

int sqrt(int x);

#endif
