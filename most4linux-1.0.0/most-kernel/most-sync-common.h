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
#ifndef MOST_SYNC_COMMON_H
#define MOST_SYNC_COMMON_H

/**
 * @file most-sync-common.h
 * @ingroup sync
 *
 * @brief Common functionality between real-time and non-real-time synchronous
 * driver.
 *
 * @todo Find a way to implement this as inline functions and not as macros. 
 *        The reasons why this doesn't have to be done are
 *         - the data structures of the Linux driver can be unchanged 
 *           => easier documentation in the Diploma Thesis
 *         - one way would be a common substructure. The problem would be the 
 *           linked list (sync_dev list which contains all files) because then
 *           the generic structure of sync_dev would contain what -- the
 *           generic files ?, the specialised files
 *           => problematic (but maybe solvable in a "clean" way)
 *        => macros were easier
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include "most-constants.h"
#include "most-base.h"

struct most_sync_file;


/**
 * Sets the Synchronous Bandwidth And Node Position (SBC_NPOS) Register
 *
 * @param dev the MOST device
 */
static inline void most_sync_set_sbc_reg(struct most_dev *dev)
{
    unsigned char buf;

    /* locking is not needed because this register doesn't change very often */
    most_readreg8104(dev, &buf, 1, MOST_8104_SBC_REG);
    most_changereg(dev, MOST_PCI_SBC_NPOS_REG, buf - 1, 0xf);
}

/**
 * See documentation of MOST_SYNC_RT_SETUP_RX.
 *
 * Most be locked, this is done in most_sync_setup_rx() or
 * most_sync_rt_setup_rx() respectively.
 *
 * The device must be stopped before calling this function! 
 *
 * This must be a macro because it can be used with RT and NRT structures,
 * so a function is not suitable. Using a common "base" structure leads to more
 * problems than it solves (because of the list issue).
 *
 * @param param the frame part as described in MOST_SYNC_RT_SETUP_RX
 * @param file the struct most_sync_file or struct most_sync_rt_file 
 *        structure
 * @param sync_dev the synchronous device (struct most_sync_dev or 
 *        struct most_sync_dev_rt)
 * @param hw_buffer_size the hardware buffer size (kernel module 
 *        parameter)
 * @param sw_buffer_size the software buffer size
 * @param error_var the variable where errors (negative value) are stored
 * @param most_sync_file_name the name of the structure (some kind of 
 *        <tt>typeof(file)</tt>)
 * @return 0 on success, a negative error code on failure.
 */
