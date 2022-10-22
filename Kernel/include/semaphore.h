
#include <ddlADT.h>
#include <memoryManager.h>
#include <scheduler.h>

extern int _xchg(int *lock, int value);

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
    ddlADT blockedProcessList;
    semPointer next;
} semBlock;

void initSemaphoreHub();

int initializeSemaphore(char * name, int initialValue);

int wait_sem(int id);

void post_sem(int id);

int close_sem(int id);
