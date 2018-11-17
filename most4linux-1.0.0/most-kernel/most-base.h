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
#ifndef MOST_BASE
#define MOST_BASE

#ifdef __KERNEL__

/**
 * @file most-base.h
 * @ingroup base
 *
 * @brief Definitions for the MOST Base driver
 *
 *
 */
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>

#ifdef RT_RTDM
#   include <rtdm/rtdm_driver.h>
#endif

#include "most-common.h"
#include "most-constants.h" 


/*
 * Forward declarations -------------------------------------------------------
 */
struct most_dev;

/*
 * Typedefs -------------------------------------------------------------------
 */

/**
 * Function prototype for reading a register from an OS8604 chip.
 *
 * @param dev the device to read from
 * @param addr the address to read from
 * @return the value which was read
 */
typedef u32 (*readreg_func) (struct most_dev* dev, u32 addr);

/**
 * Function prototype for writing to a register on a OS8604 chip.
 *
 * @param dev the device to write to
 * @param val the value to write to
 * @param addr the address where to write
 */
typedef void (*writereg_func) (struct most_dev* dev, u32 val, u32 addr);

/**
 * It's a common operation to change the value of a register. I.e. change only
 * a few bits and leave the other bits untouched. The bits that are 1 in
 * @p mask are set to the corresponding bits in @p value. 
 *
 * @param dev the device
 * @param address the register address
 * @param value the value to set
 * @param mask which bits should be set
 */
typedef void (*changereg_func) (struct most_dev     *dev, 
                                u32                 address, 
                                u32                 value, 
                                u32                 mask);

/**
 * Function prototype for reading one or more registers from an OS8104 chip.
 *
 * @param dev the device to read from
 * @param dest the destination array (kernelspace) where the bytes should be
 *        stored
 * @param len the number of bytes which should be read
 * @param addr the address (consists of address and page part) to read from
 * @return the number of bytes that have been read successfully or a negative 
 *         error code
 */
typedef int (*read_os8104_func) (struct most_dev*   dev,
                                 unsigned char      *dest, 
                                 size_t             len, 
                                 u32                addr);

/**
 * Function prototype for writing one or more registers on a OS8104 chip.
 *
 * @param dev the device to write to
 * @param src the bytes that should be written (kernelspace)
 * @param len the number of bytes that should be written
 * @param addr the address where to write (consists of address and page part)
 * @return the number of bytes that have been written successfully or a
 *         negative error code
 */
typedef int (*write_os8104_func) (struct most_dev*  dev,
                                  unsigned char     *src,
                                  size_t            len,
                                  u32               addr);

/**
 * Changes the interrupt mask. All bits that are set in @p mask are set as
 * specified in @p interrupts.
 *
 * @p interrupts and @p mask consists of the components which have to be
 * modified with bit operators to get the requested effect.
 * IOMLOCK_MASK, IEMAINT_MASK, IEMINT_MASK, IEGP_MASK, IESRX_MASK,
 * IESTX_MASK, IEARX_MASK, IEATX_MASK and IECPFSM_MASK.
 *
 * @param dev the device
 * @param interrupts the interrupt bits to set
 * @param mask the mask; a bit in @p interrupts only apply if the corresponding
 *        bit in @p mask is set
 * @param oldmask if not @c NULL, the function fills the value with the old
 *        interrupt mask (valid are only the bits (oldmask & mask)), used
 *        to restore the interrupts
 */
typedef void (*intset_func) (struct most_dev      *dev,
                             unsigned int         interrupts, 
                             unsigned int         mask,
                             unsigned int         *oldmask);

/**
 * Clears pending interrupts.
 */
typedef void (*intclear_func) (struct most_dev    *dev,
                               unsigned int        interrupts);

/**
 * Resets the MOST Transceiver.
 *
 * @param dev the MOST device
 * @return 0 on success, an error code (e.g. -ERESTARTSYS) on failure
 */
typedef void (*reset_func) (struct most_dev *dev);

/**
 * DMA buffer
 */
struct dma_buffer 
{
    dma_addr_t      addr_bus;           /**< bus address of the buffer */
    void            *addr_virt;         /**< kernel (= virtual) address of the
                                             buffer */
    unsigned int    size;               /**< page size of a DMA buffer */
};

/**
 * Allocates a DMA buffer.
 *
 * @param dev the MOST device
 * @param buf the buffer
 * @return 0 on success, a negative error vlaue1 on failure
 */
typedef int (*dma_alloc_fun) (struct most_dev *dev, struct dma_buffer *buf);

/**
 * Deallocates a DMA buffer.
 *
 * @param dev the MOST device
 * @param buf the buffer
 * @return 0 on success, a negative error vlaue1 on failure
 */
typedef void (*dma_dealloc_fun) (struct most_dev *dev, struct dma_buffer *buf);

/**
 * the probe function that gets called if the card was found or the module was
 * load (i.e. the driver was registered) and the card is already present 
 *
 * @param dev the MOST device
 * @return 0 on success, a negative error vlaue1 on failure
 */
typedef int (*probe_func)(struct most_dev *dev);

