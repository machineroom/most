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
#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifndef MOST_ALSA_H
#define MOST_ALSA_H

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>

/**
 * @file most-alsa.h
 * @ingroup alsa
 *
 * @brief Definitions for the ALSA driver for MOST
 */

/**
 * Represents the @c chip structure of the ALSA driver. It contains MOST 
 * specific information. Because the device can only be opened once for 
 * playback and once for record, there's no need of a file specific structure
 * or somthing like this. This may change in future.
 */
struct most_alsa_dev {
    /* --- general members -------------------------------------------------- */
    struct snd_card      *card;                 /**< the sound card */
	struct most_dev		 *most_dev;             /**< the MOST device */
    struct snd_pcm       *pcm;                  /**< the PCM structure */

    /* --- members for playback --------------------------------------------- */
    struct snd_pcm_substream  *p_substream;      /**< the playback substream */
    atomic_t             p_cur_period;      /**< indicates the current period
                                                 for accessing the ALSA buffer
                                                 (used in various places) */
    volatile bool        p_silent;          /**< @c true if playback is 
                                                 stopped (i. e. silent), 
                                                 @c false otherwise */
    int                  p_thread_id;       /**< the kernel thread that is used
                                                 for playback handling */
    struct completion    p_completion;      /**< the completion object that is 
                                                 used for the kernel thread which
                                                 is used for playback */
    struct semaphore     p_event;           /**< the semaphore that is triggered
                                                 in the interrupt handler that
                                                 is used to wake up the kernel
                                                 thread that is used for 
                                                 playback */
    struct semaphore     p_buffer_mutex;    /**< mutex to aprevent the buffer
                                                 from being accessed in the
                                                 kernel thread while it's
                                                 reallocated in the hw_param
                                                 method */
    struct semaphore     p_thread_sema;     /**< needed that the kernel thread
                                                 can execute daemonize() and
                                                 unblock signals before the
                                                 creator continues */
    struct semaphore     p_buf_setup_sema;  /**< needed to wait until the
                                                 buffer was setup */

    /* --- members for capturing -------------------------------------------- */
    struct snd_pcm_substream  *c_substream;      /**< the playback substream */
    atomic_t             c_cur_period;      /**< indicates the current period
                                                 for accessing the ALSA buffer
                                                 (used in various places) */
    volatile bool        c_silent;          /**< @c true if playback is 
                                                 stopped (i.e.  silent), 
                                                 @c false otherwise */
    int                  c_thread_id;       /**< the kernel thread that is used
                                                 for playback handling */
    struct completion    c_completion;      /**< the completion object that is 
                                                 used for the kernel thread which
                                                 is used for playback */
    struct semaphore     c_buffer_mutex;    /**< mutex to aprevent the buffer
                                                 from being accessed in the
                                                 kernel thread while it's
                                                 reallocated in the hw_param
                                                 method */
    struct semaphore     c_thread_sema;     /**< needed that the kernel thread
                                                 can execute daemonize() and
                                                 unblock signals before the
                                                 crator continues */
    struct semaphore     c_buf_setup_sema;  /**< needed to wait until the
                                                 buffer was setup */
};


/**
 * Extracts the ALSA device from the card structure.
 */
#define ALSA_DEV(card)              \
    (struct most_alsa_dev *)((card)->private_data)

#endif /* MOST_ALSA_H */
