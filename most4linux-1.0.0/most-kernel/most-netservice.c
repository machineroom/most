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
 * @file most-netservice.c
 * @ingroup netservice
 *
 * @brief Implementation of the MOST NetService driver.
 */
#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>

#include <asm/siginfo.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "most-pci.h"
#include "most-netservice.h"
#include "most-constants.h"
#include "most-base.h"

/**
 * The name of the driver.
 */
#define DRIVER_NAME "most-netservice"

/**
 * The prefix for printk statements in this driver
 */
#define PR          DRIVER_NAME ": "


/**
 * Variable that holds the driver version.
 */
static char *version = "$Rev: 639 $";

/* forward declarations ---------------------------------------------------- */
static int  most_nets_open            (struct inode *, struct file *);
static int  most_nets_release         (struct inode *inode, struct file *file);
static int  most_nets_ioctl           (struct inode *, struct file *, 
                                       unsigned int, unsigned long);
static void process_sigsend_handler   (unsigned long);

/* ioctls */
static int  ioctl_write_register  (struct most_nets_dev *, unsigned long);
static int  ioctl_read_register   (struct most_nets_dev *, unsigned long);
static int  ioctl_write_regblock  (struct most_nets_dev *, unsigned long);
static int  ioctl_read_regblock   (struct most_nets_dev *, unsigned long);
static int  ioctl_read_int        (struct most_nets_dev *);
static int  ioctl_irq_set         (struct most_nets_dev *, unsigned long);
static int  ioctl_irq_reset       (struct most_nets_dev *, unsigned long);
static int  ioctl_reset           (struct most_nets_dev *);


/* general static data elements -------------------------------------------- */

/**
 * Array for each device.
 */
struct most_nets_dev *most_nets_devices[MOST_DEVICE_NUMBER];


/**
 * Create a set of file operations for our new device.
 */
static struct file_operations most_nets_file_operations = {
	.owner   = THIS_MODULE,
	.open    = most_nets_open,
    .ioctl   = most_nets_ioctl,
	.release = most_nets_release
};

/**
 * A bitmask of tasks which should be sent an interrupt. The interrupt service
 * routine just sets a bit in this mask and schedules the corresponding
 * tasklet. The tasklet then sends the interrupt.
 */
static unsigned long cards_to_send_interrupt = 0;

/**
 * Tasklet that is responsible for sending signals to the processes, see
 * documentation of process_sigsend_handler.
 */
static DECLARE_TASKLET(sigsend_tasklet, process_sigsend_handler, 0);

/**
 * Non real-time signalling service handler. This is needed because the
 * interrupt handler runs in RT context if compiled with RT_RTDM and some 
 * tasks must be done from Linux context.
 */
DEFINE_NRTSIG(nrt_signal);

/* functions --------------------------------------------------------------- */

/**
 * Tasklet that gets started if an interrupt occured. It sends all processes
 * that belongs to a PCI card that has an interrupt signalled in 
 * cards_to_send_interrupt the requested signal.
 *
 * @param data the "cookie" (required if more tasklets have been assigned the
 *        same function)
 */
static void process_sigsend_handler(unsigned long data)
{
    int i;

    /*
     * TODO: kill_proc() may be slow because it has to find out the task struct
     * from the PID. Because we have already the PID, we may use send_sig_info
     * or something like this. But send_sig_info() didn't work in my tests
     * (I probably did something wrong), but looking in the code of kill_proc()
     * should find that out. For now, it just works!
     */
    
    for (i = 0; i < MOST_DEVICE_NUMBER; i++) {
        if (test_and_clear_bit(i, &cards_to_send_interrupt)) {
            struct most_nets_dev* nets_dev = most_nets_devices[i];
            int data = 0;

            assert(nets_dev != NULL);

            if (nets_dev->task == NULL) {
                continue;
            }
            if ((nets_dev->intstatus & ISMAINT) && 
                    (nets_dev->signo_async != 0)) {
                data = MNS_AINT;
            }
            if ((nets_dev->intstatus & (ISMINT)) && nets_dev->signo != 0) {
                data = MNS_INT;
            }

            assert(data != 0);

            /* send the signal finally */
            most_kill(nets_dev->signo, nets_dev->task, data);

            nets_dev->intstatus = 0;
        }
    }
}

