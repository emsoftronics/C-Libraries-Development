
#ifndef __LINKLIST_H__
#define __LINKLIST_H__
#include <pthread.h>
#define SLLIST_INITIALIZER    {.first_entry = NULL, .last_entry = NULL, \
                              .lock = PTHREAD_MUTEX_INITIALIZER, \
                              .item_count = 0}

#define SLSTACK_INITIALIZER(limit)  {SLLIST_INITIALIZER, limit}
#define SLQUEUE_INITIALIZER(limit)  {SLLIST_INITIALIZER, limit}

typedef enum {
    ITEM_BEGIN  = 0,
    ITEM_NEXT   = 1,
    ITEM_END    = 2,
    ITEM_FIRST  = 3,
    ITEM_LAST   = 4
} sllist_traverse_t;


typedef enum {
    DT_CHAR     = 0,
    DT_SHORT    = 1 << 8,
    DT_INT      = 2 << 8,
    DT_LONG     = 3 << 8,
    DT_FLOAT    = 4 << 8,
    DT_DOUBLE   = 5 << 8,
    DT_UNSIGNED = 1 << 15,
    DT_PRIMARY  = 0xff00,
    DT_USRDEF   = 0xff,
} sllist_itemtype_t;

struct sll_list;
struct sll_stack;
struct sll_queue;

struct sll_node {
    struct sll_node *next;
    unsigned int item_length;
    unsigned int item_type;
};

struct sll_item_info {
    void *item;
    unsigned int item_length;
    unsigned int item_type;
};

typedef struct sll_item_info sliteminfo_t;

struct sll_list {
    struct sll_node *first_entry;
    struct sll_node *last_entry;
    int item_count;
    pthread_mutex_t lock;
};

struct sll_stack {
    struct sll_list list;
    int stack_limit;
};

struct sll_queue {
    struct sll_list list;
    int queue_limit;
};

typedef struct sll_list  slist_t;
typedef struct sll_stack  slstack_t;
typedef struct sll_queue  slqueue_t;
typedef int (*compare_t)(sliteminfo_t *, sliteminfo_t *); /* return 0 => equal, -1 => less, 1 => greater */

extern int sll_addToList(slist_t *list, void *data, unsigned int data_length, int data_type);
extern int sll_addAtHead(slist_t *list, void *data, unsigned int data_length, int data_type);
extern int sll_addAtTail(slist_t *list, void *data, unsigned int data_length, int data_type);
extern int sll_removeFromHead(slist_t *list, void *outbuf, int *data_type);
extern int sll_removeFromTail(slist_t *list, void *outbuf, int *data_type);
extern int sll_getListItemCount(slist_t *list);
extern int sll_sortList(slist_t *list, compare_t compare, int reverse);
extern void *sll_getListItem(sllist_traverse_t cmd, slist_t *list, int *item_size, int *data_type);
extern int sll_push(slstack_t *stack, void *data, unsigned int data_length, int data_type);
extern int sll_pop(slstack_t *stack, void *outbuf, int *data_type);
extern int sll_enqueue(slqueue_t *queue, void *data, unsigned int data_length, int data_type);
extern int sll_dequeue(slqueue_t *queue, void *outbuf, int *data_type);
extern int sll_itemCount(slist_t *list);
extern int sll_clear(slist_t *list);

#endif /* __LINKLIST_H__ */
