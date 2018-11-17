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
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/list.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/moduleparam.h>
#include <linux/rwsem.h>
#include <linux/proc_fs.h>
#include <asm/atomic.h>

#include "most-constants.h"
#include "most-base.h"
#include "most-common.h"

#ifdef RT_RTDM
#   include <rtdm/rtdm_driver.h>
#   include "most-common-rt.h"
#endif

/**
 * @file most-base.c 
 * @ingroup base
 * @brief Implementation of the MOST Base Driver
 */

/**
 * The name of the driver.
 */
#define DRIVER_NAME                     "most-base"

/**
 * The prefix for printk outputs.
 */
#define PR                              DRIVER_NAME       ": "

/**
 * Variable that holds the driver version.
 */
static char *version = "$Rev: 639 $";

/**
 * @copydoc most_base_low_drivers
 */
RWSEMA_LOCKED_LIST(most_base_low_drivers);

/**
 * @copydoc most_base_high_drivers_sema
 */
RWSEMA_LOCKED_LIST(most_base_high_drivers_sema);

/**
 * @copydoc most_base_high_drivers_spin
 */
SPIN_LOCKED_LIST(most_base_high_drivers_spin);

/**
 * The major id for all MOST devices.
 */
static dev_t major_id = 0;

/**
 * The device count (includes ALL devices)
 */
static atomic_t device_count = ATOMIC_INIT(0);


/*
 * Documentation: see header
 */
int most_register_high_driver(struct most_high_driver *driver)
{
    struct list_head            *cursor;
    struct most_low_driver      *low_driver;
    rtnrt_lockctx_t             flags;

    /* add the driver to the global list of drivers */
    down_write(&most_base_high_drivers_sema.lock);
    list_add_tail(&driver->sema_list, &most_base_high_drivers_sema.list);
    up_write(&most_base_high_drivers_sema.lock);

    /* add the driver to the global list of drivers */
    rtnrt_lock_get_irqsave(&most_base_high_drivers_spin.lock, flags);
    list_add_tail(&driver->spin_list, &most_base_high_drivers_spin.list);
    rtnrt_lock_put_irqrestore(&most_base_high_drivers_spin.lock, flags);

    /* 
     * now call each probe function if there are already PCI cards
     * available in the system
     */
    down_read(&most_base_low_drivers.lock);
    list_for_each(cursor, &most_base_low_drivers.list) {
        low_driver = list_entry(cursor, struct most_low_driver, list);
        low_driver->high_driver_registered(driver);
    }
    up_read(&most_base_low_drivers.lock);

    return 0;
}


/*
 * Documentation: see header
 */
void most_deregister_high_driver(struct most_high_driver *driver)
{
    struct list_head            *cursor_low_driver;
    struct most_low_driver      *low_driver;
    rtnrt_lockctx_t             flags;

    /* call each probe function */
    down_read(&most_base_low_drivers.lock);
    list_for_each(cursor_low_driver, &most_base_low_drivers.list) {
        low_driver = list_entry(cursor_low_driver, struct most_low_driver, list);
        low_driver->high_driver_deregistered(driver);
    }
    up_read(&most_base_low_drivers.lock);
    
    /* remove from the list */
    rtnrt_lock_get_irqsave(&most_base_high_drivers_spin.lock, flags);
    list_del(&driver->spin_list);
    rtnrt_lock_put_irqrestore(&most_base_high_drivers_spin.lock, flags);

    /* remove from the list */
    down_write(&most_base_high_drivers_sema.lock);
    list_del(&driver->sema_list);
    up_write(&most_base_high_drivers_sema.lock);
}

/*
 * Documentation: see header
 */
void most_register_low_driver(struct most_low_driver *driver)
{
    /* add the driver to the global list of drivers */
    down_write(&most_base_low_drivers.lock);
    list_add_tail(&driver->list, &most_base_low_drivers.list);
    up_write(&most_base_low_drivers.lock);
}

/*
 * Documentation: see header
 */
void most_deregister_low_driver(struct most_low_driver *driver)
{
    /* remove from the list */
    down_write(&most_base_low_drivers.lock);
    list_del(&driver->list);
    up_write(&most_base_low_drivers.lock);
}


/*
 * documentation: see header
 */
struct most_dev *most_dev_new(void)
{
    struct most_dev *ret;
    
    ret = kmalloc(sizeof(struct most_dev), GFP_KERNEL);
    if (!ret) {
        return NULL;
    }

    memset(ret, sizeof(struct most_dev), 0);

    spin_lock_init(&ret->lock);
    INIT_LIST_HEAD(&ret->list);
    ret->major_device_no = MAJOR(major_id);
    ret->card_number = atomic_read(&device_count);

