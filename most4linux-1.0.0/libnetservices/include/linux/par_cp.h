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
#ifndef PAR_CP_H
#define PAR_CP_H

#ifdef HAVE_CONFIG_H
#include <config/config.h>
#endif

#include "mostnetsdll.h"

/**
 * Performs a single write in parallel mode of control port. On error, a debug
 * message is printed to the console if DEBUG was enabled. (The interface was
 * fixed for Windows compatiblity.)
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 *
 * @param map the address (16 bit, but depends on a preprocessor definition)
 * @param value the value to write
 */
void ParWrite(tMostMap map, byte value);

/**
 * Performs a single read in parallel mode of the control port. On error, a
 * debug message is printed to the console if DEBUG was enabled. (The
 * interface was fixed for Windows compatiblity.)
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 *
 * @param map the address (16 bit, but depends on preprocessor definition)
 * @param value the value to write
 */
byte ParRead(tMostMap map);

/**
 * Performs a block write in parallel mode of control port. On error, a debug
 * message is printed to the console if DEBUG was enabled. (The interface was
 * fixed for Windows compatiblity.)
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 *
 * @param map the address (16 bit, but depends on a preprocessor definition)
 * @param value the value to write
 */
void ParWriteBlock(tMostMap map, byte num, byte *data);

/**
 * Performs a block read in parallel mode of the control port. On error, a
 * debug message is printed to the console if DEBUG was enabled. (The
 * interface was fixed for Windows compatiblity.)
 *
 * See MOST NetServices Layer I, Application Programming Interface, p. 16
 *
 * @param map the address (16 bit, but depends on a preprocessor definition)
 * @param value the value to write
 */
void ParReadBlock(tMostMap map, byte num, byte *data);


#endif /* PAR_CP_H */

/* vim: set ts=4 et sw=4: */