#define most_sync_setup_rx_common(param, file, sync_dev, hw_buffer_size,     \
                                  sw_buffer_size, error_var,                 \
                                  most_sync_file_name)                       \
    do {                                                                     \
        struct most_sync_file_name  *entry;                                  \
        int                         number_quadlets;                         \
        unsigned int                dma_size;                                \
        int                         reader_count = 0;                        \
        unsigned int                max_byte     = 0;                        \
        struct list_head            *ptr;                                    \
                                                                             \
        /* enable the interrupt */                                           \
        most_intset(sync_dev->most_dev, IESRX, IESRX, NULL);                 \
                                                                             \
        /* set the state of this device to running */                        \
        file->rx_running = true;                                             \
        file->part_rx = param;                                               \
                                                                             \
        /* get the total number of quadlets */                               \
        max_byte = 0;                                                        \
        list_for_each(ptr, &sync_dev->file_list) {                           \
            entry = list_entry(ptr, struct most_sync_file_name, list);       \
                                                                             \
            if (entry->rx_running) {                                         \
                unsigned int last_byte;                                      \
                                                                             \
                entry->reader_index = reader_count++;                        \
                                                                             \
                last_byte = entry->part_rx.count + entry->part_rx.offset - 1;\
                if (last_byte > max_byte) {                                  \
                    max_byte = last_byte;                                    \
                }                                                            \
            }                                                                \
        }                                                                    \
        number_quadlets = (max_byte + 3) / 4; /* integer ceil */             \
                                                                             \
        /* adjust the number of quadlets */                                  \
        most_writereg(sync_dev->most_dev, number_quadlets - 1,               \
                MOST_PCI_SRXCA_REG);                                         \
        pr_sync_debug(PR "Setting number of qualdets to %d\n",               \
                number_quadlets);                                            \
        most_sync_set_sbc_reg(sync_dev->most_dev);                           \
                                                                             \
        /* set the page size */                                              \
        dma_size = number_quadlets * 4 * 2 * hw_buffer_size;                 \
                                                                             \
        most_writereg(sync_dev->most_dev, dma_size / 2, MOST_PCI_SRXPS_REG); \
        pr_sync_debug(PR "Setting receive page size to %d\n",                \
                dma_size/2);                                                 \
                                                                             \
        /* allocate the DMA buffer if needed */                              \
        if (dma_size > sync_dev->hw_receive_buf.size) {                      \
            if (sync_dev->hw_receive_buf.size != 0) {                        \
                most_dma_deallocate(sync_dev->most_dev,                      \
                        &sync_dev->hw_receive_buf);                          \
            }                                                                \
                                                                             \
            sync_dev->hw_receive_buf.size = dma_size;                        \
            error_var = most_dma_allocate(sync_dev->most_dev,                \
                    &sync_dev->hw_receive_buf);                              \
            if (error_var < 0) {                                             \
                sync_dev->hw_receive_buf.size = 0;                           \
                rtnrt_err(PR "most_dma_allocate failed");                    \
                goto out;                                                    \
            }                                                                \
        }                                                                    \
        pr_sync_debug(PR "Allocated DMA buffer with size %d "                \
                "(Bus: 0x%x)\n", sync_dev->hw_receive_buf.size,              \
                sync_dev->hw_receive_buf.addr_bus);                          \
        pr_measure_debug(PR "==> DMA Buffer RX 0x%X-0x%X <==\n",             \
                sync_dev->hw_receive_buf.addr_bus,                           \
                sync_dev->hw_receive_buf.addr_bus +                          \
                sync_dev->hw_receive_buf.size);                              \
                                                                             \
        memset(sync_dev->hw_receive_buf.addr_virt,                           \
                sync_dev->hw_receive_buf.size, 0);                           \
                                                                             \
        /* set the hardware start address */                                 \
        most_writereg(sync_dev->most_dev, sync_dev->hw_receive_buf.addr_bus, \
                     MOST_PCI_SRXSA_REG);                                    \
                                                                             \
        /* free the old ringbuffer */                                        \
        if (sync_dev->sw_receive_buf) {                                      \
            rxbuf_free(sync_dev->sw_receive_buf);                            \
            sync_dev->sw_receive_buf = NULL;                                 \
        }                                                                    \
                                                                             \
        /* allocate ring buffer */                                           \
        sync_dev->sw_receive_buf = rxbuf_alloc(reader_count, sw_buffer_size, \
                                               number_quadlets * 4);         \
        if (unlikely(!sync_dev->sw_receive_buf)) {                           \
            rtnrt_err(PR "Not enough memory available\n");                   \
            error_var = -ENOMEM;                                             \
            goto out;                                                        \
        }                                                                    \
                                                                             \
        /*                                                                   \
         * ensure that no reordering takes place between setting the         \
         * start bit and configuration bits                                  \
         */                                                                  \
        wmb();                                                               \
                                                                             \
        most_changereg(sync_dev->most_dev, MOST_PCI_SRXCTRL_REG,             \
                SRXST, SRXST);                                               \
        atomic_inc(&sync_dev->receiver_count);                               \
        break;                                                               \
                                                                             \
out:                                                                         \
        /* release the lock for the sync device */                           \
        file->rx_running = false;                                            \
                                                                             \
    } while (0)

/**
 * See documentation of MOST_SYNC_RT_SETUP_TX.
 *
 * Most be locked, this is done in most_sync_setup_tx() or
 * most_sync_rt_setup_tx() respectively.
 *
 * The device must be stopped before calling this function! 
 *
 * This must be a macro because it can be used with RT and NRT structures,
 * so a function is not suitable. Using a common "base" structure leads to more
 * problems than it solves (because of the list issue).
 *
 * @param param the frame part as described in MOST_SYNC_RT_SETUP_RX
 * @param file the struct most_sync_file or struct most_sync_rt_file structure
 * @param sync_dev the synchronous device (struct most_sync_dev or struct
 *        most_sync_dev_rt)
 * @param hw_buffer_size the hardware buffer size (kernel module parameter)
 * @param sw_buffer_size the software buffer size
 * @param error_var the variable where errors (negative value) are stored
 * @param most_sync_file_name the name of the structure (some kind of
 *        <tt>typeof(file)</tt>)
 * @return 0 on success, a negative error code on failure.
 */
