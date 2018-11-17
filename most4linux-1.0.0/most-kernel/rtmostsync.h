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
#ifndef RTMOSTSYNC_H
#define RTMOSTSYNC_H

/**
 * @file rtmostsync.h
 * @ingroup rtsync
 *
 * @brief RTDM Profile for MOST Synchronous devices
 *
 * Defines a device group for MOST Synchronous devices. It doesn't make sense to define a
 * group for MOST in general, because the MOST NetServices part is handled by Linux in a
 * non-realtime way and a device profile should determine how the device is programmed,
 * and a MOST Synchronous device has nothing to do with a MOST Asynchronous device
 * in its programming way. 
 *
 * While a synchronous device handles stream data and is accessed via device files in the
 * Linux driver and the usual read/write calls in the real-time driver, an asynchronous
 * device handles network data and is programmed with sockets.
 *
 * @par Device Characteristics
 * Device Flags: @c RTDM_NAMED_DEVICE @n
 * @n
 * Device Name: @c "mostsync<N>", N >= 0 @n
 * @n
 * Device Class: @c RTDM_CLASS_MOSTSYNC @n
 * @n
 *
 * @par Supported Operations
 * @b Open @n
 * Environments: non-RT @n
 * Specific return values: none @n
 * @n
 * @b Close @n
 * Environments: non-RT @n
 * Specific return values: none @n
 * @n
 * @b IOCTL @n
 * Environments: non-RT @n
 * See @ref IOCTLs "below" @n
 * @n
 * @b Read @n
 * Environments: RT @n
 * Specific return values:
 * - -EINT (interrupted explicitly or by signal)
 * - -EAGAIN (no data available in non-blocking mode)
 * .
 * @n
 * @b Write @n
 * Environments: RT @n
 * - -EINT (interrupted explicitly or by signal)
 * - -EAGAIN (no data available in non-blocking mode)
 *
 * @{
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include "most-common.h"

/**
 * The device class for Synchronous MOST devices. Must be synchronised with
 * addons/rtdm/rtdm.h or RTAI.
 */
#define RTDM_CLASS_MOSTSYNC         15

/**
 * Use the RTDM_CLASS_MOSTSYNC magic number for IOCTLs.
 */
#define MOST_SYNC_RT_IOCTL_MAGIC    RTDM_CLASS_MOSTSYNC

/**
 * @anchor IOCTLs @name IOCTLs
 * MOST Synchronous IOCTLs
 * @{ */

/**
 * @copydoc MOST_SYNC_SETUP_RX
 * 
 * This service can be called from:
 * - Kernel module initialization/cleanup code
 * - User-space task (non-RT)
 */
#define MOST_SYNC_RT_SETUP_RX     _IOW(MOST_SYNC_RT_IOCTL_MAGIC, 0, struct frame_part)

/**
 * @copydoc MOST_SYNC_SETUP_TX
 *
 * This service can be called from:
 * - Kernel module initialization/cleanup code
 * - User-space task (non-RT)
 */
#define MOST_SYNC_RT_SETUP_TX     _IOW(MOST_SYNC_RT_IOCTL_MAGIC, 1, struct frame_part)

/**
 * The maximum ioctl number. This value may change in future.
 */
#define MOST_SYNC_RT_MAXIOCTL     1

/** @} */

/*!
 * @name Sub-Classes of RTDM_CLASS_MOSTSYNC
 *
 * Currently prepared only for this one subclass. Device naming mechanism
 * must be changed if more subclasses are used.
 *
 * This way, it's consitent with the Linux MOST driver.
 * @{ */
#define RTDM_SUBCLASS_MOSTSYNC_OASIS        0
/** @} */

/** @} */

#endif /* RTMOSTSYNC_H */

/* vim: set ts=4 et sw=4: */
