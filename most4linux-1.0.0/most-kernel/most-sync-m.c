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
 * @file most-sync-m.c
 * @ingroup sync
 *
 * @brief Implementation of the MOST Synchronous driver.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <asm/msr.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <linux/rwsem.h>

#include "most-constants.h"
#include "most-base.h"
#include "most-sync.h"
#include "most-measurements.h"
#include "most-sync-common.h"

/**
 * The name of the driver.
 */
#define DRIVER_NAME                     "most-sync"

/**
 * The prefix for printk outputs.
 */
#define PR                              DRIVER_NAME       ": "

/**
 * Variable that holds the version.
 */
static char *version = "$Rev: 639 $";

/* general static data elements -------------------------------------------- */

/**
 * Array for each device.
 */
struct most_sync_dev *most_sync_devices[MOST_DEVICE_NUMBER];

/* forward declarations ---------------------------------------------------- */

static int        most_sync_probe       (struct most_dev *);
static int        most_sync_remove      (struct most_dev *);
static void       most_sync_int_handler (struct most_dev *, unsigned int);
static int        most_sync_do_open     (struct inode *, struct file *);
static int        most_sync_do_release  (struct inode *, struct file *);
static ssize_t    most_sync_do_read     (struct file *, char __user *, 
                                         size_t, loff_t *);
static ssize_t    most_sync_do_write    (struct file *, const char __user *, 
                                         size_t, loff_t *);
static int        most_sync_do_ioctl    (struct inode *, struct file *,
                                         unsigned int, unsigned long);
static inline int most_sync_do_setup_tx (struct file *, unsigned long);
static inline int most_sync_do_setup_rx (struct file *, unsigned long);

/* module parameters ------------------------------------------------------- */

/*
 * see header
 */
long sw_rx_buffer_size = STD_MOST_FRAMES_PER_SEC; /* 1 s */

/*
 * see header
 */
long sw_tx_buffer_size = STD_MOST_FRAMES_PER_SEC; /* 1 s */

/*
 * see header
 */
long hw_rx_buffer_size = 44; /* 1 ms */

/*
 * see header
 */
long hw_tx_buffer_size = 44; /* 1 ms */

#ifndef DOXYGEN
module_param(sw_rx_buffer_size, long, S_IRUGO);
MODULE_PARM_DESC(sw_rx_buffer_size, 
        "Size of the software receive buffer in frame parts "
        "(default: " __MODULE_STRING(STD_MOST_FRAMES_PER_SEC) ")");

module_param(sw_tx_buffer_size, long, S_IRUGO);
MODULE_PARM_DESC(sw_tx_buffer_size, 
        "Size of the software transmit buffer in frame parts "
        "(default: " __MODULE_STRING(STD_MOST_FRAMES_PER_SEC) ")");

module_param(hw_rx_buffer_size, long, S_IRUGO);
MODULE_PARM_DESC(hw_rx_buffer_size,
        "Size of the hardware receive buffer in frame parts "
        "(default: " __MODULE_STRING(44) ")");

module_param(hw_tx_buffer_size, long, S_IRUGO);
MODULE_PARM_DESC(hw_tx_buffer_size, 
        "Size of the hardware transmit buffer in frame parts "
        "(default: " __MODULE_STRING(44) ")");
#endif


/**
 * Create a set of file operations for the MOST Sync device.
 */
static struct file_operations most_sync_file_operations = {
    .owner   = THIS_MODULE,
    .open    = most_sync_do_open,
    .ioctl   = most_sync_do_ioctl,
    .release = most_sync_do_release,
    .read    = most_sync_do_read,
    .write   = most_sync_do_write
};

#ifdef DEBUG
/**
 * Prints the device list which is inside a most_sync_file.
 *
 * @param file the synchonous file
 */
