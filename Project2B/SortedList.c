#include "SortedList.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

/* Global Variables */
int opt_yield;          /* Resolve reference in header file */

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    /* Error handling for passed in parameter */
    if (list==NULL || element==NULL)
        return;
    SortedListElement_t* ptr = list -> next;
    
//    if (list->next == NULL)             /* Insert at the end of list */
//    {
//        if (opt_yield & INSERT_YIELD)
//            sched_yield();
//        list->next = element;
//        element->prev = list;
//        element->next = NULL;
//    }
    
    
    
    
    while(ptr != list)
    {
        if (strcmp(element->key, ptr->key) <= 0)
            break;
        ptr = ptr -> next;
    }
    if (opt_yield & INSERT_YIELD)
        sched_yield();

    /* Update */
    element->next = ptr;
    element->prev = ptr->prev;
    ptr->prev->next = element;
    ptr->prev = element;
    
}

int SortedList_delete( SortedListElement_t *element)
{
    if (element == NULL)
        return 1;
    if (element->prev==NULL || element->next==NULL)
        return 1;
    if (element->prev->next != element || element->next->prev!=element)
        return 1;
    if (opt_yield & DELETE_YIELD)
        sched_yield();
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    if (list==NULL || key==NULL)
        return NULL;
    SortedListElement_t* ptr = list;
    ptr = ptr->next;
    while(1)
    {
        if (strcmp(ptr->key, key) > 0)
            return NULL;
        if (strcmp(ptr->key, key) == 0)
            return ptr;
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        ptr = ptr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
    if (list == NULL)           /* List corrupted */
        return -1;
    int len = 0;
    SortedListElement_t* ptr = list;
    ptr = ptr->next;
    while(ptr != list)
    {
        if (ptr==NULL)
            return -1;
        if (ptr->next==NULL || ptr->prev==NULL)
            return -1;
        if (ptr->next->prev!=ptr || ptr->prev->next!=ptr)
            return -1;
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        len += 1;
        ptr = ptr->next;
    }
    return len;
}

