#define NULL 0
#include <semaphore.h>
#include <stringslib.h>


static char initialized;
static blockHub hub;

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

int lock = 0;
int lockList = 0;
int lockListPid = 0;

int initializeSemaphore(char * name, int initialValue) 
{
    
    semPointer curr = (semPointer) find(hub.semBlockList, cmpByName, name);

    if(curr==NULL)
    {
        curr = memalloc(sizeof(struct semBlock));
        strncpy(curr->name, name, 32); 
        curr->next = NULL;
        curr->id = hub.globalid++;
        curr->value = initialValue;
        curr->blockedProcessList = newList();
        // spinlock
        while(_xchg(lock,1)!=0);   
        add(hub.semBlockList, curr);
        _xchg(lock,0);
    }

    return curr->id;
}

int wait_sem(int id)
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpById, &id);

    if(curr==NULL)
        return -1;
    
    // hay que hacer lo de xchg
    while(_xchg(lock,1)!=0);
    if(curr->value) {
        curr->value--;
        _xchg(lock,0);
    }
    else{
        int * pid;
        *pid = getPid();
        BlockedReason_t reason;
        reason.id = curr->id;
        reason.source = WAIT_SEM;
        while(_xchg(lockListPid,1)!=0);
        add(curr->blockedProcessList, pid);
        _xchg(lockListPid,0);
        blockProcessWithReason(*pid, reason);
    }
    return curr->value;
}

void post_sem(int id)
{
    semPointer curr = (semPointer) find(hub.semBlockList, cmpById, &id);
    if(curr!=NULL)
    {
        while(_xchg(lock,1)!=0);
        if(!curr->value)
        {
            while(_xchg(lockListPid,1)!=0);
            toBegin(curr->blockedProcessList);
            int * pid = (int *) next(curr->blockedProcessList);
            if(pid!=NULL){
                remove(curr->blockedProcessList);
                BlockedReason_t reason;
                reason.id = curr->id;
                reason.source = WAIT_SEM;
                unblockProcessWithReason(*pid, reason);
                _xchg(lockListPid,0);
                _xchg(lock,0);
                return;
            }
            _xchg(lockListPid,0);
        }
        curr->value++;
        _xchg(lock,0);
    }
}

int close_sem(int id)
{
    while(_xchg(lockList,1)!=0);
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        if(curr->id == id)
        {
            remove(hub.semBlockList);
            memfree(curr);
            _xchg(lockList,0);
            return 0;
        }
    }
    _xchg(lockList,0);
    return -1;
}