static inline void most_sync_print_devices(struct most_sync_file *file)
{
    struct list_head        *ptr;
    struct most_sync_file   *entry;
    struct most_sync_dev    *sync_dev = file->sync_dev;
    int                     i         = 0;
    
    list_for_each(ptr, &sync_dev->file_list) {
        entry = list_entry(ptr, struct most_sync_file, list);

        rtnrt_debug(PR "-------------------------- %d -----------\n", i);
        rtnrt_debug(PR "Number of quadlets RX   : %d\n", entry->part_rx.count);
        rtnrt_debug(PR "Offset of quadlets RX   : %d\n", entry->part_rx.offset);
        rtnrt_debug(PR "Number of quadlets TX   : %d\n", entry->part_tx.count);
        rtnrt_debug(PR "Offset of quadlets TX   : %d\n", entry->part_tx.offset);
        rtnrt_debug(PR "RX running              : %d\n", entry->rx_running);
        rtnrt_debug(PR "TX running              : %d\n", entry->tx_running);
        rtnrt_debug(PR "Reader index            : %d\n", entry->reader_index);
        rtnrt_debug(PR "Writer index            : %d\n", entry->writer_index);

        i++;
    }
}
#endif


/**
 * Gets called by the MOST driver when a new MOST device was discovered.
 *
 * @param[in,out] most_dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int most_sync_probe(struct most_dev *most_dev)
{
    int                   number = MOST_DEV_CARDNUMBER(most_dev);
    dev_t                 devno  = MKDEV(MOST_DEV_MAJOR(most_dev),
                                         number + MOST_SYNC_MINOR_OFFSET);
    int                   err    = 0;
    struct most_sync_dev  *sync_dev;
    char                  buffer[10];

    return_value_if_fails_dbg(number < MOST_DEVICE_NUMBER, -EINVAL);

    print_dev_t(buffer, devno);
    pr_sync_debug(PR "most_sync_probe called, registering %s", buffer);

    /* create a new device structure */
    sync_dev = (struct most_sync_dev *)kmalloc(sizeof(struct most_sync_dev), GFP_KERNEL);
    if (unlikely(sync_dev == NULL)) {
        rtnrt_warn(PR "Allocation of private data structure failed\n");
        err = -ENOMEM;
        goto out;
    }
    memset(sync_dev, 0, sizeof(struct most_sync_dev));

    /* initialize some members */
    sync_dev->most_dev = most_dev;
    INIT_LIST_HEAD(&sync_dev->file_list);
    init_waitqueue_head(&sync_dev->rx_queue);
    init_waitqueue_head(&sync_dev->tx_queue);
    init_rwsem(&sync_dev->config_lock_rx);
    init_rwsem(&sync_dev->config_lock_tx);
    atomic_set(&sync_dev->receiver_count, 0);
    atomic_set(&sync_dev->transmitter_count, 0);
    atomic_set(&sync_dev->open_count, -MOST_SYNC_OPENS);

    /* register the new character device */
    cdev_init(&sync_dev->cdev, &most_sync_file_operations);
    sync_dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&sync_dev->cdev, devno, 1);
    if (unlikely(err)) {
        rtnrt_warn(PR "cdev_add failed\n");
        goto out_free;
    }

    /* put the device in the global list of devices */
    most_sync_devices[number] = sync_dev;

    return 0;

out_free:
    kfree(sync_dev);
out:
    return err;
}

/**
 * Gets called by the MOST Base driver when a MOST device was removed.
 *
 * @param dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int most_sync_remove(struct most_dev *dev)
{
    int                  number    = MOST_DEV_CARDNUMBER(dev);
    struct most_sync_dev *sync_dev = most_sync_devices[number];

    pr_sync_debug(PR "most_sync_remove called, number = %d\n", number);

    /* delete device */
    cdev_del(&sync_dev->cdev);

    /* free memory and remove from the global device list */
    most_sync_devices[number] = NULL;
    kfree(sync_dev);

    return 0;
}

/**
 * Interrupt handler of a synchronous driver.
 *
 * @param[in] dev the MOST device
 * @param[in] intstatus the interrupt status register content
 */
