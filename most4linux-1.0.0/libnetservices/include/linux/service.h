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
#ifndef SERVICE_H
#define SERVICE_H

/**
 * Number of real-time signal to use. Because we don't support LinuxThread
 * implementation but rely on POSIX Threads (NTPL), we can safely use SIGRTMIN
 * here.
 */
#define MOST_RT_SIGNAL          SIGRTMIN

/**
 * The interrupt used for timers.
 */
#define MOST_RT_SIGNAL_TIMER    (SIGRTMIN + 1)


/**
 * Initializes the service thread including the interrupt (= signal) handling.
 * The thread has to be killed with service_thread_finish.
 *
 * @return an error code of type TErrorCode (delared in mostnetsdll.h).
 */
int service_thread_init(void);

/**
 * Deinitializes the service thread including interrupt handling.
 */
void service_thread_finish(void);



#endif /* SERVICE_H */

/* vim: set ts=4 et sw=4: */
