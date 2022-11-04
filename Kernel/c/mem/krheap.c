// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifndef BUDDY

#include <memoryManager.h>
#include <stringslib.h>
#include <files.h>

#define BLOCK_SIZE sizeof(Header)
#define BLOCK_AMOUNT (baseSize / BLOCK_SIZE)
typedef uint64_t Align;

union header
{
    struct
    {
        union header *next;
        uint32_t size;
    } s;
    Align x;
} header;
typedef union header Header;

static Header *base;
static uint32_t baseSize;    
static Header *freep = NULL; 

void memInitialize(void *memBase, uint32_t memSize)
{
    base = (Header *)memBase;
    baseSize = memSize;
    freep = base;
    freep->s.next = NULL;
    freep->s.size = BLOCK_AMOUNT;
}

void *memalloc(uint32_t nbytes)
{
    Header *currp;
    Header *prevp = freep;
    if (prevp == NULL || nbytes == 0)
    {
        return NULL;
    }
    uint32_t nunits = ((nbytes + sizeof(Header) - 1) / BLOCK_SIZE) + 1; // Cantidad de unidades de bloque que necesitamos
    // Si currp es NULL entonces llegamos al final de la lista y no encontramos el suficiente espacio
    for (currp = freep; currp != NULL; prevp = currp, currp = currp->s.next)
    {
        if (currp->s.size >= nunits)
        {
            if (currp->s.size == nunits)
            {
                if (currp == freep)
                {
                    freep = currp->s.next;}
                else
                {
                    prevp->s.next = currp->s.next;
                }
            }
            else
            {
                currp->s.size -= nunits;
                currp += currp->s.size;
                currp->s.size = nunits;
            }
            return (void *)(currp + 1);
        }

    }
    return NULL;
}

void memfree(void *ap)
{
    if (ap == NULL || ap < (void *)base || ap >= (void *)base + baseSize)
    {
        return;
    }

    Header *insertp;
    insertp = (Header *)ap - 1;
    
    if (freep == NULL)
    {
        insertp->s.next = freep;
        freep = insertp;
    }
    else if (insertp < freep)
    {
        if ((insertp + insertp->s.size) == freep)
        {
            insertp->s.size += freep->s.size;
            insertp->s.next = freep->s.next;
        }
        else
        {
            insertp->s.next = freep;
        }
        freep = insertp;
    }
    else
    {
        Header *currp = freep;
        while (currp->s.next < insertp && currp->s.next != NULL)
        {
            currp = currp->s.next;
        }
        if ((insertp + insertp->s.size) == currp->s.next) // el Insert esta en el medio de nuestro current y el siguiente
        {
            insertp->s.size += currp->s.next->s.size;
            insertp->s.next = currp->s.next->s.next;
        }
        else
        {
            insertp->s.next = currp->s.next;
        }

        if ((currp + currp->s.size) == insertp) // El insertp esta justo despues del current
        {                                       // Uno los bloques si tengo uno atras
            currp->s.size += insertp->s.size;
            currp->s.next = insertp->s.next;
        }
        else
        {
            currp->s.next = insertp;
        }
    }
}

memInfoPointer getMemInfo()
{
    uint32_t amountOfFreeBlocks = 0;
    memInfoPointer meminfo = memalloc(sizeof(memInfo));
    if (meminfo == NULL)
        OUT_OF_MEM_ERROR(NULL);
    Header *mem = freep;
    if (mem != NULL)
    {
        do
        {
            amountOfFreeBlocks += mem->s.size;
            mem = mem->s.next;
        } while (mem != NULL);
    }
    meminfo->blockSize = BLOCK_SIZE;
    meminfo->freeBlocks = amountOfFreeBlocks;
    meminfo->memSize = baseSize;
    meminfo->system = "K&R heap malloc";
    return meminfo;
}
#endif