/**
 * deregistration function 
 *
 * @param dev the MOST device
 */
typedef int (*remove_func)(struct most_dev *dev);

/**
 * The interrupt handler function.
 *
 * @param dev the MOST device
 * @param intstatus the interrupt status register value
 */
typedef void (*irq_func)(struct most_dev *dev, unsigned int intstatus);

/**
 * This file implements the show handler of the MOST device. 
 * 
 * @param file the sequence file
 */
typedef void (*proc_show_func)(struct seq_file *file);

/**
 * Returns the features, a bitwise combination of ...
 */
typedef int (*feature_fun)(struct most_dev *dev);

/**
 * Manages the usage count of the device. If a device is opened, the usage
 * count must be incremented. If it's closed, it must be decremented. Currently
 * this is used to prevent the low modules from being unloaded if the device is
 * opened (used) by a high module.
 *
 * @param dev the MOST device (currently unused)
 * @param change @c 1 if the usage count should be increased, @c -1 if it
 *        should be decreased
 */
typedef void (*usage_fun)(struct most_dev *dev, int change);

/*
 * Types ----------------------------------------------------------------------
 */

/**
 * Non-real-time operations of a MOST device
 */
struct most_ops {
    readreg_func       readreg;        /**< see description of readreg_func */
    writereg_func      writereg;       /**< see description of writereg_func */
    changereg_func     changereg;      /**< see description of changereg_func */
    read_os8104_func   readreg8104;    /**< see description of read_os8104_func */
    write_os8104_func  writereg8104;   /**< see description of write_os8104_func */
    intset_func        intset;         /**< see description of intset_func */
    intclear_func      intclear;       /**< see description of intclear_func */
    reset_func         reset;          /**< see description of reset_func */
    dma_alloc_fun      dma_allocate;   /**< see description of dma_alloc_fun */
    dma_dealloc_fun    dma_deallocate; /**< see description of dma_dealloc_fun */
    feature_fun        features;       /**< see description of feature_fun */
};

/**
 * Real-time operations of a MOST device
 */
struct most_ops_rt {
    readreg_func       readreg;        /**< see description of readreg_func */
    writereg_func      writereg;       /**< see description of writereg_func */
};


/**
 * MOST Base Driver. Contains all attributes and function pointers a MOST
 * Device must provide. The functions are used by higher-level drivers so that
 * they don't need direct PCI access. It increased code readability and
 * abstracts from the real hardware so that a MOST device using the same MOST
 * Transceivers (OS8104 and OS8604) can be accessed without modifying
 * high-level code.
 */
struct most_dev {
    spinlock_t         lock;           /**< lock access to the struct, also implement
                                            some kind of global device lock if needed
                                            (such as reading a register, modifying
                                            it and writing it back) */
    struct list_head   list;           /**< list of all devices of a specific category */
    int                major_device_no;/**< the major device number registered for the 
                                            MOST PCI driver. Each driver has to take care
                                            himself that there no conflicts occure with
                                            minor numbers */
    int                card_number;    /**< the card number, used to register the right
                                            character devices. */
    char               name[20];       /**< @c most-pci%d formatted name, used as device
                                             name for the /proc/iomem and /proc/interrupt
                                             entry to identify the card */
    u64                serial_number;  /**< the serial number */
    unsigned char      product_id;     /**< the product ID */
    void               *impl;          /**< concrete implementation, here it is a 
                                            struct most_pci */
    usage_fun          manage_usage;   /**< see documentation of usage_fun */
    struct most_ops    ops;            /**< operations */
#ifdef RT_RTDM
    struct most_ops_rt rt_ops;         /**< real-time operations */
#endif
};

/**
 * @see readreg_func
 */
#define most_readreg(dev, addr)                             \
    (dev)->ops.readreg((dev), addr)

/**
 * @see writereg_func
 */
#define most_writereg(dev, value, addr)                     \
    (dev)->ops.writereg((dev), (value), (addr))

/**
 * @see changereg_func
 */
#define most_changereg(dev, addr, value, mask)              \
    (dev)->ops.changereg((dev), (addr), (value), (mask))

/**
 * @see read_os8104_func
 */
#define most_readreg8104(dev, dest, len, addr)              \
    (dev)->ops.readreg8104((dev), (dest), (len), (addr))

/**
 * @see write_os8104_func
 */
#define most_writereg8104(dev, src, len, addr)              \
    (dev)->ops.writereg8104((dev), (src), (len), (addr))

/**
 * @see intset_func
 */
#define most_intset(dev, interrupts, mask, oldmask)         \
    (dev)->ops.intset((dev), (interrupts), (mask), (oldmask))

/**
 * @see intclear_func
 */
#define most_intclear(dev, interrupts)                      \
    (dev)->ops.intclear((dev), (interrupts))

/**
 * @see dma_alloc_fun
 */
#define most_dma_allocate(dev, dma_buffer)                  \
    (dev)->ops.dma_allocate(dev, dma_buffer)

/**
 * @see dma_dealloc_fun
 */
#define most_dma_deallocate(dev, dma_buffer)                \
    (dev)->ops.dma_deallocate(dev, dma_buffer)