static void most_sync_int_handler(struct most_dev        *dev,
                                  unsigned int           intstatus)
{
    int                   card = MOST_DEV_CARDNUMBER(dev);
    struct most_sync_dev  *sync_dev = most_sync_devices[card];
    u32                   val;
    int                   err;
    int                   current_page;

    assert(sync_dev != NULL);

    if (intstatus & ISSRX) {
        void          *dma_start;
        size_t        siz;
        
        pr_irq_debug(PR "RX INT\n");

        val = most_readreg(sync_dev->most_dev, MOST_PCI_SRXCTRL_REG);
        dma_start = sync_dev->hw_receive_buf.addr_virt;
        siz = sync_dev->hw_receive_buf.size / 2;

        /* 
         * current page == 0 
         * -> increase address (because then we must read out page 1!!!!)
         */
        if (!(val & SRXPP)) {
            dma_start += siz;
        }

        measuring_receive_isr_start(sync_dev->sw_receive_buf);
        err = rxbuf_put(sync_dev->sw_receive_buf, dma_start, siz);
        if (unlikely(err < 0)) {
            rtnrt_warn(PR "rxbuf_put in most_pci_int_handler returned %d\n", err);
        } else {
            memset(dma_start, 0, siz);
            measuring_receive_isr_wakeup();
            wake_up_interruptible(&sync_dev->rx_queue);
        }

        current_page = (val & SRXPP) ? 1 : 2;
        if ((sync_dev->rx_current_page != 0) 
                && (sync_dev->rx_current_page == current_page)) {
            rtnrt_warn(PR "sync_dev->rx_current_page == current_page\n");
        }
        sync_dev->rx_current_page = current_page;
    }
    
    if (intstatus & ISSTX) {
        void          *dma_start;
        ssize_t       read;
        size_t        siz;
        unsigned int  *ptr;

        pr_irq_debug(PR "TX INT\n");

        val = most_readreg(sync_dev->most_dev, MOST_PCI_STXCTRL_REG);
        dma_start = sync_dev->hw_transmit_buf.addr_virt;
        siz = sync_dev->hw_transmit_buf.size / 2;

        /* 
         * current page == 0 
         * -> increase address (because then we must read out page 1!!!!)
         */
        if (!(val & STXPP)) {
            dma_start += siz;
        }

        ptr = dma_start;

        memset(dma_start, 0, siz);
        read = txbuf_get(sync_dev->sw_transmit_buf, dma_start, siz);
        if (unlikely(read < 0)) {
            rtnrt_warn(PR "txbuf_get in most_pci_int_handler returned %d\n", read);
        } else {
            wake_up_interruptible(&sync_dev->tx_queue);
        }

        current_page = (val & STXPP) ? 1 : 2;
        if ((sync_dev->tx_current_page != 0) && (sync_dev->tx_current_page == current_page)) {
            rtnrt_warn(PR "TX: sync_dev->tx_current_page == current_page\n");
        }
        sync_dev->tx_current_page = current_page;
    }
}

/**
 * Stops reception (of the whole device)
 * See p. 33 of OS8604 specification
 *
 * @param[in,out] file the MOST synchronous file structure
 */
static inline void most_sync_stop_rx(struct most_sync_file *file)
{
    most_sync_stop_rx_common(file->sync_dev, file);
}

/**
 * Stops transmission (of the whole device)
 * See p. 33 of OS8604 specification
 *
 * @param[in,out] file the MOST synchronous file structure
 */
static inline void most_sync_stop_tx(struct most_sync_file *file)
{
    most_sync_stop_tx_common(file->sync_dev, file);
}

/**
 * The structure for the MOST High driver that is registered by the MOST PCI
 * driver
 */
static struct most_high_driver most_sync_high_driver = {
    .name               = "most-sync",
    .sema_list          = LIST_HEAD_INIT(most_sync_high_driver.sema_list),
    .spin_list          = LIST_HEAD_INIT(most_sync_high_driver.spin_list),
    .probe              = most_sync_probe,
    .remove             = most_sync_remove,
    .int_handler        = most_sync_int_handler,
    .interrupt_mask     = (IESTX | IESRX)
};

/**
 * Open the device. Create a per-file structure. There's no fixed file count
 * per device, so the open() method cannot fail. The setup ioctl() method of
 * the opened device can fail.
 *
 * @param inode the inode
 * @param filp the file pointer
 * @return 0 on success, an error code on failure
 */