/**
 * Opens the device. Creates a per-file structure.
 *
 * @param inode the inode 
 * @param filp the file pointer
 * @return 0 on success, an error code on failure
 */
static int most_nets_open(struct inode *inode, struct file *filp)
{
    int                  err     = 0;
    struct most_nets_dev *dev;

	dev = container_of(inode->i_cdev, struct most_nets_dev, cdev);
    most_manage_usage(dev->most_dev, 1);
    
    pr_nets_debug(PR "most_nets_open called for PCI card %d\n", 
                  MOST_DEV_CARDNUMBER(dev->most_dev));
    
    /* ... and register it */
	filp->private_data = dev; 

    /* manage the open counter */
    if (!atomic_inc_and_test(&dev->open_count)) {
        rtnrt_info(PR "Only one device allowed to access "
                "MostNetService device.\n");
        err = -EBUSY;
        goto out_dec;
    }
    
	return 0;

out_dec:
    atomic_dec(&dev->open_count);
    most_manage_usage(dev->most_dev, -1);

    return err;
}


/**
 * The ioctl() system call which implements the basic operations. The valid
 * commands are described in the header file most-netservices.h where they are
 * defined.
 * 
 * @param inode the inode
 * @param file the file 
 * @param cmd the command
 * @param arg the command argument
 * @return 0 on success, a negative value on error
 */