    atomic_inc(&device_count);
    sprintf(ret->name, "most-%d", MOST_DEV_CARDNUMBER(ret));
    
    return ret;
}

/*
 * Documentation: see header
 */
void most_dev_free(struct most_dev* dev)
{
    atomic_dec(&device_count);
    if (dev->impl) {
        kfree(dev->impl);
    }
    kfree(dev);
}

/**
 * Sequence file operation for proc device. This function is executed on start
 * of the sequence file operation. Only one sequence is used, so the function
 * return NULL if *pos is not 0.
 *
 * @param s the seq_file
 * @param pos the position
 */
static void *most_base_seq_start(struct seq_file *s, loff_t *pos)
{
    return *pos == 0
        ? &version
        : NULL;
}

/**
 * Sequence file operation for the proc device. This function is executed
 * to get the next sequence. Because only one sequence is used, NULL
 * is returned always.
 *
 * @param s the seq_file
 * @param v the data pointer
 * @param pos the position
 */
static void *most_base_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    return NULL;
}

/**
 * Sequence file operation for the proc device. This function is executed
 * on stop. There's nothing to do.
 *
 * @param sfile the seq_file
 * @param v the data pointer
 */
static void most_base_seq_stop(struct seq_file *sfile, void *v)
{}

/**
 * Shows the information for the proc device. Calls all registered function.
 *
 * @param s the seq_file
 * @param v the data pointer
 */
static int most_base_seq_show(struct seq_file *s, void *v)
{
    struct list_head            *cursor_low_driver;
    struct most_low_driver      *low_driver;
    
    seq_printf(s, ">> MOST for Linux <<\n\n");
    seq_printf(s, "Devices:\n");

    /* now traverse the low drivers first */
    down_read(&most_base_low_drivers.lock);
    list_for_each(cursor_low_driver, &most_base_low_drivers.list) {
        low_driver = list_entry(cursor_low_driver, struct most_low_driver, list);
        if (low_driver->proc_show) {
            low_driver->proc_show(s);
        }
    }
    up_read(&most_base_low_drivers.lock);

    return 0;
}

/**
 * The sequence operations for the proc file.
 */
static struct seq_operations most_base_seq_ops = {
    .start          = most_base_seq_start,
    .next           = most_base_seq_next,
    .stop           = most_base_seq_stop,
    .show           = most_base_seq_show
};

/**
 * The open function for the most_base_proc_ops.
 *
 * @param inode the inode
 * @param file the file
 */
static int most_base_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &most_base_seq_ops);
}

/**
 * The device operations for the MOST Base driver proc file.
 */
static struct file_operations most_base_proc_ops = {
    .owner          = THIS_MODULE,
    .open           = most_base_proc_open,
    .read           = seq_read,
    .llseek         = seq_lseek,
    .release        = seq_release
};

/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_base_init(void)
{
    int                   err;
    struct proc_dir_entry *entry;

    /* get a device id for all MOST drivers */
    err = alloc_chrdev_region(&major_id, 0, MOST_MINOR_IDS, DRIVER_NAME);
    if (unlikely(err != 0)) {
        rtnrt_err("Error in alloc_chrdev_region, err = %d\n", err);
        return err;
    }
    rtnrt_info("Registered major id %d\n", MAJOR(major_id));

    /* register proc */
    entry = create_proc_entry("most", 0, NULL);
    if (likely(entry)) {
        entry->proc_fops = &most_base_proc_ops;
    }

    return 0;
}


/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_base_exit(void)
{
    rtnrt_info("Unloading module %s, version %s\n", DRIVER_NAME, version);

    /* unregistering id */
    unregister_chrdev_region(major_id, MOST_MINOR_IDS);

    /* unregistering proc */
    remove_proc_entry("most", NULL);
}

#ifndef DOXYGEN
EXPORT_SYMBOL(most_register_high_driver);
EXPORT_SYMBOL(most_deregister_high_driver);
EXPORT_SYMBOL(most_register_low_driver);
EXPORT_SYMBOL(most_deregister_low_driver);
EXPORT_SYMBOL(most_base_high_drivers_sema);
EXPORT_SYMBOL(most_base_high_drivers_spin);
EXPORT_SYMBOL(most_dev_new);
EXPORT_SYMBOL(most_dev_free);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 639 $");
MODULE_DESCRIPTION("Base driver for the MOST devices from OASIS Silicon systems.");
module_init(most_base_init);
module_exit(most_base_exit);
#endif

/* vim: set ts=4 et sw=4: */
