
#include <memoryManager.h>
#define BLOCK_SIZE sizeof(Header)
#define BLOCK_AMOUNT (baseSize/BLOCK_SIZE);
typedef uint64_t Align;

union header{
    struct
    {
        union header *next;
        uint32_t size;
    } s;
    Align x;
} header;
typedef union header Header;

static Header* base;
static uint32_t baseSize; //El tamanio de la memoria
static Header* freep=NULL; // Inicamos la lista en nada

void memInitialize(void * memBase,uint32_t memSize)
{
    base = (Header*) memBase; // Es necesario para tener la direccion base del memoryManager y saber si se pasa o no
    baseSize = memSize;
    freep = base;
    freep->s.next = NULL;
    freep->s.size = BLOCK_AMOUNT;
}

void* memalloc(uint32_t nbytes)
{
    Header* currp;
    Header* prevp=freep;
    if(prevp==NULL||nbytes==0){
        return NULL; // Error la lista no esta inicializada o no hay espacio
    }
    uint32_t nunits=( (nbytes+sizeof(Header)-1) / BLOCK_SIZE) + 1;//Cantidad de unidades de bloque que necesitamos
    //Si currp es NULL entonces llegamos al final de la lista y no encontramos el suficiente espacio
    for (currp=freep ; currp!=NULL ; prevp=currp , currp=currp->s.next)
    {
        if (currp->s.size >= nunits)
        {
            if(currp->s.size==nunits)
            {   
                if(currp==freep) //Necesitamos el primero
                {
                    freep=currp->s.next; 
                    // ahora el segundo pasa a ser el primero 
                    // (si el primero era el único, entonces ahora freep=NULL (no hay mas espacio))
                }
                else
                {
                    prevp->s.next = currp->s.next;
                }
            }
            else
            {
                currp->s.size-=nunits; //cambiamos el tamaño del cacho que nos queda libre
                currp += currp->s.size; //nos movemos al cacho que vamos a entregar
                currp->s.size=nunits; //seteamos el tamaño en el header del nuevo cacho entregado
            }
            return (void *)(currp+1);//Nos movemos un sizeof(Header) y devolvemos el cacho de memoria
        }
    
    }//No encontramos lugar
    return NULL;
}

//TODO revisar caso de borde cuando la lista esta toda usada, es decir todos los bloques sin espacio
void memfree(void * ap){
    Header* insertp;
    insertp=(Header*)ap - 1;//Miramos el header de la direccion provista
    //Si el insertp esta dentro de estos valores permitidos entonces nunca deberiamos no encontrarlo, entonces currp no llegaria a ser NULL
    if(insertp == NULL || (void*) insertp < (void*) base || (void*) insertp >= (void*) base + baseSize)
    {
        return; // Error en parametros, fuera de rango
    }
    

    if(freep==NULL)
    {   
        insertp->s.next=freep;
        freep=insertp;
    }
    else if(insertp < freep)
    {
        if((insertp + insertp->s.size) == freep)
        {
            insertp->s.size+=freep->s.size;
            insertp->s.next=freep->s.next;
        }
        else
        {
            insertp->s.next=freep;
        }
        freep = insertp;
    }
    else
    {
        Header* currp = freep;
        //Si currp->next > insertp entonces nos vamos a pasar y si llegamos al final dela lista (currp->s.next==NULL) entonces currp > NULL es el ultimo bloque
        while(currp->s.next < insertp && currp->s.next != NULL )
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