/**
 * @see reset_func
 */
#define most_reset(dev)                                     \
    (dev)->ops.reset((dev))

/**
 * @see feature_fun
 */
#define most_features(dev)                                  \
    (dev)->ops.features(dev)

/**
 * @see readreg_func
 */
#define most_readreg_rt(dev, addr)                          \
    (dev)->rt_ops.readreg((dev), addr)

/**
 * @see writereg_func
 */
#define most_writereg_rt(dev, value, addr)                  \
    (dev)->rt_ops.writereg((dev), (value), (addr))

/**
 * @see usage_func
 */
#define most_manage_usage(dev, change)                      \
    (dev)->manage_usage((dev), (change))

/**
 * Returns the major device number from a struct most_pci_device data
 * structure.
 *
 * @param most_device a pointer to a most_pci_device structure
 */
#define MOST_DEV_MAJOR(most_device)                 \
        ((most_device)->major_device_no)

/**
 * Returns the card number (from 0 to 7 including). Each card number is unique.
 *
 * @param most_device a pointer to a most_pci_device structure
 */
#define MOST_DEV_CARDNUMBER(most_device)            \
        ((most_device)->card_number)


/**
 * This structure holds the information for all drivers which register
 * themselves in the MOST base driver.
 */
struct most_high_driver {
    const char         *name;          /**< the name of the driver */
    struct list_head   sema_list;      /**< list protected by a semaphore */
    struct list_head   spin_list;      /**< list protected by a spinlock */
    probe_func         probe;          /**< the probe function that gets called if the card
                                            was found or the module was load (i.e. the
                                            driver was registered) and the card is already
                                            present */
    remove_func        remove;         /**< deregistration function */
    u8                 interrupt_mask; /**< if interrupt_handler is non-NULL, you have to 
                                            set the interrupt mask because this determines
                                            when the interrupt handler is called. It's the
                                            same as the INTMASK register on p. 54 of OS8604 
                                            specification (see most-general.h for symbolic
                                            constants). If 0, the interrupt_handler never
                                            will be called. */
    irq_func           int_handler;    /**< interrupt handler, gets executed if an interrupt
                                            is handles by the most_pci driver */
};

/**
 * This structure holds the information for all drivers which register themselves in the
 * MOST base driver. This are so-called low drivers, i. e. drivers that handle 
 * device-specific functions like interrupts.
 */ 
struct most_low_driver {
    const char*      name;             /**< the name of the driver */
    struct list_head list;             /**< this will be a linked-list */
    void (*high_driver_registered)(struct most_high_driver *high_drv);
                                       /**< see documentation of high_driver_registered */
    void (*high_driver_deregistered)(struct most_high_driver *high_drv);
                                       /**< see documentation of high_driver_deregistered */
    proc_show_func   proc_show;        /**< see documentation of proc_show_func */
};

/*
 * Function prototypes * ------------------------------------------------------
 */

/**
 * Registers a higher level driver.
 *
 * @param driver the driver to register
 * @return 0 on success, an error code on failure
 */
int most_register_high_driver(struct most_high_driver *driver);

/**
 * Deregisters a higher level driver. This means that interrupts and remove
 * function calls will not be processed to the driver any more from now. This
 * means that the remove function is called.
 *
 * @param driver the driver to remove from the list
 */
void most_deregister_high_driver(struct most_high_driver *driver);

/**
 * Registers a low driver.
 *
 * @param low_driver the driver to register. The list element must be
 *        (normally statically) initialized
 */
void most_register_low_driver(struct most_low_driver *low_driver);

/**
 * Deregisters a low driver.
 *
 * @param low_driver the driver to deregister
 */
void most_deregister_low_driver(struct most_low_driver *low_driver);

/**
 * Creates a new most_dev structure (dynamically by allocating memory and
 * filling following elements
 *
 *  - lock: initializing the spinlock value
 *  - list: initializing the list
 *  - major_device_no: filling the allocated device number
 *  - card_number: next card number
 *  - impl: setting to NULL
 *
 * @return the allocated and initialized sturct most_dev on success, NULL on
 *         failure
 */
struct most_dev *most_dev_new(void);

/**
 * Frees a most_dev structure. This frees the memory of the given pointer plus
 * the dev->impl. It decrements also the device count which is the reason why
 * this must be implemented as function (we avoid exporting the device_count
 * variable here, and because we only need this in
 * initializiation/deinitialization code, speed is no issue here).
 *
 * @param dev the device
 */
void most_dev_free(struct most_dev* dev);

/**
 * Linked list of all MOST PCI Low Drivers. 
 */
extern struct rwsema_locked_list most_base_low_drivers;

/**
 * Linked list of all MOST High Drivers, protected by a semaphore.
 */
extern struct rwsema_locked_list most_base_high_drivers_sema;

/**
 * Linked list of all MOST High drivers, protected by a spinlock.
 */
extern struct spin_locked_list most_base_high_drivers_spin;


#endif /* __KERNEL__ */
#endif /* MOST_BASE */

/* vim: set ts=4 et sw=4: */
