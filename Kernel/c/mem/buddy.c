// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifdef BUDDY

#include <memoryManager.h>
#include <files.h>

#define MAX_DIVISIONS 27
#define MIN_SIZE 32

// espacio mínimo 2^5, espacio maximo 2^26
typedef struct buddyNode * buddyPointer;
typedef struct base * basePointer;
struct base
{
    void * start;
    uint32_t memSize;
    uint8_t largestDivision;
    buddyPointer occupied[MAX_DIVISIONS];  // empiezan en NULL
} typedef theBase;

struct buddyNode
{
    uint8_t division;
    buddyPointer prev;
    buddyPointer next;
} typedef buddyNode;

static basePointer myBase;

static uint64_t memUsed;

static void setOccupiedToZero(buddyPointer occupied[])
{
    for(int i=0; i < MAX_DIVISIONS; i++)
        occupied[i]=NULL;
}

uint8_t logBase2(uint32_t nbytes, uint8_t ceiling)
{
    if(nbytes==0)
        return 0;
    uint8_t value = 0;
    while(ceiling? 1<<value < nbytes : 1<<(value+1) <= nbytes)
    {
        value++;
    }
    return value;
}

static buddyPointer removeLast(uint8_t division)
{
    buddyPointer curr = myBase->occupied[division];

    if(curr == NULL)
        return NULL;

    if(curr->next == NULL)
    {
        myBase->occupied[division] = NULL;
        return curr;
    }

    while(curr->next != NULL)
        curr = curr->next;

    curr->prev->next = NULL;

    return curr;
}

static void addToList(uint8_t division, buddyPointer bp)
{
    buddyPointer aux = myBase->occupied[division];
    if(aux != NULL)
        aux->prev = bp;
    myBase->occupied[division] = bp;
    bp->division = division;
    bp->prev = NULL;
    bp->next = aux;
}

// al principio solo la primera división (la más grande) tiene algo
void memInitialize(void * memBase, uint32_t memSize)
{
    if(memSize <= sizeof(struct base))
        return;
    myBase = (basePointer) memBase;
    myBase->start = ((void*)myBase + sizeof(struct base));
    myBase->largestDivision = logBase2(memSize - sizeof(struct base), 0);
    if(myBase->largestDivision >= MAX_DIVISIONS)
        myBase->largestDivision = MAX_DIVISIONS-1;
    myBase->memSize = 1 << myBase->largestDivision;
    memUsed = 0;
    setOccupiedToZero(myBase->occupied);
    addToList(myBase->largestDivision, (buddyPointer) myBase->start);
}



static void * updatingLists(uint8_t division)
{
    if(division > myBase->largestDivision)
        return NULL;

    if(myBase->occupied[division] != NULL)
    {
        return removeLast(division);
    }

    void * actualMemory;
    // TODO hay que ver si se rompe con el llamado recursivo
    if((actualMemory = updatingLists(division+1)) == NULL)
        return NULL;

    uint32_t size = 1<<division;

    buddyPointer firstHalf = (buddyPointer) actualMemory;
    uint8_t * secondHalf = ((uint8_t *) actualMemory) + size;
    *secondHalf = division;

    addToList(division, firstHalf);

    return (void *) (secondHalf);
}



void * memalloc(uint32_t nbytes)
{
    if(nbytes > myBase->memSize || nbytes == 0)
    {
        return NULL;
    }

    // espacio mínimo
    nbytes = ((nbytes+1)<MIN_SIZE)? MIN_SIZE: nbytes+1;
    uint8_t divisionSelected = logBase2(nbytes, 1);

    
    uint8_t * mem = updatingLists(divisionSelected);

    if(mem==NULL)
        return NULL;
    
    memUsed += TWO_TO_POWER_OF(divisionSelected)
    return(void*) (mem + 1);
}

static int8_t findAndRemove(uint8_t division, void * buddyPairStart, uint64_t buddyPairSize)
{
    buddyPointer possibleBuddy = myBase->occupied[division];
    if(possibleBuddy == NULL)
        return 0;
    if(buddyPairStart <= (void*) possibleBuddy && buddyPairStart + buddyPairSize > (void*) possibleBuddy)
    {
        if(possibleBuddy->next!=NULL)
            possibleBuddy->next->prev = NULL;
        myBase->occupied[division] = possibleBuddy->next;
        return 1;
    }
    possibleBuddy = possibleBuddy->next;
    while(possibleBuddy != NULL)
    {
        if(buddyPairStart <= (void*) possibleBuddy && buddyPairStart + buddyPairSize > (void*) possibleBuddy)
        {
            if(possibleBuddy->prev != NULL)
                possibleBuddy->prev->next = possibleBuddy->next;
            if(possibleBuddy->next != NULL)
                possibleBuddy->next->prev = possibleBuddy->prev;
            return 1;
        }
        possibleBuddy = possibleBuddy->next;
    }
    return 0;
}

static void restoreToList(uint8_t division, void * pointerToBlock)
{
    if(division > myBase->largestDivision)
        return;
    uint64_t buddyPairSize =TWO_TO_POWER_OF(division + 1)
    void* pointerToBuddyPair = pointerToBlock - ((((uint64_t)(pointerToBlock - myBase->start)) % buddyPairSize == 0) ? 0 : buddyPairSize / 2);
    if(!findAndRemove(division, pointerToBuddyPair, buddyPairSize))
    {
        addToList(division, (buddyPointer) pointerToBlock);
        return;
    }
    restoreToList(division+1, pointerToBuddyPair);
}

void memfree(void * ap)
{
    if(ap == NULL || ap < myBase->start || ap >= myBase->start + myBase->memSize)
    {
        return;
    }
    
    uint8_t * pointerToStart = (uint8_t * ) ap - 1;
    uint8_t division = *(pointerToStart);
    if(division<0 || division > myBase->largestDivision)
        return;
    restoreToList(division, (void *) pointerToStart);
    memUsed -= TWO_TO_POWER_OF(division)
}

memInfoPointer getMemInfo()
{
    memInfoPointer meminfo=memalloc( sizeof(memInfo) );
    if(meminfo==NULL)
        OUT_OF_MEM_ERROR(NULL);
    meminfo->blockSize = MIN_SIZE;
    meminfo->memSize = myBase->memSize;
    meminfo->freeBlocks = (meminfo->memSize - memUsed) / MIN_SIZE;
    meminfo->system = "Buddy";
    return meminfo;
}


#endif

