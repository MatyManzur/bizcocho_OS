#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_
#include <ddlADT.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <files.h>

int _xchg(int *lock, int value);

typedef struct blockHub
{
    int globalid; // se inicializa en 0 al principio de todo, también podría ser un static
    ddlADT semBlockList;
} blockHub;

typedef struct semBlock * semPointer;

typedef struct semBlock
{
    char name[32];
    uint8_t id;
    uint64_t value;
    uint8_t semLock;
    ddlADT blockedProcessList;
    semPointer next;
} semBlock;

void initSemaphoreHub();

//SYSCALLS
int initializeSemaphore(char * name, uint64_t initialValue);

uint64_t wait_sem(uint8_t id);

void post_sem(uint8_t id);

int8_t close_sem(uint8_t id);

void print_all_semaphores();

#endif