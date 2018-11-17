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
#ifndef RT_NRT_H
#define RT_NRT_H

/**
 * @file rt-nrt.h
 * @ingroup rtnrt
 *
 * @brief Common interface for RT / NRT issues
 *
 * This header provides a common interface between RTDM and Linux code.
 * See the sections for more details. The sectioning was choosed to be the same
 * (whenever possible and sensible) like the Xenomai API documentation.
 *
 * It also includes some macros for compatiblity between different RTDM
 * versions.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#ifdef RT_RTDM
#   include <rtdm/rtdm_driver.h>
#   include "rtseqlock.h"
#   ifdef XENO_VERSION_NAME
#       include <native/timer.h>
#   endif
#endif

/**
 * @anchor Utilities @name Utilities
 *
 * This functions don't belong in another category.
 * @{
 */ /* {{{ */

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Executes @p rt if compiled with @c RT_RTDM and @p nrt otherwise.
 * This could be used with NRT signalling services for example like:
 *
 * @param[in] rt executed if compiled with @c RT_RTDM
 * @param[in] nrt executed if compiled without @c RT_RTDM
 */
#define rt_nrt_exec(rt, nrt)                                \
    rt

#if defined(XENO_VERSION_NAME) || defined(DOXYGEN)
/**
 * Sets the timer to oneshot mode. On Linux, it does nothing. This function
 * was developed to be able to create simple kernel modules (like using the
 * rtnrt_clock_read() function) without calling native services.
 *
 * Does following:
 *
 *  - On RTAI, it calls start_rt_timer(0)
 *  - On Xenomai, it calls rt_timer_set_mode(TM_ONESHOT)
 *  - On Linux, it does nothing
 *
 * @return 0 on success or a negative error value on failure
 */
#define rtnrt_start_timer_oneshot()                         \
    rt_timer_set_mode(TM_ONESHOT)
#elif defined(RTAI_RELEASE)
#define rtnrt_start_timer_oneshot()                         \
    start_rt_timer(0)
#else
#error "Unsupported operating system for RTDM"
#endif

#else

#define rt_nrt_exec(rt, nrt)                                \
    nrt

#define rtnrt_start_timer_oneshot()                         \
    0

#endif /* RT_RTDM */

/** @} */ /* }}} */


/**
 * @anchor Debugging @name Debug Messages Service
 *
 * These functions provide debug messages that use rtdm_printk() if compiled
 * with @c RT_RTDM and printk() otherwise.
 * 
 * Because it's safe to call rtdm_printk() if available also from Linux
 * context, it is possible to implement that this way.
 *
 * @{
 */ /* {{{ */

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Debug macro usable for normal Linux drivers as well as RTDM drivers
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments
 */
