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
#ifndef USP_TEST_H
#define USP_TEST_H

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

/* no user memory */
#define __user

/* define some struct members as int */
#define rwlock_t                int
#define wait_queue_head_t       int

/* use printf for debugging */
#define pr_debugm               printf
#define most_printk             printf
#define pr_err                  printf
#define pr_warning              printf
#define pr_crit                 printf
#define pr_alert                printf
#define pr_emerg                printf
#define pr_rxbuf_debug			printf
#define pr_txbuf_debug			printf

/* suppress compiler warnings */
#define do_nothing              do{} while (0);

/* memory allocation functions */
#define kfree(m)                free(m)
#define ker_malloc(m)           malloc(m)
#define vmalloc(m)              malloc(m)
#define vfree(m)                free(m)

/* memory copying */
static inline int my_memcpy(void *dst, void *src, int size)
{
    memcpy(dst, src, size);
    return 0;
}
#define copy_from_user(a, b, c) \
    my_memcpy(a, b, c)

#define copy_to_user(a, b, c)   \
    my_memcpy(a, b, c)

/* some algorithms */
#define min(a,b)                (((a) < (b)) ? (a) : (b))
#define max(a,b)                (((a) > (b)) ? (a) : (b))

/* no locking in userspace */
#define read_lock_irqsave(a,b)          do_nothing
#define read_unlock_irqrestore(a,b)     do_nothing
#define write_lock_irqsave(a,b)         do_nothing
#define write_unlock_irqrestore(a,b)    do_nothing

/* no initialization for locking function */
#define rwlock_init(a)                  do_nothing
#define init_waitqueue_head(a)          do_nothing

/* no wakeup */
#define wake_up(a)                      do_nothing

/* compile specific */
#define unlikely(x)  __builtin_expect(!!(x), 0)

#endif /* USP_TEST_H */
