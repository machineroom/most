/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */
#ifndef SERIAL_RT_DEBUG_H
#define SERIAL_RT_DEBUG_H

/**
 * @file serial-rt-debug.h
 * @ingroup rtcommon
 *
 * @brief Output debigging information to the serial line (real-time).
 *
 * Can be used by a driver to output debugging information on serial devices.
 * This is necessary in RT because rt_printk() only ouptuts information after
 * the Linux kernel can continue operation, but this is not sufficient in
 * realtime because if the RT part hangs, the output never is displayed.
 *
 * Uses the RTDM sample driver. Configuration is done statically in this header
 * file.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <rtdm/rtserial.h>

/**
 * The serial port, <tt>"rtser0"</tt> is the first serial device, the second
 * port is <tt>"rtser1"</tt>.
 */
#define SERIAL_RT_DEVICE        "rtser0"

/**
 * The baudrate used for the serial connection.
 */
#define SERIAL_RT_BAUDRATE      115200

/**
 * The parity bits used for the serial connection.
 */
#define SERIAL_RT_PARITY        RTSER_NO_PARITY

/**
 * The number of data bits used for the serial connection
 */
#define SERIAL_RT_DATABITS      RTSER_8_BITS

/**
 * The number of stop bits used for the serial connection.
 */
#define SERIAL_RT_STOPBITS      RTSER_1_STOPB

/**
 * The handshake protocol used for the serial connection
 */
#define SERIAL_RT_HANDSHAKE     RTSER_NO_HAND

/**
 * Opaque type that is returned from open and that must be passed on write and close.
 */
typedef int serial_rt_t;

/**
 * Error value for the serial_rt_t type.
 */
#define SERIAL_RT_ERROR     0

#if defined(SERIAL_RT_DEBUG) || defined(DOXYGEN)

/**
 * Initialises serial debugging for a driver.
 *
 * @return the file descriptor that must be passed to write and close.
 */
int serial_rt_debug_init(void);

/**
 * Prints the output on the serial line.
 *
 * @param format the format string, same as printk()
 */
int serial_rt_debug_write(const char *format, ...);

/**
 * Deinitalises serial debugging for a driver.
 */
void serial_rt_debug_finish(void);

#else

#define serial_rt_debug_init() \
    do { } while (0)

#define serial_rt_debug_write(format, arg...) \
    do { } while (0)

#define serial_rt_debug_finish() \
    do { } while (0)

#endif


#endif /* SERIAL_RT_DEBUG_H */

/* vim: set ts=4 et sw=4: */
