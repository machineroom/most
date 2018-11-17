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
#ifndef MOST_SYNC_RT_H
#define MOST_SYNC_RT_H

/**
 * @file most-sync-rt.h
 * @ingroup rtsync
 *
 * @brief Declarations for the MOST Synchronous RT driver.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <linux/cdev.h>
#include <asm/semaphore.h>
#include <asm/ioctl.h>
#include <linux/rwsem.h>
#include "most-rxbuf.h"
#include "most-txbuf.h"

#include "rtmostsync.h"
#include "most-base.h"
#include "most-common.h"

/**
 * This structure is used for synchronisation between most_sync_nrt_setup_rx()
 * /most_sync_rt_read() and most_sync_nrt_setup_tx()/most_sync_rt_write().
 * If a task is inside read()/write(), no reconfiguring can take place but other tasks
 * can also be in read() or write(). However, if one task is reconfiguring, no
 * other tasks can read() or write().
 *
 * This is a classical n-readers/1-writers approach, but since reconfiguring
 * takes place from NRT and configuration takes place from RT, things are more
 * difficult.
 *
 * @b Note: If reconfiguring takes place, the real-time task must wait until
 *          reconfiguration is finished. Until this is done, the behaviour
 *          is non-deterministic.
 */
struct most_conf_sync {
    bool                    reconfigure_flag;       /**< flag that is set to 1 if 
                                                         most_sync_nrt_setup_rx() or
                                                         most_sync_nrt_setup_tx() is
                                                         executed */
    int                     counter;                /**< counts the tasks that are inside
                                                         most_sync_rt_read() or
                                                         most_sync_rt_write() */
    rtdm_event_t            wait_read_write;        /**< wait event that is used inside
                                                         most_sync_rt_read() or 
                                                         most_sync_rt_write() to wait until
                                                         reconfiguration is finished */
    wait_queue_head_t       wait_reconfigure;       /**< wait event that is used inside
                                                         most_sync_nrt_setup_tx() or
                                                         most_sync_nrt_setup_rx() to wait
                                                         until read() and write() are
                                                         finished */
};

/**
 * Initialises a struct most_conf_sync at runtime.
 *
 * @param[out] sync the structure to initialise
 */
static inline void most_conf_sync_init(struct most_conf_sync *sync)
{
    rtdm_event_init(&sync->wait_read_write, 0);
    init_waitqueue_head(&sync->wait_reconfigure);
}

/**
 * Deinitialises a struct most_conf_sync at runtime.
 *
 * @param[in] sync the structure to initialise
 */
static inline void most_conf_sync_destroy(struct most_conf_sync *sync)
{
    rtdm_event_destroy(&sync->wait_read_write);
}

/**
 * Data structure for each most_sync_rt device. If the probe function is called, such
 * a device is created and if the remove function is called, the device is destroyed.
 *
 * @todo common base with the most_sync_dev structure
 */
struct most_sync_rt_dev {
    struct most_dev         *most_dev;           /**< the corresponding most_dev structure */
    struct dma_buffer       hw_receive_buf;      /**< the receive buffer */
    struct dma_buffer       hw_transmit_buf;     /**< the transmit buffer */
    struct rx_buffer        *sw_receive_buf;     /**< the receive ring buffer */
    struct tx_buffer        *sw_transmit_buf;    /**< the transmit ring buffer */
    struct list_head        file_list;           /**< list of all opened files in a device */
    atomic_t                open_count;          /**< open counter */
    atomic_t                receiver_count;      /**< count of running receivers */
    atomic_t                transmitter_count;   /**< count of running transmittes */
    rtdm_lock_t             lock;                /**< lock for the file */
    struct rtdm_device      rtdm_dev;            /**< the RTDM device which is registered,
                                                      it @b must be embedded in this structure
                                                      (and not referenced as pointer) because then
                                                      it's posible to obtain the most_sync_rt_dev
                                                      structure in the open() method of the device
                                                      (see also most_sync_rt_open */
    rtdm_event_t            rx_wait;             /**< wait event for sleeping processes that want
                                                      to read data but cannot because there are
                                                      no data to read */
    rtdm_event_t            tx_wait;             /**< wait queue for sleeping processes that want
                                                      to write data but cannot write because
                                                      the queue is full */
    struct most_conf_sync   rx_sync;             /**< synchronisation for reading */
    struct most_conf_sync   tx_sync;             /**< synchronisation for writing */
    unsigned char           rx_current_page;     /**< current page to keep track of pages and be able
                                                      to print a warning if page is wrong (RX) */
    unsigned char           tx_current_page;     /**< current page to keep track of pages and be able
                                                      to print a warning if page is wrong (TX) */
};		

/**
 * The data structure which is per-opened file.
 *
 * @todo common base with the most_sync_file structure
 */
struct most_sync_rt_file {
    struct list_head        list;                /**< embeddable in the most_sync_rt_dev 
												      structure */
    struct most_sync_rt_dev *sync_dev;           /**< the synchronous device which belongs to
                                                      that file */
    struct frame_part       part_rx;             /**< frame part for reception */
    struct frame_part       part_tx;             /**< frame part for transmission */
    bool                    rx_running;          /**< state of the sender */
    bool                    tx_running;          /**< state of the receiver */
    int                     reader_index;        /**< reader number for the rx buffer, only valid
                                                      if rx_running is true */
    int                     writer_index;        /**< writer number for the tx buffer, only valid
                                                      if tx_running is true */
};
	
#endif /* MOST_SYNC_RT_H */

/* vim: set ts=4 et sw=4: */