static int most_sync_do_open(struct inode *inode, struct file *filp)
{
    unsigned long            flags;
    struct most_sync_dev     *sync_dev;
    struct most_sync_file    *file;

    sync_dev = container_of(inode->i_cdev, struct most_sync_dev, cdev);

    pr_sync_debug(PR "most_sync_do_open called for PCI card %d\n",
                                MOST_DEV_CARDNUMBER(sync_dev->most_dev));
    most_manage_usage(sync_dev->most_dev, 1);

    /* 
     * create the private data structure, this is not the device because each
     * device can be opened by more than one process
     */
    file = kmalloc(sizeof(struct most_sync_file), GFP_KERNEL);
    if (unlikely(!file)) {
        return -ENOMEM;
    }

    /* 
     * initializes quadlet_count_rx, quadlet_count_tx, state_rx, state_tx in a
     * sensible way
     */
    memset(file, 0, sizeof(struct most_sync_file));
    file->sync_dev = sync_dev;
    INIT_LIST_HEAD(&file->list);

    /* and register the structure */
    filp->private_data = file;

    /* check and increase the counter */
    if (atomic_inc_and_test(&sync_dev->open_count)) {
        rtnrt_err(PR "Too much open (%d) for a MOST device, only %d allowed\n",
               atomic_read(&sync_dev->open_count), MOST_SYNC_OPENS);
        goto out_dec;
    }

    /* add the file to the list */
    spin_lock_irqsave(&sync_dev->most_dev->lock, flags);
    list_add_tail(&file->list, &sync_dev->file_list);
    spin_unlock_irqrestore(&sync_dev->most_dev->lock, flags);

    return 0;

out_dec:
    atomic_dec(&sync_dev->open_count);
    most_manage_usage(sync_dev->most_dev, -1);
    kfree(file);
    return -EBUSY;
}

/**
 * Releases the driver. Deletes the file from the global list of open files
 * per device and frees the memory.
 * 
 * @param inode the inode
 * @param filp the file pointer
 * @return 0 on success
 */
static int most_sync_do_release(struct inode *inode, struct file *filp)
{
    unsigned long         flags;
    struct most_sync_file *file      = filp->private_data;
    struct most_sync_dev  *sync_dev  = file->sync_dev;

    pr_sync_debug(PR "most_sync_do_release called for PCI card %d\n",
            MOST_DEV_CARDNUMBER(sync_dev->most_dev));

    /* remove the device from the list */
    spin_lock_irqsave(&sync_dev->most_dev->lock, flags);
    list_del(&file->list);
    spin_unlock_irqrestore(&sync_dev->most_dev->lock, flags);

    /* check if it's the last reader */
    if (file->rx_running && atomic_dec_and_test(&sync_dev->receiver_count)) {
        pr_sync_debug(PR "Last Reader\n");
        down_write(&sync_dev->config_lock_rx);
        most_sync_last_closed_rx(sync_dev, file, most_sync_stop_rx);
        up_write(&sync_dev->config_lock_rx);
    }

    /* check if it's the last writer */
    if (file->tx_running && atomic_dec_and_test(&sync_dev->transmitter_count)) {
        pr_sync_debug(PR "Last Transmitter\n");
        down_write(&sync_dev->config_lock_tx);
        most_sync_last_closed_tx(sync_dev, file, most_sync_stop_tx);
        up_write(&sync_dev->config_lock_tx);
    }

    /* free the memory */
    kfree(file);
    most_manage_usage(sync_dev->most_dev, -1);
    atomic_dec(&sync_dev->open_count);

    return 0;
}

/*
 * see header
 */
ssize_t most_sync_read(struct file                  *filp,
                       void                         *buff,
                       size_t                       count,
                       struct rtnrt_memcopy_desc    *copy)
{
    struct most_sync_file       *file = filp->private_data;
    struct most_sync_dev        *sync_dev = file->sync_dev;
    ssize_t                     copied = 0;
    int                         err;

    pr_sync_debug(PR "Entering most_sync_read %d, c=%d\n", 
            file->reader_index, count);

    /* optimisation */
    return_value_if_fails(count != 0, 0);
    
    /* check if we can read */
    if (!file->rx_running) {
        rtnrt_err(PR "Cannot read at this time\n");
        return -EBUSY;
    }

    down_read(&sync_dev->config_lock_rx);

