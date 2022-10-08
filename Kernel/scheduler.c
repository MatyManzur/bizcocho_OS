#include <scheduler.h>

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
    NO_BLOCK,
    ASKED_TO,
    PIPE_READ,
    WAIT_CHILD,
    WAIT_SEM
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

typedef struct nodePCB_t *pointerPCBNODE_t;

typedef struct nodePCB_t
{
    pointerPCBNODE_t previous;
    struct PCB_t *process;
    pointerPCBNODE_t next;
    pointerPCBNODE_t children;
    pointerPCBNODE_t prevSibling;
    pointerPCBNODE_t nextSibling;
    int8_t remainingQuantum;
} nodePCB_t;

typedef struct pbrr_t
{
    pointerPCBNODE_t processes[PRIORITY_COUNT];
    pointerPCBNODE_t nowRunning;
} pbrr_t;

static pbrr_t schedule = {0};
static int pidToGive = 1;

static pointerPCBNODE_t findNextProcess()
{
    uint8_t currentPriority = schedule.nowRunning->process->priority;
    pointerPCBNODE_t head = schedule.nowRunning->next;
    while (head == NULL || head->process->state != READY)
    {
        if (head == NULL)
        {
            currentPriority = (currentPriority + 1) % PRIORITY_COUNT;
            head = schedule.processes[currentPriority];
        }
        else
        {
            head = head->next;
        }
    }
    return head;
}

static uint8_t getCockatoo(uint8_t pid)
{
    return (pid * ticks_elapsed()) % 256;
}

uint8_t startParentProcess(char *name, uint8_t argc, char **argv, void (*processCodeStart)(uint8_t, void **), uint8_t priority)
{
    PCB_t *processPCB = memalloc(sizeof(struct PCB_t));
    processPCB->pid = pidToGive++;
    processPCB->ppid = 1; // el init
    strncpy(processPCB->name, name, NAME_MAX);
    processPCB->argc = argc;
    processPCB->argv = argv;
    processPCB->processCodeStart = processCodeStart;
    processPCB->processMemStart = memalloc(PROCESS_MEM_SIZE);
    processPCB->cockatoo = getCockatoo(processPCB->pid);
    *(uint8_t *)(processPCB->processMemStart) = processPCB->cockatoo;
    processPCB->processMemStart += PROCESS_MEM_SIZE - 1; // el stack empieza en el final de la memoria y el rsp baja
    processPCB->stackPointer = 0;                        // usamos que el stackPointer == 0 cuando nunca se ejecutó el proceso
    processPCB->state = READY;
    processPCB->statusCode = -1;
    for (int i = 0; i < MAX_FD_COUNT; i++)
    {
        processPCB->fds[i] = (i < 3) ? i : -1;
    }
    processPCB->priority = priority;
    processPCB->blockedReason.source = NO_BLOCK;
    processPCB->blockedReason.id = 0;

    // Agrega a las listas este PCB creado
    pointerPCBNODE_t pnode = memalloc(sizeof(struct nodePCB_t));
    pnode->process = processPCB;
    pnode->children = NULL;
    pnode->prevSibling = NULL;
    pnode->nextSibling = NULL;
    pnode->remainingQuantum = PRIORITY_COUNT - priority;
    pnode->next = schedule.processes[priority];
    schedule.processes[priority] = pnode;

    return processPCB->pid;
}

// Hacer un startChild (equivalente a un fork exec)
uint8_t startChildProcess(char *name, uint8_t argc, char **argv, void (*processCodeStart)(uint8_t, void **))
{
    // todo -> modularizar para no repetir el codigo de startParent
}

