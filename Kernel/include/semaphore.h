#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <lib.h>
#include <files.h>

int _xchg(uint8_t *lock, int value);

typedef struct blockHub
{
    int globalid; // se inicializa en 0 al principio de todo, también podría ser un static
    ddlADT semBlockList;
} blockHub;

typedef struct semBlock * semPointer;

typedef struct semBlock
{
    char name[32];
    uint32_t id;
    uint64_t value;
    uint8_t semLock;
    ddlADT blockedProcessList;
    semPointer next;
} semBlock;

/*
typedef struct semInfo * semInfoPointer;

typedef struct semInfo
{
    char name[32];
    uint32_t id;
    uint64_t value;
    uint32_t * blocked;
}
*/

void initSemaphoreHub();

//SYSCALLS
uint32_t initializeSemaphore(char * name, uint64_t initialValue);

uint64_t wait_sem(uint32_t id);

uint32_t getSemCount();

void post_sem(uint32_t id);

int8_t close_sem(uint32_t id);

void print_all_semaphores();

#endif