    while (copied == 0) {
        copied = rxbuf_get(sync_dev->sw_receive_buf, file->reader_index, 
                file->part_rx, buff, count, copy);
        if (unlikely(copied < 0)) {
            rtnrt_err(PR "Error in rxbuf_get: %d\n", copied);
            goto out_read;
        }

        if (copied == 0) {
            err = wait_event_interruptible(sync_dev->rx_queue, 
                    !rxbuf_is_empty(sync_dev->sw_receive_buf, file->reader_index) );
            if (err < 0) {
                pr_sync_debug(PR "wait_event_interruptible "
                        " returned with %d\n", err);
                copied = err;
                goto out_read;
            }
        }
    }

out_read:
    up_read(&sync_dev->config_lock_rx);
    pr_sync_debug(PR "Finishing most_sync_read %d with %d\n",
            file->reader_index, copied);
    
    return copied;
}

/**
 * Read method for a synchronous MOST device
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param buff the userspace buffer that contains the destination
 * @param count the number of bytes allocated for @p buff
 * @param offp used together with llseek system call, unused here
 */
static ssize_t most_sync_do_read(struct file      *filp,
                                 char __user      *buff,
                                 size_t           count,
                                 loff_t           *offp)
{
    struct rtnrt_memcopy_desc   copy = { rtnrt_copy_to_user, NULL };
    return most_sync_read(filp, buff, count, &copy);
}

/*
 * see header 
 */
ssize_t most_sync_write(struct file                 *filp,
                        void                        *buff,
                        size_t                      count,
                        struct rtnrt_memcopy_desc   *copy)
{
    struct most_sync_file       *file = filp->private_data;
    struct most_sync_dev        *sync_dev = file->sync_dev;
    size_t                      copied = 0;
    int                         err;

    pr_sync_debug(PR "Write called, count = %d\n", count);

    /* optimisation */
    return_value_if_fails(count != 0, 0);

    /* check if we can write */
    if (!file->tx_running) {
        rtnrt_err(PR "Cannot write at this time\n");
        return -EBUSY;
    }

    down_read(&sync_dev->config_lock_tx);
    
    while (copied != count) {
        err = txbuf_put(sync_dev->sw_transmit_buf, file->writer_index, 
                file->part_tx, buff + copied, count - copied, copy);
        if (unlikely(err < 0)) {
            rtnrt_err(PR "Error in txbuf_put: %d\n", err);
            goto out_write;
        }
        
        copied += err;
        if (err == 0) {
            err = wait_event_interruptible(sync_dev->tx_queue, 
                     !txbuf_is_full(sync_dev->sw_transmit_buf, 
                         file->writer_index) );
            if (unlikely(err < 0)) {
                pr_sync_debug(PR "wait_event_interruptible returned"
                        " with %d\n", err);
                copied = err;
                goto out_write;
            }
        }
    }

out_write:
    up_read(&sync_dev->config_lock_tx);
    pr_sync_debug(PR "Finishing most_sync_do_write %d with %d\n",
            file->writer_index, copied);

    return copied;
}

/**
 * Write method for a synchronous MOST device
 *
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param buff the userspace buffer that contains the destination
 * @param count the number of bytes allocated for @p buff
 * @param offp used together with llseek system call, unused here
 */
static ssize_t most_sync_do_write(struct file         *filp,
                                  const char __user   *buff,
                                  size_t              count,
                                  loff_t              *offp)
{
    struct rtnrt_memcopy_desc copy = { rtnrt_copy_from_user, NULL };
    return most_sync_write(filp, (void *)buff, count, &copy);
}

/**
 * Implements the ioctl method of a MOST Synchronous device.
 *
 * @param inode the inode (unused here)
 * @param filp the file pointer of Linux, holds the private_data which is of type
 *        struct most_sync_file.
 * @param cmd the ioctl command value
 * @param arg the ioctl argument
 */
