#include <scheduler.h>
#include <string.h>

#define MAX_FD_COUNT 64
#define PRIORITY_COUNT 5
#define NAME_MAX 64
#define PROCESS_MEM_SIZE 4096
enum state {BLOCKED, READY, FINISHED};
typedef enum state State_t;

typedef struct PCB_t
{
    uint8_t pid;  
    uint8_t ppid;
    char name[NAME_MAX];
    uint8_t argc;
    void** argv;
    void (*processCodeStart)(uint8_t argc, void **argv);
    void* processMemStart;
    uint64_t stackPointer;        //valor del stackPointer que guarda para poder restablecer todos los registros al volver a esta task. Si es la primera vez que se llamó, stackPointer = 0
    State_t state;
    uint8_t statusCode;
    uint8_t fds[MAX_FD_COUNT];
    uint8_t priority;
} PCB_t;

typedef struct nodePCB_t * pointerPCBNODE_t;

typedef struct nodePCB_t
{
    pointerPCBNODE_t previous;
    struct PCB_t * process;
    pointerPCBNODE_t next;
    pointerPCBNODE_t * children;
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
    pointerPCBNODE_t head = schedule.nowRunning->next;
    if(head!=NULL)
    {
        return head;
    }
    uint8_t index = schedule.nowRunning->process->priority;
    for(uint8_t i=0; i<PRIORITY_COUNT; i++)
    {
        head = schedule.processes[(index+i)%PRIORITY_COUNT];
        if(head!=NULL)
        {
            schedule.nowRunning = head;
            return head;
        }
    }
    return head; // solo hay un proceso
}


uint8_t startParentProcess(char* name, uint8_t argc, char** argv, void (*processCodeStart)(uint8_t, void **), uint8_t priority)
{
    PCB_t* processPCB = memalloc(sizeof(struct PCB_t));
    processPCB->pid = pidToGive++;
    processPCB->ppid = 0;
    strncpy(processPCB->name, name, NAME_MAX);
    processPCB->argc = argc;
    processPCB->argv = argv;
    processPCB->processCodeStart = processCodeStart;
    processPCB->processMemStart = memalloc(PROCESS_MEM_SIZE);
    processPCB->stackPointer = 0;
    processPCB->state = READY;
    processPCB->statusCode = 0;
    processPCB->fds = {0,1,2,0};
    processPCB->priority = priority;

    //Agregar a las listas este PCB creado
    pointerPCBNODE_t pnode = memalloc(sizeof(struct nodePCB_t));
    pnode->process=processPCB;
    pnode->children=NULL;
    pnode->remainingQuantum = PRIORITY_COUNT - priority;
    pnode->next = schedule.processes[priority];
    schedule.processes[priority] = pnode;

    return pid;
}

//Hacer un startChild (equivalente a un fork exec)
uint8_t startChildProcess(char* name, uint8_t argc, char** argv, void (*processCodeStart)(uint8_t, void **))
{
    //todo -> modularizar para no repetir el codigo de startParent 
}

void scheduler()
{
    if(schedule.nowRunning==NULL)
    {
        // Inicializar con algun proceso en particular?
    }
    uint8_t canContinue = (schedule.nowRunning->remainingQuantum>0);
    if(schedule.nowRunning->process->state==READY && canContinue)
    {
        schedule.nowRunning->remainingQuantum--;
        return;
    }
    // se quedó sin quantum, se lo actualizamos pero vamos al siguiente proceso
    schedule.nowRunning->remainingQuantum += (canContinue)? 0 : PRIORITY_COUNT - schedule.nowRunning->process->priority;

    pointerPCBNODE_t nextProcess = findNextProcess();
    if(nextProcess == NULL)
        return;

    if (schedule.nowRunning != NULL) //si hubo un proceso anterior
        saveStackPointer(&(schedule.nowRunning->process->stackPointer)); //deja en el puntero del argumento el rbp viejo

    schedule.nowRunning = nextProcess;

    if (schedule.nowRunning->process->stackPointer == 0) //si nunca se inicio esta task
    {
        initializeTask(schedule.nowRunning->process->argc, schedule.nowRunning->process->argv,
                       schedule.nowRunning->process->processCodeStart,
                       schedule.nowRunning->process->processMemStart);
        //mueve el rsp a donde indica el 4to parametro, hace el EOI para el pic, y llama la funcion del 3er parametro con los primeros dos argumentos
        return;
    }
    swapTasks(schedule.nowRunning->process->stackPointer); //cambia el rsp al que le paso en el parametro
}

uint8_t getPid()
{
    return schedule.nowRunning->process->pid;
}

static void freeNode(pointerPCBNODE_t head)
{
    char* name = head->process->name;
    if(name!=NULL)
        memfree(name);
    memfree(head->process);
    memfree(head);
}


static pointerPCBNODE_t findByPidRec(uint8_t pid, pointerPCBNODE_t head)
{
    if(head==NULL)
        return NULL;
    if(head->process->pid == pid)
        return head;
    return findByPidRec(pid, head->next);
}


static pointerPCBNODE_t findByPid(uint8_t pid)
{
    pointerPCBNODE_t foundPointer = NULL;
    for(int i=0; i < PRIORITY_COUNT && foundPointer==NULL; i++)
    {
        foundPointer = findByPidRec(pid, schedule.processes[i]);
    }
    return foundPointer;
}


uint8_t killProcess(uint8_t pid)
{
    pointerPCBNODE_t head = findByPid(pid);
    if(head != NULL)
    {
        if(head->previous != NULL)
            head->previous->next = head->next;
        if(head->next != NULL)
            head->next->previous = head->previous;
        freeNode(head);
    }
    scheduler();
    return head!=NULL;
}

uint8_t blockProcess(uint8_t pid)
{
    pointerPCBNODE_t head = findByPid(pid);
    if(head != NULL)
    {
        head->process->state = BLOCKED;
    }
    scheduler();
    return head!=NULL;
}

uint8_t unblockProcess(uint8_t pid)
{
    pointerPCBNODE_t head = findByPid(pid);
    if(head != NULL)
    {
        head->process->state = READY;
    }
    return head!=NULL;
}

uint8_t changePriority(uint8_t pid, uint8_t newPriority)
{
    if(0 <newPriority || newPriority >= PRIORITY_COUNT)
        return 0;
    pointerPCBNODE_t head = findByPid(pid);
    if(head != NULL)
    {
        if(head->previous != NULL)
            head->previous->next = head->next;
        if(head->next != NULL)
            head->next->previous = head->previous;
        head->process->priority = newPriority;

        head->remainingQuantum = PRIORITY_COUNT-newPriority;
        
        if(schedule.processes[newPriority] != NULL)
        {
            schedule.processes[newPriority]->previous=head;
        } 
        schedule.processes[newPriority] = head;
    }
    return head != NULL;
}
