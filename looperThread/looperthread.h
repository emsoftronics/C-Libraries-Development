/*
 * =====================================================================================
 *
 *       Filename:  looperthread.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Saturday 27 October 2018 08:32:05  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish22@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#ifndef __LOOPERTHREAD_H__
#define __LOOPERTHREAD_H__

typedef void *lt_looper_ref_t;

typedef void *lt_wait_condition_t;

typedef void (*lt_loop_handler_t)(void *);

lt_looper_ref_t lt_createLooper(lt_loop_handler_t loop_handler, void *handler_arg);
int lt_startLooper(lt_looper_ref_t looper);
void lt_stopLooper(lt_looper_ref_t looper);
void lt_destroyLooper(lt_looper_ref_t looper);
int lt_waitForSignal(lt_looper_ref_t looper, lt_wait_condition_t *cond, int timeout);
int lt_signalToContinue(lt_looper_ref_t looper, lt_wait_condition_t cond);


#endif /* __LOOPERTHREAD_H__ */
