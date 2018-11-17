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
#ifndef MOST_SYNC_H
#define MOST_SYNC_H

/**
 * @file most-sync.h
 * @ingroup sync
 *
 * @brief Declarations for the MOST Synchronous driver.
 *
 * This header file can also be included in userspace. It contains the ioctl() 
 * definitions necessary to implement userspace programs.
 */

#ifdef __KERNEL__
#   include <linux/cdev.h>
#   include <asm/semaphore.h>
#   include <asm/ioctl.h>
#   include <linux/rwsem.h>
#   include "most-rxbuf.h"
#   include "most-txbuf.h"
#endif

#include "most-common.h"

/*
 * types and constants for userspace and kernelspace ----------------------- 
 */

/**
 * Magic number. Should be unique in the whole system to simplify debugging, but
 * it is not a requirement.
 */
#define MOST_SYNC_IOCTL_MAGIC                           'h'

/**
 * Setup ioctl() call. Performs following tasks:
 *
 *  - Stop the MOST receiver if it's currently running on that device
 *  - Sets up the new number of synchronous channels
 *  - Starts receiving again.
 *
 * This stops ALL reception on this device. The function call make take some
 * while because it has to wait until each other file has finished the current
 * reading/writing system call.
 *
 * The function does not allocate any synchronous channels nor does it modify
 * the routing engine. This has to be done with MOST NetServices. The @p
 * frame_part argument contains the frame part from the view of the MOST PCI
 * card, i.e. it's not the position of the part in the MOST frame but of the
 * routed frames. See the documentation of the routing engine of the OS 8104
 * for details or the example code which comes with this driver.
 *
 * Returns 0 on success, a negative error value on failure.
 */
#define MOST_SYNC_SETUP_RX  \
    _IOW(MOST_SYNC_IOCTL_MAGIC, 0, struct frame_part)

/**
 * Setup ioctl() call. Performs following tasks:
 *
 *  - Stop the MOST transmitter if it's currently running on that device
 *  - Sets up the new number of synchronous channels
 *  - Starts transmission again.
 *
 * This stops ALL transmission on this device. The function call make take some
 * while because it has to wait until each other file has finished the current
 * reading/writing system call.
 *
 * The function does not allocate any synchronous channels nor does it modify
 * the routing engine. This has to be done with MOST NetServices. The @p
 * frame_part argument contains the frame part from the view of the MOST PCI
 * card, i.e. it's not the position of the part in the MOST frame but of the
 * routed frames. See the documentation of the routing engine of the OS 8104
 * for details or the example code which comes with this driver.
 *
 * Returns 0 on success, a negative error value on failure.
 */
#define MOST_SYNC_SETUP_TX \
    _IOW(MOST_SYNC_IOCTL_MAGIC, 1, struct frame_part)

/**
 * The maximum ioctl number. This value may change in future.
 */
#define MOST_SYNC_MAXIOCTL                  1


#ifdef __KERNEL__

/*
 * constants --------------------------------------------------------------- 
 */

/**
 * Offset for minor device numbers from zero.
 */
#define MOST_SYNC_MINOR_OFFSET              8


/*
 * type definitions -------------------------------------------------------- 
 */

/**
 * Data structure for each most_sync device. If the probe function is called,
 * such a device is created and if the remove function is called, the device is
 * destroyed.
 */
struct most_sync_dev {
    struct cdev             cdev;                /**< the character device of the 
                                                      Linux kernel */
    struct most_dev         *most_dev;           /**< the corresponding most_dev
                                                      structure */
    struct dma_buffer       hw_receive_buf;      /**< the receive buffer */
    struct dma_buffer       hw_transmit_buf;     /**< the transmit buffer */
    struct rx_buffer        *sw_receive_buf;     /**< the receive ring buffer */
    struct tx_buffer        *sw_transmit_buf;    /**< the transmit ring buffer */
    struct list_head        file_list;           /**< list of all opened files in a 
                                                      device */
    atomic_t                open_count;          /**< open counter */
    atomic_t                receiver_count;      /**< count of running receivers */
    atomic_t                transmitter_count;   /**< count of running transmittes */
    wait_queue_head_t       rx_queue;            /**< wait queue for sleeping processes 
                                                      that want to read data but cannot
                                                      because there are no data to read */
    wait_queue_head_t       tx_queue;            /**< wait queue for sleeping processes
                                                      that want to write data but cannot
                                                      write because the queue is full */
    struct rw_semaphore     config_lock_rx;      /**< semaphore to model the fact that
                                                      a device can only be set up if no
                                                      other file is in a read method */
    struct rw_semaphore     config_lock_tx;      /**< semaphore to model the fact that a
                                                      device can only be set up if no
                                                      other file is in a write method */
    unsigned char           rx_current_page;     /**< current page to keep track of pages
                                                      and be able to print a warning if
                                                      page is wrong (RX) */
    unsigned char           tx_current_page;     /**< current page to keep track of
                                                      pages and be able to print a 
                                                      warning if page is wrong (TX) */
};

/**
 * The data structure which is per-opened file.
 */
struct most_sync_file {
    struct list_head       list;                /**< embeddable in the most_sync_dev 
                                                     structure */
    struct most_sync_dev   *sync_dev;           /**< the synchronous device which
                                                     belongs to that file */
    struct frame_part      part_rx;             /**< frame part for reception */
    struct frame_part      part_tx;             /**< frame part for transmission */
    bool                   rx_running;          /**< state of the sender */
    bool                   tx_running;          /**< state of the receiver */
    int                    reader_index;        /**< reader number for the rx buffer,
                                                     only valid if @c rx_running is
                                                     @c true */
    int                    writer_index;        /**< writer number for the tx buffer,
                                                     only valid if @c tx_running is
                                                     @c true */
};


/**
 * Read implementation for a synchronous MOST device. Can be called from other
 * kernel modules.
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param buff the userspace buffer that contains the destination
 * @param count the number of bytes allocated for @p buff
 * @param copy how the memory must be copied
 */
ssize_t most_sync_read(struct file                  *filp, 
                       void                         *buff,
                       size_t                       count,
                       struct rtnrt_memcopy_desc    *copy);

/**
 * Write implementation for a synchronous MOST device. Can be called from other
 * kernel modules.
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param buff the userspace buffer that contains the destination
 * @param count the number of bytes allocated for @p buff
 * @param copy how the memory must be copied
 */
ssize_t most_sync_write(struct file                 *filp, 
                        void                        *buff,
                        size_t                      count,
                        struct rtnrt_memcopy_desc   *copy);

/**
 * See documentation of MOST_SYNC_SETUP_RX. Can be called from other
 * kernel modules.
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param frame_part the frame part for which the reader should be set up
 */
int most_sync_setup_rx(struct file *filp, struct frame_part *frame_part);

/**
 * See documentation of MOST_SYNC_SETUP_TX.
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param frame_part the frame part for which the writer should be set up
 */
int most_sync_setup_tx(struct file *filp, struct frame_part *frame_part);

#endif /* __KERNEL__ */

#endif /* MOST_SYNC_H */


/* vim: set ts=4 et sw=4: */

