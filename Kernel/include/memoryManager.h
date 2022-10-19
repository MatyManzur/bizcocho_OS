#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <stdint.h>
#include <stddef.h>

void memInitialize(void * memBase,uint32_t memSize);

void* memalloc(uint32_t nbytes);

void memfree(void * ap);

#endif
