/*
 * =====================================================================================
 *
 *       Filename:  testrb.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Thursday 25 October 2018 11:19:51  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish22@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ringbuffer.h"
#include <errno.h>

char rand_char(void)
{
    return (char)((rand() % 26) + 'a');
}

char *rand_str(char *out_buff, int bufsize)
{
    int i = 0;
    while(i < bufsize - 1) out_buff[i++] = rand_char();
    out_buff[i] = 0;
    return out_buff;
}

void fill_ringbuffer(ringbuf_t *rb, int overwrite)
{
    char buffer[rb->block_size];
    int i = 0;
    while(i++ < rb->max_length + 5) {
        if (rb_put(rb, rand_str(buffer, sizeof(buffer)), sizeof(buffer), overwrite) == -EOVERFLOW) {
            printf("*****************RingBuffer is full*************\n");
        }
        else printf("%d. %s\n", i , buffer);
    }
}

void empty_ringbuffer(ringbuf_t *rb)
{
    char buffer[rb->block_size];
    int i = 0;

    while(i++ < rb->max_length + 5) {
        if (rb_get(rb, buffer, sizeof(buffer)) == -ENOENT) {
            printf("*****************RingBuffer is empty*************\n");
        }
        else {
            printf("%d. %s\n", i , buffer);
        }
    }
}


int main(int argc, char **argv)
{
    ringbuf_t *rb = rb_create(5, 12);
    if(!rb) return 1;
    printf("----------:Ring Buffer overflow testing:---------------\n");
    fill_ringbuffer(rb, 0);
    printf("----------:Ring Buffer empty testing:---------------\n");
    empty_ringbuffer(rb);
    printf("----------:Ring Buffer overwrite testing:---------------\n");
    fill_ringbuffer(rb, 1);
    printf("----------:Ring Buffer empty testing:---------------\n");
    empty_ringbuffer(rb);
    rb_destroy(rb);
    return 0;
}