void scheduler()
{
    if (schedule.nowRunning == NULL)
    {
        startParentProcess("init", 0, NULL, initProcess, PRIORITY_COUNT - 1);
        schedule.nowRunning = findNextProcess();
        return;
    }
    // chequeamos si se piso el cockatoo
    if (*(uint8_t *)(schedule.nowRunning->process->processMemStart - PROCESS_MEM_SIZE + 1) != schedule.nowRunning->process->cockatoo)
    {
        // perdiste capo -> tirar algun tipo de error y matar el proceso? todo
    }
    uint8_t canContinue = (schedule.nowRunning->remainingQuantum > 0);
    if (schedule.nowRunning->process->state == READY && canContinue)
    {
        schedule.nowRunning->remainingQuantum--;
        return;
    }
    // se quedó sin quantum, se lo actualizamos pero vamos al siguiente proceso
    schedule.nowRunning->remainingQuantum += (canContinue) ? 0 : PRIORITY_COUNT - schedule.nowRunning->process->priority;

    pointerPCBNODE_t nextProcess = findNextProcess();
    if (nextProcess == NULL)
        return;

    if (schedule.nowRunning != NULL)                                     // si hubo un proceso anterior
        saveStackPointer(&(schedule.nowRunning->process->stackPointer)); // deja en el puntero del argumento el rbp viejo

    schedule.nowRunning = nextProcess;

    if (schedule.nowRunning->process->stackPointer == 0) // si nunca se inicio esta task
    {
        initializeTask(schedule.nowRunning->process->argc, schedule.nowRunning->process->argv,
                       schedule.nowRunning->process->processCodeStart,
                       schedule.nowRunning->process->processMemStart);
        // mueve el rsp a donde indica el 4to parametro, hace el EOI para el pic, y llama la funcion del 3er parametro con los primeros dos argumentos
        return;
    }
    swapTasks(schedule.nowRunning->process->stackPointer); // cambia el rsp al que le paso en el parametro
}

uint8_t getPid()
{
    return schedule.nowRunning->process->pid;
}

static void freeNode(pointerPCBNODE_t head)
{
    char *name = head->process->name;
    if (name != NULL)
        memfree(name);
    memfree(head->process);
    memfree(head);
}

static pointerPCBNODE_t findByPidRec(uint8_t pid, pointerPCBNODE_t head)
{
    if (head == NULL)
        return NULL;
    if (head->process->pid == pid)
        return head;
    return findByPidRec(pid, head->next);
}

static pointerPCBNODE_t findByPid(uint8_t pid)
{
    pointerPCBNODE_t foundPointer = NULL;
    for (int i = 0; i < PRIORITY_COUNT && foundPointer == NULL; i++)
    {
        foundPointer = findByPidRec(pid, schedule.processes[i]);
    }
    return foundPointer;
}

void exit(int8_t statusCode)
{
    schedule.nowRunning->process->statusCode = statusCode;
    killProcess(schedule.nowRunning->process->pid);
}

/*
Notas para cuando lo hagamos (borrar dps):

! - no es lo mismo borrarlo de la lista que no borrarlo y ponerlo como FINISHED
Situación 1) Padre se va a borrar:
    - le pasa los hijos al abuelo, estén vivos o sean zombies
Situación 2) Hijo se va a matar:
    2a) el padre está esperando por él:
        - le aviso que no me espere más
        - => caso 2b)
    2b) el padre no está esperando por él (está vivo o está zombie, no importa):
        - dejo el statusCode en el struct
        - me paso a FINISHED
        - no me borro de la lista, ya me va a borrar el wait()
Situación 3) Padre va a esperar a un hijo: wait(pid)
    3a) el hijo está vivo:
        - marcamos como que está bloqueado por esperar a este hijo (o uno en general con un 0 ponele)
        - cómo devuelve el wait() el statusCode cuando este termine ??
            que al ponerlo en blocked llame a scheduler() y se va a ir
            cuando retome, retoma desde justo después de la llamada a scheduler() ? creo que si
    3b) el hijo está FINISHED:
        - agarro el statusCode
        - lo borro de la lista
        - devuelvo el statusCode
*/

static void inheritChildren(pointerPCBNODE_t *childrenListFrom, pointerPCBNODE_t *childrenListTo)
{
    pointerPCBNODE_t lastChild = NULL;
    while(*childrenListTo != NULL)
    {
        lastChild = *childrenListTo;
        childrenListTo = &((*childrenListTo)->nextSibling);
    }
    *childrenListTo = *childrenListFrom;
    (*childrenListFrom)->prevSibling = lastChild;
}

static void removeFromList(pointerPCBNODE_t node, pointerPCBNODE_t parentNode)
{
    if (node->previous != NULL)
    {
        node->previous->next = node->next;
    }
    else //era el primero
    {
        for (int i = 0; i < PRIORITY_COUNT; i++)
        {
            if (schedule.processes[i] == node)
            {
                schedule.processes[i] = node->next;
            }
        }
    }
    if (node->next != NULL)
    {
        node->next->previous = node->previous;
    }
    inheritChildren(&node->children, &parentNode->children);
    freeNode(node);
}

