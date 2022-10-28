#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <lib.h>
#include <interrupts.h>
#include <files.h>
#include <time.h>

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
    PIPE_WRITE,
    WAIT_CHILD,
    WAIT_SEM,
    BLOCK_REASON_COUNT
};
typedef enum blockedSource BlockedSource_t;

typedef struct BlockReason_t
{
    BlockedSource_t source;
    int8_t id; // para indicar qué pipe/hijo/semaforo está esperando (¡si no aplica poner en 0!)
} BlockedReason_t;

typedef struct fileDescriptor_t
{
    int16_t fileID;
    uint8_t mode;
} fileDescriptor_t;

typedef struct PCB_t
{
    uint8_t pid;
    uint8_t ppid;
    char name[NAME_MAX];
    uint8_t argc;
    void **argv;
    int8_t (*processCodeStart)(uint8_t argc, void **argv);
    void *processMemStart;
    void *processMemEnd;
    uint8_t cockatoo;      // un canary medio trucho que ponemos al final de la memoria para ver que no se pase (pero podría saltarlo tranquilamente)
    uint64_t stackPointer; // valor del stackPointer que guarda para poder restablecer todos los registros al volver a esta task. Si es la primera vez que se llamó, stackPointer = 0
    State_t state;
    int8_t statusCode;
    fileDescriptor_t fds[MAX_FD_COUNT];
    ddlADT fdReplacements;
    uint8_t priority;
    BlockedReason_t blockedReason;
} PCB_t;

typedef struct lostFd_t{

    int16_t lostID;
    uint8_t lostMode;

    uint8_t index;
}lostFd_t; 

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

void revertFdReplacements();

//KERNEL ONLY
void initializeScheduler();

void scheduler();

uint8_t blockProcessWithReason(uint8_t pid, BlockedReason_t blockReason);

uint8_t unblockProcessWithReason(uint8_t pid, BlockedReason_t blockReason);

void unblockAllProcessesBecauseReason(BlockedReason_t blockReason);

int16_t fdToFileId(uint8_t fd, uint8_t mode);

int8_t openFile(int16_t fileId, uint8_t mode, uint8_t* fd);

int8_t closeFile(uint8_t fd);

#endif
