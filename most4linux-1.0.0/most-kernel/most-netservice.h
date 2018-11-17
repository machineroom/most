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
#ifndef MOST_NETSERVICE_H
#define MOST_NETSERVICE_H

/**
 * @file most-netservice.h
 * @ingroup netservice
 * 
 * @brief NetServices driver declarations.
 *
 * This header file can also be included in userspace. It contains the ioctl()
 * definitions necessary to implement userspace programs.
 */

#ifdef __KERNEL__
#   include <linux/cdev.h>
#endif

#include <asm/types.h>
#include <asm/ioctl.h>

/*
 * types and constants for userspace and kernelspace ----------------------- 
 */

/**
 * Magic number. Should be unique in the whole system to simplify debugging,
 * but it is not a requirement.
 */
#define MOST_NETS_IOCTL_MAGIC                           'g'

/**
 * The argument for a MOST_NETS_WRITEREG or MOST_NETS_READREG ioctl call.
 */
struct single_transfer_arg {
    __u32   address;          /**< the address where the lower 8 bits are the
                                   real adress part and the bits 8..9 are the
                                   page part */
    __u8    value;            /**< the 8 bit value which should be written */
};

/**
 * The argument for a MOST_NETS_READREG_BLOCK or MOST_NETS_WRITEREG_BLOCK
 * ioctl() call.
 */
struct block_transfer_arg {
    __u32   address;          /**< the address where the lower 8 bits are the
                                   real adress part and the bits 8..9 are the
                                   page part */
    __u8    count;            /**< the number of items that should be read 
                                   or written */
    __u8    *data;            /**< a pointer to an array of length count that
                                   holds the data that should be written or 
                                   that is to be read */
};


/**
 * Enumeration that defines numbers that must be used as intmask in interrupt_set_arg.
 */
enum most_nets_interrupt {
    MNS_INT  = (1<<0),        /**< MOST Interrupt (/INT pin) */
    MNS_AINT = (1<<1)         /**< MOST Asynchronous Interrupt (/AINT pin) */
};

/**
 * Argument for registering the interrupt.
 */
struct interrupt_set_arg {
    __u32   signo;            /**< signal number which the process gets 
                                   signalled if an interrupt occured */
    __u32   sigmask;          /**< signals that the process is interested 
                                   in, must be a bitwise or'd combination
                                   of values defined in most_nets_interrupt 
                                   enumeration. */
};

/**
 * Writes a register. The argument must be a pointer to a single_transfer_arg
 * structure. The return value is 0 on success and a negative error code on
 * failure. The data in the transferred date structure remains unchanged.
 */
#define MOST_NETS_WRITEREG \
    _IOW(MOST_NETS_IOCTL_MAGIC,  0, struct single_transfer_arg)

/**
 * Reads a register. The argument must be a pointer to a single_transfer_arg
 * structure. The return value is 0 on success and a negative error code on
 * failure. The read byte will be filled in the given data structure (the
 * value field).
 */
#define MOST_NETS_READREG \
    _IOWR(MOST_NETS_IOCTL_MAGIC, 1, struct single_transfer_arg)

/**
 * Reads a block of registers starting from an address for count values. The
 * argument must be a pointer to a block_transfer_arg structure. The return
 * value is the numbers of bytes read on success and a negative error code on
 * failure. The bytes that have been read are stored in the data part of the
 * data structure. The user is responsible for allocating at least count bytes
 * in this data structure.
 */
#define MOST_NETS_READREG_BLOCK \
    _IOWR(MOST_NETS_IOCTL_MAGIC, 2, struct block_transfer_arg)

/**
 * Writes a block of registers starting from an address for count values. The
 * argument must be a pointer to a block_transfer_arg structure. The return
 * value is the number of bytes written on success and a negative error code on 
 * failure. The data which should be written is stored in the data part of
 * the strucuture.
 */
#define MOST_NETS_WRITEREG_BLOCK \
    _IOW(MOST_NETS_IOCTL_MAGIC, 3, struct block_transfer_arg)

/**
 * Reads the value of the /INT pin of the OS8104 transceiver (i.e. not /INTA pin!). 
 * Returns true if /INT is low and returns false if /INT is high. On error,
 * a negative error value is returned.
 */
#define MOST_NETS_READ_INT \
    _IO(MOST_NETS_IOCTL_MAGIC, 4)