#define rtnrt_printk(fmt, arg...) \
        rtdm_printk(fmt, ##arg)
#else

#define rtnrt_printk(fmt, arg...) \
        printk(fmt, ##arg)

#endif

#if defined(DEBUG) || defined(DOXYGEN)

/**
 * pr_debug replacement usable for normal Linux drivers as well as RTDM drivers
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments
 */
#define rtnrt_debug(fmt, arg...) \
        rtnrt_printk(KERN_DEBUG fmt, ##arg)
#else

#define rtnrt_debug(fmt, arg...) \
        do { } while (0)
#endif

/**
 * pr_info replacement usable for normal Linux drivers as well as RTDM drivers
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments
 */
#   define rtnrt_info(fmt, arg...) \
        rtnrt_printk(KERN_INFO fmt, ##arg)


/**
 * normal but significant condition
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_notice(fmt, arg...) \
    rtnrt_printk(KERN_NOTICE fmt,##arg)

/**
 * warning conditions
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_warn(fmt, arg...) \
    rtnrt_printk(KERN_WARNING "- " fmt,##arg)

/**
 * error conditions
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_err(fmt, arg...) \
    rtnrt_printk(KERN_ERR "* " fmt,##arg)

/**
 * critical conditions
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_crit(fmt, arg...) \
    rtnrt_printk(KERN_CRIT "# " fmt,##arg)

/**
 * action must be taken immediately
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_alert(fmt, arg...) \
    rtnrt_printk(KERN_ALERT "!!!! " fmt,##arg)

/**
 * system is unusable
 *
 * @param[in] fmt the format string
 * @param[in] arg the arguments for the format string
 */
#define rtnrt_emeg(fmt, arg...) \
    rtnrt_printk(KERN_EMERG "***!!!!*** " fmt,##arg)


/** @} */ /* }}} */


/**
 * @anchor Spinlocks @name Spinlocks
 * 
 * In some situations, if compiled with real-time support there should be
 * used RTDM spinlocks and if compiled on Linux there should be used Linux
 * spinlocks. 
 *
 * This makes sense in two cases:
 *  - @b Linux: The code path is used in task context and interrupt context.<br>
 *    @b RTDM: The code path is used in Linux task context and RTDM interrupt 
 *    context.
 *  - @b Linux: The code path is used in task context and interrupt context.<br>
 *    @b RTDM: The code path is used in RTDM task context and RTDM interrupt
 *    context.
 *
 * In these two cases the _irqsave() functions are used in task context
 * Because Linux cannot be preemted if interrupts are stalled, there's no
 * need to disable preemption in that case.
 *
 * @{ */ /* {{{ */
#if defined(CONFIG_PREEMPT_RT) || defined(DOXYGEN)

/**
 * Defines a spinlock statically. Used because the RT_PREEMPT patch requires an
 * argument which containes the lock name and the normal Linux implementation
 * doesn't. Same meaning as SPIN_LOCK_UNLOCKED.
 *
 * @param[in,out] lock the name of the lock
 */
#   define RTNRT_LSPINLOCK_UNLOCKED(lock)                  \
        SPIN_LOCK_UNLOCKED(lock)
#else
#   define RTNRT_LSPINLOCK_UNLOCKED(lock)                  \
        SPIN_LOCK_UNLOCKED
#endif


#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Static lock initialisation.
 *
 * @param[in,out] lock the lock variable
 */
#define RTNRT_LOCK_UNLOCKED(lock)                          \
    RTDM_LOCK_UNLOCKED

/**
 * Dynamic lock initialisation.
 *
 * @param[in,out] lock the lock variable
 */
#define rtnrt_lock_init(lock)                               \
    rtdm_lock_init(lock)

/**
 * Acquire lock from non-preemptible contexts.
 *
 * @param[in,out] lock the lock variable
 */
#define rtnrt_lock_get(lock)                                \
    rtdm_lock_get(lock)

/**
 * Release lock without preemption restoration.
 *
 * @param[in,out] lock the lock variable
 */
#define rtnrt_lock_put(lock)                                \
    rtdm_lock_put(lock)

/**
 * Acquire lock and disable preemption.
 *
 * @param[in,out] lock the lock variable
 * @param[out] flags the context, should be type rtnrt_lockctx_t
 */
#define rtnrt_lock_get_irqsave(lock, flags)                 \
    rtdm_lock_get_irqsave(lock, flags)

/**
 * Release lock and restore preemption state.
 *
 * @param[in,out] lock the lock variable
 * @param[in] flags the context, should be type rtnrt_lockctx_t
 */
#define rtnrt_lock_put_irqrestore(lock, flags)              \
    rtdm_lock_put_irqrestore(lock, flags)

/**
 * Lock variable.
 */
typedef rtdm_lock_t rtnrt_lock_t;

/**
 * Variable to save the context
 */
typedef rtdm_lockctx_t rtnrt_lockctx_t;

#else /* NRT variant */

#if defined(CONFIG_PREEMPT_RT)
#   define RTNRT_LOCK_UNLOCKED(lock)                        \
        SPIN_LOCK_UNLOCKED(lock)
#else /* not CONFIG_PREEMPT_RT */
#   define RTNRT_LOCK_UNLOCKED(lock)                        \
        SPIN_LOCK_UNLOCKED
#endif /* CONFIG_PREEMPT_RT */

#define rtnrt_lock_init(lock)                               \
    spin_lock_init(lock)

#define rtnrt_lock_get(lock)                                \
    spin_lock(lock)

#define rtnrt_lock_put(lock)                                \
    spin_unlock(lock)

#define rtnrt_lock_get_irqsave(lock, flags)                 \
    spin_lock_irqsave(lock, flags)

#define rtnrt_lock_put_irqrestore(lock, flags)              \
    spin_unlock_irqrestore(lock, flags)

typedef spinlock_t rtnrt_lock_t;

typedef unsigned long rtnrt_lockctx_t;

#endif

/** @} */ /* }}} */

/**
 * @anchor seqlocks @name Sequence Locks
 *
 * Because sequence locks are based on spinlocks it makes also sense to provide
 * sequence locks in the RTNRT framework. The usage condition is the same as 
 * for the spinlocks above.
 *
 * @{ */ /* {{{ */
#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Initialises a sequence lock statically.
 */
#define RTNRT_SEQLOCK_UNLOCKED                       \
    RT_SEQLOCK_UNLOCKED

/**
 * Initialises a sequence lock dynamically.
 */
#define rtnrt_seqlock_init(x)                        \
    rt_seqlock_init(x)

/**
 * Locks out other writers and updates the count.
 *
 * @param[in,out] sl the sequence lock
 */
#define rtnrt_write_seqlock(sl)                      \
    rt_write_seqlock(seqlock_t *sl)

/**
 * Unlocks other writers and updates the count again.
 *
 * @param[in,out] sl the sequence lock
 */
#define rtnrt_write_sequnlock(sl)                    \
    rt_write_sequnlock(seqlock_t *sl)

/**
 * Start of read calculation -- fetch last complete writer token 
 *
 * @param[in,out] sl the sequence lock
 */
#define rtnrt_read_seqbegin(sl)                      \
    rt_read_seqbegin(sl)

/**
 * Test if reader processed invalid data.
 *
 * @param[in,out] sl the sequence lock
 * @param[in,out] iv the value that was fetched
 */
#define rtnrt_read_seqretry(sl, iv)                   \
    rt_read_seqretry(sl, iv)

/** 
 * Same as rtnrt_write_seqlock() but with IRQ protection.
 *
 * @param[in,out] sl the sequence lock
 * @param[out] flags the flags of type @c rtdm_lockctx_t that are safed
 */
#define rtnrt_write_seqlock_irqsave(sl, flags)        \
    rt_write_seqlock_irqsave(sl, flags)

/**
 * Same as rtnrt_write_sequnlock() but with IRQ protection.
 *
 * @param[in,out] sl the sequence lock
 * @param[in] flags the flags of type @c rtdm_lockctx_t that are safed
 */
#define rtnrt_write_sequnlock_irqrestore(sl, flags)   \
    rt_write_sequnlock_irqrestore(sl, flags)

/**
 * Basic type for sequence locks.
 */
typedef rt_seqlock_t rtnrt_seqlock_t;

#else

#define RTNRT_SEQLOCK_UNLOCKED                       \
    SEQLOCK_UNLOCKED

#define rtnrt_seqlock_init(x)                        \
    seqlock_init(x)

#define rtnrt_write_seqlock(sl)                      \
    write_seqlock(seqlock_t *sl)

#define rtnrt_write_sequnlock(sl)                    \
    write_sequnlock(seqlock_t *sl)

#define rtnrt_read_seqbegin(sl)                      \
    read_seqbegin(sl)

#define rtnrt_read_seqretry(sl, iv)                  \
    read_seqretry(sl, iv)

#define rtnrt_write_seqlock_irqsave(sl, flags)       \
    write_seqlock_irqsave(sl, flags)

#define rtnrt_write_sequnlock_irqrestore(sl, flags)  \
    write_sequnlock_irqrestore(sl, flags)

typedef seqlock_t rtnrt_seqlock_t;



#endif
/** @} */ /* }}} */

/**
 * @anchor clock @name Clock Services
 *
 * This service provides a relative nano-seconds timestamp on both platforms --
 * Linux and RTDM. However, the value returned if compiled with @c RT_RTDM is not
 * comparable to the value compiled without @c RTDM.
 *
 * If compiled with @c RT_RTDM, the function can be called also in Linux
 * context. See also @ref taskservices.
 *
 * @{
 */ /* {{{ */

/**
 * This constant is missing in the kernel sources. Maybe it'll be added
 * in future, so make this optional. Name should be obvious. <tt>:)</tt>
 */
#if !defined(NSEC_PER_MSEC) || defined(DOXYGEN)
#define NSEC_PER_MSEC       1000000L
#endif

#if (defined(RTDM_API_VER) && (RTDM_API_VER < 5)) || defined (DOXYGEN) \
    || !defined(RTDM_API_VER)
/**
 * RTDM type for representing absolute dates. Its base type is a 64 bit.
 * unsigned integer. The unit is 1 nanosecond. 
 *
 * RTDM_API_VER 5 already defines this type.
 */
typedef uint64_t nanosecs_abs_t;

/**
 * TDM type for representing relative intervals. Its base type is a 64 bit.
 * signed integer. The unit is 1 nanosecond. Relative intervals can also.
 * encode the special timeouts "infinite" and "non-blocking", see
 * RTDM_TIMEOUT_xxx.
 *
 * RTDM_API_VER 5 already defines this type.
 */
typedef uint64_t nanosecs_rel_t;

#endif

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Get a system timestamp in nanoseconds. Uses getnstimeofday() on Linux so
 * the precision is only microseconds.
 *
 * It's important that timers have been startet if using in real-time mode.
 * On RTAI, the timers must run in @e oneshot mode to give precise timing
 * information. If running in @e periodic mode, the timestamp returned by this
 * function is only as accurate as the time periods are. So if your timers
 * are running in a period of one second, the precision of this function is
 * is only one second even if the number is nanosecond.
 *
 * The function rtnrt_start_timer_oneshot() can be used. It works on all
 * three platforms (RTAI, Xenomai, Linux).
 *
 * @return the system time of type nanoseconds_ab
 */
static inline nanosecs_abs_t rtnrt_clock_read(void)
{
    return rtdm_clock_read();
}

#else

static inline nanosecs_abs_t rtnrt_clock_read(void)
{
    struct timespec ts;

    getnstimeofday(&ts);
    return (nanosecs_abs_t)ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec;
}

#endif

/** @} */ /* }}} */

/**
 * @anchor taskservices @name Task Services
 *
 * Services for managing tasks such as creation, waiting, etc.
 * It also includes a sleeping function and a function for busy waiting.
 *
 * @{
 */ /* {{{ */

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Suspends the current task until @p millisecs are elapsed. The sleeping
 * can be interrupted by a signal.
 *
 * @param[in] millisecs the time unit in milli seconds to sleep
 * @return 0 on success, otherwise
 *  - -EPERM @e may be returned if an illegal invocation environment is.
 *    detected..
 *  - a postive integer value is returned if the task was interrupted. It
 *    holds the number of milliseconds remaining in the originally expected
 *    sleep period.
 */
static inline int rtnrt_task_sleep(unsigned int millisecs)
{
    nanosecs_abs_t      sleep_until;
    nanosecs_rel_t      delay;
    int                 ret;

    delay = millisecs * NSEC_PER_MSEC;
    sleep_until = rtnrt_clock_read() + delay;

    ret = rtdm_task_sleep(delay);
    if (ret == -EINTR) {
        uint64_t time_left = sleep_until - rtnrt_clock_read();
        do_div(time_left, NSEC_PER_MSEC);

        /* return not zero because of rounding issues */
        return time_left == 0 ? 1 : time_left; 
    }

    return ret;
}

/**
 * Busy-wait a specified amount of time
 *
 * Implemented as macro because the Linux implementation can check the delay
 * (if it's a constant) at compile-time.
 *
 * @note The caller must not be migratable to different CPUs while executing.
 * this service. Otherwise, the actual delay will be undefined.
 *
 * This service can be called from:
 *  - Kernel module initialization/cleanup code.
 *  - Interrupt service routine (should be avoided or kept short).
 *  - Kernel-based task.
 *
 * @param[in] delay_nsecs the delay time in nanoseconds
 */
#define rtnrt_ndelay(delay_nsecs)                           \
    rtdm_task_busy_sleep(delay_nsecs)

/**
 * Same as rtnrt_ndelay() but with microseconds. Usage of this function should
 * be preferred over using rtnrt_ndelay() * 1000 because Linux provides different
 * implementations and if compiled without @c RT_RTDM.
 *
 * @param[in] delay_usecs the delay time in microseconds
 */
#define rtnrt_udelay(delay_usecs)                           \
    rtdm_task_busy_sleep((delay_usecs) * NSEC_PER_USEC)

/**
 * Same as rtnrt_ndelay() but with microseconds. Usage of this function should
 * be preferred over using rtnrt_ndelay() * 1000 because Linux provides different
 * implementations and if compiled without @c RT_RTDM.
 *
 * @param[in] delay_msecs the delay time in milliseconds
 */
#define rtnrt_mdelay(delay_msecs)                           \
    rtdm_task_busy_sleep((delay_msecs) * NSEC_PER_MSEC)

#else

static inline int rtnrt_task_sleep(unsigned int millisecs)
{
    return msleep_interruptible(millisecs);
}

#define rtnrt_ndelay(delay_nsecs)                           \
    ndelay(delay_nsecs)
#define rtnrt_udelay(delay_usecs)                           \
    udelay(delay_usecs)
#define rtnrt_mdelay(delay_msecs)                           \
    mdelay(delay_msecs)

#endif

/** @} */ /* }}} */

/**
 * @anchor memcpy @name Memory Copying
 *
 * Memory copying functions that work
 *
 *  - in Linux
 *  - in RTDM called from userspace
 *  - in RTDM called from kernelspace
 *
 * These functions are used in the buffer implementation of the MOST driver.
 *
 * @{
 */ /* {{{ */

/**
 * Function that copies memory from one place to another. This function must be
 * provided by the caller of this function. Normally, it's some sort of "glue"
 * function that calls copy_to_user(), copy_from_user() or memcpy() internally. 
 *
 * @param[out] to destination (kernel space or user space)
 * @param[in] from source (kernel space or user space)
 * @param[in] count number of bytes to copy
 * @param[in] cookie an additional pointer which can be used if the real memcopy
 *        function requires an additional argument
 * @return the number of bytes that not have been copied successfully, so on
 *         success the function returns 0, on total failure count and in other
 *         cases a value between 0 and count exclusively.
 */ 
typedef unsigned long (*rtnrt_memcopy_func)(void              *to, 
                                            const void        *from, 
                                            unsigned long     count,
                                            void              *cookie);


/**
 * Descriptor that describes a memory copy operation. It consits of a function
 * and additional data. If the function doesn't need the data, @p cookie should
 * be @p NULL.
 */
struct rtnrt_memcopy_desc {
    rtnrt_memcopy_func  function;       /**< the function, see rtnrt_memcopy_func */
    void                *cookie;        /**< the value passed to the @p cookie
                                             argument of @p function */
};

/**
 * Wrapper for memmove to be compinant to the rtnrt_memcopy_func signature.
 *
 * @param[out] to the destination (must be in kernel space)
 * @param[in] from the source (must be in kernel space)
 * @param[in] count the number of bytes to copy
 * @param[in] cookie ignored here
 * @return always 0
 */
static inline unsigned long rtnrt_memmove(void              *to,
                                         const void         *from,
                                         unsigned long      count,
                                         void               *cookie)
{
    memmove(to, from, count);
    return 0;
}

/**
 * Wrapper for copy_to_user to be compinant to the rtnrt_memcopy_func signature.
 *
 * @param[out] to the destination (must be in kernel space)
 * @param[in] from the source (must be in kernel space)
 * @param[in] count the number of bytes to copy
 * @param[in] cookie ignored here
 * @return always 0
 */
static inline unsigned long rtnrt_copy_to_user(void              *to, 
                                              const void         *from,
                                              unsigned long      count,
                                              void               *cookie)
{
    return copy_to_user(to, from, count);
}

/**
 * Wrapper for copy_from_user to be compinant to the rtnrt_memcopy_func
 * signature.
 *
 * @param[out] to the destination (must be in kernel space)
 * @param[in] from the source (must be in kernel space)
 * @param[in] count the number of bytes to copy
 * @param[in] cookie ignored here
 * @return the number of bytes that not have been copied successfully, so on
 *         success the function returns 0, on total failure count and in other
 *         cases a value between 0 and count exclusively
 */
static inline unsigned long rtnrt_copy_from_user(void              *to, 
                                                const void         *from, 
                                                unsigned long      count,
                                                void               *cookie)
{
    return copy_from_user(to, from, count);
}

#ifdef RT_RTDM

/**
 * Wrapper for rtnrt_copy_to_user to be compinant to the rtnrt_memcopy_func
 * signature.  Only available if @c RT_RTDM is defined.
 *
 * @param[out] to the destination (must be in kernel space)
 * @param[in] from the source (must be in kernel space)
 * @param[in] count the number of bytes to copy
 * @param[in] cookie ignored here
 * @return the number of bytes that not have been copied successfully, so on
 *         success the function returns 0, on total failure count and in other
 *         cases a value between 0 and count exclusively
 */
static inline unsigned long rtnrt_copy_to_user_rt(void              *to, 
                                                 const void         *from, 
                                                 unsigned long      count,
                                                 void               *cookie)
{
    rtdm_user_info_t *user_info = (rtdm_user_info_t *)cookie;
    
    if (user_info) {
        unsigned long ret;
        
        ret = rtdm_copy_to_user(user_info, to, from, count);
        return (ret < 0) ? count : 0;
    } else {
        memcpy(to, from, count);
        return 0;
    }
}

/**
 * Wrapper for rtdm_copy_from_user to be compinant to the rtnrt_memcopy_func
 * signature.  Only available if @c RT_RTDM is defined.
 *
 * @param[out] to the destination (must be in kernel space)
 * @param[in] from the source (must be in kernel space)
 * @param[in] count the number of bytes to copy
 * @param[in] cookie ignored here
 * @return the number of bytes that not have been copied successfully, so on
 *         success the function returns 0, on total failure count and in other
 *         cases a value between 0 and count exclusively
 */
static inline unsigned long rtnrt_copy_from_user_rt(void              *to, 
                                                   const void         *from, 
                                                   unsigned long      count,
                                                   void               *cookie)
{
    rtdm_user_info_t *user_info = (rtdm_user_info_t *)cookie;

    if (user_info) {
        unsigned long ret;
        
        ret = rtdm_copy_from_user(user_info, to, from, count);
        return (ret < 0) ? count : 0;
    } else {
        memcpy(to, from, count);
        return 0;
    }
}

#endif

/**
 * Macro to call the function from a struct rtnrt_memcopy_desc.
 *
 * @param[out] desc the pointer to struct rtnrt_memcopy_desc
 * @param[in] to the destination
 * @param[in] from the source
 * @param[in] count the number of bytes to copy
 * @return the number of bytes that not have been copied successfully, so on
 *         success the function returns 0, on total failure count and in other
 *         cases a value between 0 and count exclusively.
 */
#define rtnrt_copy(desc, to, from, count)   \
    (desc)->function(to, from, count, (desc)->cookie)

/** @} */ /* }}} */

/**
 * @anchor nrtsig @name Non real-time signalling services
 *
 * Non real-time signalling services are used in RTDM to propagate events
 * from real-time to non real-time. This is especially handy when a 
 * interrupt service routine runs in real-time context but needs to
 * trigger Linux services.
 *
 * This macros are provided to use this meachnism in RT and NRT. 
 * This is done in following way:
 *
 *  - In Linux, the variable the inisialisation function and the cleanup
 *    function do noting. The handler gets called immediately where the
 *    event is triggered (the function is just inlined).
 *
 *  - In RTDM, all function expand to their RTDM counterparts. The
 *    handler gets called normally after Linux is scheduled.
 *
 * The handler should look like
 * @code
 * static inline void nrt_handler(rtnrt_nrtsig_t arg)
 * {
 *     // arg is not usable
 *     // do actions
 * }
 * @endcode
 *
 * @{
 */ /* {{{ */

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Defines a non-real-time signalling service if compiled with
 * RTDM. If not, expands to an empty statement. It will be defined
 * as @c static.
 *
 * @param[in] name the variable name
 */
#define DEFINE_NRTSIG(name)                          \
    static rtdm_nrtsig_t name

/**
 * If compiled with @c RT_RTDM, registers a non-real-time signal handler.
 * If not, does nothing.
 *
 * @param[in] nrt_sig Signal handler
 * @param[in] handler Non-real-time signal handler
 * @return 0 on success, otherwise 
 *          -EAGAIN is returned if no free signal slot is available.
 */
#define rtnrt_nrtsig_init(nrt_sig, handler)          \
    rtdm_nrtsig_init(nrt_sig, handler)

/**
 * If compiled with @c RT_RTDM, registers a non-real-time signal handler.
 * If not, does nothing.
 *
 * @param[in] nrt_sig Signal handler
 */
#define rtnrt_nrtsig_destroy(nrt_sig)                \
    rtdm_nrtsig_destroy(nrt_sig)

/**
 * Triggers the NRT event if compiled with real-time support. In non realtime,
 * it just executes @p handler.
 *
 * @param[in] nrt_sig the event which was used as @p nrt_sig in rtnrt_nrtsig_init()
 * @param[in] handler the handler function which was used as @p handler in
 *        rtnrt_nrtsig_init().
 */
#define rtnrt_nrtsig_action(nrt_sig, handler)        \
    rtdm_nrtsig_pend(nrt_sig)

/**
 * Type of the argument used in the NRT handler. Expands to <tt>void *</tt>
 * if not realtime and to <tt>rtdm_nrtsig_t</tt> in realtime.
 */
typedef rtdm_nrtsig_t rtnrt_nrtsig_t;

#else

#define DEFINE_NRTSIG(name)

#define rtnrt_nrtsig_init(nrt_sig, handler)      0

#define rtnrt_nrtsig_destroy(nrt_sig) \
    do {} while (0)

#define rtnrt_nrtsig_action(nrt_sig, handler)            \
    handler(NULL)

typedef void * rtnrt_nrtsig_t;

#endif

/** @} */ /* }}} */

/**
 * @anchor irq @name Interrupt Handling
 *
 * Interrupt handling is important in Linux and real-time drivers. These macros
 * provide some simplification for code that uses a common interrupt handler
 * which should be registered by RTDM if RTDM is used and by Linux if 
 * Linux is used
 *
 * @{
 */ /* {{{ */

/* compatibility accross various RTDM versions */

#if defined(RT_RTDM) || defined(DOXYGEN)
#if !defined(RTDM_IRQTYPE_SHARED) || defined(DOXYGEN)
/**
 * Define RTDM_IRQTYPE_SHARED if not defined. It's defined in new versions
 * of RTDM, such as the version included with Xenomai 2.1.
 *
 * Shared interrupt. Used as flag while registering the rt-ISR.
 */
#   define RTDM_IRQTYPE_SHARED      0
#endif /* RTDM_IRQTYPE_SHARED */


#if !defined(RTDM_IRQ_NONE) || defined(DOXYGEN)
/**
 * Define RTDM_IRQ_NONE  if not defined. It's defined in new versions of
 * RTDM, such as the version included with Xenomai 2.1.
 *
 * Unhandled interrupt. (This is only needed when interrupt sharing is
 * activated an therefore not necessary in RTAI 3.3.)
 */
#   define RTDM_IRQ_NONE            0
#endif /* RTDM_IRQ_NONE */

#if !defined(RTDM_IRQ_HANDLED) || defined(DOXYGEN)
/**
 * Define RTDM_IRQ_HANDLED if not defined. It's defined in new versions of
 * RTDM, such as the version included with Xenomai 2.1.
 *
 * Denote handled interrupt. (This is only needed when interrupt sharing is
 * activated an therefore not necessary in RTAI 3.3.)
 */
#   define RTDM_IRQ_HANDLED         0
#endif /* RTDM_IRQ_HANDLED */

#if !defined(RTDM_IRQ_ENABLE) || defined(DOXYGEN)
/**
 * Define RTDM_IRQ_ENABLE if not defined. In RTDM 4, this is not needed any
 * more because re-enabling interrupts is not necessary.
 */
#define RTDM_IRQ_ENABLE             0
#endif /* RTDM_IRQ_ENABLE */
#endif /* RT_RTDM */

#if defined(RT_RTDM) || defined(DOXYGEN)

/**
 * Desclares a proxy function which can registered by Linux if
 * compiled without @c RT_RTDM or by RTDM if compiled with @c RT_RTDM.
 * The proxy function has the name @p proxy_name and calls @p calling_function
 * internally. The argument type (the type of the "cookie") which is
 * supplied by Linux and RTDM subsystems can be specified in @p arg_type.
 *
 * The function will be @c static.
 *
 * @param[in] proxy_name the name of the proxy, mostly @c int_handler or something 
 *        like this
 * @param[in] calling_function the function which will be called inside the
 *        proxy. It must have the return type rtnrt_irqreturn_t and
 *        takes one parameter of type <tt>arg_type *</tt>. It must return
 *        RTNRT_IRQ_HANDLED if the interrupt has been handled or
 *        RTNRT_IRQ_NONE if the interrupt could not be handled.
 * @param[in] arg_type the type of the argument that is supplied by the
 *        IRQ subsystems (the "cookie"). This type must be a normal type,
 *        no pointer. Where a pointer is required, it will be casted
 *        to a pointer automatically.
 */
#define DECLARE_IRQ_PROXY(proxy_name, calling_function, arg_type)         \
    static int proxy_name(rtdm_irq_t *irq_handle)                         \
    {                                                                     \
        return calling_function(rtdm_irq_get_arg(irq_handle, arg_type));  \
    }

/**
 * This macro must be returned by the interrupt proxy if the interrupt
 * could be handled (i.e. the card issued an interrupt). Important because
 * of shared IRQ handlers.
 */
#define RTNRT_IRQ_HANDLED                                                 \
    (RTDM_IRQ_HANDLED | RTDM_IRQ_ENABLE)

/**
 * This macro must be returned by the interrupt proxy if the interrupt
 * could not be handed (i.e. the card issued no interrupt). Important
 * because of shared IRQ handlers.
 */
#define RTNRT_IRQ_NONE                                                    \
    (RTDM_IRQ_NONE | RTDM_IRQ_ENABLE)

/**
 * This is the return type of the interrupt handler. It expands to
 * irqreturn_t in Linux and to int in RTDM.
 */
typedef int rtnrt_irqreturn_t;

/**
 * Registers an interrupt handler at RTDM if the code was compiled with 
 * @c RT_RTDM or at Linux if not. 
 *
 * @param[in] rtdm_irq_handle a handle of type <tt>rtdm_irq_handler_t *</tt>
 * @param[in] interrupt_line the interrupt line that should be requested
 * @param[in] interrupt_handler a function pointer to the function which 
 *        gets executed. This should be declared with DECLARE_IRQ_PROXY.
 *        Otherwise, the signature must follow the Linux/RTDM conventions
 *        for interrupt handlers.
 * @param[in] shared @c true if the interrupt line should be shared. This is only 
 *        possible between Linux handlers or between RTDM handlers (only
 *        Xenomai) but not across domains
 * @param[in] device_name the device name -- must be unique because it's needed 
 *        for identification
 * @param[in] cookie the cookie that is passed to the interrupt handler to determine
 *        the interrupt -- can also be @c NULLL
 * @return 0 or a negative number which indicates an error
 */
#define rtnrt_register_interrupt_handler(rtdm_irq_handle,                 \
                                        interrupt_line,                   \
                                        interrupt_handler,                \
                                        shared,                           \
                                        device_name,                      \
                                        cookie)                           \
    rtdm_irq_request(rtdm_irq_handle, interrupt_line, int_handler,        \
            shared, device_name, cookie)

/**
 * Enables the interrupt. This call is only needed in RTDM, not in Linux.
 * It expands to zero which means success.
 *
 * @param[in] rtdm_irq_handle a handle of type <tt>rtdm_irq_handler_t *</tt>
 * @return 0 or a negative number which indicates an error
 */
#define rtnrt_irq_enable(rtdm_irq_handle)                                 \
    rtdm_irq_enable(rtdm_irq_handle)

/**
 * Frees the interrupt handler on Linux if compiled with @c RT_RTDM not defined
 * or in RTDM if @c RT_RTDM defined.
 *
 * @param[in] rtdm_irq_handle a handle of type <tt>rtdm_irq_handler_t *</tt> that
 *        was used when calling rtnrt_register_interrupt_handler()
 * @param[in] interrupt_line the interrupt line (the same which was used when
 *        calling rtnrt_register_interrupt_handler()
 * @param[in] device_name the device name -- must be unique because it's needed
 *        for identification (the same which was used in 
 *        rtnrt_register_interrupt_handler())
 * @return 0 or a negative number which indicates an error
 */
#define rtnrt_free_interrupt_handler(rtdm_irq_handle,                     \
                                    interrupt_line,                       \
                                    device_name)                          \
    rtdm_irq_free(rtdm_irq_handle);

#else /* not RT_RTDM */

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
#define DECLARE_IRQ_PROXY(proxy_name, calling_function, arg_type)         \
    static irqreturn_t proxy_name(int              irq,                   \
                                      void             *dev_id,           \
                                      struct pt_regs   *regs)             \
    {                                                                     \
        return calling_function((arg_type *)dev_id);                      \
    }

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
#define DECLARE_IRQ_PROXY(proxy_name, calling_function, arg_type)         \
    static irqreturn_t proxy_name(int              irq,                   \
                                      void             *dev_id)           \
    {                                                                     \
        return calling_function((arg_type *)dev_id);                      \
    }
#endif


#define RTNRT_IRQ_HANDLED                                                 \
    IRQ_HANDLED

#define RTNRT_IRQ_NONE                                                    \
    IRQ_NONE

typedef irqreturn_t rtnrt_irqreturn_t;

#define rtnrt_register_interrupt_handler(rtdm_irq_handle,                 \
                                        interrupt_line,                   \
                                        interrupt_handler,                \
                                        shared,                           \
                                        device_name,                      \
                                        cookie)                           \
    request_irq(interrupt_line, interrupt_handler, shared,                \
            device_name, cookie)

#define rtnrt_irq_enable(rtdm_irq_handle)                                 \
    0

#define rtnrt_free_interrupt_handler(rtdm_irq_handle,                     \
                                    interrupt_line,                       \
                                    device_name)                          \
    free_irq(interrupt_line, device_name)

#endif /* RT_RTDM */

/** @} */ /* }}} */


#endif /* RT_NRT_H */

/* vim: set ts=4 et sw=4 foldmethod=marker: */