static int most_nets_ioctl(struct inode         *inode,
                           struct file          *file,
                           unsigned int         cmd,
                           unsigned long        arg)
{
    struct most_nets_dev *dev     = (struct most_nets_dev *)file->private_data;
    int                  err      = 0;
    
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((_IOC_TYPE(cmd) != MOST_NETS_IOCTL_MAGIC) ||
            (_IOC_NR(cmd) > MOST_NETS_MAXIOCTL)) {
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

    /* check the error */
    if (err) {
        return -EFAULT;
    }

    /* now do the command */
    switch (cmd)
    {
        case MOST_NETS_WRITEREG:
            return ioctl_write_register(dev, arg);

        case MOST_NETS_READREG:
            return ioctl_read_register(dev, arg);

        case MOST_NETS_WRITEREG_BLOCK:
            return ioctl_write_regblock(dev, arg);

        case MOST_NETS_READREG_BLOCK:
            return ioctl_read_regblock(dev, arg);

        case MOST_NETS_READ_INT:
            return ioctl_read_int(dev);

        case MOST_NETS_IRQ_SET:
            return ioctl_irq_set(dev, arg);

        case MOST_NETS_IRQ_RESET:
            return ioctl_irq_reset(dev, arg);

        case MOST_NETS_RESET:
            return ioctl_reset(dev);

        default:
            return -ENOTTY;
    }
    
    return 0;
}


/**
 * The release function. Frees the allocated memory.
 *
 * @param inode the inode
 * @param filp the file pointer
 * @return 0 on success
 */
static int most_nets_release(struct inode *inode, struct file *filp)
{
    struct most_nets_dev *dev = (struct most_nets_dev *)filp->private_data;

    pr_nets_debug(PR "most_nets_release called for PCI card %d\n", 
                  MOST_DEV_CARDNUMBER(dev->most_dev));

    /* manage the open counter */
    atomic_dec(&dev->open_count);
    most_manage_usage(dev->most_dev, -1);

    return 0;
}


/**
 * Writes the register specified in @p arg with the value specified in @p arg.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl argument (must be parsed in this function)
 * @return 0 on success, any other error code (negative) on failure
 */
static int ioctl_write_register(struct most_nets_dev    *dev, 
                                    unsigned long           ioctl_arg)
{
    int                         ret      = 0;
    struct single_transfer_arg  *arg     = (void *)ioctl_arg;
    struct single_transfer_arg  param;

    /* get the argument */
    ret = __copy_from_user(&param, arg, sizeof(struct single_transfer_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    pr_ioctl_debug(PR "MOST IOCTL WRITE 0x%x = 0x%x\n",
            param.address, param.value);

    ret = most_writereg8104(dev->most_dev, &param.value, 1, param.address); 

    return 0;
}


/**
 * Reads the register specified in @p arg with the value specified in @p arg.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl argument (must be parsed in this function)
 * @return 0 on success, any other error code (negative) on failure
 */
static int ioctl_read_register(struct most_nets_dev     *dev,
                                   unsigned long            ioctl_arg)
{
    int                         ret      = 0;
    struct single_transfer_arg  *arg     = (void *)ioctl_arg;
    struct single_transfer_arg  param;

    /* get the argument */
    ret = __copy_from_user(&param, arg, sizeof(struct single_transfer_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    ret = most_readreg8104(dev->most_dev, &param.value, 1, param.address);
    if (unlikely(ret < 0)) {
        return ret;
    }
    
    pr_ioctl_debug(PR "MOST IOCTL READ 0x%x = 0x%x\n", 
            param.address, param.value);

    ret = __copy_to_user(arg, &param, sizeof(struct single_transfer_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    return 0;
}

/**
 * Performs the MOST_NETS_READREG_BLOCK ioctl() call as described in the
 * command near the definition of MOST_NETS_WRITEREG_BLOCK.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl argument (must be parsed in this function)
 * @return the number of registers written on success, a negative error code on
 *         failure
 */
static int ioctl_read_regblock(struct most_nets_dev    *dev, 
                                    unsigned long           ioctl_arg)
{
    int                         ret      = 0;
    struct block_transfer_arg   *arg     = (void *)ioctl_arg;
    struct block_transfer_arg   param;

    /* get the argument */
    ret = __copy_from_user(&param, arg, sizeof(struct block_transfer_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    /* now perform the read */
    ret = most_readreg8104(dev->most_dev, dev->buffer, param.count, 
            param.address);
    if (ret < 0) {
        return ret;
    }

    /* copy the read values */
    ret = copy_to_user(param.data, dev->buffer, ret);
    if (ret < 0) {
        return -EFAULT;
    }
    
    pr_ioctl_debug(PR "MOST READBLOCK 0x%x - 0x%x\n",
            param.address, param.address + param.count);

    return param.count;
}


/**
 * Performs the MOST_NETS_WRITEREG_BLOCK ioctl() call as described in the
 * desciption of the definition of MOST_NETS_WRITEREG_BLOCK.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl argument (must be parsed in this function)
 * @return the number of registers written on success, a negative error code on
 *         failure
 */
static int ioctl_write_regblock(struct most_nets_dev       *dev,
                                     unsigned long              ioctl_arg)
{
    int                         ret      = 0;
    struct block_transfer_arg   *arg     = (void *)ioctl_arg;
    struct block_transfer_arg   param;

    /* get the argument */
    ret = __copy_from_user(&param, arg, sizeof(struct block_transfer_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }
    
    /* copy in the kernel buffer */
    ret = copy_from_user(dev->buffer, (unsigned char __user *)param.data, 
            param.count);

#ifdef IOCTL_DEBUG
    {
        int i;
        for (i = 0; i < param.count; i++) {
            pr_ioctl_debug(PR "MOST WRITEBLOCK 0x%x = 0x%x\n",
                    param.address + i, *(dev->buffer + i));
        }
    }
#endif
    
    ret = most_writereg8104(dev->most_dev, dev->buffer, param.count, 
            param.address);
    if (ret < 0) {
        return -EFAULT;
    }
    
    return param.count;
}


/**
 * Performs the MOST_NETS_READ_INT ioctl() call as described in the description
 * of the definition of MOST_NETS_READ_INT.
 *
 * @param dev the most_nets_dev structure
 * @return @c true if an interrupt is active (i.e. /INT is low), @c false if no
 *         interrupt is active (i.e. /INT is high) and a negative value on error
 */
static int ioctl_read_int(struct most_nets_dev *dev)
{
    int ret = 0;

    ret = most_readreg(dev->most_dev, MOST_PCI_INTSTATUS_REG) & ISMINT;

    if (ret < 0) {
        return -EFAULT;
    }

    pr_ioctl_debug(PR "MOST IOCTL READ_INT = %d\n", ret ? true : false);

    return ret ? true : false;
}

/**
 * Implements registering a userspace process for the IRQ. This function is
 * called in process context, so current is set valid.
 *
 * See documentation of MOST_NETS_IRQ_SET ioctl() constant in the header file
 * for more information.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl() argument
 * @return an error code on failure or @c 0 on success
 */
static int ioctl_irq_set(struct most_nets_dev   *dev, 
                         unsigned long          ioctl_arg)
{
    struct        interrupt_set_arg  param;
    struct        interrupt_set_arg  *arg   = (void *)ioctl_arg;
    int           ret                       = 0;

    /* get and check the argument */
    ret = __copy_from_user(&param, arg, sizeof(struct interrupt_set_arg));
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    /* check the argument */
    if ((param.signo != 0) && !is_between_excl(param.signo, 
                SIGRTMIN, SIGRTMAX)) {
        rtnrt_err(PR "Signal number out of range: %d\n", param.signo);
        return -EINVAL;
    }

    pr_ioctl_debug(PR "MOST IRQ SET %d\n", param.signo);

    dev->task = param.sigmask == 0 ? NULL : current;
    dev->signo = param.signo;

    dev->intmask = 0;
    if (param.sigmask & MNS_INT) {
        dev->intmask |= IEMINT;
    }
    if (param.sigmask & MNS_AINT) {
        dev->intmask |= IEMINT;
    }

    most_intset(dev->most_dev, dev->intmask, IEMAINT | IEMINT, NULL);

    return 0;
}

/**
 * Implements registering a userspace process for the IRQ. This function is
 * called in process context, so current is set valid.
 *
 * See documentation of MOST_NETS_IRQ_SET ioctl() constant in the header file
 * for more information.
 *
 * @param dev the most_nets_dev structure
 * @param ioctl_arg the ioctl() argument
 * @return an error code on failure or @c 0 on success
 */
static int ioctl_irq_reset(struct most_nets_dev         *dev, 
                           unsigned long                ioctl_arg)
{
    int             ret = 0;
    unsigned char   mask;

    /* get and check the argument */
    ret = __get_user(mask, (unsigned char __user *)ioctl_arg);
    if (unlikely(ret != 0)) {
        return -EFAULT;
    }

    /* reset MOST Transceiver interrupt */
    ret = most_writereg8104(dev->most_dev, &mask, 1, MSGC);
    if (unlikely(ret != 1)) {
        return ret;
    }

    pr_ioctl_debug(PR "MOST IRQ RESET\n");

    /* enable interrupts again */
    most_intset(dev->most_dev, dev->intmask, IEMAINT | IEMINT, NULL);

    return 0;
}


/**
 * Resets the MOST Transceiver.
 *
 * @param dev the MOST NetService device
 */
static int ioctl_reset(struct most_nets_dev *dev)
{
    most_reset(dev->most_dev);
    return 0;
}


/**
 * Gets called by the MOST driver when a new MOST device was discovered.
 *
 * @param most_dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int nets_probe(struct most_dev *most_dev)
{
    int                      number = MOST_DEV_CARDNUMBER(most_dev);
    struct most_nets_dev     *dev;
    int                      err;
    dev_t                    devno = MKDEV(MOST_DEV_MAJOR(most_dev),
                                           number + MOST_NETS_MINOR_OFFSET);
    char                     buffer[10];

    return_value_if_fails_dbg(number < MOST_DEVICE_NUMBER, -EINVAL);

    print_dev_t(buffer, devno);
    pr_nets_debug(PR "nets_probe called, registering %s", buffer);

    /* create a new device structure */
    dev = (struct most_nets_dev *)kmalloc(sizeof(struct most_nets_dev), GFP_KERNEL);
    if (unlikely(dev == NULL)) {
        printk(KERN_WARNING PR "Allocation of private data "
                "structure failed\n");
        err = -ENOMEM;
        goto out;
    }
    memset(dev, 0, sizeof(struct most_nets_dev));

    /* initialize some members */
    dev->most_dev = most_dev;
    atomic_set(&dev->open_count, -MAX_OPEN_PROCESSES);
    dev->task = NULL;

    /* register the new character device */
    cdev_init(&dev->cdev, &most_nets_file_operations);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (unlikely(err)) {
        printk(KERN_WARNING PR "cdev_add failed\n");
        goto out_free;
    }

    /* put the device in the global list of devices */
    most_nets_devices[number] = dev;

    return 0;
    
out_free:
    kfree(dev);
out:
    return err;
}


/**
 * Gets called by the MOST Base driver when a MOST device was removed.
 *
 * @param dev the most_pci_device that was discovered
 * @return @c 0 on success, an error code on failure
 */
int nets_remove(struct most_dev *dev)
{
    int                  number     = MOST_DEV_CARDNUMBER(dev);
    struct most_nets_dev *nets_dev  = most_nets_devices[number];

    pr_nets_debug(PR "nets_remove called, number = %d\n", number);

    /* remove the character device */
    cdev_del(&nets_dev->cdev);
    
    /* free memory and delete from the global device list */
    most_nets_devices[number] = NULL;
    kfree(nets_dev);

    return 0;
}

/**
 * Handles the NRT part of the interrupt handling if compiled with 
 * @c RT_RTDM. It simply schedules the tasklet. This must be done from
 * Linux context because RTAI could interrupt the Linux kernel at any time.
 *
 * @param nrt_sig the signal handle
 */
static inline void nrtsig_handler(rtnrt_nrtsig_t nrt_sig)
{
    tasklet_schedule(&sigsend_tasklet);
}

/**
 * Called on every interrupt. This function needs to make sure that the
 * corresponding processes gets notified on the interrupt. To do this, it sets
 * a bit in the module-global variable cards_to_send_interrupt and schedules a
 * tasklet which then sends the signal. This guarantees short durations of the
 * interrupt service routine.
 *
 * @param dev the device that fired the interrupt
 * @param intstatus the interrupt status register
 */
static void nets_int_handler(struct most_dev         *dev, 
                                    unsigned int            intstatus)
{
    int                  card      = MOST_DEV_CARDNUMBER(dev);
    struct most_nets_dev *nets_dev = most_nets_devices[card];

    assert(nets_dev != NULL);

    /* engineer that the task gets the requested interrupt */
    set_bit(card, &cards_to_send_interrupt);

    /* save the interrupt status */
    nets_dev->intstatus |= intstatus;

    /* disable MOST NetServices interrupt */
    most_intset(dev, 0, IEMINT | IEMINT, NULL);

    /* sends the signal if real-time or schedules the tasklet if NRT */
    rtnrt_nrtsig_action(&nrt_signal, nrtsig_handler);
}


/**
 * The structure for the MOST High driver that is registered by the MOST PCI
 * driver
 */
static struct most_high_driver most_netservice_high_driver = {
    .name               = "most-netservice",
    .sema_list          = LIST_HEAD_INIT(most_netservice_high_driver.sema_list),
    .spin_list          = LIST_HEAD_INIT(most_netservice_high_driver.spin_list),
    .probe              = nets_probe,
    .remove             = nets_remove,
    .int_handler        = nets_int_handler,
    .interrupt_mask     = (IEMAINT | IEMINT)
};


/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_nets_init(void)
{
    int err;

    rtnrt_info("Loading module %s, version %s\n", DRIVER_NAME, version);
    
    /* register driver */
    err = most_register_high_driver(&most_netservice_high_driver);
    if (unlikely(err != 0)) {
        return err;
    }

    return rtnrt_nrtsig_init(&nrt_signal, nrtsig_handler);
}


/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_nets_exit(void)
{
    most_deregister_high_driver(&most_netservice_high_driver);
    
    rtnrt_info("Unloading module %s, version %s\n", DRIVER_NAME, version);

    rtnrt_nrtsig_destroy(&nrt_signal);
}

#ifndef DOXYGEN
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 639 $");
MODULE_DESCRIPTION("Support driver for NetService library in userspace ");
module_init(most_nets_init);
module_exit(most_nets_exit);
#endif


/* vim: set ts=4 et sw=4: */
