#include <stddef.h>
#include <stdint.h>

#define DIVISIONS 25
#define MIN_SIZE 16


// espacio mínimo 2^4, espacio total 2^25

struct base
{
    void * start;
    uint32_t memSize;
    buddyPointer occupied[DIVISIONS];  // empiezan en NULL
} typedef theBase;

struct buddyNode
{
    uint8_t division;
    buddyPointer prev;
    buddyPointer next;
} typedef buddyNode;

typedef buddyNode * buddyPointer;

typedef theBase * basePointer;

static basePointer myBase;

static uint64_t memUsed;


static void setOccupiedToZero(buddyPointer occupied[])
{
    for(int i=0; i<DIVISIONS; i++)
        occupied[i]=NULL;
}


// al principio solo la primera división (la más grande) tiene algo
void memInitialize(void * memBase, uint32_t memSize)
{
    myBase = (basePointer) memBase;
    myBase->start = ((void*)myBase + sizeof(struct base));
    myBase->memSize = memSize;
    memUsed = 0;
    setOccupiedToZero(myBase->occupied);
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
    return;
}

static void * updatingLists(uint8_t division)
{
    if(0 > division || division >= DIVISIONS)
        return NULL;
    
    if(myBase->occupied[division] != NULL)
    {
        return removeLast(division);
    }

    void * actualMemory;
    // TODO hay que ver si se rompe con el llamado recursivo
    if((actualMemory = updatingLists(division-1)) == NULL)
        return NULL;
    
    uint32_t size = 1<<(DIVISIONS - division);
    
    buddyPointer firstHalf = (buddyPointer) actualMemory;
    uint8_t * secondHalf = ((uint8_t *) actualMemory) + size;
    firstHalf->division = division;
    firstHalf->next = NULL;
    firstHalf->prev = NULL;
    *secondHalf = division;
    
    addToList(division, firstHalf);

    return (void *) (secondHalf);
}

static uint8_t logBase2Ceil(uint32_t nbytes)
{
    uint8_t value = 0;
    while(nbytes>0)
    {
        nbytes /= 2;
        value++;
    }
    return value;
}

void * memalloc(uint32_t nbytes)
{
    if(nbytes>myBase->memSize || nbytes == 0)
    {
        return NULL;
    }

    // espacio mínimo
    nbytes = (nbytes<MIN_SIZE)? MIN_SIZE: nbytes;
    uint8_t divisionSelected = DIVISIONS - logBase2Ceil(nbytes) + 1;

    memUsed += (1<<DIVISIONS-divisionSelected);
    uint8_t * mem = updatingLists(divisionSelected);

    return (void*) (mem + 1);
}

static int8_t findAndRemove(uint8_t division, void * beginningBlockWithBuddy, uint64_t sizeWithBuddy)
{
    buddyPointer possibleBuddy = myBase->occupied[division];
    if(possibleBuddy == NULL)
        return 0;
    if(beginningBlockWithBuddy <= possibleBuddy && beginningBlockWithBuddy + sizeWithBuddy > possibleBuddy)
    {
        if(possibleBuddy->next!=NULL)
            possibleBuddy->next->prev = NULL;
        myBase->occupied[division] = possibleBuddy->next;
        return 1;
    }
    possibleBuddy = possibleBuddy->next;
    while(possibleBuddy != NULL)
    {
        if(beginningBlockWithBuddy <= possibleBuddy && beginningBlockWithBuddy + sizeWithBuddy > possibleBuddy)
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

static void restoreToList(uint8_t division, void * pointerToStart)
{
    if(division>=DIVISIONS)
        return;
    uint64_t sizeWithBuddy = 1<<(DIVISIONS - division + 1);
    pointerToStart -= (((uint64_t)pointerToStart)%sizeWithBuddy == 0)? 0 : sizeWithBuddy/2;
    if(!findAndRemove(division, pointerToStart, sizeWithBuddy))
    {
        addToList(division, (buddyPointer) pointerToStart);
        return;
    }
    if(division==0)
        return;
    restoreToList(division-1, pointerToStart);
}

void memfree(void * ap)
{
    uint8_t * pointerToStart = (uint8_t * ) ap;
    uint8_t division = *pointerToStart - 1;
    if(division<0 || division >= DIVISIONS)
        return;
    restoreToList(division, (void *) pointerToStart); 
    memUsed -= (1<<(DIVISIONS-division));
}

