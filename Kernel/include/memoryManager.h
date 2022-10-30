#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <lib.h>
#define OUT_OF_MEM_ERROR(ret) {\
    write(STDERR, "Out of memory!\n");\
    return (ret);};

typedef struct memInfo{
    uint32_t memSize;
    uint32_t blockSize;
    uint32_t freeBlocks; 
}memInfo;
typedef memInfo* memInfoPointer;
void memInitialize(void * memBase,uint32_t memSize);

void* memalloc(uint32_t nbytes);

void memfree(void * ap);

memInfoPointer getMemInfo();
#endif