static int most_sync_do_ioctl(struct inode        *inode, 
                              struct file         *filp,
                              unsigned int        cmd,
                              unsigned long       arg)
{
    int err  = 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (unlikely((_IOC_TYPE(cmd) != MOST_SYNC_IOCTL_MAGIC))) {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W transfers.
     * Type is user-orignted, while access_ok is kernel-oriented, so the concept
     * of read and write is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ) {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    /* check the rror */
    if (err) {
        return -EFAULT;
    }

    /* now do the command */
    switch (cmd) {
        case MOST_SYNC_SETUP_RX:
            return most_sync_do_setup_rx(filp, arg);

        case MOST_SYNC_SETUP_TX:
            return most_sync_do_setup_tx(filp, arg);

        default:
            return -ENOTTY;
    }

    return 0;
}

/*
 * see header
 */
int most_sync_setup_rx(struct file              *filp, 
                       struct frame_part        *frame_part)
{
    struct most_sync_file   *sync_file = (void *)filp->private_data;
    struct most_sync_dev    *sync_dev = sync_file->sync_dev;
    int                     err = 0;

    down_write(&sync_dev->config_lock_rx);

    /* if it's running, stop it*/
    if (sync_file->rx_running) {
        most_sync_stop_rx(sync_file);
    }

    most_sync_setup_rx_common(*frame_part, sync_file, sync_dev, hw_rx_buffer_size, 
                              sw_rx_buffer_size, err, most_sync_file);

    up_write(&sync_dev->config_lock_rx);

    return err;
}

/**
 * See documentation of MOST_SYNC_SETUP_RX.
 *
 * @param filp the Linux struct file
 * @param ioctl_arg the already checked ioctl argument
 */
static int most_sync_do_setup_rx(struct file        *filp,
                                 unsigned long      ioctl_arg)
{
    struct frame_part       *arg = (struct frame_part __user *)ioctl_arg;
    int                     err = 0;
    struct frame_part       param;

    /* get the argument */
    err = __copy_from_user(&param, arg, sizeof(struct frame_part));
    if (unlikely(err != 0)) {
        return -EFAULT;
    }

    return most_sync_setup_rx(filp, &param);
}

/*
 * see header
 */
int most_sync_setup_tx(struct file          *filp,
                       struct frame_part    *frame_part)
{
    struct most_sync_file   *sync_file = (void *)filp->private_data;
    struct most_sync_dev    *sync_dev = sync_file->sync_dev;
    int                     err;

    down_write(&sync_dev->config_lock_tx);

    /* if it's running, stop it*/
    if (sync_file->tx_running) {
        most_sync_stop_tx(sync_file);
    }

    most_sync_setup_tx_common(*frame_part, sync_file, sync_dev, hw_tx_buffer_size, 
                              sw_tx_buffer_size, err, most_sync_file);

    up_write(&sync_dev->config_lock_tx);

    return 0;
}

/**
 * See documentation of MOST_SYNC_SETUP_TX.
 *
 * @param filp the Linux struct file
 * @param ioctl_arg the already checked ioctl argument
 */
static inline int most_sync_do_setup_tx(struct file *filp, unsigned long ioctl_arg)
{
    struct frame_part       *arg         = (struct frame_part __user *)ioctl_arg;
    int                     err          = 0;
    struct frame_part       param;

    /* get the argument */
    err = __copy_from_user(&param, arg, sizeof(struct frame_part));
    if (unlikely(err != 0)) {
        return -EFAULT;
    }

    return most_sync_setup_tx(filp, &param);
}
    
/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_sync_init(void)
{
    int err;

    rtnrt_info("Loading module %s, version %s\n", DRIVER_NAME, version);
    print_measuring_warning();

    /* register driver */
    err = most_register_high_driver(&most_sync_high_driver);
    if (unlikely(err != 0)) {
        return err;
    }

    return 0;
}

/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_sync_exit(void)
{
    most_deregister_high_driver(&most_sync_high_driver);

    rtnrt_info("Unloading module %s, version %s\n", DRIVER_NAME, version);
}


#ifndef DOXYGEN
EXPORT_SYMBOL(hw_tx_buffer_size);
EXPORT_SYMBOL(hw_rx_buffer_size);
EXPORT_SYMBOL(sw_tx_buffer_size);
EXPORT_SYMBOL(sw_rx_buffer_size);

EXPORT_SYMBOL(most_sync_read);
EXPORT_SYMBOL(most_sync_write);
EXPORT_SYMBOL(most_sync_setup_rx);
EXPORT_SYMBOL(most_sync_setup_tx);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 639 $");
MODULE_DESCRIPTION("Driver for MOST Synchronous data.");
module_init(most_sync_init);
module_exit(most_sync_exit);
#endif

/* vim: set ts=4 et sw=4: */
