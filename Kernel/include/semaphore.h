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
    int id;
    int value;
    int semLock;
    ddlADT blockedProcessList;
    semPointer next;
} semBlock;

void initSemaphoreHub();

//SYSCALLS
int initializeSemaphore(char * name, int initialValue);

int wait_sem(int id);

void post_sem(int id);

int close_sem(int id);

void print_all_semaphores();

#endif