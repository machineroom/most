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
 * @file most-rxbuf.c
 * @ingroup common
 *
 * @brief Implementation of the receive buffer.
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
#  include "rt-nrt.h"
#else
#  include "usp-test.h"
#endif

#include "most-rxbuf.h"

/**
 * Prefix for printk() messages in this module.
 */
#define PR "rxbuf: "


/*
 * Documentation: see header
 */
struct rx_buffer *rxbuf_alloc(unsigned int reader_count, 
                     unsigned int frame_count, 
                     unsigned int bytes_per_frame)
{
    unsigned int        i;
    struct rx_buffer    *ret;

    /* one element more to determine empty and full rings */
    frame_count++;

    /* allocate the structure */
    ret = ker_malloc(sizeof(struct rx_buffer));
    if (unlikely(!ret)) {
        rtnrt_err(PR "Allocating struct rx_buffer failed\n");
        return NULL;
    }
    memset(ret, 0, sizeof(struct rx_buffer));

    /* initialize all elements */
    ret->reader_count    = reader_count;
    ret->frame_count     = frame_count;
    ret->bytes_per_frame = bytes_per_frame;

    /* allocate the ring */
    ret->buffer = vmalloc(bytes_per_frame * frame_count);
    pr_rxbuf_debug(PR "Allocating %d bytes ringbuffer (0x%p)\n", 
                   bytes_per_frame * frame_count, ret->buffer);
    if (unlikely(!ret->buffer)) {
        rtnrt_err(PR "Allocating ring buffer failed\n");
        goto err_buf;
    }
#ifdef DEBUG
    memset(ret->buffer, 0, bytes_per_frame * frame_count);
#endif

    /* set the pointers right */
    for (i = 0; i < ret->reader_count; i++) {
        ret->readptr[i] = ret->buffer;
    }
    ret->writeptr = ret->buffer;

    return ret;
    
err_buf:
    kfree(ret);
    return NULL;
}

/*
 * Documentation: see header
 */
void rxbuf_free(struct rx_buffer *ring)
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
ssize_t rxbuf_get(struct rx_buffer              *ring,
                  unsigned int                  reader_index,
                  struct frame_part             frame_part,
                  unsigned char                 *buffer,
                  size_t                        bytes,
                  struct rtnrt_memcopy_desc     *copy)
{
    int            frames_to_copy, still_to_copy;
    int            bytes_full;
    unsigned char  *readp        = ring->readptr[reader_index];
    unsigned int   ring_size     = ring->bytes_per_frame * ring->frame_count;
    unsigned char  *ring_end     = ring->buffer + ring_size;
    int            err;
    
    /* round down if necessary */
    bytes -= bytes % frame_part.count;

    /* check the ring size */
    if (bytes > ring_size) {
        rtnrt_err(PR "bytes (%d) must be smaller than ring_size (%d)\n",
               (int)bytes, ring_size);
        return -EINVAL;
    }

    /* get the number of elements to copy */
    bytes_full = ring->writeptr - readp;
    if (bytes_full < 0) {
        bytes_full = ring_size + bytes_full;
    }
    frames_to_copy = min((size_t)bytes_full / ring->bytes_per_frame, bytes / 
            frame_part.count);
    
    pr_rxbuf_debug(PR "bytes full: %d, frames to copy: %d\n", 
                   bytes_full, frames_to_copy);

    /* if the ring is empty, we return that */
    if (frames_to_copy == 0) {
        return 0;
    }

    /* now we have frames to copy, let's do it */
    still_to_copy = frames_to_copy;
    while (still_to_copy > 0) {
        //pr_debugm("frame_part.offset = %d, frame_part.count = %d\n", 
        //          frame_part.offset, frame_part.count);
        err = rtnrt_copy(copy, buffer, readp + frame_part.offset, frame_part.count);

        if (err != 0) {
            rtnrt_warn(PR "Error %d in copy, copied %d bytes\n", 
                       err, frames_to_copy - still_to_copy);
            return (frames_to_copy - still_to_copy) * frame_part.count;
        }

        readp += ring->bytes_per_frame;
        /* wrap at the end */
        if (readp >= ring_end) {
            readp = ring->buffer;
        }
        buffer += frame_part.count;
        still_to_copy--;
    }

    ring->readptr[reader_index] = readp;

    return frames_to_copy * frame_part.count;

}

/*
 * Documentation: see header
 */
bool rxbuf_is_empty(struct rx_buffer *ring, int reader_index)
{
    return ring->writeptr == ring->readptr[reader_index];
}

/*
 * Documentation: see header
 */
