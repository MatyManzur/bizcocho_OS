#ifndef STRINGSLIB_H_
#define STRINGSLIB_H_

#include <lib.h>
#include <stdarg.h>

char *strncpy(char *dest, const char *src, size_t count);
int strcmp(const char *cs, const char *ct);
char *convert(unsigned int num, int base, unsigned int minDigitCount);
size_t strcpy(char *dest, const char *src);

#endif