#define most_sync_setup_tx_common(param, file, sync_dev, hw_buffer_size,     \
                                  sw_buffer_size, error_var,                 \
                                  most_sync_file_name)                       \
    do {                                                                     \
        struct most_sync_file_name *entry;                                   \
        int                        number_quadlets;                          \
        unsigned int               dma_size;                                 \
        int                        writer_count = 0;                         \
        unsigned int               max_byte     = 0;                         \
        struct list_head           *ptr;                                     \
                                                                             \
        /* enable the interrupt */                                           \
        most_intset(sync_dev->most_dev, IESTX, IESTX, NULL);                 \
                                                                             \
        /* set the state of this device to running */                        \
        file->tx_running = true;                                             \
        file->part_tx = param;                                               \
                                                                             \
        /* get the total number of quadlets */                               \
        max_byte = 0;                                                        \
        list_for_each(ptr, &sync_dev->file_list) {                           \
            entry = list_entry(ptr, struct most_sync_file_name, list);       \
                                                                             \
            if (entry->tx_running) {                                         \
                unsigned int last_byte;                                      \
                                                                             \
                entry->writer_index = writer_count++;                        \
                                                                             \
                last_byte = entry->part_tx.count + entry->part_tx.offset - 1;\
                if (last_byte > max_byte) {                                  \
                    max_byte = last_byte;                                    \
                }                                                            \
            }                                                                \
        }                                                                    \
        number_quadlets = (max_byte + 3) / 4; /* integer ceil */             \
                                                                             \
        /* adjust the number of quadlets */                                  \
        most_writereg(sync_dev->most_dev, number_quadlets - 1,               \
                MOST_PCI_STXCA_REG);                                         \
        pr_sync_debug(PR "Setting number of qualdets to %d\n",               \
                number_quadlets);                                            \
        most_sync_set_sbc_reg(sync_dev->most_dev);                           \
                                                                             \
        /* set the page size */                                              \
        dma_size = number_quadlets * 4 * 2 * hw_buffer_size;                 \
                                                                             \
        most_writereg(sync_dev->most_dev, dma_size / 2, MOST_PCI_STXPS_REG); \
        pr_sync_debug(PR "Setting transmit page size to %d\n",               \
                dma_size/2);                                                 \
                                                                             \
        /* allocate the DMA buffer if needed */                              \
        if (dma_size > sync_dev->hw_transmit_buf.size) {                     \
            if (sync_dev->hw_transmit_buf.size != 0) {                       \
                most_dma_deallocate(sync_dev->most_dev,                      \
                        &sync_dev->hw_transmit_buf);                         \
            }                                                                \
                                                                             \
            sync_dev->hw_transmit_buf.size = dma_size;                       \
            error_var = most_dma_allocate(sync_dev->most_dev,                \
                    &sync_dev->hw_transmit_buf);                             \
            if (error_var < 0) {                                             \
                sync_dev->hw_transmit_buf.size = 0;                          \
                rtnrt_err(PR "most_dma_allocate failed");                    \
                goto out;                                                    \
            }                                                                \
        }                                                                    \
        pr_sync_debug(PR "Allocated DMA buffer with size %d (Bus: 0x%x)\n",  \
                sync_dev->hw_transmit_buf.size,                              \
                sync_dev->hw_receive_buf.addr_bus);                          \
        pr_measure_debug(PR "==> DMA Buffer TX 0x%X-0x%X <==\n",             \
                sync_dev->hw_transmit_buf.addr_bus,                          \
                sync_dev->hw_transmit_buf.addr_bus +                         \
                sync_dev->hw_transmit_buf.size);                             \
                                                                             \
        memset(sync_dev->hw_transmit_buf.addr_virt,                          \
                sync_dev->hw_transmit_buf.size, 0);                          \
                                                                             \
        /* set the hardware start address */                                 \
        most_writereg(sync_dev->most_dev, sync_dev->hw_transmit_buf.addr_bus,\
                      MOST_PCI_STXSA_REG);                                   \
                                                                             \
        /* free the old ringbuffer */                                        \
        if (sync_dev->sw_transmit_buf) {                                     \
            txbuf_free(sync_dev->sw_transmit_buf);                           \
            sync_dev->sw_transmit_buf = NULL;                                \
        }                                                                    \
                                                                             \
        /* allocate ring buffer */                                           \
        sync_dev->sw_transmit_buf = txbuf_alloc(writer_count, sw_buffer_size,\
                                                number_quadlets * 4);        \
        if (unlikely(!sync_dev->sw_transmit_buf)) {                          \
            rtnrt_err(PR "Not enough memory available\n");                   \
            error_var = -ENOMEM;                                             \
            goto out;                                                        \
        }                                                                    \
                                                                             \
        /*                                                                   \
         * ensure that no reordering takes place between setting the         \
         * start bit and configuration bits                                  \
         */                                                                  \
        wmb();                                                               \
                                                                             \
        most_changereg(sync_dev->most_dev, MOST_PCI_STXCTRL_REG,             \
                STXST, STXST);                                               \
        atomic_inc(&sync_dev->transmitter_count);                            \
                                                                             \
        break;                                                               \
                                                                             \
out:                                                                         \
        /* release the lock for the sync device */                           \
        file->tx_running = false;                                            \
                                                                             \
    } while (0)

