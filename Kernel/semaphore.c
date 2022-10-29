#define NULL 0
#include <semaphore.h>


static char initialized;
static blockHub hub;

static void acquire(int * lock)
{
    while(_xchg(lock,1)!=0);
}

int cmpById(void * inList, void * compareWith)
{
    return ((semPointer) inList)->id == *((uint32_t*) compareWith);
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

uint32_t semCount = 0;
int lock = 0;
int lockList = 0;
int lockListPid = 0;

uint32_t initializeSemaphore(char * name, uint64_t initialValue) 
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
        semCount++; 
        add(hub.semBlockList, curr);
        _xchg(&lock,0);
    }
    return curr->id;
}

uint64_t wait_sem(uint32_t id)
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
        uint32_t * pid = memalloc(sizeof(uint32_t));
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

void post_sem(uint32_t id)
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpById, &id);
    if(curr!=NULL)
    {
        acquire(&(curr->semLock));
        if(!curr->value)
        {
            acquire(&lockListPid);
            toBegin(curr->blockedProcessList);
            uint32_t * pid = (uint32_t *) next(curr->blockedProcessList);
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

int8_t close_sem(uint32_t id)
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
            acquire(&lock);
            semCount--;
            _xchg(&lock, 0);
            _xchg(&lockList,0);
            return 0;
        }
    }
    _xchg(&lockList,0);
    return -1;
}

uint32_t getSemCount()
{
    return semCount;
}


void print_all_semaphores()
{
    write(STDOUT, "LLEGADSFA\n");
    fprintf(STDOUT, "%d\n", lockList);
    acquire(&lockList);
    write(STDOUT, "LasdasADSFA\n");
    toBegin(hub.semBlockList);
    write(STDOUT, "LLEGAwerew\n");
    fprintf(STDOUT, "|         Name         | ID | Value | Blocked Processes\n");
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        fprintf(STDOUT, "|     %s     | %d | %d | ", curr->name, curr->id, curr->value);
        acquire(&(curr->semLock));
        toBegin(curr->blockedProcessList);
        uint32_t * blockedPid;
        while((blockedPid = (uint32_t *) next(curr->blockedProcessList))!=NULL)
        {
            fprintf(STDOUT, " %d ", *blockedPid);
        }
        write(STDOUT, "\n");
        _xchg(&(curr->semLock), 0);
    }
    _xchg(&lockList, 0);
}