ssize_t rxbuf_put(struct rx_buffer      *ring,
                  unsigned char         *buffer,
                  size_t                bytes)
{
    int       to_copy;
    int       ring_size = ring->bytes_per_frame * ring->frame_count;

    return_value_if_fails_dbg(ring != NULL, -EINVAL);
    return_value_if_fails_dbg(buffer != NULL, -EINVAL);
    pr_rxbuf_debug(PR "rxbuf_put=%d\n", bytes);

    /* check if bytes is ok */
    if ((bytes % ring->bytes_per_frame != 0)) {
        rtnrt_err(PR "bytes (%d) must be dividable by %d or is too large\n",
               (int)bytes, ring->bytes_per_frame);
        return -EINVAL;
    }

    /* number of bytes to copy in the first step */
    to_copy = min((int)bytes,
                  (int)(ring_size - (unsigned long)(ring->writeptr - ring->buffer)));

    if (to_copy == 0) {
        return 0;
    }

    /* copy data */
    memcpy(ring->writeptr, buffer, to_copy);

    /* no overflow? */
    if ((unsigned int)to_copy == bytes) {
        /* adjust the ring */
        ring->writeptr += to_copy;
        if (ring->writeptr >= (ring->buffer + ring_size)) {
            ring->writeptr = ring->buffer;
        }
    } else {
        /* copy the rest */
        buffer += to_copy;
        to_copy = bytes - to_copy;

        memcpy(ring->buffer, buffer, to_copy);

        /* adjust the ring */
        ring->writeptr = ring->buffer + to_copy;
    }

    return bytes;
}

#ifdef DEBUG
static spinlock_t print_lock = RTNRT_LSPINLOCK_UNLOCKED(print_lock);

/*
 * Documentation: see header
 */
void rxbuf_print_debug(struct rx_buffer *ring, bool data)
{
    unsigned int i;

    spin_lock(&print_lock);

    rtnrt_debug("Frames in ring       : %d\n", ring->frame_count);
    rtnrt_debug("Bytes per frame      : %d\n", ring->bytes_per_frame);
    rtnrt_debug("Write offset         : %d\n", ring->writeptr - ring->buffer);

    /* now print the information per writer */
    for (i = 0; i < ring->reader_count; i++) {
        rtnrt_debug("Read offset    [%2d]  : %d\n", i, 
                ring->readptr[i] - ring->buffer);
    }

    if (data) {
        for (i = 0; i < (ring->frame_count * ring->bytes_per_frame); i++)
        {
            rtnrt_printk("%-2.2X ", ring->buffer[i]);
        }
    }

    rtnrt_printk("\n");

    spin_unlock(&print_lock);
}
#endif

#ifdef USP_TEST
/* Use `gcc -DDEBUG -DUSP_TEST -o most-rxbuf most-rxbuf.c' */
/* -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    int                 i;
    unsigned char       user_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
                                        11, 12, 13, 14, 15, 16 };
    struct frame_part   part;
    unsigned char       data[1024];
    struct rx_buffer    *buffer;
    int                 err;

    pr_debugm("BEGIN\n");

    buffer = rxbuf_alloc(2, 5, 6);
    if (!buffer) {
        pr_debugm("Error in tvbuf_alloc\n\n\n");
        return -1;
    }

    printf("== Empty? %d, %d\n", rxbuf_is_empty(buffer, 0), rxbuf_is_empty(buffer, 1));

    err = rxbuf_put(buffer, user_data, 6);
    pr_debugm("Ret=%d\n", err);
    printf("== Empty? %d, %d\n", rxbuf_is_empty(buffer, 0), rxbuf_is_empty(buffer, 1));

    err = rxbuf_put(buffer, user_data + 6, 6);
    pr_debugm("Ret=%d\n", err);
    printf("== Empty? %d, %d\n", rxbuf_is_empty(buffer, 0), rxbuf_is_empty(buffer, 1));

    rxbuf_print_debug(buffer, true);
    part.count = 4;
    part.offset = 0;
    err = rxbuf_get(buffer, 0, part, data, 12);
    pr_debugm("*Ret=%d\n", err);
    printf("== Empty? %d, %d\n", rxbuf_is_empty(buffer, 0), rxbuf_is_empty(buffer, 1));

    for (i = 0; i < min(err, 12); i++) {
        most_printk("%-2.2X ", data[i]);
    }
    printf("\n");

    part.count = 4;
    part.offset = 0;
    err = rxbuf_get(buffer, 0, part, data, 4);
    pr_debugm("Ret=%d\n", err);

    for (i = 0; i < min(err, 4); i++) {
        printf("%-2.2X ", data[i]);
    }
    printf("\n");

    rxbuf_print_debug(buffer, true);

    err = rxbuf_put(buffer, user_data, 12);
    pr_debugm("Ret=%d\n", err);
    rxbuf_print_debug(buffer, true);

    part.count = 4;
    part.offset = 0;
    err = rxbuf_get(buffer, 0, part, data, 12);
    pr_debugm("Ret=%d\n", err);

    for (i = 0; i < min(err, 12); i++) {
        printf("%-2.2X ", data[i]);
    }
    printf("\n");

    rxbuf_free(buffer);
    
    return 0;
}

#endif

/* vim: set ts=4 et sw=4: */
