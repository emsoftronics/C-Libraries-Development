/*
 * =====================================================================================
 *
 *       Filename:  ringbuffer.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Wednesday 24 October 2018 12:45:09  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish22@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ringbuffer.h"

ringbuf_t *rb_create(size_t maxblocks, size_t blocksize)
{
    pthread_mutexattr_t attr;

    ringbuf_t *rb = NULL;
    if (!maxblocks || !blocksize) return NULL;
    rb = calloc(1, sizeof(ringbuf_t));
    if (!rb) return NULL;
    rb->blocks = calloc(maxblocks, blocksize);
    if (!rb->blocks) {
        free(rb);
        errno = ENOMEM;
        return NULL;
    }
    rb->read_index = rb->write_index = rb->occupied_span = 0;
    rb->max_length = maxblocks;
    rb->block_size = blocksize;

    if (pthread_mutexattr_init(&attr) != 0) goto mutex_error1;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) goto mutex_error2;
    if (pthread_mutex_init( &rb->lock, &attr) != 0) goto mutex_error2;
    pthread_mutexattr_destroy(&attr);
    return rb;
mutex_error2:
    pthread_mutexattr_destroy(&attr);
mutex_error1:
    free(rb->blocks);
    free(rb);
    errno = ENOLCK;
    return NULL;
}

void rb_destroy(ringbuf_t *rbuf)
{
    if (!rbuf) return;
    if (rbuf->blocks) free(rbuf->blocks);
    pthread_mutex_destroy(&rbuf->lock);
    free(rbuf);
}

int rb_put(ringbuf_t *rbuf, void *block, size_t blocksize, int overwrite)
{
    int ret = 0;
    if ((!rbuf) || (!block) || (blocksize <= 0)) return -EINVAL;
    if (!rbuf->blocks) return -ENOBUFS;
    if ((rbuf->block_size > 0) && (rbuf->block_size != blocksize)) return -EINVAL;

    pthread_mutex_lock(&rbuf->lock);
    if (rbuf->occupied_span == rbuf->max_length) {
        if (overwrite) {
            memcpy(((char *)rbuf->blocks) + (blocksize * (rbuf->write_index)), block, blocksize);
            rbuf->write_index = (rbuf->write_index + 1) % rbuf->max_length;
            rbuf->read_index = (rbuf->read_index + 1) % rbuf->max_length;
        }
        else ret = -EOVERFLOW;
    }
    else {
        memcpy(((char *)rbuf->blocks) + (blocksize * (rbuf->write_index)), block, blocksize);
        rbuf->write_index = (rbuf->write_index + 1) % rbuf->max_length;
        rbuf->occupied_span++;
    }
    pthread_mutex_unlock(&rbuf->lock);
    return ret;
}

int rb_write(ringbuf_t *rbuf, void *blocks, size_t nblocks, size_t blocksize, int overwrite)
{
    int i, ret = 0;

    if ((!rbuf) || (!blocks) || (blocksize <= 0) || (nblocks <= 0)) return -EINVAL;
    if (!rbuf->blocks) return -ENOBUFS;
    if ((rbuf->block_size > 0) && (rbuf->block_size != blocksize)) return -EINVAL;

    pthread_mutex_lock(&rbuf->lock);
    for (i = 0; i < nblocks; i++) {
        if ((ret = rb_put(rbuf, ((char *)blocks) + (blocksize * i), blocksize, overwrite)) < 0) break;
    }
    pthread_mutex_unlock(&rbuf->lock);
    return  (i > 0)? i : ret;
}

int rb_get(ringbuf_t *rbuf, void *output_block, size_t blocksize)
{
    if ((!rbuf) || (!output_block) || (blocksize <= 0)) return -EINVAL;
    if (!rbuf->blocks) return -ENOBUFS;
    if ((rbuf->block_size > 0) && (rbuf->block_size != blocksize)) return -EINVAL;
    if (!rbuf->occupied_span) return -ENOENT;

    pthread_mutex_lock(&rbuf->lock);
    memcpy(output_block, ((char *)rbuf->blocks) + (blocksize * (rbuf->read_index)), blocksize);
    rbuf->read_index = (rbuf->read_index + 1) % rbuf->max_length;
    rbuf->occupied_span--;
    pthread_mutex_unlock(&rbuf->lock);
    return 0;
}

int rb_read(ringbuf_t *rbuf, void *output_blocks, size_t nblocks, size_t blocksize)
{
    int i, ret = 0;

    if ((!rbuf) || (!output_blocks) || (blocksize <= 0) || (nblocks <= 0)) return -EINVAL;
    if (!rbuf->blocks) return -ENOBUFS;
    if ((rbuf->block_size > 0) && (rbuf->block_size != blocksize)) return -EINVAL;

    pthread_mutex_lock(&rbuf->lock);
    for (i = 0; i < nblocks; i++) {
        if ((ret = rb_get(rbuf, ((char *)output_blocks) + (blocksize * i), blocksize)) < 0) break;
    }
    pthread_mutex_unlock(&rbuf->lock);
    return  (i > 0)? i : ret;
}

