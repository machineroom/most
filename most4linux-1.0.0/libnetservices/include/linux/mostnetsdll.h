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
#ifndef MOSTNETSDLL_H
#define MOSTNETSDLL_H

/**
 * Maximum length of a client name string.
 */
#define CLIENT_NAME_MAX         32

/**
 * Error value. Will be returned as integer so TErrorCode as type is never
 * used.
 */
enum TErrorCode {
    E_SUCCESS        = 0,   /**< no error, success */
    E_INVALID_ARG    = -1,  /**< invalid argument */
    E_INVALID_GLOBAL = -3,  /**< invalid global variable (which must be 
                                 set correctly before calling OpenNetServices */
    E_OPEN_DRIVER    = -4,  /**< failed to open the driver */
    E_SIGNAL         = -5,  /**< failed to register or deregister signal 
                                 handlers */
    E_THREAD         = -6,  /**< failed to create service thread */
    E_IOCTL          = -7   /**< ioctl() call to the MOST NetServices driver 
                                 failed */
};

/**
 * Enumeration for the BusType. "T" prefix here because BusType is a global
 * variable and the name should be unchanged because of Windows compatiblity.
 */
enum TBusType {
    BUS_TYPE_ISA,           /**< ISA (not supported) */
    BUS_TYPE_PCI,           /**< PCI: at the moment the only valid value */
    BUS_TYPE_SER,           /**< serial interface (not supported) */
    BUS_TYPE_SIM            /**< simulation (not supported) */
};

/**
 * Global variable that holds the bus type. At the moment the only valid value
 * is BUS_TYPE_PCI.
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 */
extern enum TBusType        BusType;

/**
 * The driver instance to open if using PCI (0..n) and the number of the serial
 * interface on BUS_TYPE_SER (1..n).
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 */
extern int                  InstID;

/**
 * @c true to use the message based interface (available on BUS_TYPE_SER and
 * under Windows CE only) and @c false otherwise. Needs to be @c false on
 * Linux.
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 */
extern bool                 UseMsgInterface;

/**
 * User-defined string for the VMOST interface (only needed is BUS_TYPE_SIM).
 * Unsupported on Linux.
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 */
extern char                 ClientName[CLIENT_NAME_MAX];

/**
 * Opens the NetServices. Before calling this function, you have to set
 * following variables:
 *
 *  - BusType
 *  - InstID
 *  - UseMsgInterface
 *  - ClientName
 *
 * See the description in this header files above or MOST NetServices Layer I,
 * Application Programming Interface, p. 16.
 *
 * @return 
 */
short OpenNetServices(void);

/**
 * Closes the NetServices.
 */
void CloseNetServices(void);

/**
 * New callback function: Will be called after MostStartUp() has been called
 * to notify another part of that library (serivce.c) that MostStartUp() has
 * been called.
 */
void MostStartUpFinished(void);

/**
 * Replacement for MostStartUp.
 *
 * @param mode same meaning as MostStartUp
 * @param opt same meaning as MostStartUp
 */
void MostStartUpOriginal(byte mode, byte opt);

/**
 * Call MostStartUpFinished after MostStartUp has been called. This hack
 * allows us to include additional functionality without modifiying the
 * source code.
 *
 * @param mode same meaning as MostStartUp
 * @param opt same meaning as MostStartUp
 */
#define MostStartUp(mode, opt)                                          \
        do {                                                            \
            MostStartUpOriginal((mode), (opt));                         \
            MostStartUpFinished();                                      \
        } while (0);

/**
 * Extension to print the routing table to stdout. Useful for debugging.
 */
void LinuxPrintRoutingTable(void);

/*
 * In the main file, declare the global variables.
 */
#ifdef _CLIENT_MAIN
enum TBusType       BusType;
int                 InstID;
bool                UseMsgInterface;
char                ClientName[CLIENT_NAME_MAX];
#endif

#endif /* MOSTNETSDLL_H */

/* vim: set ts=4 et sw=4: */
