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
            return 0;
        strncpy(curr->name, name, 32); 
        curr->next = NULL;
        curr->amountBlocked = 0;
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

uint64_t waitSem(uint32_t id)
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
        curr->amountBlocked++;
        BlockedReason_t reason;
        reason.id = curr->id;
        reason.source = WAIT_SEM;
        add(curr->blockedProcessList, pid);
        _xchg(&(curr->semLock),0);
        blockProcessWithReason(*pid, reason);
    }
    return curr->value;
}

void postSem(uint32_t id)
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
                curr->amountBlocked--;
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

int8_t closeSem(uint32_t id)
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


// desde userland se deben realizar los malloc
semInfoPointer getSemaphoreInfo(uint32_t * semAmount)
{
    acquire(&lockList);
    toBegin(hub.semBlockList);
    *semAmount = 0;
    semInfoPointer * informationPointer = memalloc(sizeof(semInfoPointer)*semCount);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        informationPointer[*semAmount] = memalloc(sizeof(semInfo));
        informationPointer[*semAmount]->id = curr->id;
        strncpy(informationPointer[*semAmount]->name, curr->name,MAX_SEM_NAME);
        informationPointer[*semAmount]->value = curr->value;
        acquire(&(curr->semLock));
        informationPointer[*semAmount]->blocked = memalloc(sizeof(uint32_t)*(curr->amountBlocked+1));
        toBegin(curr->blockedProcessList);
        uint32_t * blockedPid;
        int i=0;
        while((blockedPid = (uint32_t *) next(curr->blockedProcessList))!=NULL)
        {
            informationPointer[*semAmount]->blocked[i++] = *blockedPid;
        }
        informationPointer[*semAmount]->blocked[i] = 0; // marcamos el último
        (*semAmount)++;
        _xchg(&(curr->semLock), 0);
    }
    _xchg(&lockList, 0);
    return informationPointer;
}