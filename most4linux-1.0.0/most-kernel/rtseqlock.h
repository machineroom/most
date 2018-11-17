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
#ifndef RTSEQLOCK_H
#define RTSEQLOCK_H

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <asm/system.h>
#include <rtdm/rtdm_driver.h>

/**
 * @file rtseqlock.h
 * @ingroup rtcommon
 *
 * @brief Provides real-time sequence locks.
 *
 * Provides real-time sequence lock. The implementation follows the structure 
 * of Linux sequence locks.
 *
 * Reader/writer consistent mechanism without starving writers. This type of
 * lock for data where the reader wants a consitent set of information and is
 * willing to retry if the information changes.  Readers never block but they
 * may have to retry if a writer is in progress. Writers do not wait for
 * readers. 
 *
 * This is not as cache friendly as brlock. Also, this will not work for data
 * that contains pointers, because any writer could invalidate a pointer that a
 * reader was following.
 *
 * Expected reader usage:
 * @code
 *  do {
 *      seq = rt_read_seqbegin(&foo);
 *      // ...
 *  } while (rt_read_seqretry(&foo, seq));
 * @endcode
 *
 * On non-SMP the spin locks disappear but the writer still needs to increment
 * the sequence variables because an interrupt routine could change the state
 * of the data.
 *
 * @todo Implement trylock, this needs implementation of spin_trylock
 *       mechanisms in RTDM. Not difficult, but left out because it's not so
 *       very important and because of time shortage ... (the usual stuff)
 */

/**
 * The lock structure. A variable of this type must always be provided in the
 * functions below.
 */
typedef struct {
    unsigned int        sequence;       /**< the counting variable */
    rtdm_lock_t         lock;           /**< providing the mutual exclusion */
} rt_seqlock_t;

/**
 * Initialises a sequence lock statically at compile-time.
 */
#define RT_SEQLOCK_UNLOCKED { 0, RTDM_LOCK_UNLOCKED }

/**
 * Initialises a sequence lock dynamically at runtime
 */
#define rt_seqlock_init(x)                                          \
    do {                                                            \
        *(x) = (rt_seqlock_t)RT_SEQLOCK_UNLOCKED;                   \
    } while (0)

/** 
 * Lock out other writers and update the count.
 * Acts like a normal spin_lock/unlock.
 * Don't need preempt_disable() because that is in the spin_lock already.
 *
 * @param sl the sequence lock
 */
static inline void rt_write_seqlock(seqlock_t *sl)
{
    rtdm_lock_get(&sl->lock);
    ++sl->sequence;
    smp_wmb();
}

/**
 * Unlocks other writer and update the count again.
 *
 * @param sl the sequence lock
 */
static inline void rt_write_sequnlock(seqlock_t *sl) 
{
    smp_wmb();
    sl->sequence++;
    rtdm_lock_put(&sl->lock);
}

/**
 * Start of read calculation -- fetch last complete writer token 
 *
 * @param sl the sequence lock
 * @return a the value of the @p sl->sequence at the beginning
 *         before reading has started (in a SMP-safe manner)
 */
static inline unsigned int rt_read_seqbegin(const seqlock_t *sl)
{
    unsigned ret = sl->sequence;
    smp_rmb();
    return ret;
}

/**
 * Test if reader processed invalid data.
 *
 *  - If initial values is odd, then writer had already started when section
 *    was entered
 *  - If sequence value changed then writer changed data while in section
 *
 * Using xor saves one conditional branch (it's the same as <tt>==</tt> here).
 *
 * @param sl the sequence lock
 * @param iv the value that was fetched
 *
 * @return @c true if the read must be repeated and @c false otherwise
 */
static inline int rt_read_seqretry(const seqlock_t *sl, unsigned int iv)
{
    smp_rmb();
    return (iv & 1) | (sl->sequence ^ iv);
}

/** 
 * Same as rt_write_seqlock() but with IRQ protection.
 *
 * @param sl the sequence lock
 * @param flags the flags of type @c rtdm_lockctx_t that are safed
 */
#define rt_write_seqlock_irqsave(sl, flags)                         \
    do {                                                            \
        rtdm_lock_get_irqsave(&sl->lock, flags);                    \
        ++sl->sequence;                                             \
        smp_wmb();                                                  \
    } while (0)


/**
 * Same as rt_write_sequnlock() but with IRQ protection.
 *
 * @param sl the sequence lock
 * @param flags the flags of type @c rtdm_lockctx_t that are safed
 */
#define rt_write_sequnlock_irqrestore(sl, flags)                    \
    do {                                                            \
        smp_wmb();                                                  \
        sl->sequence++;                                             \
        rtdm_lock_put_irqrestore(&sl->lock, flags);                 \
    } while (0)


#endif /* RTSEQLOCK_H */

/* vim: set ts=4 sw=4 et: */
