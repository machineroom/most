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
#ifndef MOST_TXBUF
#define MOST_TXBUF

/**
 * @file most-txbuf.h
 * @ingroup common
 *
 * @brief Synchronous transmit buffer for MOST.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifndef USP_TEST
#  include <asm/uaccess.h>            /* copy_from_user() */
#  include <linux/module.h>
#  include <linux/wait.h>
#else
#  include "usp-test.h"
#endif

#include "most-common.h"
#include "most-constants.h"
#include "rt-nrt.h"

/**
 * Transfer buffer for MOST, implemented as ringbuffer. The buffer is one large
 * buffer. It contains the complete MOST frames without the unused quadlets,
 * i.e. exactly as they are needed in the interrupt service routine to place
 * them in the change buffer.
 *
 * The processes (write syscall) place their contents in this ring buffer. As
 * this happen asynchronously, there's a write pointer for each writer which
 * points to the frame which should be written next (to the beginning of the
 * frame).
 *
 * To determine the number of full elements in the interrupt service fast,
 * there's a special variable. This is only a performance optimization.
 */
struct tx_buffer {
    unsigned char     *buffer;                    /**< the ring buffer */
    unsigned char     *writeptr[MOST_SYNC_OPENS]; /**< The write pointers. Points 
                                                       to the first byte of the
                                                       quadlet which should be
                                                       written next. */
    int               writer_count;               /**< number of writers */
    unsigned char     *readptr;                   /**< the read pointer */
    unsigned int      full_count;                 /**< number of bytes which are filled
                                                       and can be transferred */
    rtnrt_lock_t      lock;                       /**< locking for the element count
                                                       because it's motified by every
                                                       reader, maybe concurrently */
    int               frame_count;                /**< number of maximum frames in the
                                                       ring */
    int               bytes_per_frame;            /**< number of quadlets per frame */
};


/**
 * @param writer_count the number of writers
 * @param frame_count the number of frames that are in the ring buffer
 * @param bytes_per_frame the number of bytes needed per frame
 *  
 * @return the allocated ring buffer or NULL if the ring buffer could not be
 *         allocated (an error message will be printed)
 */
struct tx_buffer *txbuf_alloc(unsigned int writer_count, 
                              unsigned int frame_count,
                              unsigned int bytes_per_frame);

/**
 * Frees the ring buffer. Don't use @p ring after calling this function any
 * more.
 *
 * @param ring the ring buffer
 */
void txbuf_free(struct tx_buffer *ring);

/**
 * Reads element_count frames from the ring buffer.
 *
 * @param ring the ring buffer
 * @param buffer the buffer to copy (usually a DMA buffer)
 * @param bytes the number of bytes that should be copied. If there are
 *        less than bytes elements in the ring, only the number of elements in
 *        the ring are copied
 * @return the number of elements copied
 */
ssize_t txbuf_get(struct tx_buffer      *ring, 
                  unsigned char         *buffer,
                  size_t                bytes);

/**
 * Puts @p bytes bytes in the ring.
 *
 * @param ring the ring buffer
 * @param writer_index the index of the writer that wants to put data in the
 *        buffer. The index is not checked!
 * @param frame_part the part of the frame the user is interested
 * @param buffer userspace buffer from which the data is copied. The function
 *        uses the copy function that checks if it is valid memory, no
 *        additional checks must be done before
 * @param bytes the number of bytes that are in the userspace buffer
 * @param copy the copy descriptor, see description of struct rtnrt_memcopy_desc.
 * @return the number of bytes that have been copied successfully
 */
ssize_t txbuf_put(struct tx_buffer              *ring,
                  int                           writer_index,
                  struct frame_part             frame_part,
                  const char                    *buffer,
                  size_t                        bytes,
                  struct rtnrt_memcopy_desc     *copy);

/**
 * Checks if the buffer is full for the specified writer.
 *
 * @param ring the ring buffer
 * @param writer_index the index of the writer
 */
bool txbuf_is_full(struct tx_buffer *ring, int writer_index);

/**
 * Prints debug information (printk()) of the ring buffer. Don't call this
 * function on large ring buffers because the whole buffer is printed in hex
 * values and large buffers would fill the kernel ring buffer here and slowdown
 * the whole system!
 *
 * @param ring the ring buffer
 */
void txbuf_print_debug(struct tx_buffer *ring);


#endif /* MOST_TXBUF */
