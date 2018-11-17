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
#ifndef MOST_RXBUF
#define MOST_RXBUF

/**
 * @file most-rxbuf.h
 * @ingroup common
 *
 * @brief Synchronous receive buffer for MOST.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifndef USP_TEST
#  include <asm/uaccess.h>            /* copy_from_user() */
#  include <linux/module.h>
#  include <linux/wait.h>
#  include "most-common.h"
#else
#  include "usp-test.h"
#endif

#include "most-constants.h"
#include "most-common.h"

/**
 * Receive buffer for MOST, implemented as ringbuffer. The buffer is one large
 * buffer. It contains the complete MOST frames as passed by the PCI card
 * without the unused quadlets, i.e. exactly as they are needed in the
 * interrupt service routine to place them in the alternating buffer.
 *
 * The processes (read syscall) read the contents from this ring buffer. As
 * this happen asynchronously, there's a read pointer for each reader which
 * points to the frame which should be read next (to the beginning of the
 * frame).
 *
 * There's no need to determine the number of full/empty frames in the
 * interrupt service routine, it just overwrites old frames. The processes read
 * garbage then.
 */
struct rx_buffer {
    unsigned char     *buffer;                    /**< the ring buffer */
    unsigned char     *readptr[MOST_SYNC_OPENS];  /**< The read pointers. Points to 
                                                       the first byte of the quadlet
                                                       which should be read next. */
    unsigned int      reader_count;               /**< number of readers */
    unsigned char     *writeptr;                  /**< the write pointer */
    unsigned int      frame_count;                /**< number of maximum frames in
                                                       the ring */
    unsigned int      bytes_per_frame;            /**< number of quadlets per frame */
};

/**
 * @param reader_count the number of readers
 * @param frame_count the number of frames that are in the ring buffer
 * @param bytes_per_frame the number of bytes needed per frame
 * 
 * @return the allocated ring buffer or NULL if the ring buffer could not be 
 *         allocated (an error message will be printed)
 */
struct rx_buffer *rxbuf_alloc(unsigned int reader_count, 
                     unsigned int frame_count, 
                     unsigned int bytes_per_frame);

/**
 * Frees the ring buffer. Don't use @p ring after calling this function any more.
 *
 * @param ring the ring buffer
 */
void rxbuf_free(struct rx_buffer *ring);

/**
 * Reads @p bytes bytes from the ring buffer. 
 *
 * @param ring the ring buffer
 * @param reader_index the index of the reader
 * @param frame_part the interesting frame part
 * @param buffer buffer from which the data is copied
 * @param bytes the number of frames that should be copied
 * @param copy the copy descriptor, see description of struct rtnrt_memcopy_desc.
 * @return the number of bytes that have been copied successfully
 */
ssize_t rxbuf_get(struct rx_buffer              *ring,
                  unsigned int                  reader_index,
                  struct frame_part             frame_part,
                  unsigned char                 *buffer,
                  size_t                        bytes,
                  struct rtnrt_memcopy_desc     *copy);

/**
 * Checks if data is available for reading.
 *
 * @param ring the ring buffer
 * @param reader_index the reader index
 */
bool rxbuf_is_empty(struct rx_buffer *ring, int reader_index);

/**
 * Puts @p bytes in the ring.
 *
 * @param ring the ring buffer
 * @param buffer buffer from which the data is copied
 * @param bytes the number of frames that should be copied. This function
 *        overwrites old frames (because it is usually called from a interrupt
 *        service routine and cannot be blocked)
 * @return the number of bytes that have been copied successfully
 */
ssize_t rxbuf_put(struct rx_buffer      *ring,
                  unsigned char         *buffer,
                  size_t                bytes);

/**
 * Prints debug information (printk()) of the ring buffer. Don't call this
 * function on large ring buffers because the whole buffer is printed in hex
 * values and large buffers would fill the kernel ring buffer here and slowdown
 * the whole system!
 *
 * @param ring the ring buffer
 * @param data the data
 */
void rxbuf_print_debug(struct rx_buffer *ring, bool data);


#endif /* MOST_RXBUF */

/* vim: set ts=4 et sw=4: */
