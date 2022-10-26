#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <printing.h>
#include <lib.h>
#include <string.h>
#include <interrupts.h>
#include <memoryManager.h>

#define MAX_FD_COUNT 64
#define PRIORITY_COUNT 5
#define NAME_MAX 64
#define PROCESS_MEM_SIZE 4096

enum state
{
    BLOCKED,
    READY,
    FINISHED
};
typedef enum state State_t;

enum blockedSource
{
    NO_BLOCK = 0,//Poner ID en 0
    ASKED_TO,//Igual con este
    PIPE_READ,
    WAIT_CHILD,
    WAIT_SEM,
    BLOCK_REASON_COUNT
};
typedef enum blockedSource BlockedSource_t;

typedef struct BlockReason_t
{
    BlockedSource_t source;
    uint8_t id; // para indicar qué pipe/hijo/semaforo está esperando (¡si no aplica poner en 0!)
} BlockedReason_t;

typedef struct PCB_t
{
    uint8_t pid;
    uint8_t ppid;
    char name[NAME_MAX];
    uint8_t argc;
    void **argv;
    void (*processCodeStart)(uint8_t argc, void **argv);
    void *processMemStart;
    uint8_t cockatoo;      // un canary medio trucho que ponemos al final de la memoria para ver que no se pase (pero podría saltarlo tranquilamente)
    uint64_t stackPointer; // valor del stackPointer que guarda para poder restablecer todos los registros al volver a esta task. Si es la primera vez que se llamó, stackPointer = 0
    State_t state;
    int8_t statusCode;
    int8_t fds[MAX_FD_COUNT];
    uint8_t priority;
    BlockedReason_t blockedReason;
} PCB_t;

//SYSCALLS
uint8_t startParentProcess(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority);

uint8_t startChildProcess(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **));

uint8_t getPid();

int8_t waitchild(uint8_t childpid);

void exit(int8_t statusCode);

uint8_t killProcess(uint8_t pid);

uint8_t blockProcess(uint8_t pid);

uint8_t unblockProcess(uint8_t pid);

uint8_t changePriority(uint8_t pid, uint8_t newPriority);

void yield();

//KERNEL ONLY
void initializeScheduler();

void scheduler();

uint8_t blockProcessWithReason(uint8_t pid, BlockedReason_t blockReason);

uint8_t unblockProcessWithReason(uint8_t pid, BlockedReason_t blockReason);

void unblockAllProcessesBecauseReason(BlockedReason_t blockReason);

#endif
