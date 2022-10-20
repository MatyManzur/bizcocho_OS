#define NULL 0
#include <semaphore.h>
#include <stringslib.h>


static char initialized;
static blockHub hub; 

void initSemaphoreHub()
{
    if(initialized)
        return;
    hub.globalid = 0;
    initialized = 1;
    hub.semBlockList = newList();
}

// si dos hacen el initialize al mismo tiempo se rompe por el toBegin
// habría que hacer un find, o directamente no usar adt
int initializeSemaphore(char * name, int initialValue) 
{
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        
        if(!strcmp(curr->name, name))
        {
            return curr->id;
        }
        
    }
    curr = memalloc(sizeof(struct semBlock));
    strncpy(curr->name, name, 32); 
    curr->next = NULL;
    curr->id = hub.globalid++;
    curr->value = initialValue;
    curr->blockedProcessList = newList();
    add(hub.semBlockList, curr);
    return curr->id;
}

int wait(int id)
{
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        if(curr->id == id)
        {
            // hay que hacer lo de xchg
            if(curr->value) 
                curr->value--;
            else{
                int pid = getPid();
                add(curr->blockedProcessList, pid);
                blockProcess(pid);
            }
            return curr->value;
        }
    }
    return -1;
}

void post(int id)
{
    toBegin(hub.semBlockList);
    semPointer curr;
    while((curr = (semPointer) next(hub.semBlockList))!=NULL)
    {
        if(curr->id == id)
        {
            // hay que hacer lo de xchg
            if(!curr->value)
            {
                toBegin(curr->blockedProcessList);
                int * pid = (int *) next(curr->blockedProcessList);
                if(pid!=NULL){
                    remove(curr->blockedProcessList);
                    unblockProcess(*pid);
                }
            }
            curr->value++;
        }
    }
}
