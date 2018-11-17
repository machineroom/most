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

/**
 * @file most-txbuf.c
 * @ingroup common
 *
 * @brief Implementation of the transmit buffer.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifndef USP_TEST
#  include <asm/uaccess.h>            /* copy_from_user() */
#  include <linux/module.h>
#  include <linux/wait.h>
#  include <linux/vmalloc.h>
#  include "most-constants.h"
#else
#  include "usp-test.h"
#endif

#include "most-txbuf.h"


/**
 * Prefix for printk() messages from this module.
 */
#define PR "txbuf: "


/*
 * Documentation: see header
 */
struct tx_buffer *txbuf_alloc(unsigned int writer_count, 
                              unsigned int frame_count,
                              unsigned int bytes_per_frame)
{
    int                 i;
    struct tx_buffer    *ret;

    /* one element more to detemerine emtpy and full rings */
    frame_count++;
 
    /* allocate the structure */
    ret = ker_malloc(sizeof(struct tx_buffer));
    if (unlikely(!ret)) {
        rtnrt_err(PR "Allocating struct tx_buffer failed\n");
        return NULL;
    }
    memset(ret, 0, sizeof(struct tx_buffer));

    /* initialize all elements */
    ret->writer_count    = writer_count;
    ret->frame_count     = frame_count;
    ret->bytes_per_frame = bytes_per_frame;
    ret->full_count      = 0;
    rtnrt_lock_init(&ret->lock);

    /* allocate the ring */
    ret->buffer = vmalloc(bytes_per_frame * frame_count);
    pr_txbuf_debug(PR "Allocating %d bytes ringbuffer (0x%p)\n", 
             bytes_per_frame * frame_count, ret->buffer);
    if (unlikely(!ret->buffer)) {
        pr_txbuf_debug(PR "Allocating ring buffer failed\n");
        goto err_buf;
    }
#ifdef DEBUG
    memset(ret->buffer, 0, bytes_per_frame * frame_count);
#endif

    /* set the pointers right */
    for (i = 0; i < ret->writer_count; i++) {
        ret->writeptr[i] = ret->buffer;
    }
    ret->readptr = ret->buffer;

    return ret;
    
err_buf:
    kfree(ret);
    return NULL;
}

/*
 * Documentation: see header
 */
void txbuf_free(struct tx_buffer *ring)
{
    if (ring) {
        if (ring->buffer) {
            vfree(ring->buffer);
        }
        kfree(ring);
    }
}

/*
 * Documentation: see header
 */
ssize_t txbuf_get(struct tx_buffer     *ring, 
                  unsigned char         *buffer,
                  size_t                bytes)
{
    int           to_copy;
    unsigned long flags;
    int           byte_count; 
    unsigned int  ring_size   = ring->frame_count * ring->bytes_per_frame;

    if ((bytes % ring->bytes_per_frame != 0) || (bytes > ring_size)) {
        rtnrt_err(PR "bytes (%d) must be dividable by %d\n",
               (int)bytes, ring->bytes_per_frame);
        return -EINVAL;
    }

    /* only copy the number of ring elements */
    byte_count = min((size_t)ring->full_count, bytes);

    /* number of bytes to copy in the first step */
    to_copy = min(byte_count,
                 (int)(ring_size - (unsigned long)(ring->readptr - ring->buffer)));

    /* ring is empty */
    if (to_copy == 0) {
        return 0;
    }

    /* copy the data */
    memcpy(buffer, ring->readptr, to_copy);

    /* no overflow? */
    if (to_copy == byte_count) {
        /* adjust the ring */
        if ((ring->readptr + to_copy) >= (ring->buffer + ring_size)) {
            ring->readptr = ring->buffer;
        } else {
            ring->readptr += to_copy;
        }
    } else {
        /* copy the rest */
        buffer += to_copy;
        to_copy = byte_count - to_copy;
        
        memcpy(buffer, ring->buffer, to_copy);

        /* adjust the ring */
        ring->readptr = ring->buffer + to_copy;
    }
    
    rtnrt_lock_get_irqsave(&ring->lock, flags);
    ring->full_count -= byte_count;
    rtnrt_lock_put_irqrestore(&ring->lock, flags);
    
    return byte_count;
}

/**
 * Updates the full frame count
 *
 * @param ring the ring buffer
 */
static void txbuf_update_full_frame_count(struct tx_buffer *ring)
{
    unsigned int  ring_size   = ring->frame_count * ring->bytes_per_frame;
    int           min_val     = ring_size;
    int           i;

    for (i = 0; i < ring->writer_count; i++) {
        int bytes_full = ring->writeptr[i] - ring->readptr;
        if (bytes_full < 0) {
            bytes_full = ring_size + bytes_full;
        }
        min_val = min(min_val, bytes_full);
    }
    ring->full_count = min_val;
}

/*
 * Documentation: see header
 */
bool txbuf_is_full(struct tx_buffer *ring, int writer_index)
{
    unsigned int   ring_size    = ring->frame_count * ring->bytes_per_frame;
    int            bytes_full;

    bytes_full = ring->writeptr[writer_index] - ring->readptr;
    if (bytes_full < 0) {
        bytes_full = ring_size + bytes_full;
    }

    return bytes_full == (int)(ring_size - ring->bytes_per_frame);
}

/*
 * Documentation: see header
 */
