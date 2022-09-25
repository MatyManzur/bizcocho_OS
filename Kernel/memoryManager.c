
#include <stdint.h>
#include <stdlib.h>
#define BLOCK_SIZE sizeof(Header)
#define BLOCK_AMOUNT (baseSize/BLOCK_SIZE);
typedef uint64_t Align;

union header{
    struct memoryManager
    {
        union header *next;
        uint32_t size;
    } s;
    Align x;
};
typedef union header Header;

static Header* base;
static uint32_t baseSize; //El tamanio de la memoria
static Header* freep=NULL; // Inicamos la lista en nada

void memInitialize(void * memBase,uint32_t memSize)
{
    base= (Header*) memBase;
    baseSize=memSize;
    freep=base;
    freep->s.next=NULL;
    freep->s.size=BLOCK_AMOUNT;//Iniciamos con el total del heap en cantidad de bloques
}

void* memalloc(uint32_t nbytes)
{
    Header* currp;
    Header* prevp=freep;
    if(prevp==NULL||nbytes==0){
        return; // Error la lista no esta inicializada o no hay espacio
    }
    uint32_t nunits=( (nbytes+sizeof(Header)-1) / BLOCK_SIZE) + 1;//Cantidad de unidades de bloque que necesitamos
    //Si currp es NULL entonces llegamos al final de la lista y no encontramos el suficiente espacio
    for (currp=freep ; currp!=NULL ; prevp=currp , currp=currp->s.next)
    {
        if (currp->s.size >= nunits)
        {
            if(currp->s.size==nunits)
            {   if(currp==freep)//Ya usamos toda la lista, pues agarramos el total de los nunits que le quedaba a freep
                {
                    freep=currp->s.next; // equivalente a decir freep=NULL
                }
                else
                {
                prevp->s.next = currp->s.next;
                }
            }
            else
            {
                currp->s.size-=nunits;
                currp += currp->s.size;
                currp->s.size=nunits;
            }
            return (void *)(currp+1);
        }
    
    }//No encontramos lugar
    return NULL;
}
//TODO revisar caso de borde cuando la lista esta toda usada, es decir todos los bloques sin espacio
void memfree(void * ap){
    Header* insertp;
    Header* currp;
    insertp=(Header*)ap - 1;//Miramos el header de la direccion provista
    //Si el insertp esta dentro de estos valores permitidos entonces nunca deberiamos no encontrarlo, entonces currp no llegaria a ser NULL
    if(insertp==NULL||insertp<(void*) base||insertp>=(void*)base+baseSize){
        return; // Error en parametros, fuera de rango
    }
    //Si currp->next> insertp entonces nos vamos a pasar y si llegamos al final dela lista entonces currp->s.next==NULL entonces currp > NULL es el ultimo bloque
    currp=freep;
    while(currp->s.next < insertp && currp < currp->s.next )
    {  
        currp=currp->s.next;
    }
    if ((insertp + insertp->s.size) == currp->s.next) //el Insert esta en el medio de nuestro current y el siguiente
    {
        insertp->s.size += currp->s.next->s.size;
        insertp->s.next = currp->s.next->s.next;
    }
    else //Sino hacemos un cambio de puntero
    {
        insertp->s.next = currp->s.next;
    }
    if(currp==freep)//Vemos si el insertp esta antes del primero o si la lista esta toda ocupada
    {
        freep=insertp;
    }
    else
    {
        if ((currp + currp->s.size) == insertp) //El insertp esta justo despues del current
        {   //Uno los bloques si tengo uno atras
            currp->s.size += insertp->s.size;
            currp->s.next = insertp->s.next;
        }
        else 
        {
            currp->s.next = insertp; //Hago que el de atras me apunte
        }
    }
}