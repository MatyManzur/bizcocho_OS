#include <ddlADT.h>

typedef struct node
{
    elemType elem;
    struct node * next;
    struct node * prev;
}TNode;

typedef TNode * TList;

struct ddlCDT
{
    TList first; //El primero de la lista
    TList current;//El current node por el que se esta iterando
    int nextCalled;
};

ddlADT newList()
{
    ddlADT ans = memalloc(sizeof(struct ddlCDT));
    ans->first=NULL;
    ans->current=NULL;
    ans->nextCalled=0;
    return ans;
}

void add(ddlADT list, elemType elem) //agrega al principio
{
    TList second = list->first;
    list->first = memalloc(sizeof(struct node));
    list->first->elem = elem;
    list->first->next = second;
    list->first->prev = NULL;
    if(second!=NULL)
        second->prev = list->first;
}

void toBegin(ddlADT list)
{
    list->current = list->first;
}

int hasNext(ddlADT list)
{
    return list->current != NULL;
}

elemType next(ddlADT list)
{   
    if(hasNext(list)){
        return NULL;
    }
    elemType aux = list->current->elem;
    list->current = list->current->next;
    list->nextCalled = 1;
    return aux;
}

void remove(ddlADT list)
{
    if(hasNext(list) && list->nextCalled) //si llamaron a next es porque list->current->prev != NULL porque es el ultimo devuelto
    {
        list->nextCalled = 0;
        TList aborrar = list->current->prev;
        if(aborrar->prev==NULL) // es el primero
        {
            list->first = aborrar->next; 
        }
        else
        {
            aborrar->prev->next = aborrar->next;
        }
        
        if(aborrar->next!=NULL)
        {
            aborrar->next->prev = aborrar->prev;
        }
        free(aborrar);
    }
}

void freeList(ddlADT list)
{
    TList head = list->first;
    while(head != NULL)
    {
        TList aux = head->next;
        free(head);
        head = aux;
    }
    free(list);
}