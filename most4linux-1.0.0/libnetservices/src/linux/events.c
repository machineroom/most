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
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <adjust.h>
#include <mostdef1.h>
#include <mostns1.h>
#include <mostnetsdll.h>
#include <par_cp.h>

#include "global.h"
#include "most-netservice.h"
#include "service.h"


/* -------------------------------------------------------------------------- */
void MnsRequest(word flags)
{
    union sigval val;

    PRINT_TRACE("flags = %d", flags);

    val.sival_int = MNS_INT;

    /*
     * send the current process a MOST_RT_SIGNAL signal with a MNS_INT value
     * because that's the same as an interrupt
     *
     * The process blocks the signal and the service thread gets notified
     */
    sigqueue(getpid(), MOST_RT_SIGNAL, val);
}

/* -------------------------------------------------------------------------- */
void MnsRequestTimer(void)
{
    union sigval val;

    PRINT_TRACE();

    val.sival_int = 0;

    /* ensure that si_value.sigval_int is zero */
    sigqueue(getpid(), MOST_RT_SIGNAL_TIMER, val);
}

/* -------------------------------------------------------------------------- */
byte Most_Por_Int(void)
{
    int ret;

    PRINT_TRACE();
    CHECK_FILEDESCRIPTOR_RET_VAL(-1);

    ret = ioctl(g_control_fd, MOST_NETS_READ_INT);
    if (ret < 0) {
        PERR_DEBUG("Reading interrupt register failed");
        return FALSE;
    }

    PRINT_TRACE("Interrupt status: %s", ret ? "active" : "not active");

    return ret;
}

/* -------------------------------------------------------------------------- */
void MostResetInt(byte mask)
{
    int ret;

    PRINT_TRACE();
    CHECK_FILEDESCRIPTOR_RET();

    ret = ioctl(g_control_fd, MOST_NETS_IRQ_RESET, &mask);
    if (ret != 0) {
        PERR_DEBUG("MOST_NETS_IRQ_RESET failed");
    }
}

/* vim: set ts=4 et sw=4: */
