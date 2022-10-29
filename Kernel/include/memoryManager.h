#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <lib.h>
#define OUT_OF_MEM_ERROR(ret) {\
    write(STDERR, "Out of memory!\n");\
    return (ret);};

void memInitialize(void * memBase,uint32_t memSize);

void* memalloc(uint32_t nbytes);

void memfree(void * ap);

void printMemState();
#endif
