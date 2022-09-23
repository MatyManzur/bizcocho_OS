#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void *memset(void *destination, int32_t character, uint64_t length);

void *memcpy(void *destination, const void *source, uint64_t length);

char *cpuVendor(char *result);

void load_idt();

uint8_t getScanCode();

void _hlt();

int memdumpasm(uint64_t address, uint8_t buffer[]); //en libasm.asm

//syscall
int memdump(uint64_t address, uint8_t buffer[]);

#endif