/**
 * Allows a userspace process to register for interrupts. You have to specify
 * an argument of type struct interrupt_set_arg. The interrupt number must be
 * a real-time interrupt (i.e. in the range SIGRTMIN .. SIGRTMAX).
 *
 * The process gets the siginfo.si_value.sigval_int value which contains the
 * interrupt (i.e. MNS_AINT or MNS_INT) which is responsible for that signal.
 *
 * Because only one process can open a MOST NetService device, only one
 * process can register for interrupts. The process that has opened the file
 * gets signalled for interrupts.
 *
 * If an interrupt of the MOST PCI card occures, the hardware does following
 *  
 *  1. Disable the interrupt of the PCI card. Because the interrupt is
 *     handled in userspace, it the ISR would be called in an infinite
 *     loop until userspace handles the interrupt (and it cannot because ISRs
 *     have a higher priority than tasks).
 *
 *  2. Notify the tasks that is responsible for this interrupt with the
 *     signal.
 *
 * After the task has handled the interrupt, it calls the MOST_NETS_IRQ_RESET
 * ioctl() to enable interrupts again.
 *
 * @todo Check what is if the process forks() and inherits the file descriptors?
 */
#define MOST_NETS_IRQ_SET \
    _IOW(MOST_NETS_IOCTL_MAGIC, 5, struct interrupt_set_arg)

/**
 * Please read also the description of the MOST_NETS_IRQ_SET ioctl above.
 *
 * This ioctl() does following:
 *
 *  1. It resets the MOST Interrupt by writing mask to MSGC register in
 *     OS 8104.
 *
 *  2. It enables MOST PCI interrupts again.
 *
 * After callling this ioctl(), the next interrupt can occure. 
 */
#define MOST_NETS_IRQ_RESET \
    _IOW(MOST_NETS_IOCTL_MAGIC, 6, unsigned char)

/**
 * Reset the MOST Transceiver.
 */
#define MOST_NETS_RESET \
    _IO(MOST_NETS_IOCTL_MAGIC, 7)

/**
 * The maximum ioctl number. This value may change in future.
 */
#define MOST_NETS_MAXIOCTL                  7


#ifdef __KERNEL__

/*
 * constants --------------------------------------------------------------- 
 */

/**
 * Offset for minor device numbers from zero.
 */
#define MOST_NETS_MINOR_OFFSET              0

/**
 * Buffer size for some ioctls(). The value of 255 was chosen because one page
 * is 256 bytes large.
 */
#define NETS_BUFSIZ                         256

/**
 * Defines the maximum number of processes which are allowed to open a MOST
 * NetServices device. Currently, only one process is supported.
 *
 * WARNING: Changing this constant only is not sufficient to increase the
 * number of allowed processes. You have to change the structure of the driver
 * and of the Userspace program. So this constant is meant to be _constant_,
 * it's only a define to increase code readability.
 */
#define MAX_OPEN_PROCESSES                  1


/*
 * type definitions -------------------------------------------------------- 
 */

/**
 * Data structure for each most_netservice device. If the probe function is
 * called, such a device is created and if the remove function is called, the
 * device is destroyed. Because one NetService device could be only opened by
 * one process, an instance of this structure is assigned as private data of
 * the file pointer object.
 */
struct most_nets_dev {
    struct cdev            cdev;                /**< the character device of the
                                                     Linux kernel */
    struct most_dev        *most_dev;           /**< the corresponding most_dev
                                                     structure */
    unsigned char          buffer[NETS_BUFSIZ]; /**< buffer for ioctl() calls */
    atomic_t               open_count;          /**< open counter */
    struct task_struct     *task;               /**< the task struct of the task 
                                                     that gets signalled on interrupts 
                                                     or NULL if there's no registered
                                                     task */
    int                    signo;               /**< the signal number (a realtime 
                                                     signal number) which is to be
                                                     sent if a task has registered
                                                     on signals (this is for /INT) */
    int                    signo_async;         /**< signal number for /AINT */
    unsigned int           intmask;             /**< interrupt mask, valid are only 
                                                     IEMAINT_MASK | IEMINT_MASK */
    unsigned char          intstatus;           /**< current interrupt status. Set
                                                     by the ISR, cleared by the bottom
                                                     half. (Must be private to the
                                                     NetService because it's cleared
                                                     if the process receives the
                                                     signal.)*/
};



#endif /* __KERNEL__ */

#endif /* MOST_NETSERVICE_H */


/* vim: set ts=4 et sw=4: */
