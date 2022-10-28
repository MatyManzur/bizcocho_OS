#define NULL 0
#include <semaphore.h>


static char initialized;
static blockHub hub;

static void acquire(uint8_t * lock)
{
    while(_xchg(lock,1)!=0);
}

int cmpById(void * inList, void * compareWith)
{
    return ((semPointer) inList)->id == *((int*) compareWith);
}

int cmpByName(void * inList, void * compareWith)
{
    return !strcmp(((semPointer) inList)->name, ((char*) compareWith));
}

void initSemaphoreHub()
{
    if(initialized)
        return;
    hub.globalid = 1;
    initialized = 1;
    hub.semBlockList = newList();
}

uint8_t lock = 0;
uint8_t lockList = 0;
uint8_t lockListPid = 0;

int initializeSemaphore(char * name, uint64_t initialValue) 
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpByName, name);

    if(curr==NULL)
    {
        curr = memalloc(sizeof(struct semBlock));
        if(curr==NULL)
            return -1;
        strncpy(curr->name, name, 32); 
        curr->next = NULL;
        curr->id = hub.globalid++;
        curr->value = initialValue;
        curr->semLock = 0;
        curr->blockedProcessList = newList();
        acquire(&lock);   
        add(hub.semBlockList, curr);
        _xchg(&lock,0);
    }
    return (int) curr->id;
}

uint64_t wait_sem(uint8_t id)
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpById, &id);

    if(curr==NULL)
        return -1;
    
    acquire(&(curr->semLock));
    if(curr->value) {
        curr->value--;
        _xchg(&(curr->semLock),0);
    }
    else{
        uint8_t * pid = memalloc(sizeof(int));
        *pid = getPid();
        BlockedReason_t reason;
        reason.id = curr->id;
        reason.source = WAIT_SEM;
        add(curr->blockedProcessList, pid);
        _xchg(&(curr->semLock),0);
        blockProcessWithReason(*pid, reason);
    }
    return curr->value;
}

void post_sem(uint8_t id)
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpById, &id);
    if(curr!=NULL)
    {
        acquire(&(curr->semLock));
        if(!curr->value)
        {
            acquire(&lockListPid);
            toBegin(curr->blockedProcessList);
            uint8_t * pid = (uint8_t *) next(curr->blockedProcessList);
            if(pid!=NULL){
                memfree(pid);
                remove(curr->blockedProcessList);
                BlockedReason_t reason;
                reason.id = curr->id;
                reason.source = WAIT_SEM;
                unblockProcessWithReason(*pid, reason);
                _xchg(&lockListPid,0);
                _xchg(&(curr->semLock),0);
                return;
            }
            _xchg(&lockListPid,0);
        }
        curr->value++;
        _xchg(&(curr->semLock),0);
    }
}

int8_t close_sem(uint8_t id)
{
    acquire(&lockList);
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        if(curr->id == id)
        {
            remove(hub.semBlockList);
            memfree(curr);
            _xchg(&lockList,0);
            return 0;
        }
    }
    _xchg(&lockList,0);
    return -1;
}

static void printNum(int value)
{
    if(!value)
    {
        write(STDOUT,"0");
    }
    char printable[16];
    printable[15] = 0;
    printable[14] = '\n';
    int index=14;
    while(value!=0)
    {
        index--;
        printable[index] = value%10 + '0';
        value /= 10;
    }
    write(STDOUT, printable+index);
}

void print_all_semaphores()
{
    acquire(&lockList);
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        printNum(curr->id);
        write(STDOUT, curr->name);
        write(STDOUT, "\n");
        printNum(curr->value);
        acquire(&(curr->semLock));
        toBegin(curr->blockedProcessList);
        int * blockedPid;
        write(STDOUT, "Bloqueados: ");
        while((blockedPid = (int *) next(curr->blockedProcessList))!=NULL)
        {
            printNum(*blockedPid);
        }
        write(STDOUT, "\n");
        _xchg(&(curr->semLock), 0);
    }
    _xchg(&lockList, 0);
}
