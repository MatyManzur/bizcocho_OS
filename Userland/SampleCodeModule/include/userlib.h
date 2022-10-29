
#ifndef USERLIB_H
#define USERLIB_H

#include <syslib.h>
#include <stdarg.h>
#include <stddef.h>

int strToNum(const char *str);

size_t strlen(const char *s);

uint8_t strPrefix(const char *prefix, const char *str, char **afterPrefix);

int strcmp(const char *cs, const char *ct);

void fprintf(int fd, char *format, ...);

void printf(char *format, ...);

size_t snprintf(char* buffer,size_t n, char *format, va_list *args);

char *convert(unsigned int num, int base, unsigned int minDigitCount);

uint8_t ulongToStr(unsigned long num, char *ans);

int sqrt(int x);

int parser(char *string, char **buffer, char separator, int maxTokenCount, int maxTokenLenght);

int xtou64(const char *str, uint64_t *ans);

size_t strcpy(char *dest, const char *src);

size_t strncpy(char *dest, const char *src, size_t count);

int removeBackspaces(char str[]);
#endif
