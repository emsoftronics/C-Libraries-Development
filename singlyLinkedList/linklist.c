


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "linklist.h"
#define SIGNED_MAX_VAL(x) ((typeof(x))(((typeof(x))(~(0))) >> 1))
//#define SIGNED_MAX_VAL(x) ((typeof(x))(1) << ((sizeof(x)<<3)-1))
struct sll_list;
struct sll_stack;
struct sll_queue;


struct sll_node {
    struct sll_node *next;
    unsigned int item_id;
    unsigned short item_length;
    unsigned short item_type;
};

struct sll_list {
    struct sll_node *first_entry;
    struct sll_node *last_entry;
    unsigned int item_count;
    unsigned int auto_inc_id_trace;
    signed int   sorted_flag;
    unsigned int data_struct;
    compare_t item_compare;
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

slist_ref_t sll_createList(sllist_sort_type_t sort_type, compare_t compare_handler)
{
    int ret = 0;
    struct sll_list *list = malloc(sizeof(struct sll_list));
    if (!list) {
        errno = ENOMEM;
        return NULL;
    }
    list->first_entry = list->last_entry = NULL;
    list->item_count = 0;
    list->auto_inc_id_trace = 0;
    list->sorted_flag = (compare_handler)? sort_type : SORTED_NONE;
    list->item_compare = compare_handler;
    list->data_struct = 0;
    if ((ret = pthread_mutex_init(&list->lock, NULL)) != 0 ) {
        free(list);
        errno = (ret < 0)? -ret : ret;
        return NULL;
    }
    return list;
}

void sll_destroyList(slist_ref_t slist)
{
    struct sll_list *list = slist;
    if (!list) return;
    sll_clear(list);
    pthread_mutex_destroy(&list->lock);
    free(list);
}

int sll_itemCount(slist_ref_t slist)
{
    struct sll_list *list = slist;
    int count = 0;
    pthread_mutex_lock(&list->lock);
    count = list->item_count;
    pthread_mutex_unlock(&list->lock);
    return count;
}

int sll_clear(slist_ref_t slist)
{
    struct sll_list *list = slist;
    struct sll_node *tmp = NULL;
    void *mem;
    if (!list) return -EINVAL;
    pthread_mutex_lock(&list->lock);
    tmp = list->first_entry;
    while (tmp != NULL) {
        mem = tmp;
        tmp = tmp->next;
        free(mem);
    }
    list->item_count = 0;
    list->first_entry = list->last_entry = NULL;
    pthread_mutex_unlock(&list->lock);

    return 0;
}


int sll_addAtTail(slist_ref_t slist, void *data, unsigned int data_length, int data_type)
{
    struct sll_list *list = slist;
    struct sll_node *tmp = NULL;
    int ret = 0;
    if ((!data) || (!list)) return -EINVAL;
    tmp = (struct sll_node *)malloc(sizeof(struct sll_node) + data_length);
    if (!tmp) return -ENOMEM;
    memcpy((char *)tmp + sizeof(struct sll_node), data, data_length);
    tmp->item_length = data_length;
    tmp->item_type = data_type;
    tmp->next = NULL;
    pthread_mutex_lock(&list->lock);
    if (list->first_entry == NULL) list->first_entry = tmp;

    if (list->last_entry == NULL) list->last_entry = tmp;
    else {
        list->last_entry->next = tmp;
        list->last_entry = tmp;
    }
    list->item_count++;
    list->last_entry->item_id = 0;
    if (!list->data_struct) {
        if (list->auto_inc_id_trace < SIGNED_MAX_VAL(list->auto_inc_id_trace)) {
            list->auto_inc_id_trace++;
            list->last_entry->item_id = list->auto_inc_id_trace;
        }
    }
    ret = list->last_entry->item_id;
    if (list->sorted_flag != SORTED_BY_ID) list->sorted_flag = SORTED_NONE;
    pthread_mutex_unlock(&list->lock);
    return ret;
}

int sll_addAtHead(slist_ref_t slist, void *data, unsigned int data_length, int data_type)
{
    int ret = -1;
    struct sll_list *list = slist;
    struct sll_list temp = {.first_entry = NULL,
                            .last_entry = NULL,
                            .lock = PTHREAD_MUTEX_INITIALIZER,
                            .item_count = 0,
                            .auto_inc_id_trace = 0};
    if ((!list) || (!data)) return -EINVAL;
    ret = sll_addAtTail(&temp, data, data_length, data_type);
    if (ret < 0) return ret;

    pthread_mutex_lock(&list->lock);
    temp.first_entry->next = list->first_entry;
    list->first_entry = temp.first_entry;
    if (!list->last_entry) list->last_entry = list->first_entry;
    list->item_count++;
    list->first_entry->item_id = 0;
    if (!list->data_struct) {
        if (list->auto_inc_id_trace < SIGNED_MAX_VAL(list->auto_inc_id_trace)) {
            list->auto_inc_id_trace++;
            list->first_entry->item_id = list->auto_inc_id_trace;
        }
    }
    ret = list->first_entry->item_id;
    if (list->sorted_flag != SORTED_REV_BY_ID) list->sorted_flag = SORTED_NONE;
    pthread_mutex_unlock(&list->lock);
    return ret;
}


static int sll_addToListSortedByData(slist_ref_t slist, void *data, unsigned int data_length, int data_type)
{
    struct sll_list *list = slist;
    struct sll_node *tmp = NULL;
    struct sll_node *p = NULL;
    struct sll_node *c = NULL;
    int ret = 0;
    if ((!data) || (!list)) return -EINVAL;
    if (!list->item_compare || ((list->sorted_flag != SORTED_BY_DCF)
                && (list->sorted_flag != SORTED_REV_BY_DCF)))
        return sll_addAtTail(slist, data, data_length, data_type);
    tmp = (struct sll_node *)malloc(sizeof(struct sll_node) + data_length);
    if (!tmp) return -ENOMEM;
    memcpy((char *)tmp + sizeof(struct sll_node), data, data_length);
    tmp->item_length = data_length;
    tmp->item_type = data_type;
    tmp->next = NULL;

    pthread_mutex_lock(&list->lock);
    for (c = list->first_entry; c != NULL; c = c->next) {
        if ((list->sorted_flag == SORTED_BY_DCF)
                && (list->item_compare(data, (char *)c + sizeof(struct sll_node)) < 0)) break;
        if ((list->sorted_flag == SORTED_REV_BY_DCF)
                && (list->item_compare(data, (char *)c + sizeof(struct sll_node)) > 0)) break;
        p = c;
    }
    if (!c && !p) {
        list->first_entry = list->last_entry = tmp;
    }
    else if (!c && p) {
        list->last_entry->next = tmp;
        list->last_entry = tmp;
    }
    else if (!p && c) {
        tmp->next = list->first_entry;
        list->first_entry = tmp;
    }
    else {
        tmp->next = c;
        p->next = tmp;
    }
    list->item_count++;
    tmp->item_id = 0;
    if (!list->data_struct) {
        if (list->auto_inc_id_trace < SIGNED_MAX_VAL(list->auto_inc_id_trace)) {
            list->auto_inc_id_trace++;
            tmp->item_id = list->auto_inc_id_trace;
        }
    }
    ret = tmp->item_id;
    pthread_mutex_unlock(&list->lock);
    return ret;
}

int sll_addToList(slist_ref_t slist, void *data, unsigned int data_length, int data_type)
{
    int ret = -EINVAL;
    struct sll_list *list = slist;
    if ((!list) || (!data)) return -EINVAL;
    switch (list->sorted_flag) {
        case SORTED_NONE:
        case SORTED_BY_ID: ret = sll_addAtTail(slist, data, data_length, data_type);
            break;
        case SORTED_REV_BY_ID: ret = sll_addAtHead(slist, data, data_length, data_type);
            break;
        case SORTED_BY_DCF:
        case SORTED_REV_BY_DCF: ret = sll_addToListSortedByData(slist, data, data_length, data_type);
            break;
        default: ret = sll_addAtTail(slist, data, data_length, data_type);
    }
    return ret;
}

int sll_removeFromHead(slist_ref_t slist, void *outbuf, int *data_type)
{
    int length = 0;
    void *tmp = NULL;
    struct sll_list *list = slist;
    if (!list) return -EINVAL;
    if (!list->first_entry) return -ENOENT;
    pthread_mutex_lock(&list->lock);
    if (outbuf) {
        memcpy(outbuf, ((char*)list->first_entry) + sizeof(struct sll_node), list->first_entry->item_length);
        length = list->first_entry->item_length;
    }
    if (data_type) {
        *data_type = list->first_entry->item_type;
    }
    tmp = list->first_entry;
    list->first_entry = list->first_entry->next;
    list->item_count--;
    pthread_mutex_unlock(&list->lock);
    free(tmp);
    return length;
}


int sll_removeFromTail(slist_ref_t slist, void *outbuf, int *data_type)
{
    int length = 0;
    struct sll_list *list = slist;
    struct sll_node *tmp = NULL;
    if (!list) return -EINVAL;
    if (!list->first_entry) return -ENOENT;
    pthread_mutex_lock(&list->lock);
    for(list->last_entry = list->first_entry;
                (list->last_entry->next != NULL) && (list->last_entry->next->next != NULL);
                list->last_entry = list->last_entry->next);

    if (!list->last_entry->next) {
        tmp = list->last_entry;
        list->last_entry = list->first_entry = NULL;
    }
    else {
        tmp = list->last_entry->next;
        list->last_entry->next = NULL;
    }
    if (outbuf) {
        memcpy(outbuf, ((char*)tmp) + sizeof(struct sll_node), tmp->item_length);
        length = tmp->item_length;
    }
    if (data_type) {
        *data_type = tmp->item_type;
    }
    list->item_count--;
    pthread_mutex_unlock(&list->lock);
    free(tmp);
    return length;
}

int sll_removeById(slist_ref_t slist, int id, void *outbuf, int *data_type)
{
    int length = -ENAVAIL;
    struct sll_list *list = slist;
    struct sll_node *p = NULL;
    struct sll_node *c = NULL;
    if (!list) return -EINVAL;
    if (!list->first_entry) return -ENOENT;
    pthread_mutex_lock(&list->lock);
    for(c = list->first_entry; c != NULL; c = c->next) {
        if (c->item_id == (unsigned int)id) break;
        p = c;
    }

    if (c) {
        if (!p) list->first_entry = c->next;
        else p->next = c->next;

        if (!c->next) list->last_entry = p;
        length = c->item_length;
        if (outbuf) memcpy(outbuf, (char *)c + sizeof(struct sll_node), length);
        if (data_type) *data_type = c->item_type;

        list->item_count--;
        free(c);
    }
    pthread_mutex_unlock(&list->lock);
    return length;
}

int sll_getListItemCount(slist_ref_t slist)
{
    int ret = 0;
    struct sll_list *list = slist;
    pthread_mutex_lock(&list->lock);
    ret = list->item_count;
    pthread_mutex_unlock(&list->lock);
    return ret;
}

void *sll_getDataRefById(slist_ref_t slist, int id)
{
    struct sll_list *list = slist;
    struct sll_node *t;
    void *mem = NULL;
    if (!list) {
        errno = EINVAL;
        return NULL;
    }
    pthread_mutex_lock(&list->lock);
    for(t = list->first_entry; t != NULL; t = t->next) {
        if (t->item_id == (unsigned int)id) break;
    }

    if(!t) errno = ENAVAIL;
    else mem = (char *)t + sizeof(struct sll_node);
    pthread_mutex_unlock(&list->lock);

    return mem;
}

int sll_getIdByDataRef(void *data)
{
    struct sll_node *t;
    int id = 0;
    if (!data) return -EINVAL;
    t = (struct sll_node *)((char *)data -  sizeof(struct sll_node));
    id = (int)(t->item_id);
    return id;
}

static void sll_swap(slist_ref_t slist, struct sll_node *prev)
{
    struct sll_list *list = slist;
    struct sll_node *cur = NULL;
    if (!list || !list->first_entry) return;
    if ((!prev) && list->first_entry->next) {
        cur = list->first_entry;
        list->first_entry = cur->next;
        cur->next = cur->next->next;
        list->first_entry->next = cur;
    }
    else if (prev && prev->next && prev->next->next){
        cur = prev->next;
        prev->next = cur->next;
        cur->next = prev->next->next;
        prev->next->next = cur;
    }

    if (cur && !cur->next) list->last_entry = cur;
}

int sll_sortList(slist_ref_t slist, sllist_sort_type_t sort_type)
{
    struct sll_list *list = slist;
    struct sll_node *cur = NULL;
    struct sll_node *prev = NULL;
    int swap = 0, count, comp, i;
    void *cur_data, *next_data;
    if (!list) return -EINVAL;
    if (sort_type == SORTED_NONE) {
        list->sorted_flag = sort_type;
        return 0;
    }
    if (!list->first_entry) return -ENOENT;
    pthread_mutex_lock(&list->lock);

    for(count = 0; count < list->item_count; count++)
    for(cur = list->first_entry, prev = NULL, i = 0;
            (i < list->item_count - count) && (cur->next != NULL);
            cur = cur->next, i++) {
        swap = 0;
        cur_data = (char *)cur + sizeof(struct sll_node);
        next_data = (char *)(cur->next) + sizeof(struct sll_node);
        if (list->item_compare  && ((sort_type == SORTED_BY_DCF) || (sort_type == SORTED_REV_BY_DCF))) {
            comp = list->item_compare(cur_data, next_data);
            if (((comp > 0) && (sort_type == SORTED_BY_DCF))
                    || ((comp < 0) && (sort_type == SORTED_REV_BY_DCF))) swap = 1;
        }
        else {
            if (((cur->item_id > cur->next->item_id) && (sort_type == SORTED_BY_ID))
                    || ((cur->item_id < cur->next->item_id) && (sort_type == SORTED_REV_BY_ID))) swap = 1;
        }

        if (swap) {
            sll_swap(list, prev);
            if (!prev) cur = list->first_entry;
            else cur = prev->next;
        }
        prev = cur;
    }
    list->sorted_flag = sort_type;
    pthread_mutex_unlock(&list->lock);
    return 0;
}

void *sll_getListItem(sllist_traverse_t cmd, slist_ref_t slist, int *item_size, int *data_type)
{
    struct sll_list *list = slist;
    static struct sll_node *tmp = NULL;
    static struct sll_node *head = NULL;
    static int count = 0;
    struct sll_node *t = NULL;
    void *mem = NULL;
    int ret = 0;


    if (!list) {errno = EINVAL; return NULL;}
    if (!list->first_entry) {errno = ENOENT; return NULL;}

    pthread_mutex_lock(&list->lock);
    if ((head != list->first_entry) || (count != list->item_count)) {
        tmp = head = list->first_entry;
        count = list->item_count;
    }

    switch(cmd) {
        case ITEM_BEGIN:
            t = tmp = list->first_entry;
            break;
        case ITEM_NEXT:
            if (tmp) tmp = tmp->next;
            t = tmp;
            break;
        case ITEM_END:
            t = tmp = list->last_entry;
            break;
        case ITEM_FIRST:
            t = list->first_entry;
            break;
        case ITEM_LAST:
            t = list->last_entry;
            break;
        default: ret = -EINVAL;
    }

    if(ret < 0) errno = - ret;
    else if (!t) errno =  ENOENT;
    else {
        mem = ((char *)t) + sizeof(struct sll_node);
        if (item_size) *item_size = t->item_length;

        if (data_type) *data_type = t->item_type;
    }
    pthread_mutex_unlock(&list->lock);
    return mem;
}


slstack_ref_t sll_createStack(int max_limit)
{
    int ret = 0;
    struct sll_stack *stack = malloc(sizeof(struct sll_stack));
    if (!stack) {
        errno = ENOMEM;
        return NULL;
    }
    stack->list.first_entry = stack->list.last_entry = NULL;
    stack->list.item_count = 0;
    stack->list.auto_inc_id_trace = 0;
    stack->list.sorted_flag = 0;
    stack->list.item_compare = NULL;
    stack->list.data_struct = 1;
    stack->stack_limit = max_limit;
    if ((ret = pthread_mutex_init(&stack->list.lock, NULL)) != 0 ) {
        free(stack);
        errno = (ret < 0)? -ret : ret;
        return NULL;
    }
    return stack;
}

void sll_destroyStack(slstack_ref_t sstack)
{
    struct sll_stack *stack = sstack;
    if (!stack) return;
    sll_clear(&stack->list);
    pthread_mutex_destroy(&stack->list.lock);
    free(stack);
}

int sll_push(slstack_ref_t sstack, void *data, unsigned int data_length, int data_type)
{
    struct sll_stack *stack = sstack;
    if ((!stack) || (!data)) return -EINVAL;
    if ((stack->stack_limit > 0) && (stack->list.item_count >= stack->stack_limit))
        return -EOVERFLOW;
    return sll_addAtHead(&stack->list, data, data_length, data_type);
}

int sll_pop(slstack_ref_t sstack, void *outbuf, int *data_type)
{
    struct sll_stack *stack = sstack;
    if (!stack) return -EINVAL;
    return sll_removeFromHead(&stack->list, outbuf, data_type);
}


slqueue_ref_t sll_createQueue(int max_limit)
{
    int ret = 0;
    struct sll_queue *queue = malloc(sizeof(struct sll_queue));
    if (!queue) {
        errno = ENOMEM;
        return NULL;
    }
    queue->list.first_entry = queue->list.last_entry = NULL;
    queue->list.item_count = 0;
    queue->list.auto_inc_id_trace = 0;
    queue->list.sorted_flag = 0;
    queue->list.item_compare = NULL;
    queue->list.data_struct = 2;
    queue->queue_limit = max_limit;
    if ((ret = pthread_mutex_init(&queue->list.lock, NULL)) != 0 ) {
        free(queue);
        errno = (ret < 0)? -ret : ret;
        return NULL;
    }
    return queue;
}

void sll_destroyQueue(slqueue_ref_t squeue)
{
    struct sll_queue *queue = squeue;
    if (!queue) return;
    sll_clear(&queue->list);
    pthread_mutex_destroy(&queue->list.lock);
    free(queue);
}

int sll_enqueue(slqueue_ref_t squeue, void *data, unsigned int data_length, int data_type)
{
    struct sll_queue *queue = squeue;
    if ((!queue) || (!data)) return -EINVAL;
    if ((queue->queue_limit > 0) && (queue->list.item_count >= queue->queue_limit))
        return -EOVERFLOW;
    return sll_addAtTail(&queue->list, data, data_length, data_type);
}

int sll_dequeue(slqueue_ref_t squeue, void *outbuf, int *data_type)
{
    struct sll_queue *queue = squeue;
    if (!queue) return -EINVAL;
    return sll_removeFromHead(&queue->list, outbuf, data_type);
}

