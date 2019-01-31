/*
 * =====================================================================================
 *
 *       Filename:  looperthread.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Saturday 27 October 2018 08:28:52  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish22@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include <pthread.h>
#include <errno.h>
#include "looperthread.h"

struct lt_looper {
    pthread_t thread;
    lt_loop_handler_t handler;
    void *handler_arg;
    pthread_mutex_t lock;
    pthread_cond_t private_cond;
    pthread_cond_t *dynamic_cond;
    unsigned char thread_running;
    unsigned char thread_joinable;
    unsigned char thread_suspended;
    unsigned char reserved;
};

static void *lt_threadHandler(void *context)
{
    struct lt_looper *looper = context;
}

lt_looper_ref_t lt_createLooper(lt_loop_handler_t loop_handler, void *handler_arg)
{
    pthread_mutexattr_t attr;
    pthread_attr_t tattr;
    struct lt_looper *looper = malloc(sizeof(struct lt_looper));

    if(!looper) return NULL;
    looper->handler = loop_handler;
    looper->handler_arg = handler_arg;
    looper->dynamic_cond = NULL;
    looper->thread_joinable = 0;
    looper->thread_suspended = 0;
    looper->reserved = 0;
    if (pthread_cond_init(&looper->private_cond, NULL) != 0) goto pthread_cond_error;
    if (pthread_mutexattr_init(&attr) != 0) goto pthread_mutexattr_error;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) goto pthread_mutex_error;
    if (pthread_mutex_init( &looper->lock, &attr) != 0) goto pthread_mutex_error;
    if (pthread_attr_init(&tattr) != 0) goto pthread_attr_error;
    if (pthread_attr_setdetachstate(&tattr, PTHREAD_JOINABLE) != 0) goto pthread_attr_error;
    looper->thread_running = 1;
    if (pthread_create(&looper->thread, &tattr, lt_threadHandler, looper) != 0) {
        looper->thread_running = 0;
        goto pthread_error;
    }
    else {
        looper->thread_joinable = 1;
    }
    pthread_attr_destroy(&tattr);
    pthread_mutexattr_destroy(&attr);
    return looper;
pthread_error:
    pthread_attr_destroy(&tattr);
pthread_attr_error:
    pthread_mutex_destroy(&looper->lock);
pthread_mutex_error:
    pthread_mutexattr_destroy(&attr);
pthread_mutexattr_error:
    pthread_cond_destroy(&looper->private_cond);
pthread_cond_error:
    free(looper);
    errno = ENOTSUP;
    return NULL;
}

int lt_startLooper(lt_looper_ref_t looper)
{

}

void lt_stopLooper(lt_looper_ref_t looper)
{
}

void lt_destroyLooper(lt_looper_ref_t llooper)
{
    int status = 0;
    char *state;
    struct lt_looper *looper = llooper;
    if (!looper) return;
    pthread_mutex_lock(&looper->lock);
    status = looper->thread_running;
    pthread_mutex_unlock(&looper->lock);
    if (status) {
        pthread_mutex_lock(&looper->lock);
        looper->thread_running = 0;
        if (looper->thread_suspended) {
            pthread_cond_signal(&looper->private_cond);
            looper->thread_suspended = 0;
        }
        if (looper->dynamic_cond) {
            lt_signalToContinue(looper, looper->dynamic_cond);
            looper->dynamic_cond = NULL;
        }
        status = looper->thread_joinable;
        pthread_mutex_unlock(&looper->lock);
        if (status) pthread_join(looper->thread, &state);
    }
    pthread_mutex_destroy(&looper->lock);
    pthread_cond_destroy(&looper->private_cond);
    free(looper);
}

int lt_waitForSignal(lt_looper_ref_t looper, lt_wait_condition_t *cond, int timeout)
{
}

int lt_signalToContinue(lt_looper_ref_t looper, lt_wait_condition_t cond)
{
}


