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
}

static void *lt_threadHandler(void *context)
{
    struct lt_looper *looper = context;
}

lt_looper_ref_t lt_createLooper(lt_loop_handler_t loop_handler, void *handler_arg)
{
}

int lt_startLooper(lt_looper_ref_t looper)
{
}

void lt_stopLooper(lt_looper_ref_t looper)
{
}

void lt_destroyLooper(lt_looper_ref_t looper)
{
}

int lt_waitForSignal(lt_looper_ref_t looper, lt_wait_condition_t *cond, int timeout)
{
}

int lt_signalToContinue(lt_looper_ref_t looper, lt_wait_condition_t cond)
{
}