static inline void setProcessReady(PCB_t *process)
{
    process->state = READY;
    process->blockedReason.source = NO_BLOCK;
    process->blockedReason.id = 0;
}

uint8_t killProcess(uint8_t pid)
{
    pointerPCBNODE_t head = findByPid(pid);
    if (head != NULL) // si lo encontró
    {
        pointerPCBNODE_t parentHead = findByPid(head->process->ppid);
        if (parentHead == NULL) // no debería, porque si no está el padre, a lo sumo tiene que ser el init, y el init está siempre
            return 0;
        // si el padre estaba esperando por él (en específico o por cualquier hijo)
        if (parentHead->process->blockedReason.source == WAIT_CHILD && (parentHead->process->blockedReason.id == pid || parentHead->process->blockedReason.id == 0))
        {
            setProcessReady(parentHead->process);
            // le decimos que no espere más, el statusCode lo puede agarrar de nuestro struct
        }
        head->process->state = FINISHED;
    }
    scheduler();
    return head != NULL; // devuelve si lo encontró y lo mató
}

int8_t waitchild(uint8_t childpid)
{
    pointerPCBNODE_t head = schedule.nowRunning->children;
    if (head == NULL) // No tiene hijos
    {
        return -2;
    }
    pointerPCBNODE_t child = (childpid == 0)? head : NULL;
    while (child == NULL && head != NULL)
    {
        if (head->process->pid == childpid)
            child = head;
        head = head->nextSibling;
    }
    if (child == NULL)
    {
        return -2; // No tiene ese hijo, o ya había esperado por él y ya había muerto
    }
    if (child->process->state != FINISHED)
    {
        BlockedReason_t reason;
        reason.source = WAIT_CHILD;
        reason.id = childpid;
        blockProcessWithReason(schedule.nowRunning->process->pid, reason); //-> llama a scheduler()
        // cuando vuelva acá es porque ahora sí está FINISHED
    }
    int8_t statusCode = child->process->statusCode;
    if(child->prevSibling!=NULL)
    {
        child->prevSibling->nextSibling = child->nextSibling;
    }
    else //era el primero
    {
        schedule.nowRunning->children = child->nextSibling;
    }
    if(child->nextSibling!=NULL)
    {
        child->nextSibling->prevSibling = child->prevSibling;
    }
    removeFromList(child, schedule.nowRunning);
    return statusCode;
}

// Para uso del kernel, no es syscall
uint8_t blockProcessWithReason(uint8_t pid, BlockedReason_t blockReason)
{
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t found = head != NULL && head->process->state != BLOCKED;
    if (found)
    {
        head->process->state = BLOCKED;
        head->process->blockedReason = blockReason;
    }
    scheduler();
    return found;
}

// Para uso del kernel, no es syscall
uint8_t unblockProcessWithReason(uint8_t pid, BlockedReason_t blockReason)
{
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t found = (head != NULL) && (head->process->blockedReason.source == blockReason.source) && (head->process->blockedReason.id == blockReason.id);
    if (found)
    {
        setProcessReady(head->process);
    }
    return found;
}

// estas sí son syscalls
uint8_t blockProcess(uint8_t pid)
{
    BlockedReason_t reason;
    reason.source = ASKED_TO;
    reason.id = 0;
    return blockProcessWithReason(pid, reason);
}

uint8_t unblockProcess(uint8_t pid)
{
    BlockedReason_t reason;
    reason.source = ASKED_TO;
    reason.id = 0;
    return unblockProcessWithReason(pid, reason);
}

uint8_t changePriority(uint8_t pid, uint8_t newPriority)
{
    if (0 < newPriority || newPriority >= PRIORITY_COUNT)
        return 0;
    pointerPCBNODE_t head = findByPid(pid);
    if (head != NULL)
    {
        if (head->previous != NULL)
            head->previous->next = head->next;
        if (head->next != NULL)
            head->next->previous = head->previous;
        head->process->priority = newPriority;

        head->remainingQuantum = PRIORITY_COUNT - newPriority;

        if (schedule.processes[newPriority] != NULL)
        {
            schedule.processes[newPriority]->previous = head;
        }
        head->next = schedule.processes[newPriority];
        head->previous = NULL;
        schedule.processes[newPriority] = head;
    }
    return head != NULL;
}

static void initProcess(int argc, void **argv)
{
    while (1)
    {
        waitchild(0); // si no tenia hijos zombies devuelve -2 pero no nos importa
    }
}
