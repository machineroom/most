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
#ifndef MOST_COMMON_H
#define MOST_COMMON_H

/**
 * @file most-common.h
 * @ingroup common
 *
 * @brief Common types and definitions used in various MOST drivers. 
 *
 * This has nothing to do with MOST, it could be used in any other drivers. 
 * It can also be included in userspace as it defines struct frame_part.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifdef __KERNEL__
#   include <linux/version.h>
#   include <linux/spinlock.h>
#   include <linux/rwsem.h>
#   include <linux/list.h>
#   include <linux/delay.h>
#   include "most-constants.h"
#   include "rt-nrt.h"
#endif

#include <asm/types.h>

/**
 * Part of a MOST frame with offset and length.
 */
struct frame_part {
    __u32    count;         /**< number of bytes */
    __u32    offset;        /**< offset of the first byte */
};


#ifdef __KERNEL__

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,17))
#define kill_proc_info_as_uid(sig, info, pid, uid, euid)    \
    kill_proc_info_as_uid(sig, info, pid, uid, euid)
  
#elif (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18))
#define kill_proc_info_as_uid(sig, info, pid, uid, euid)    \
    kill_proc_info_as_uid(sig, info, pid, uid, euid, 0)
  
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
#define kill_proc_info_as_uid(sig, info, pid, uid, euid)    \
    kill_pid_info_as_uid(sig, info, pid, uid, euid, 0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
/**
 * Define true and false.
 */
enum {
	false	= 0,
	true	= 1
};

/**
 * Define a boolean type. Don't include stdbool.h.
 */
typedef _Bool bool;
#endif

/**
 * Define an unsigned integer type that can hold pointers as in C99.
 */
typedef unsigned long uintptr_t;

/**
 * Define a integer type that can hold pointers as in C99.
 */
typedef long intptr_t;

/**
 * Allocate memory, mainly because of usp-test.h
 */
#define ker_malloc(m) kmalloc(m, GFP_KERNEL)

/**
 * List with a lock operator. The list is protected by a normal spinlock.
 *
 * Before you access the list, call spin_lock() or spin_lock_irqsave() if you use
 * the list in interrupt service routines. After accessing, call spin_unlock() or
 * spin_unlock_irqrestore().
 *
 * Using a read/write spinlock makes no sense in the most driver. Because the
 * read access only takes place in the interrupt service routine.
 */
struct spin_locked_list {
    struct list_head    list;           /**< the list element */
    rtnrt_lock_t        lock;           /**< the spinlock to protect the list */
};


/**
 * Declare a spin_locked_list variable and initialize the list elements
 * statically.
 * 
 * @param name the name for the variable
 */
#define SPIN_LOCKED_LIST(name)                                              \
    struct spin_locked_list name = {                                        \
        .list       = LIST_HEAD_INIT(name.list),                            \
        .lock       = RTNRT_LOCK_UNLOCKED(name.lock)                        \
    }


/**
 * List with a lock operator. The list is protected by a read/write semaphore. 
 *
 * Before you read any data from the list, call down_read(). After reading the data,
 * call up_read(). For write, call down_write() or up_write() respectively. If you use
 * the list in an interrupt service routine, call the _irqsave() variants.
 */
struct rwsema_locked_list
{
    struct list_head    list;           /**< the list header */
    struct rw_semaphore lock;           /**< the rwlock to protect the list */
};

/**
 * Declare a rwsema_locked_list variable and initialize the list elements
 * statically
 * 
 * @param name the name for the variable
 */
#define RWSEMA_LOCKED_LIST(name)                                            \
    struct rwsema_locked_list name = {                                      \
        .list       = LIST_HEAD_INIT(name.list),                            \
        .lock       = __RWSEM_INITIALIZER(name.lock)                        \
    };

/**
 * Checks if @p value is between @p a and @p b, i.e. if a <= value <= b.
 *
 * @param value the value to check
 * @param a the lower bound
 * @param b the upper bound
 * @return @c true if the condition is met, false otherwise
 */
#define is_between_incl(value, a, b)                                        \
    (((value) >= (a)) && ((value) <= (b)))

/**
 * Checks if @p value is between @p a and @p b, i.e. if a < value < b.
 *
 * @param value the value to check
 * @param a the lower bound
 * @param b the upper bound
 * @return @c true if the condition is met, false otherwise
 */
#define is_between_excl(value, a, b)                                        \
    (((value) > (a)) && ((value) < (b)))

/**
 * Sends a signal to a specified process. Calls kill_proc_info_as_uid()
 * internally.
 *
 * @param signo the signal number
 * @param pid the PID of the current process
 * @param data the data value that should be sent with the signal
 * @return 0 on sucess, error code on failure
 */
static inline int most_kill(int signo, struct task_struct* p, int data)
{
    struct siginfo sinfo;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
    pid_t pid;
    pid = p->pid;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
    struct pid *pid;
    pid = task_pid(p);
#endif
    sinfo.si_signo = signo;
    sinfo.si_errno = 0;
    sinfo.si_code  = SI_MESGQ;
    sinfo.si_addr  = 0;
    sinfo.si_value.sival_int = data;

    /* send the signal */
    return kill_proc_info_as_uid(signo, &sinfo, pid, 0, 0);
}


#if defined(DEBUG) || defined(DOXYGEN)
/**
 * Assetion macro. In contrast to BUG_ON(), only prints an error message but
 * doesn't panic. Works in RT and NRT. Only executed if in @c DEBUG mode, so
 * @p expression should not contain any side effects.
 *
 * @param expression the expression that should be checked. If @p expression
 *        is @c true, nothing is done. 
 */
#define assert(expression)                                                  \
       do {                                                                 \
           if (!(expression)) {                                             \
               rtnrt_err("Assertion \"%s\" failed: file \"%s\", line %d\n", \
                       #expression, __FILE__, __LINE__);                    \
           }                                                                \
       } while (0)
#else
#define assert(expression)                                                  \
       do { } while (0)
#endif

#if defined(DEBUG) || defined(DOXYGEN)
/**
 * If @p expression fails, returns from the current function and print a debug
 * message. Only evaluated in @c DEBUG mode, expands to nothing in release
 * mode.
 *
 * This macro is meant as kind of assertions, i.e. a warning is printed if
 * the condition fails. If it's a normal state that @p expression fails,
 * use return_if_fails().
 *
 * @param expression the expression to check
 */
#define return_if_fails_dbg(expression)                                     \
        do {                                                                \
            if (!(expression)) {                                            \
                rtnrt_err("\"%s\" failed: file \"%s\", line %d\n",          \
                       #expression, __FILE__, __LINE__);                    \
                return;                                                     \
            }                                                               \
        } while (0)
#else
#define return_if_fails_dbg(expression)                                     \
        do { } while (0)
#endif

#if defined(REG_ACCESS_DEBUG) || defined(DOXYGEN)
/**
 * Low-level register access debugging
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_reg_access_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_reg_access_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(IOCTL_DEBUG) || defined(DOXYGEN)
/**
 * ioctl() debugging.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_ioctl_debug(fmt, arg...)  \
        rtnrt_debug(fmt,##arg)
#else
#   define pr_ioctl_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(DEVFUNC_DEBUG) || defined(DOXYGEN)
/**
 * Device function debugging.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_devfunc_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_devfunc_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(IRQ_DEBUG) || defined(DOXYGEN)
/**
 * Debugging in interrupt service routines 
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_irq_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_irq_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(RT_IRQ_DEBUG) || defined(DOXYGEN)
/**
 * Debugging in real-time interrupt service routines 
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_rt_irq_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_rt_irq_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(TXBUF_DEBUG) || defined(DOXYGEN)
/**
 * Debugging function for txbuf.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_txbuf_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_txbuf_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(RXBUF_DEBUG) || defined(DOXYGEN)
/**
 * Debugging function for rxbuf.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_rxbuf_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_rxbuf_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(SYNC_DEBUG) || defined(DOXYGEN)
/**
 * Debugging function for most-sync.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_sync_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_sync_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(NETS_DEBUG) || defined(DOXYGEN)
/**
 * Debugging function for most-netservice.
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_nets_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_nets_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(ALSA_DEBUG) || defined(DOXYGEN)
/**
 * Debugging function for the ALSA driver
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define pr_alsa_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_alsa_debug(fmt, arg...) \
        do { } while (0)
#endif

#if defined(MEASURING_SCHED) || defined(MEASURING_PCI) || defined(DOXYGEN)
/**
 * Prints debug messages that are needed for measurings.
 *
 * @param fmt the format string
 * @param arg the arguments for the format string
 */
#define pr_measure_debug(fmt, arg...) \
        rtnrt_debug(fmt,##arg)
#else
#define pr_measure_debug(fmt, arg...) \
        do { } while (0)
#endif

/**
 * If @p expression failes, returns @p val from the current function and print
 * a debug message. Only evaluated in @c DEBUG mode, expands to nothing in
 * release mode.
 *
 * This macro is meant as kind of assertions, i.e. a warning is printed if
 * the condition fails. If it's a normal state that @p expression fails,
 * use return_value_if_fails().
 *
 * @param expression the expression to check
 * @param value the value to return
 */
#ifdef DEBUG
#   define return_value_if_fails_dbg(expression, value)                     \
       do {                                                                 \
           if (!(expression)) {                                             \
               rtnrt_err("\"%s\" failed: file \"%s\", line %d\n",           \
                      #expression, __FILE__, __LINE__);                     \
               return value;                                                \
           }                                                                \
       } while (0)
#else
#   define return_value_if_fails_dbg(expression, value)                     \
        do { } while (0)
#endif


/**
 * If @p expression fails, return from the current function. This macro should
 * be used if a failure of @p expression is a valid program state.
 *
 * @param expression the expression to check
 */
#define return_if_fails(expression)                                         \
    do {                                                                    \
        if (!(expression)) {                                                \
            return;                                                         \
        }                                                                   \
    } while (0)


/**
 * If @p expression fails, returns @p val from the current function. This macro
 * should be used if a failure of @p expression is a valid program state.
 *
 * @param expression the expression to check
 * @param value the value to return
 */
#define return_value_if_fails(expression, value)                            \
      do {                                                                  \
          if (!(expression)) {                                              \
              return value;                                                 \
          }                                                                 \
      } while (0)

/**
 * Swaps bytes. Needed to convert big endian to little endian.
 * From ./drivers/media/dvb/ttusb-dec/ttusb_dec.c
 *
 * @param[in,out] b the bytes to swap
 * @param[in] length the length
 */
static inline void swap_bytes(u8 *b, int length)
{
    u8 c;

        length -= length % 2;
        for (; length; b += 2, length -= 2) {
            c = *b;
                *b = *(b + 1);
                *(b + 1) = c;
        }
}


#endif /* __KERNEL__ */

#endif /* MOST_COMMON_H */


/* vim: set ts=4 et sw=4: */
