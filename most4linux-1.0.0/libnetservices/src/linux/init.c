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
#include <limits.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <most-netservice.h>

#include "adjust.h"
#include "mostdef1.h"
#include "mostns1.h"
#include "mostnetsdll.h"

#include "global.h"
#include "service.h"

/* see header file 'global.h' */
int g_control_fd;

/* mutex */
pthread_mutex_t g_nets_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * The device files for the control driver in printf() syntax.
 */
#define NETSERVICE_DEVICE_FILE      "/dev/mostnets%d"

/* -------------------------------------------------------------------------- */
short OpenNetServices(void)
{
    char buffer[PATH_MAX];
    
    PRINT_TRACE();

    /* open the driver */
    snprintf(buffer, PATH_MAX, NETSERVICE_DEVICE_FILE, InstID);
    g_control_fd = open(buffer, O_RDWR);
    if (g_control_fd < 0) {
        PERR_DEBUG("Failed to open control driver");
        return E_OPEN_DRIVER;
    }

    /* initialize the NetServices */
    InitNetServices();

    /* 
     * create the service thread and initialize interrupt handling from
     * userspace 
     */
    service_thread_init();

    PRINT_TRACE("fd = %d", g_control_fd);
	
	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
void CloseNetServices(void)
{
    PRINT_TRACE("Close NetServices, fd = %d", g_control_fd);

    /* destroy the service thread and unregister interrupt handling */
    service_thread_finish();

    close(g_control_fd);
    g_control_fd = 0;
}

/* vim: set ts=4 et sw=4: */
