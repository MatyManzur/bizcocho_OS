#ifndef DDLADT_H_
#define DDLADT_H_

#include <lib.h>
#include <memoryManager.h>

typedef struct ddlCDT *ddlADT;

typedef void *elemType;

// Retorna una lista vacía.
ddlADT newList();

// Agrega un elemento.
int8_t add(ddlADT list, elemType elem);

/* Resetea el iterador que recorre la lista en el orden de inserción */
void toBegin(ddlADT list);

/* Retorna 1 si hay un elemento siguiente en el iterador que
** recorre la lista en el orden de inserción. Sino retorna 0
*/
int hasNext(ddlADT list);

/* Retorna el elemento siguiente del iterador que recorre la lista
** en el orden de inserción.
** Si no hay un elemento siguiente o no se invocó a toBegin aborta la ejecución.
*/
elemType next(ddlADT list);

/*Es necesario llamarlo luego de un next, y borra lo que devuelve dicho next, NO ES POSIBLE
LLARMARLO DOS VECES SEGUIDAS, es necesario realizar un next antes */
void remove(ddlADT list);

/* Libera la memoria reservada por la lista */
void freeList(ddlADT list);

elemType find(ddlADT list, int(cmpfunction(void *a, void *b)), void *toCmp);

uint32_t getSize(ddlADT list);

#endif