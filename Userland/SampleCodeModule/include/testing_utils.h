#ifndef TESTING_UTILS_H
#define TESTING_UTILS_H

#include "userlib.h"
#include "syslib.h"

uint32_t GetUint();

uint32_t GetUniform(uint32_t max);

void memseter(void * beginning, uint32_t value, uint32_t size);

int memcheck(void * beginning, uint32_t value, uint32_t size);

int64_t satoi(char* str);

void bussy_wait(uint64_t n);

void endless_loop_print();

void endless_loop();

#endif
