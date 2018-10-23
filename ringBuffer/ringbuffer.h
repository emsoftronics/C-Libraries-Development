/*
 * =====================================================================================
 *
 *       Filename:  ringbuffer.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Wednesday 24 October 2018 12:46:48  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish22@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <stdint.h>
#include <pthread.h>


typedef struct {
    void *blocks;
    uint32_t read_index;
    uint32_t write_index;
    uint32_t max_length;
    uint32_t occupied_span;
    uint32_t block_size;
    pthread_mutex_t lock;
} ringbuf_t;

extern ringbuf_t *rb_create(size_t maxblocks, size_t blocksize);
extern void rb_destroy(ringbuf_t *rbuf);
extern int rb_put(ringbuf_t *rbuf, void *block, size_t blocksize, int overwrite);
extern int rb_write(ringbuf_t *rbuf, void *blocks, size_t nblocks, size_t blocksize, int overwrite);
extern int rb_get(ringbuf_t *rbuf, void *output_block, size_t blocksize);
extern int rb_read(ringbuf_t *rbuf, void *output_blocks, size_t nblocks, size_t blocksize);

#endif /*  __RINGBUFFER_H__ */
