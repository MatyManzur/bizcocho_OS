#include <scheduler.h>


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

static int schedulerRunning = 0;
static pbrr_t schedule = {{0}};
static uint32_t pidToGive = 1;
static pointerPCBNODE_t init;

static ddlADT blockedProcesses[BLOCK_REASON_COUNT];

static int8_t initProcess(uint8_t argc, void **argv);

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

static uint8_t getCockatoo(uint32_t pid)
{
    return (pid * ticks_elapsed()) % 256;
}

static pointerPCBNODE_t startProcess(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority, uint8_t ppid, fileDescriptor_t fds[MAX_FD_COUNT], pointerPCBNODE_t parent)
{
    PCB_t *processPCB = memalloc(sizeof(struct PCB_t));
    processPCB->pid = pidToGive++;
    processPCB->ppid = ppid; //El parent provisto
    strncpy(processPCB->name, name, NAME_MAX);
    processPCB->argc = argc;
    processPCB->argv = (void **)argv;
    processPCB->processCodeStart = processCodeStart;
    processPCB->processMemEnd = memalloc(PROCESS_MEM_SIZE);
    processPCB->cockatoo = getCockatoo(processPCB->pid);
    *(uint8_t *)(processPCB->processMemEnd) = processPCB->cockatoo;
    processPCB->processMemStart = processPCB->processMemEnd + PROCESS_MEM_SIZE - 1; // el stack empieza en el final de la memoria y el rsp baja
    processPCB->stackPointer = 0;                        // usamos que el stackPointer == 0 cuando nunca se ejecutó el proceso
    processPCB->state = READY;
    processPCB->statusCode = -1;
    if(fds==NULL)
    {
        for (int i = 0; i < MAX_FD_COUNT; i++)
        {
            processPCB->fds[i].fileID = (i < 3) ? i : -1;
            processPCB->fds[i].mode = (i==0)? 'R':( (i < 3)? 'W':'N' ); 
        }
    }
    else{
        for (int i = 0; i < MAX_FD_COUNT; i++)
        {
            processPCB->fds[i].fileID = fds[i].fileID;
            processPCB->fds[i].mode = fds[i].mode;
            if(fds[i].fileID > 2 && fds[i].mode != 'N'){
                modifyOpenCount(fds[i].fileID, 1);
            }
        }
    }
    processPCB->fdReplacements = newList();
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
    schedule.processes[priority]->previous = pnode;
    schedule.processes[priority] = pnode;
    //Agregamos el child a la lista del padre
    if(parent!=NULL)
    {
        pointerPCBNODE_t aux;
        if(parent->children==NULL)
        {
            parent->children=pnode;
        }
        else
        {
            aux=parent->children;
            while(aux->nextSibling!=NULL)
            {
                aux=aux->nextSibling;
            }
            aux->nextSibling=pnode;
            pnode->prevSibling=aux;
        }
    }
    return pnode;
}

uint32_t startParentProcess(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority)
{
    pointerPCBNODE_t pnode = startProcess(name,argc,argv,processCodeStart,priority,init->process->pid,NULL,init);
    schedulerRunning = 1;
    return pnode->process->pid;
}

// Hacer un startChild (equivalente a un fork exec)
uint32_t startChildProcess(char *name, uint8_t argc, char **argv, int8_t (*processCodeStart)(uint8_t, void **))
{
    uint8_t priority = schedule.nowRunning->process->priority;
    pointerPCBNODE_t parent= schedule.nowRunning;

    pointerPCBNODE_t pnode= startProcess(name,argc,argv,processCodeStart,priority,schedule.nowRunning->process->pid,schedule.nowRunning->process->fds,parent);

    return pnode->process->pid;
}

void initializeScheduler()
{
    for(int i = 0; i< BLOCK_REASON_COUNT; i++)
    {
        blockedProcesses[i] = newList();
    }
    init = startProcess("init", 0, NULL, initProcess, PRIORITY_COUNT - 1,0,NULL,NULL);
    schedule.nowRunning = NULL;
    return;
}

void scheduler()
{
    if (!schedulerRunning)  //No se agregó nada aparte del init
    {
        return;
    }
    if(schedule.nowRunning == NULL)
    {
        schedule.nowRunning = init;
        initializeTask(schedule.nowRunning->process->argc, schedule.nowRunning->process->argv,
                       schedule.nowRunning->process->processCodeStart,
                       schedule.nowRunning->process->processMemStart);
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

uint32_t getPid()
{
    return schedule.nowRunning->process->pid;
}

static void freeNode(pointerPCBNODE_t head)
{
    memfree(head->process->processMemEnd);
    freeList(head->process->fdReplacements);
    memfree(head->process);
    memfree(head);
}

static pointerPCBNODE_t findByPidRec(uint32_t pid, pointerPCBNODE_t head)
{
    if (head == NULL)
        return NULL;
    if (head->process->pid == pid)
        return head;
    return findByPidRec(pid, head->next);
}

static pointerPCBNODE_t findByPid(uint32_t pid)
{
    if(pid == schedule.nowRunning->process->pid)
        return schedule.nowRunning;
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

static void removeFromList(pointerPCBNODE_t node)
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
    inheritChildren(&node->children, &init->children);
    freeNode(node);
}

static inline void setProcessReady(PCB_t *process)
{
    process->state = READY;
    process->blockedReason.source = NO_BLOCK;
    process->blockedReason.id = 0;
}

uint8_t killProcess(uint32_t pid)
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
    _int20();
    return head != NULL; // devuelve si lo encontró y lo mató
}

int8_t waitchild(uint32_t childpid)
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
    removeFromList(child);
    return statusCode;
}

