/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *                           All rights reserved.
 *
 * ----------------------------------------------------------------------------
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Siemens code.
 * 
 * The Initial Developer of the Original Code is Siemens AG.
 * Portions created by the Initial Developer are Copyright (C) 2005-06
 * the Initial Developer. All Rights Reserved.
 * ----------------------------------------------------------------------------
 */
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>

#include <adjust.h>
#include <mostdef1.h>
#include <mostns1.h>
#include <mostnetsdll.h>
#include <par_cp.h>

#include "global.h"
#include "most-netservice.h"
#include "service.h"

static pthread_t    service_thread_descriptor;
static sigset_t     old_sigset;
static sem_t        startup_sem;
static sem_t        thread_sem;

/* inspired by ns_main.c (Coguar example) */
static unsigned int events_mns                  = 0;

/* -------------------------------------------------------------------------- */
void MostStartUpFinished(void)
{
    /* wakeup the main thread after MostStartUp has been called */
    sem_post(&startup_sem);
}

/* -------------------------------------------------------------------------- */
void *service_thread_func(void *cookie)
{
    int                 ret;
    sigset_t            sig;
    struct timespec     next_timeout         = { 0, 0 };
    siginfo_t           sinfo;
    long long           last_time, cur_time, time_diff;
    int                 tmp;
    int                 most_service         = 0;

    PRINT_TRACE();

    /* initialize the sigset_t value */
    sigemptyset(&sig);
    sigaddset(&sig, MOST_RT_SIGNAL);
    sigaddset(&sig, MOST_RT_SIGNAL_TIMER);

    /* signal the init function to continue */
    sem_post(&thread_sem);

    PRINT_TRACE("Waiting until MostStartUp has been called");

    /* wait until MostStartUp has been called or just continue */
    sem_wait(&startup_sem);

    TIME_IN_MS(last_time);
        
    for (;;) {
        PRINT_TRACE("Waiting for a signal, timeout = { %ld, %ld }", next_timeout.tv_sec,
                                                                    next_timeout.tv_nsec);

        /* wait for the next interrupt or request */
        ret = sigtimedwait(&sig, &sinfo, &next_timeout);
        if (ret == -1 && errno != EAGAIN) {
            PERR_DEBUG("sigtimedwait error");
            continue;
        }

        PRINT_TRACE("Signal received, ret = %d, value = %d", ret, sinfo.si_value.sival_int);

        /* now ret == -1 is timeout, all other values are signals */

        /* calculate the difference */
        TIME_IN_MS(cur_time);
        time_diff = cur_time - last_time;
        last_time = cur_time;

        PRINT_TRACE("time_diff = %lld", time_diff);
        assert(time_diff >= 0);

        MostTimerIntDiff( (word)time_diff );

        /* calculate the next timeout */
        tmp = MostGetMinTimeout();
        if (tmp == 0xffff) {
            /* wait one second, that should be ok for "infinite" */
            next_timeout.tv_sec     = 1;
            next_timeout.tv_nsec    = 0;
        } else {
            next_timeout.tv_sec  = tmp / 1000;
            tmp %= 1000;
            next_timeout.tv_nsec = tmp * 1000 * 1000;
        }

        /* maximum timeout is 1 s */
        if (next_timeout.tv_sec > 0) {
            next_timeout.tv_sec = 1;
            next_timeout.tv_nsec = 0;
        }

        /* only timeout, nothing to do, timers are updated */
        if (ret == -1 && !most_service) {
            continue;
        }

        /* clear the events from last time */
        events_mns = 0;

        /* now get the event that caued the process to continue */
        if (ret == MOST_RT_SIGNAL && sinfo.si_value.sival_int == MNS_INT) {
            events_mns |= MNS_E_INT | MNS_E_REQ;
        }

        /* timer events */
        if (ret == MOST_RT_SIGNAL_TIMER) {
            events_mns |= MNS_E_TIMER;
        }
    
        /* pending events */
        if (most_service > 0) {
            events_mns |= MNS_E_PEN;
        }

        /* workaround for recv msg before NET_ON, from MostNetsDll.cpp */
        if (MostGetState() != NET_ON) {
            events_mns |= MNS_E_TIMER;
        }

        /* now any layer I event */
        if (events_mns > 0) {
            /*
             * XXX
             * MostNetsDLL.cpp locks the access here, but since no other threads
             * are calling MostService, I don't think that is necessary here 
             */
            pthread_mutex_lock(&g_nets_mutex);
            most_service = MostService(0, events_mns);
            pthread_mutex_unlock(&g_nets_mutex);
        }
    }
}

/* -------------------------------------------------------------------------- */
int service_thread_init(void)
{
    int                         ret;
    sigset_t                    sig;
    struct interrupt_set_arg    interrupt_set = {MOST_RT_SIGNAL, MNS_INT};

    PRINT_TRACE();

    /* initialize the semaphore (in "locked" state) */
    sem_init(&startup_sem, 0, 0);
    sem_init(&thread_sem, 0, 0);
        
    /* modify the interrupt mask of the process */
    sigemptyset(&sig);
    sigaddset(&sig, MOST_RT_SIGNAL);
    sigaddset(&sig, MOST_RT_SIGNAL_TIMER);
    
    ret = sigprocmask(SIG_BLOCK, &sig, &old_sigset);
    if (ret != 0) {
        PERR_DEBUG("Setting procmask failed");
        return E_SIGNAL;
    }

    /* register the signal at the driver */
    ret = ioctl(g_control_fd, MOST_NETS_IRQ_SET, &interrupt_set);
    if (ret < 0) {
        PERR_DEBUG("Registering interrupts at the driver failed");
        ret = E_IOCTL;
        goto out_procmask;
    }
    
    /* start the service thread */
    ret = pthread_create(&service_thread_descriptor, NULL, service_thread_func, NULL);
    if (ret < 0) {
        PERR_DEBUG("Thread creation failed");
        ret = E_THREAD;
        goto out_ioctl;
    }

    /* wait until the service thread has finished his initialization */
    sem_wait(&thread_sem);

    return E_SUCCESS;

out_procmask:
    sigprocmask(SIG_SETMASK, &old_sigset, NULL);
out_ioctl:
    interrupt_set.sigmask   = 0;
    ioctl(g_control_fd, MOST_NETS_IRQ_SET, &interrupt_set);
    return ret;
}

/* -------------------------------------------------------------------------- */
void service_thread_finish(void)
{
    struct interrupt_set_arg    interrupt_set = {0, 0};
    int                         ret;
    
    /* deregister the signal of the driver */
    ret = ioctl(g_control_fd, MOST_NETS_IRQ_SET, &interrupt_set);
    if (ret < 0) {
        PERR_DEBUG("Deregistering interrupts at the driver failed");
    }

    /* stop the service thread */
    ret = pthread_cancel(service_thread_descriptor);
    if (ret != 0) {
        PERR_DEBUG("Cancelling service thread failed");
    }

    /* restore the interrupt mask of the process */ 
    ret = sigprocmask(SIG_SETMASK, &old_sigset, NULL);
    if (ret != 0) {
        PERR_DEBUG("Restoring procmask failed");
    }
}

/* vim: set ts=4 et sw=4: */
