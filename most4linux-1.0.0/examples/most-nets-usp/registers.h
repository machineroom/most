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

#ifndef REGISTERS_H
#define REGISTERS_H

/**
 * Interrupt Enable Register
 */
#define MOSTREG_IE                      0x88

/**
 * Node Address Low Register
 */
#define MOSTREG_NAL                     0x8b

/**
 * Node Address High Register
 */
#define MOSTREG_NAH                     0x8a

/**
 * Transceiver Control Register
 */
#define MOSTREG_XCR                     0x80

/**
 * Transceiver Status Register
 */
#define MOSTREG_XSR                     0x81

/**
 * Source Data Control Register 1
 */
#define MOSTREG_SDC1                    0x82

/**
 * Message Control Register
 */
#define MOSTREG_MSGC                    0x85

/**
 * Source Data Control Register 2
 */
#define MOSTREG_SDC2                    0x8c

/**
 * Source Data Control Register 3
 */
#define MOSTREG_SDC3                    0x8d

/**
 * Packet Priority Register 
 */
#define MOSTREG_PPI                     0xf2

/**
 * Clock Manager 1 Register
 */
#define MOSTREG_CM1                     0x83

/**
 * Clock Manager 2 Register
 */
#define MOSTREG_CM2                     0x8e

/**
 * Clock Manager 4 Register
 */
#define MOSTREG_CM4                     0x93

/**
 * Synchronous Bandwidth Control Register 
 */
#define MOSTREG_SBC                     0x96

/**
 * Transceiver Status Register 2
 */
#define MOSTREG_XSR2                    0x97


/**
 * Control message buffer for transmitting control messages
 */
#define MOSTREG_XCMB                    0xc0

/**
 * Transmit status register for control messages 
 */
#define MOSTREG_XTS                     0xd5
#endif /* REGISTERS_H */

/* vim: set sw=4 ts=4 et: */
