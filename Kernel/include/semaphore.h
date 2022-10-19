

#include <ddlADT.h>
#include <memoryManager.h>

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

int wait(int id);

void post(int id);
