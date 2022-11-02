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
    uint32_t processCount;
} pbrr_t;

static int schedulerRunning = 0;
static pbrr_t schedule = {{0}};
static uint32_t pidToGive = 1;
static pointerPCBNODE_t init;

static ddlADT blockedProcesses[BLOCK_REASON_COUNT];

static ddlADT deathList;

static int8_t initProcess(uint8_t argc, void **argv);

static uint8_t _killProcess(pointerPCBNODE_t head);
static void _unblockProcessWithReason(pointerPCBNODE_t head, BlockedReason_t blockReason);

static char stateChars[] = {'?', 'R', 'F'};
static char blockedReasonChars[] = {'?', 'B', 'P', 'p', 'C', 'S'};

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

static pointerPCBNODE_t findByPid(uint32_t pid)
{
    if(pid == schedule.nowRunning->process->pid)
        return schedule.nowRunning;
    pointerPCBNODE_t foundPointer = NULL;
    for (int i = 0; i < PRIORITY_COUNT && foundPointer == NULL; i++)
    {
        pointerPCBNODE_t head = schedule.processes[i];
        while(head!=NULL && foundPointer == NULL)
        {
            if (head->process->pid == pid)
                foundPointer = head;
            else
                head = head->next;
        }
    }
    return foundPointer;
}


static pointerPCBNODE_t startProcess(char *name, uint8_t argc, void **argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority, uint8_t ppid, fileDescriptor_t fds[MAX_FD_COUNT], pointerPCBNODE_t parent, uint8_t diesOnEsc)
{
    PCB_t *processPCB = memalloc(sizeof(struct PCB_t));
    processPCB->pid = pidToGive++;
    processPCB->ppid = ppid; //El parent provisto
    strncpy(processPCB->name, name, NAME_MAX);
    processPCB->argc = argc;
    processPCB->argv = argv;
    processPCB->processCodeStart = processCodeStart;
    processPCB->processMemEnd = memalloc(PROCESS_MEM_SIZE);
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
                modifyOpenCount(fds[i].fileID, 1, fds[i].mode);
            }
        }
    }
    processPCB->fdReplacements = newList();
    processPCB->priority = priority;
    processPCB->blockedReason.source = NO_BLOCK;
    processPCB->blockedReason.id = 0;
    processPCB->inDeathList = diesOnEsc;
    
    
    // Agrega a las listas este PCB creado
    pointerPCBNODE_t pnode = memalloc(sizeof(struct nodePCB_t));
    pnode->process = processPCB;
    pnode->children = NULL;
    pnode->prevSibling = NULL;
    pnode->nextSibling = NULL;
    pnode->remainingQuantum = PRIORITY_COUNT - priority;
    pnode->previous = NULL;
    pnode->next = schedule.processes[priority];
    if(schedule.processes[priority] != NULL)
    {
        schedule.processes[priority]->previous = pnode;
    }
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
    if(diesOnEsc)
    {
        add(deathList, pnode);
    }
    schedule.processCount++;
    return pnode;
}

uint32_t startParentProcess(char *name, uint8_t argc, void ** argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t priority, uint32_t pidToCopyFds)
{
    pointerPCBNODE_t processToCopyFds= findByPid(pidToCopyFds);
    fileDescriptor_t* fdsToCopy=NULL;
    if(processToCopyFds != NULL && pidToCopyFds !=0 )
    {
        fdsToCopy=processToCopyFds->process->fds;
    }
    pointerPCBNODE_t pnode = startProcess(name,argc,argv,processCodeStart,priority,init->process->pid,fdsToCopy,init, 0);
    schedulerRunning = 1;
    return pnode->process->pid;
}

// Hacer un startChild (equivalente a un fork exec)
uint32_t startChildProcess(char *name, uint8_t argc, void ** argv, int8_t (*processCodeStart)(uint8_t, void **), uint8_t diesOnEsc)
{
    uint8_t priority = schedule.nowRunning->process->priority;
    pointerPCBNODE_t parent= schedule.nowRunning;

    pointerPCBNODE_t pnode= startProcess(name,argc,argv,processCodeStart,priority,schedule.nowRunning->process->pid,schedule.nowRunning->process->fds,parent, diesOnEsc);

    return pnode->process->pid;
}

