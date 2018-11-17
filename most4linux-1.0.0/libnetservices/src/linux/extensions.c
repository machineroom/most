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
void LinuxPrintRoutingTable(void)
{
    int  i, j;
    unsigned char buffer[128];

    ParReadBlock(0, 128, buffer);

    pthread_mutex_lock(&g_nets_mutex);
    printf("======= TX (Network Output) =======\n");
    for (i = 0; i < 16; i++) {
        printf("%2X:   ", i*8);
        for (j = 0; j < 8; j++) {
            printf("%2X  ", buffer[i*8 + j]);
        }

        printf("\n");
        if (i == 7) {
            printf("\n======= RX (Source Port Output) =======\n");
        }
    }
    pthread_mutex_unlock(&g_nets_mutex);
}

