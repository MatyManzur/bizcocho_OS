// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ddlADT.h>
#include <files.h>

typedef struct node
{
    elemType elem;
    struct node *next;
    struct node *prev;
} TNode;

typedef TNode *TList;

struct ddlCDT
{
    TList first;   
    TList current; 
    TList toRemove;
    uint32_t size;
};

ddlADT newList()
{
    ddlADT ans = memalloc(sizeof(struct ddlCDT));
    if (ans == NULL)
        OUT_OF_MEM_ERROR(NULL);
    ans->first = NULL;
    ans->current = NULL;
    ans->toRemove = NULL;
    return ans;
}

int8_t add(ddlADT list, elemType elem) // agrega al principio
{
    TList second = list->first;
    list->first = memalloc(sizeof(struct node));
    if (list->first == NULL)
        OUT_OF_MEM_ERROR(-1);
    list->first->elem = elem;
    list->first->next = second;
    list->first->prev = NULL;
    if (second != NULL)
        second->prev = list->first;
    list->size += 1;
    return 0;
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
    if (!hasNext(list))
    {
        return NULL;
    }
    elemType aux = list->current->elem;
    list->toRemove = list->current;
    list->current = list->current->next;
    return aux;
}

void remove(ddlADT list)
{
    if (list->toRemove != NULL)
    {
        if (list->toRemove->prev == NULL) // es el primero
        {
            list->first = list->toRemove->next;
        }
        else
        {
            list->toRemove->prev->next = list->toRemove->next;
        }

        if (list->toRemove->next != NULL)
        {
            list->toRemove->next->prev = list->toRemove->prev;
        }
        memfree(list->toRemove);
        list->toRemove = NULL;
        list->size -= 1;
    }
}

void freeList(ddlADT list)
{
    TList head = list->first;
    while (head != NULL)
    {
        TList aux = head->next;
        memfree(head);
        head = aux;
    }
    memfree(list);
}

elemType find(ddlADT list, int(cmpfunction(void *a, void *b)), void *toCmp)
{
    TList searcher = list->first;
    while (searcher != NULL)
    {
        if (cmpfunction(searcher->elem, toCmp))
            return searcher->elem;
        searcher = searcher->next;
    }
    return NULL;
}
uint32_t getSize(ddlADT list)
{
    return list->size;
}