// Para uso del kernel, no es syscall
uint8_t blockProcessWithReason(uint32_t pid, BlockedReason_t blockReason)
{
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t found = head != NULL && head->process->state != BLOCKED;
    if (found)
    {
        head->process->state = BLOCKED;
        head->process->blockedReason = blockReason;
        add(blockedProcesses[blockReason.source], head->process);
        _int20();
    }else{
        return unblockProcessWithReason(pid,blockReason);
    }
    return found; 
}

// Para uso del kernel, no es syscall
uint8_t unblockProcessWithReason(uint32_t pid, BlockedReason_t blockReason)
{
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t found = (head != NULL) && (head->process->blockedReason.source == blockReason.source) && (head->process->blockedReason.id == blockReason.id);
    if (found)
    {
        setProcessReady(head->process);
        toBegin(blockedProcesses[blockReason.source]);
        while(hasNext(blockedProcesses[blockReason.source]))
        {
            PCB_t * current = (PCB_t*) next(blockedProcesses[blockReason.source]);
            if(current->pid == head->process->pid)
            {
                remove(blockedProcesses[blockReason.source]);
                return found;
            }
        }
    }
    return found;
}

void unblockAllProcessesBecauseReason(BlockedReason_t blockReason)
{
    toBegin(blockedProcesses[blockReason.source]);
    while(hasNext(blockedProcesses[blockReason.source]))
    {
        PCB_t * current = (PCB_t*) next(blockedProcesses[blockReason.source]);
        if(current->blockedReason.id == blockReason.id)
        {
            setProcessReady(current);
            remove(blockedProcesses[blockReason.source]);
        }
    }
}

// estas sí son syscalls
uint8_t blockProcess(uint32_t pid)
{
    BlockedReason_t reason;
    reason.source = ASKED_TO;
    reason.id = 0;
    return blockProcessWithReason(pid, reason);
}

uint8_t unblockProcess(uint32_t pid)
{
    BlockedReason_t reason;
    reason.source = ASKED_TO;
    reason.id = 0;
    return unblockProcessWithReason(pid, reason);
}

uint8_t changePriority(uint32_t pid, uint8_t newPriority)
{
    if (0 > newPriority || newPriority >= PRIORITY_COUNT)
        return 0;
    pointerPCBNODE_t head = findByPid(pid);
    if (head != NULL)
    {
        if (head->previous != NULL)
        {
            head->previous->next = head->next;
        }
        else //es el primero
        {
            schedule.processes[head->process->priority] = head->next;
        }

        if (head->next != NULL)
        {
            head->next->previous = head->previous;
        }
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

static int8_t initProcess(uint8_t argc, void **argv)
{
    while (1)
    {
        pointerPCBNODE_t child = init->children;
        while(child != NULL)
        {
            pointerPCBNODE_t nextChild = child->nextSibling;
            if(child->process->state == FINISHED)
            {
                waitchild(child->process->pid);
            }
            child = nextChild;
        }
        _hlt();
    }
    return 0;
}

void yield(){
    schedule.nowRunning->remainingQuantum=0;
    _int20();
}

int16_t fdToFileId(uint8_t fd, uint8_t mode)
{
    if(fd >= MAX_FD_COUNT)
        return -1;
    if(mode != 0 && mode != schedule.nowRunning->process->fds[fd].mode)
        return -1;
    return schedule.nowRunning->process->fds[fd].fileID;
}

int8_t openFile(int16_t fileId, uint8_t mode, uint8_t* fd)
{
    uint8_t ans = 4;
    while(schedule.nowRunning->process->fds[ans].fileID != -1)
    {
        ans++;
        if(ans >= MAX_FD_COUNT)
        {
            return -1;
        }            
    }
    schedule.nowRunning->process->fds[ans].fileID = fileId;
    schedule.nowRunning->process->fds[ans].mode = mode;
    *fd = ans;
    return 0;
}

int8_t closeFile(uint8_t fd)
{
    if(fd >= MAX_FD_COUNT)
    {
        return -1;
    }
    if(schedule.nowRunning->process->fds[fd].mode=='N')
    {
        return -1;
    }
    schedule.nowRunning->process->fds[fd].fileID= -1;
    schedule.nowRunning->process->fds[fd].mode='N';
    return 0;
}

int8_t dup2(uint8_t fromFd, uint8_t toFd)
{
    if(fromFd >= MAX_FD_COUNT || toFd >= MAX_FD_COUNT)
    {
        return -1;
    }
    lostFd_t* lost=memalloc(sizeof(lostFd_t));
    if(lost==NULL){
        return -1;
    }
    lost->index=toFd;
    lost->lostID=schedule.nowRunning->process->fds[toFd].fileID;
    lost->lostMode=schedule.nowRunning->process->fds[toFd].mode;
    add(schedule.nowRunning->process->fdReplacements,(void *) lost);

    schedule.nowRunning->process->fds[toFd].fileID = schedule.nowRunning->process->fds[fromFd].fileID;
    schedule.nowRunning->process->fds[toFd].mode = schedule.nowRunning->process->fds[fromFd].mode;
    modifyOpenCount(schedule.nowRunning->process->fds[toFd].fileID, 1);
    return 0;
}

void revertFdReplacements()
{
    lostFd_t* last;
    ddlADT list=schedule.nowRunning->process->fdReplacements;
    toBegin(list);
    while(hasNext(list)){
        last=(lostFd_t *)next(list);
        modifyOpenCount(schedule.nowRunning->process->fds[last->index].fileID,-1);
        schedule.nowRunning->process->fds[last->index].fileID=last->index;
        schedule.nowRunning->process->fds[last->index].mode=last->lostMode;
        memfree((void *) last);
        remove(list);
    }
}

void printAllProcesses()
{
     
}