void initializeScheduler()
{
    for(int i = 0; i< BLOCK_REASON_COUNT; i++)
    {
        blockedProcesses[i] = newList();
    }
    schedule.processCount = 0;
    init = startProcess("init", 0, NULL, initProcess, PRIORITY_COUNT - 1,0,NULL,NULL, 0);
    schedule.nowRunning = NULL;
    deathList = newList();
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

void exit(int8_t statusCode)
{
    schedule.nowRunning->process->statusCode = statusCode;
    _killProcess(schedule.nowRunning);
    _int20();
}

static void inheritChildren(pointerPCBNODE_t *childrenListFrom, pointerPCBNODE_t *childrenListTo, uint32_t ppid)
{
    pointerPCBNODE_t lastChild = NULL;
    while(*childrenListTo != NULL)
    {
        lastChild = *childrenListTo;
        childrenListTo = &((*childrenListTo)->nextSibling);
    }
    *childrenListTo = *childrenListFrom;
    (*childrenListFrom)->prevSibling = lastChild;
    while(*childrenListFrom != NULL)
    {
        (*childrenListFrom)->process->ppid = ppid;
        childrenListFrom = &((*childrenListFrom)->nextSibling);
    }
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
    inheritChildren(&node->children, &init->children, init->process->pid);
    freeNode(node);
    schedule.processCount--;
}

static inline void setProcessReady(PCB_t *process)
{
    process->state = READY;
    process->blockedReason.source = NO_BLOCK;
    process->blockedReason.id = 0;
}

static uint8_t _killProcess(pointerPCBNODE_t head)
{
    if (head != NULL) // si lo encontró
    {
        for(uint8_t i=0; i<MAX_FD_COUNT;i++) //cerramos los files que haya dejado abiertos
        {
            if(head->process->fds[i].mode != 'N' && head->process->fds[i].fileID > STDERR)
            {
                closeForKilling(head->process->fds[i].fileID, head->process->fds[i].mode);
            }
        }
        if(head->process->inDeathList)
        {
            toBegin(deathList);
            while(hasNext(deathList))
            {
                if(next(deathList) == head)
                {
                    remove(deathList);
                }
            }
        }
        if(head->process->state==BLOCKED)
        {   //para que no quede en la lista de bloqueados
            unblockProcessWithReason(head->process->pid, head->process->blockedReason);
        }
        pointerPCBNODE_t parentHead = findByPid(head->process->ppid);
        if (parentHead == NULL) // no debería, porque si no está el padre, a lo sumo tiene que ser el init, y el init está siempre
            return 0;
        // si el padre estaba esperando por él (en específico o por cualquier hijo)
        if (parentHead->process->blockedReason.source == WAIT_CHILD && (parentHead->process->blockedReason.id == head->process->pid || parentHead->process->blockedReason.id == 0))
        {
            _unblockProcessWithReason(parentHead, parentHead->process->blockedReason);
            // le decimos que no espere más, el statusCode lo puede agarrar de nuestro struct
        }
        head->process->state = FINISHED;
    }
    return head != NULL; // devuelve si lo encontró y lo mató
}

uint8_t killProcess(uint32_t pid)
{
    if(pid == init->process->pid)
        return 0;
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t ans = _killProcess(head);
    _int20();
    return ans; // devuelve si lo encontró y lo mató
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
    uint8_t found = head != NULL && head->process->state == READY;
    if (found)
    {
        head->process->state = BLOCKED;
        head->process->blockedReason.source = blockReason.source;
        head->process->blockedReason.id = blockReason.id;
        add(blockedProcesses[blockReason.source], head->process);
        _int20();
    }
    return found; 
}

static void _unblockProcessWithReason(pointerPCBNODE_t head, BlockedReason_t blockReason)
{
    if(head!=NULL)
    {
        setProcessReady(head->process);
        toBegin(blockedProcesses[blockReason.source]);
        while(hasNext(blockedProcesses[blockReason.source]))
        {
            PCB_t * current = (PCB_t*) next(blockedProcesses[blockReason.source]);
            if(current->pid == head->process->pid)
            {
                remove(blockedProcesses[blockReason.source]);
                return;
            }
        }
    }
}

// Para uso del kernel, no es syscall
uint8_t unblockProcessWithReason(uint32_t pid, BlockedReason_t blockReason)
{
    pointerPCBNODE_t head = findByPid(pid);
    uint8_t found = head != NULL && head->process->blockedReason.source == blockReason.source && head->process->blockedReason.id == blockReason.id && head->process->state == BLOCKED;
    if (found)
    {
        _unblockProcessWithReason(head, blockReason);
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

// syscall: bloquea o desbloquea el proceso segun corresponda
uint8_t blockProcess(uint32_t pid)
{
    if(pid==init->process->pid)
        return 0;
    BlockedReason_t reason;
    reason.source = ASKED_TO;
    reason.id = 0;
    uint8_t couldBlock = blockProcessWithReason(pid, reason);
    if(!couldBlock)
    {
        return unblockProcessWithReason(pid, reason);
    }
    return couldBlock;
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

int16_t fdToFileId(uint8_t fd, uint8_t* mode)
{
    if(fd >= MAX_FD_COUNT)
        return -1;
    if(mode != NULL && *mode != 0 && *mode != schedule.nowRunning->process->fds[fd].mode)
        return -1;
    if(mode != NULL && *mode == 0)
        *mode = schedule.nowRunning->process->fds[fd].mode;
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
    modifyOpenCount(schedule.nowRunning->process->fds[toFd].fileID, 1, schedule.nowRunning->process->fds[toFd].mode);
    return 0;
}

void revertFdReplacements()
{
    lostFd_t* last;
    ddlADT list=schedule.nowRunning->process->fdReplacements;
    toBegin(list);
    while(hasNext(list)){
        last=(lostFd_t *)next(list);
        modifyOpenCount(schedule.nowRunning->process->fds[last->index].fileID,-1, schedule.nowRunning->process->fds[last->index].mode);
        schedule.nowRunning->process->fds[last->index].fileID=last->index;
        schedule.nowRunning->process->fds[last->index].mode=last->lostMode;
        memfree((void *) last);
        remove(list);
    }
}

void killAllInDeathList()
{
    toBegin(deathList);
    while(hasNext(deathList))
    {
        pointerPCBNODE_t pnode = next(deathList);
        pnode->process->inDeathList = 0;
        remove(deathList);
        _killProcess(pnode);
    }
}

static char getStateChar(State_t state, BlockedSource_t blockedSource)
{
    if(state==BLOCKED)
    {
        return blockedReasonChars[(blockedSource > 5)? 0 : blockedSource];
    }
    else
    {
        return stateChars[(state > 2)? 0 : state];
    }
}

processInfoPointer * getProcessInfo(uint32_t * procAmount)
{
    uint32_t j = 0;
    processInfoPointer * procInfo = memalloc(sizeof(processInfoPointer) * schedule.processCount);
    for(int i=0 ; i<PRIORITY_COUNT ; i++)
    {
        pointerPCBNODE_t head = schedule.processes[i];
        while(head!=NULL)
        {
            PCB_t* process = head->process;
            procInfo[j] = memalloc(sizeof(processInfo));
            strcpy(procInfo[j]->name, process->name);
            procInfo[j]->pid = process->pid;
            procInfo[j]->ppid = process->ppid;
            procInfo[j]->status = getStateChar(process->state, process->blockedReason.source);
            procInfo[j]->priority = process->priority;
            procInfo[j]->stackPointer = process->stackPointer;
            procInfo[j]->processMemStart = process->processMemStart;
            head = head->next;
            j++;
        }
    }
    *procAmount = schedule.processCount;
    return procInfo;
}
ddlADT getBlockedList(uint8_t blockedSource){
    if(blockedSource>BLOCK_REASON_COUNT){
        return NULL;
    }
    return blockedProcesses[blockedSource];
}

