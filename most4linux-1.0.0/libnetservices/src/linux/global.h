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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"

/**
 * The file descriptor for the global control driver.
 */
extern int g_control_fd;

/**
 * Mutex for locking MOST NetServices because of multi-threading */
extern pthread_mutex_t g_nets_mutex;

/**
 * Checks if the global file descriptor fd is valid (i.e. not zero since the 
 * CloseNetServices function sets it to zero if it closes the function).
 * If it is invalid, it prints a debug message and returns.
 *
 * This macro is for void functions that have no return value.
 */
#define CHECK_FILEDESCRIPTOR_RET()                                              \
    do {                                                                        \
        if (g_control_fd == 0) {                                                \
            PRINT_DBG("File descriptor invalid. Library was not initalized");   \
            return;                                                             \
        }                                                                       \
    } while (FALSE)

/**
 * Checks if the file descriptor fd is valid (i.e. not zero since the
 * CloseNetServices function sets it to zero if it closes the function).
 * If it is invalid, it prints a debug message and returns the specified
 * value. 
 *
 * This macro is vor functions that have a return value.
 *
 * @param fd the file descriptor
 * @param val the value that is returned if the file descriptor is invalid.
 */
#define CHECK_FILEDESCRIPTOR_RET_VAL(val)                                       \
    do {                                                                        \
        if (g_control_fd == 0) {                                                \
            PRINT_DBG("File descriptor invalid. Library was not initalized");   \
            return val;                                                         \
        }                                                                       \
    } while (FALSE)


/**
 * Stores the time in milliseconds since the epoch in the 64 bit variable.
 *
 * @param time a 64 bit variable (unsigned long long) to store the time
 */
#define TIME_IN_MS(time)                                                        \
    do {                                                                        \
        struct timeval tmp;                                                     \
        gettimeofday(&tmp, NULL);                                               \
        time = (unsigned long long)tmp.tv_sec * 1000 + tmp.tv_usec / 1000;      \
    } while(0);



#endif /* GLOBAL_H */

/* vim: set ts=4 et sw=4: */
