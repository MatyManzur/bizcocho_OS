#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <lib.h>
#include <files.h>

#define MAX_SEM_NAME 32

int _xchg(uint8_t *lock, int value);

typedef struct blockHub
{
    int globalid; // se inicializa en 0 al principio de todo, también podría ser un static
    ddlADT semBlockList;
} blockHub;

typedef struct semBlock * semPointer;

typedef struct semBlock
{
    char name[MAX_SEM_NAME];
    uint32_t id;
    uint64_t value;
    int semLock;
    ddlADT blockedProcessList;
    uint32_t amountBlocked;
    semPointer next;
} semBlock;


typedef struct semInfo * semInfoPointer;

typedef struct semInfo
{
    char name[MAX_SEM_NAME];
    uint32_t id;
    uint64_t value;
    uint32_t * blocked;
} semInfo;


void initSemaphoreHub();

//SYSCALLS
uint32_t initializeSemaphore(char * name, uint64_t initialValue);

uint64_t waitSem(uint32_t id);

uint32_t getSemCount();

void postSem(uint32_t id);

int8_t closeSem(uint32_t id);

semInfoPointer getSemaphoreInfo(uint32_t * semAmount);

#endif