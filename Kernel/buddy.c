
#define DIVISIONS 6

struct buddyNode
{
    uint32_t baseSize;
    uint64_t occupied[DIVISIONS];  //0 for free, 1 for occupied
    void * start;
}typedef buddyNode;

typedef buddyNode* buddyp;

static buddyp base;

void setOccupiedToZero(uint64_t occupied[])
{
    for(int i=0; i<DIVISIONS; i++){
        occupied[i]=0;
    }
}


void memInitialize(void * memBase, uint32_t memSize)
{
    base = (buddyp) memBase;
    base->chunkSize = memSize;
}

static uint32_t closestPow2(uint32_t nbytes, char * times)
{
    uint32_t returnValue=1;
    *times = DIVISIONS-1;
    while(returnValue<=nbytes)
    {
        returnValue*=2;
        (*times)--;
    }
    return returnValue;
}

char pow2(char exponent)
{
    char returnValue = 1;
    while(exponent>0)
    {
        returnValue*=2;
    }
    return returnValue;
}

static void recursiveFill(uint64_t occupied[], char newindex, char times, char movement)
{
    if(newindex<0)
    {
        return;
    }
    uint64_t mask=1;
    mask << times*movement;
    recursiveFill(occupied, newindex-1, (times-times%2)/2, movement*2);
}

void * memalloc(uint32_t nbytes)
{
    if(nbytes>buddyp->baseSize)
    {
        return NULL;
    }

    char index;
    uint32_t powerOf2 = closestPow2(nbytes, &index);

    uint64_t mask;
    char movement = pow2(((DIVISIONS-1)-index));
    char i;

    for(mask=1, i=0;mask!=0; mask<<movement, i++)
    {
        if(!(buddyp->occupied[index] & mask))
        {
            buddyp->occupied[index] |= mask;
            recursiveFill(buddyp->occupied, index-1, (i-i%2)/2, movement*2);
            return buddyp->start + buddyp->baseSize - ((buddyp->baseSize)/pow2(index+1))*i;
        }
    }
}
