#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <lib.h>
#include <interrupts.h>
#include <files.h>

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
    NO_BLOCK = 0, // Poner ID en 0
    ASKED_TO,     // Igual con este
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
    int32_t id; // para indicar qué pipe/hijo/semaforo está esperando (¡si no aplica poner en 0!)
} BlockedReason_t;

typedef struct fileDescriptor_t
{
    int16_t fileID;
    uint8_t mode;
} fileDescriptor_t;

typedef struct PCB_t
{
    uint32_t pid;
    uint32_t ppid;
    char name[NAME_MAX];
    uint8_t argc;
    void **argv;
    int8_t (*processCodeStart)(uint8_t argc, void **argv);
    void *processMemStart;
    void *processMemEnd;
    uint64_t stackPointer; // valor del stackPointer que guarda para poder restablecer todos los registros al volver a esta task. Si es la primera vez que se llamó, stackPointer = 0
    State_t state;
    int8_t statusCode;
    fileDescriptor_t fds[MAX_FD_COUNT];
    ddlADT fdReplacements;
    uint8_t priority;
    BlockedReason_t blockedReason;
    uint8_t inDeathList;
} PCB_t;

typedef struct lostFd_t
{

    int16_t lostID;
    uint8_t lostMode;

    uint8_t index;
} lostFd_t;

typedef struct processInfo *processInfoPointer;

typedef struct processInfo
{
    char name[NAME_MAX];
    uint32_t pid;
    uint32_t ppid;
    char status;
    uint8_t priority;
    uint64_t stackPointer;
    void *processMemStart;
} processInfo;

// SYSCALLS
uint32_t startParentProcess(char *name, uint8_t argc, void **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority, uint32_t pidToCopyFds);

uint32_t startChildProcess(char *name, uint8_t argc, void **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t diesOnEsc);

uint32_t getPid();

int8_t waitchild(uint32_t childpid);

void exit(int8_t statusCode);

uint8_t killProcess(uint32_t pid);

uint8_t blockProcess(uint32_t pid);

uint8_t changePriority(uint32_t pid, uint8_t newPriority);

void yield();

int8_t dup2(uint8_t fromFd, uint8_t toFd);

void revertFdReplacements();

processInfoPointer *getProcessInfo(uint32_t *procAmount);

// KERNEL ONLY
void initializeScheduler();

void scheduler();

uint8_t blockProcessWithReason(uint32_t pid, BlockedReason_t blockReason);

uint8_t unblockProcessWithReason(uint32_t pid, BlockedReason_t blockReason);

void unblockAllProcessesBecauseReason(BlockedReason_t blockReason);

int16_t fdToFileId(uint8_t fd, uint8_t *mode);

int8_t openFile(int16_t fileId, uint8_t mode, uint8_t *fd);

int8_t closeFile(uint8_t fd);

ddlADT getBlockedList(uint8_t blockedSource);

void killAllInDeathList();

#endif