ssize_t txbuf_put(struct tx_buffer              *ring,
                  int                           writer_index,
                  struct frame_part             frame_part,
                  const char                    *buffer,
                  size_t                        bytes,
                  struct rtnrt_memcopy_desc     *copy)
{
    int            frames_to_copy, still_to_copy;
    unsigned long  flags;
    int            bytes_full;
    unsigned int   elements_free;
    int            err;
    int            offset_bytes = frame_part.offset;
    int            count_bytes  = frame_part.count;
    unsigned int   ring_size    = ring->frame_count * ring->bytes_per_frame;
    unsigned char  *writep      = ring->writeptr[writer_index];
    unsigned char  *ring_end    = ring->buffer + ring_size;

#ifdef DEBUG
    /* check the bytes */
    if (bytes % count_bytes != 0) {
        rtnrt_err(PR "bytes (%d) must be dividable by %d\n", 
                bytes, count_bytes);
        return -EINVAL;
    }
#endif

    /* don't overwrite something, get the number of elements to copy */
    bytes_full = writep - ring->readptr;
    if (bytes_full < 0) {
        bytes_full = ring_size + bytes_full;
    }
    elements_free  = ring->frame_count - (bytes_full / (ring->bytes_per_frame));
    frames_to_copy = min((size_t)elements_free - 1, bytes / count_bytes);

    pr_txbuf_debug(PR "elements free: %d, frames to copy: %d\n", 
                           elements_free, frames_to_copy);

    /* if the ring is full, we return that */
    if (elements_free == 0) {
        return 0;
    }
    
    /* now we have frames to copy in the ring, then let's do it! */
    still_to_copy = frames_to_copy;
    while (still_to_copy > 0) {
        err = rtnrt_copy(copy, writep + offset_bytes, buffer, count_bytes);

        if (err != 0) {
            rtnrt_err(PR "Error %d in copy, copied %d frames\n", 
                   err, frames_to_copy - still_to_copy);
            return (frames_to_copy - still_to_copy) * count_bytes;
        }

        writep += ring->bytes_per_frame;

        /* wrap at the end */
        if (writep >= ring_end) {
            writep = ring->buffer;
        }
        buffer += count_bytes;
        still_to_copy--;
    }

    /* adjust the ring */
    rtnrt_lock_get_irqsave(&ring->lock, flags);
    ring->writeptr[writer_index] = writep;
    txbuf_update_full_frame_count(ring);
    rtnrt_lock_put_irqrestore(&ring->lock, flags);

    return frames_to_copy * count_bytes;

}

#ifdef DEBUG
static spinlock_t print_lock = RTNRT_LSPINLOCK_UNLOCKED(print_lock);

/*
 * Documentation: see header
 */
void txbuf_print_debug(struct tx_buffer *ring)
{
    unsigned int ring_size = ring->bytes_per_frame * ring->frame_count;
    int i;

    spin_lock(&print_lock);
    rtnrt_debug("Frames in ring       : %d\n", ring->frame_count);
    rtnrt_debug("Full bytes           : %d\n", ring->full_count);
    rtnrt_debug("Bytes per frame      : %d\n", ring->bytes_per_frame);
    rtnrt_debug("Read offset          : %d\n", ring->readptr - ring->buffer);

    /* now print the information per writer */
    for (i = 0; i < ring->writer_count; i++) {
        rtnrt_debug("Write offset   [%2d]  : %d\n", i, 
                ring->writeptr[i] - ring->buffer);
    }

    for (i = 0; i < min(16, (int)ring_size); i++) {
        rtnrt_printk("%-2.2X ", ring->buffer[i]);
    }

    printk("\n");
    spin_unlock(&print_lock);
}
#endif


#ifdef USP_TEST
/* gcc -g -DDEBUG -DUSP_TEST -o most-txbuf most-txbuf.c */
int main(int argc, char *argv[])
{
    struct frame_part frame_part;
    unsigned char       user_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 
                                       10, 11, 12, 13, 14, 15, 16 };
    unsigned char       data[1024];
    struct tx_buffer    *buffer;
    int                 err;

    pr_debugm("BEGIN\n");

    buffer = txbuf_alloc(2, 5, 6);
    if (!buffer) {
        pr_err("Error in tvbuf_alloc\n\n\n");
        return -1;
    }

    frame_part.offset = 0;
    frame_part.count  = 4;
    err = txbuf_put(buffer, 0, frame_part, user_data, 4);
    pr_debugm("Err=%d\n", err);
    printf("==Full=%d, %d\n", txbuf_is_full(buffer, 0), txbuf_is_full(buffer, 1));

    err = txbuf_put(buffer, 0, frame_part, user_data + 4, 8);
    pr_debugm("Err=%d\n", err);
    printf("==Full=%d, %d\n", txbuf_is_full(buffer, 0), txbuf_is_full(buffer, 1));

    err = txbuf_put(buffer, 0, frame_part, user_data, 12);
    pr_debugm("Err=%d\n", err);
    printf("==Full=%d, %d\n", txbuf_is_full(buffer, 0), txbuf_is_full(buffer, 1));

    frame_part.offset = 4;
    frame_part.count = 2;
    err = txbuf_put(buffer, 1, frame_part, user_data, 6);
    pr_debugm("Err=%d\n", err);

    err = txbuf_get(buffer, data, 24);
    pr_debugm("Err=*%d\n", err);

    txbuf_print_debug(buffer);

    frame_part.offset = 0;
    frame_part.count  = 4;
    err = txbuf_put(buffer, 0, frame_part, user_data, 12);
    pr_debugm("Err=%d\n", err);
    txbuf_print_debug(buffer);

    txbuf_free(buffer);
    
    return 0;
}
#endif

/* vim: set ts=4 et sw=4: */
