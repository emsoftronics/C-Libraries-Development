
#ifndef __LINKLIST_H__
#define __LINKLIST_H__
#include <pthread.h>

typedef enum {
    ITEM_BEGIN  = 0,
    ITEM_NEXT   = 1,
    ITEM_END    = 2,
    ITEM_FIRST  = 3,
    ITEM_LAST   = 4
} sllist_traverse_t;

typedef enum {
    SORTED_NONE     = 0,
    SORTED_BY_ID    = 1,
    SORTED_BY_DCF   = 2,
} sllist_sort_type_t;


typedef void *slist_ref_t;
typedef void *slstack_ref_t;
typedef void *slqueue_ref_t;

typedef int (*compare_t)(void *, void*); /* return 0 => equal, -1 => less, 1 => greater */

extern slist_ref_t sll_createList(sllist_sort_type_t sort_type, compare_t compare_handler);
extern void sll_destroyList(slist_ref_t list);
extern int sll_itemCount(slist_ref_t list);
extern int sll_clear(slist_ref_t list);
extern int sll_addAtHead(slist_ref_t list, void *data, unsigned int data_length, int data_type);
extern int sll_addAtTail(slist_ref_t list, void *data, unsigned int data_length, int data_type);
extern int sll_removeFromHead(slist_ref_t list, void *outbuf, int *data_type);
extern int sll_removeFromTail(slist_ref_t list, void *outbuf, int *data_type);
extern int sll_getListItemCount(slist_ref_t list);
extern int sll_sortList(slist_ref_t list, compare_t compare, int reverse);
extern void *sll_getListItem(sllist_traverse_t cmd, slist_ref_t list, int *item_size, int *data_type);

extern slstack_ref_t sll_createStack(int max_limit);
extern void sll_destroyStack(slstack_ref_t stack);
extern int sll_push(slstack_ref_t stack, void *data, unsigned int data_length, int data_type);
extern int sll_pop(slstack_ref_t stack, void *outbuf, int *data_type);

extern slqueue_ref_t sll_createQueue(int max_limit);
extern void sll_destroyQueue(slqueue_ref_t queue);
extern int sll_enqueue(slqueue_ref_t  queue, void *data, unsigned int data_length, int data_type);
extern int sll_dequeue(slqueue_ref_t  queue, void *outbuf, int *data_type);

#endif /* __LINKLIST_H__ */
