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
#ifndef MOST_COMMON_RT_H
#define MOST_COMMON_RT_H

/**
 * @file most-common-rt.h
 * @ingroup common
 *
 * @brief Common types and definitions used in the real-time part of the MOST driver.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/list.h>

#ifndef RT_RTDM
#   warning "This file is only usable with RTDM driver."
#endif

/**
 * List with a lock operator. The list is protected by a real-time spinlock.
 *
 * Using a read/write spinlock makes no sense in the most driver. Because the
 * read access only takes place in the interrupt service routine.
 */
struct rt_spin_locked_list {
    struct list_head    list;           /**< the list element */
    rtdm_lock_t         lock;           /**< the lock to protect the list */
};


/**
 * Declare a rt_spin_locked_list variable and initialize the list elements
 * statically
 * 
 * @param name the name for the variable
 */
#define RT_SPIN_LOCKED_LIST(name)                                                   \
    struct rt_spin_locked_list name = {                                             \
        .list       = LIST_HEAD_INIT(name.list),                                    \
        .lock       = RTDM_LOCK_UNLOCKED                                            \
    }


/**
 * If @p user_info is NULL, copies from kernel space to kernel space. If @p
 * user_info is not NULL, copy from user space to kernel space.
 *
 * @p src and @p dst must be valid pointers, i.e. checked with
 * rtdm_read_user_ok if it's userspace.
 *
 * @param err the variable where the return value is stored. 0 is success, 
 *        a negative error value (-EFAULT) indicates an error
 * @param user_info the RTDM struct user_info value
 * @param dst the destination
 * @param src the source
 * @param size the number of bytes to copy
 */
#define copy_from_user_or_kernel(err, user_info, dst, src, size)    \
    do {                                                            \
        if (user_info) {                                            \
            err = rtdm_copy_from_user(user_info, dst, src, size);   \
        } else {                                                    \
            memcpy(dst, src, size);                                 \
            err = 0;                                                \
        }                                                           \
    } while (0)

#endif /* MOST_COMMON_RT_H */


/* vim: set ts=4 et sw=4: */