/**
 * Common part of most_sync_stop_rx() and most_sync_nrt_stop_rx().
 *
 * @param sync_dev the synchronous device (struct most_sync_rt_dev or struct
 *        struct most_sync_dev
 * @param file the synchronous file (struct most_sync_file or struct
 *        most_sync_rt_file)
 */
#define most_sync_stop_rx_common(sync_dev, file)                              \
    do {                                                                      \
        most_intset(sync_dev->most_dev, 0, IESRX, NULL);                      \
        most_changereg(sync_dev->most_dev, MOST_PCI_SRXCTRL_REG, 0, SRXST);   \
        most_intclear(sync_dev->most_dev, ISSRX);                             \
                                                                              \
        file->rx_running = false;                                             \
    } while (0)

/**
 * Common part of most_sync_stop_tx() and most_sync_nrt_stop_tx().
 *
 * @param sync_dev the synchronous device (struct most_sync_rt_dev or struct
 *        struct most_sync_dev
 * @param file the synchronous file (struct most_sync_file or struct
 *        most_sync_rt_file)
 */
#define most_sync_stop_tx_common(sync_dev, file)                              \
    do {                                                                      \
        most_intset(sync_dev->most_dev, 0, IESTX, NULL);                      \
        most_changereg(sync_dev->most_dev, MOST_PCI_STXCTRL_REG, 0, STXST);   \
        most_intclear(sync_dev->most_dev, ISSTX);                             \
                                                                              \
        file->tx_running = false;                                             \
    } while (0)

/**
 * Must be called if the device is closed and this is the last device.  This is
 * for reception. See most_sync_last_closed_tx to do the same action for
 * transmission. The config_lock must be held if this macro is called.
 *
 * @param sync_dev the synchronous device
 * @param file the sync file
 * @param stop_rx the function that stops the receiver
 */
#define most_sync_last_closed_rx(sync_dev, file, stop_rx)                     \
    do {                                                                      \
        sync_dev->rx_current_page = 0;                                        \
                                                                              \
        stop_rx(file);                                                        \
        rxbuf_free(sync_dev->sw_receive_buf);                                 \
                                                                              \
        /* free the DMA buffers */                                            \
        if (sync_dev->hw_receive_buf.size != 0) {                             \
            most_dma_deallocate(sync_dev->most_dev,                           \
                    &sync_dev->hw_receive_buf);                               \
        }                                                                     \
                                                                              \
        /* mark all as deallocated */                                         \
        sync_dev->sw_receive_buf = NULL;                                      \
        sync_dev->hw_receive_buf.size = 0;                                    \
    } while (0)

/**
 * Must be called if the device is closed and this is the last device.  This is
 * for transmission. See most_sync_last_closed_rx to do the same action for
 * transmission. The config_lock must be held if this macro is called.
 *
 * @param sync_dev the synchronous device
 * @param file the sync file
 * @param stop_tx the function that stops the transmitter
 */
#define most_sync_last_closed_tx(sync_dev, file, stop_tx)                     \
    do {                                                                      \
        sync_dev->tx_current_page = 0;                                        \
                                                                              \
        stop_tx(file);                                                        \
        txbuf_free(sync_dev->sw_transmit_buf);                                \
                                                                              \
        /* free the DMA buffer */                                             \
        if (sync_dev->hw_transmit_buf.size != 0) {                            \
            most_dma_deallocate(sync_dev->most_dev,                           \
                    &sync_dev->hw_transmit_buf);                              \
        }                                                                     \
                                                                              \
        /* mark as deallocated */                                             \
        sync_dev->sw_transmit_buf = NULL;                                     \
        sync_dev->hw_transmit_buf.size = 0;                                   \
    } while (0)


/**
 * Module parameter that holds the size of the software receive buffer in
 * number of stored frame parts.
 */
extern long sw_rx_buffer_size;

/**
 * Module parameter that holds the size of the software transmit buffer in
 * number of stored frame parts.
 */
extern long sw_tx_buffer_size;

/**
 * Module parameter that holds the size of the hardware receive buffer in
 * number of stored frame parts.
 */
extern long hw_rx_buffer_size;

/**
 * Module parameter that holds the size of the hardware transmit buffer in number of
 * stored frame parts.
 */
extern long hw_tx_buffer_size;

#endif /* MOST_SYNC_COMMON_H */

/* vim: set sw=4 ts=4 et: */
