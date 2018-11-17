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
#include <linux/seq_file.h>
#include <linux/rwsem.h>
#include <linux/delay.h>

#ifdef RT_RTDM
#   include <rtdm/rtdm_driver.h>
#   include "most-common-rt.h"
#endif

#include "most-constants.h"
#include "most-pci.h"
#include "most-base.h"
#include "most-measurements.h"

/**
 * @file most-pci.c
 * @ingroup pci
 *
 * @brief Implementation of the MOST PCI driver
 */

/**
 * The name of the driver.
 */
#define DRIVER_NAME                     "most-pci"

/**
 * The prefix for printk outputs.
 */
#define PR                              DRIVER_NAME ": "

/**
 * Maximum loop cound for reading the DSCTRL register.
 */
#define SCTL_MAXLOOP                    10000

/**
 * Variable that holds the driver version.
 */
static char *version = "$Rev: 639 $";

/* forward declarations ------------------------------------------------------*/
static u32         readreg           (struct most_dev *, u32);
static void        writereg          (struct most_dev *, u32, u32);
static void        changereg         (struct most_dev *, u32, u32, u32);
static int         writereg_8104     (struct most_dev *, unsigned char *, size_t, u32);
static int         readreg_8104      (struct most_dev *, unsigned char *, size_t, u32);
static bool        get_license       (struct most_dev *);
static void        revision          (struct most_dev *);
static void        intset            (struct most_dev *, unsigned int, unsigned int, 
                                      unsigned int *);
static void        reset             (struct most_dev *);
static void        intclear          (struct most_dev *, unsigned int);
static int         features          (struct most_dev *);
static int         dma_allocate      (struct most_dev *, struct dma_buffer *); 
static void        dma_deallocate    (struct most_dev *, struct dma_buffer *);

/* module parameters ------------------------------------------------------- */

/**
 * Module parameter which disables IRQ sharing.
 */
static int disable_shared_irq = false;

#ifndef DOXYGEN
module_param(disable_shared_irq, bool, S_IRUGO);
MODULE_PARM_DESC(disable_shared_irq, "Don't support IRQ sharing if set to true (default: false)");
#endif

/* general static data elements -------------------------------------------- */

/**
 * Array of all MOST PCI devices. 
 */
static struct most_dev* devices[MOST_DEVICE_NUMBER];

/**
 * Lock for @b all PCI devices. This is needed to travers the devices list.
 */
static struct rw_semaphore sema = __RWSEM_INITIALIZER(sema);

/**
 * The ID table for this PCI device.
 */
static const struct pci_device_id ids[] = { 
    { PCI_DEVICE(PCI_VENDOR_ID_OASIS, PCI_DEVICE_ID_OASIS_MOST_PCI_INTERFACE) },
    { PCI_DEVICE(0, 0) },
};

/**
 * The module device table for Linux
 */
MODULE_DEVICE_TABLE(pci, ids);


/* functions --------------------------------------------------------------- */

/**
 * Reads out an address until a given bit is clear. This is done in a 
 * "intelligent" way, i.e. it's only looped @c MOST_MAX_POLL times and
 * a delay (without bus transfers) is added between the tries.
 *
 * @param addr the address to read from
 * @param bitmask the bit mask to check for
 * @param timeout if not NULL this value will be set to @c true if the 
 *        operation finished because a timeout (and not because of the
 *        condition is met) and to @c false if the operation finished
 *        because the condition was met
 * @return the value at @p addr
 */
static inline u32 loop_until_bit_is_clear(void __iomem  *addr,
                                          u32           bitmask,
                                          bool          *timeout)
{
    volatile unsigned long  count = 0;
    u32                     val;
    unsigned long           delay_ns = 0;

    do {
        val = ioread32(addr);
        ndelay(delay_ns);
        delay_ns += MOST_DELAY_INCREMENT;
    } while ((val & bitmask) && (count++ < MOST_MAX_POLL));

    if (val == MOST_MAX_POLL) {
        if (timeout) {
            *timeout = true;
        }

        rtnrt_err("Busy loop timed out\n");
    } else if (timeout) {
        *timeout = false;
    }

    return val;
}

/**
 * Internal readreg function, inlined.
 *
 * @param[in] dev the MOST device
 * @param[in] address address the address to read
 */
static inline u32 readreg_int(struct most_dev* dev, u32 address)
{
    u32 val = ioread32(PCI_DEV(dev)->mem + address);
    pr_reg_access_debug(PR "REGREAD 0x%x=0x%x\n", address, val);
    return val;
}

/**
 * Internal  writereg function, inlined.
 *
 * @param[in] dev the MOST device
 * @param[in] value the value to write at @p address
 * @param[in] address address the address to read
 */
static inline void writereg_int(struct most_dev* dev, u32 value, u32 address)
{
    iowrite32(value, PCI_DEV(dev)->mem + address);
    pr_reg_access_debug(PR "REGWRITE 0x%x=0x%x\n", address, value);
}

/**
 * Resets the Dallas Silicon Key (LIC)
 *
 * @param[in] dev the MOST device
 */
static inline bool keyctrl_reset(struct most_dev *dev)
{
    u32 val;
    int i = SCTL_MAXLOOP;

    val = readreg_int(dev, MOST_PCI_DSCTRL_REG);
    val |= DSCTL_RST;
    writereg_int(dev, val, MOST_PCI_DSCTRL_REG);
	do {
		if (!(readreg_int(dev, MOST_PCI_DSCTRL_REG) & DSCTL_RST)) {
			break;
        }
	} while (--i);

	if (!(readreg_int(dev, MOST_PCI_DSCTRL_REG) & DSCTL_PRE)) {
        rtnrt_err(PR "keyctrl_reset failed\n");
        return false;
    }

	return true;
}

/**
 * (LIC)
 * Dallas Silicon Key Control (DSCTRL) Register
 *
 * @param[in] dev the MOST device
 */
static unsigned char dsc_shift_read_byte(struct most_dev *dev)
{
    unsigned char data = 0x00;
	int i, count;
    u32 val;

	for (count = 0; count < 8; count++) {
        /* set SK_RD bit */
        val = readreg_int(dev, MOST_PCI_DSCTRL_REG);
        val |= DSCTL_RD;
        writereg_int(dev, val | DSCTL_RD, MOST_PCI_DSCTRL_REG);
        
        /* Wait till SK_RD-Bit is cleared */
		i = SCTL_MAXLOOP;
		do {
            val = readreg_int(dev, MOST_PCI_DSCTRL_REG);
			if (!(val & DSCTL_RD)) {
				break;
            }
		} while(--i);

        if (val & DSCTL_RD_VAL) {
            data |= (1 << count);
        }
	}

    return data;
}

/**
 * (LIC)
 * Dallas Silicon Key Control (DSCTRL) Register
 *
 * @param[in] dev the MOST device
 * @param[in] data the data to write
 */
static void dsc_shift_write_byte(struct most_dev *dev, unsigned char data)
{
	int i, count;
    u32 val;

	for (count = 0; count < 8; count++) {
        val = readreg_int(dev, MOST_PCI_DSCTRL_REG);
		if (data & (1 << count)) {
            /* write a '1' */
            val |= DSCTL_WR | DSCTL_WR_VAL;
        } else {
            /* write a '0' */
            val |= DSCTL_WR;
            val &= ~DSCTL_WR_VAL;
        }
        writereg_int(dev, val, MOST_PCI_DSCTRL_REG);
        
        /* Wait till SK_WR-Bit is cleared */
		i = SCTL_MAXLOOP;
		do {
			if (!(readreg_int(dev, MOST_PCI_DSCTRL_REG) & DSCTL_WR)) {
				break;
            }
		} while(--i);
	}
}

/**
 * Gets the license.
 *
 * @return @c true on success, @c false on failure
 */
static bool get_license(struct most_dev *dev)
{
    unsigned short license[4];
	int i;

    /* reset DS2430A */
    if (!keyctrl_reset(dev)) {
        return false;
    }

    /* 'Skip ROM' */
	dsc_shift_write_byte(dev, 0xCC); 
    
    /* 'Read Memory' (copys EEPROM to ScratchP) */
	dsc_shift_write_byte(dev, 0xF0);  
    
    /* Transfer Address to read */
	dsc_shift_write_byte(dev, LICENSE_ADDRESS_HW + 0x00);

	for (i = 0; i < 4; i++) {
		license[i]  = dsc_shift_read_byte(dev);
		license[i] |= dsc_shift_read_byte(dev) << 8;
	}
    rtnrt_info(PR "License: %04X-%04X-%04X-%04X\n", 
            license[3], license[2], license[1], license[0]);
    
    /* Set CRC_RST-Bit and clear it after that */
    writereg_int(dev, DSCTL_CRC_RST, MOST_PCI_DSCTRL_REG);
    writereg_int(dev, 0x00, MOST_PCI_DSCTRL_REG);
    
    /* reset */
    if (!keyctrl_reset(dev)) {
        return false;
    }

    /*'Read ROM' Command */
    dsc_shift_write_byte(dev, 0x33);
    
    /* Enable CRC */
    writereg_int(dev, DSCTL_CRC_EN, MOST_PCI_DSCTRL_REG);

    /* Family Code */
    dev->serial_number = (u64)dsc_shift_read_byte(dev);
    /* Serial.Byte0 */
    dev->serial_number |= dsc_shift_read_byte(dev) << 8;

    /* Move LicenseKey to LicenseRegister */
    writereg_int(dev, license[0], MOST_PCI_LICENSE_REG);
    /* Write to activate Comparator */
    writereg_int(dev, 0x00000000, MOST_PCI_LICCOMP_REG);

    /* Serial.Byte1 */
	dev->serial_number |= dsc_shift_read_byte(dev) << 16;
    /* Serial.Byte2 */
	dev->serial_number |= (u64)dsc_shift_read_byte(dev) << 24;

    /* Move LicenseKey LicenseRegister */
	writereg_int(dev, license[1], MOST_PCI_LICENSE_REG);
    /* Write to activate Comparator */
	writereg_int(dev, 0x00000000, MOST_PCI_LICCOMP_REG);

    /* Serial.Byte3 */
	dev->serial_number |= (u64)dsc_shift_read_byte(dev) << 32;
    /* Serial.Byte4 */
	dev->serial_number |= (u64)dsc_shift_read_byte(dev) << 40;

    /* Move LicenseKey LicenseRegister */
	writereg_int(dev, license[2], MOST_PCI_LICENSE_REG);
    /* Write to activate Comparator */
	writereg_int(dev, 0x00000000, MOST_PCI_LICCOMP_REG);

    /* Serial.Byte5 */
	dev->serial_number |= (u64)dsc_shift_read_byte(dev) << 48;
    /* CRC of Serial */
	dev->serial_number |= (u64)dsc_shift_read_byte(dev) << 56;

    /* Move LicenseKey LicenseRegister */
	writereg_int(dev, license[3], MOST_PCI_LICENSE_REG);
    /* Write to activate Comparator */
	writereg_int(dev, 0x00000000, MOST_PCI_LICCOMP_REG);

    /* Enable CRC */
    writereg_int(dev, 0x00, MOST_PCI_DSCTRL_REG);

    return true;
}

/**
 * Increase usage count
 *
 * @param dev unused
 * @param change change @c 1 if the usage count should be increased, @c -1 if it
 *        should be decreased
 */
static void manage_usage(struct most_dev *dev, int change)
{
    if (change == 1) {
        try_module_get(THIS_MODULE);
    } else if (change == -1) {
        module_put(THIS_MODULE);
    } else {
        rtnrt_err(PR "manage_usage: change(%d) invalid\n", change);
    }
}

/**
 * Check which features are available. Returns a bitwise combination of
 * MOST_PCI_FEATURE_ASYNC, MOST_PCI_FEATURE_SYNC, MOST_PCI_FEATURE_MASTER
 * and MOST_PCI_FEATURE_CTRL.
 */
int features(struct most_dev *dev)
{
    u32 val;
    int ret = 0;

    val = readreg_int(dev, MOST_PCI_DSCTRL_REG);

    if (val & DSCTL_EN_ASYNC) {
        ret |= MOST_FEATURE_ASYNC;
    }
    if (val & DSCTL_EN_MASTER) {
        ret |= MOST_FEATURE_MASTER;
    }
    if (val & DSCTL_EN_SYNC) {
        ret |= MOST_FEATURE_SYNC;
    }
    if (val & DSCTL_EN_CTRL) {
        ret |= MOST_FEATURE_CTRL;
    }

    return ret;
}

/**
 * Sets the fpga_revision, product_id and fpga_features of the device
 * structure.
 *
 * @param dev the device
 */
static void revision(struct most_dev *dev)
{
    u32 version_reg;

    version_reg = readreg_int(dev, MOST_PCI_VERSION_REG);

    PCI_DEV(dev)->fpga_revision  = version_reg & 0xfff;
    dev->product_id     = (version_reg >> 24) & 0xff;

    PCI_DEV(dev)->fpga_features = 0;
    if (PCI_DEV(dev)->fpga_revision > 27) {
        PCI_DEV(dev)->fpga_features |= MOST_PCI_FW_NEW_PACKET_LEN_POS;
    }
    if (PCI_DEV(dev)->fpga_revision > 220) {
        PCI_DEV(dev)->fpga_features |= MOST_PCI_FW_FOT_STATUS;
    }
    if (PCI_DEV(dev)->fpga_revision > 227) {
        PCI_DEV(dev)->fpga_features |= MOST_PCI_FW_SEC_ASYNC_ADDR;
    }
    if (PCI_DEV(dev)->fpga_revision > 233) {
        PCI_DEV(dev)->fpga_features |= MOST_PCI_FW_RX_TX_BIG_ENABLE;
    }

    rtnrt_info(PR "Product ID: %d, Revision %03d, Features: 0x%X\n",
             dev->product_id, PCI_DEV(dev)->fpga_revision, 
             PCI_DEV(dev)->fpga_features);
}

/**
 * Called from most_base.ko if a high-level driver was registered. Calls the probe
 * function for the high driver for each device already present in the system.
 *
 * @param drv the high driver which was just registered
 */
void high_driver_registered(struct most_high_driver* drv)
{
    int i;

    /* now call each probe function */
    for (i = 0; i < MOST_DEVICE_NUMBER; i++) {
        down_read(&sema);
        if (devices[i] != NULL) {
            drv->probe(devices[i]);
        }
        up_read(&sema);
    }
}

/**
 * Called from most_base.ko if a high-level driver was registered. Calls the
 * remove function for the high driver for each device present in the
 * system.
 *
 * @param drv the high driver which was just deregistered
 */
void high_driver_deregistered(struct most_high_driver* drv)
{
    int i;

    /* now call each remove function */
    for (i = 0; i < MOST_DEVICE_NUMBER; i++) {
        down_read(&sema);
        if (devices[i] != NULL) {
            drv->remove(devices[i]);
        }
        up_read(&sema);
    }
}


/**
 * Shows information about PCI devices in the MOST proc file.
 *
 * @param s the sequence file
 */
void proc_show(struct seq_file *s)
{
    int i;

    for (i = 0; i < MOST_DEVICE_NUMBER; i++) {
        down_read(&sema);
        if (devices[i] != NULL) {
            struct most_dev *dev = devices[i];

            seq_printf(s, "%d : %04X-%04X-%04X-%04X\n", dev->card_number,
                    (unsigned int)(((dev->serial_number) >> 48) & 0xffff),
                    (unsigned int)(((dev->serial_number) >> 32) & 0xffff),
                    (unsigned int)(((dev->serial_number) >> 16) & 0xffff),
                    (unsigned int)(((dev->serial_number)      ) & 0xffff));
        }
        up_read(&sema);
    }
}

/**
 * Does the actual interrupt handling.
 *
 * @param dev the MOST device
 */
static inline rtnrt_irqreturn_t handle_interrupt(struct most_dev *dev)
{
    struct list_head            *cursor;
    struct most_high_driver     *high_driver;
    u32                         intstatus, intmask;
    u32                         intstatus_new  = 0;

    measuring_int_begin();

    /* determine if the interrupt was from this PCI card */
    intstatus = readreg_int(dev, MOST_PCI_INTSTATUS_REG) & 0xff;
    intmask = readreg_int(dev, MOST_PCI_INTMASK_REG) & 0xff;
    intstatus &= intmask;
    if (unlikely(!intstatus)) {
        measuring_int_error_sharing();
        return RTNRT_IRQ_NONE;
    }

    pr_irq_debug(PR "int_handler, status = 0x%x, mask = 0x%x\n", 
            intstatus, intmask);

    /* now call the registered IRQ handlers. */
    rtnrt_lock_get(&most_base_high_drivers_spin.lock);
    list_for_each(cursor, &most_base_high_drivers_spin.list) {
        high_driver = list_entry(cursor, struct most_high_driver, spin_list);

        if (high_driver->int_handler && high_driver->interrupt_mask & intstatus) {
            high_driver->int_handler(dev, intstatus);

            /* interrupt handled => clear the bits */
            intstatus_new |= high_driver->interrupt_mask & intstatus;
        }
    }
    rtnrt_lock_put(&most_base_high_drivers_spin.lock);

    /* now clear the handled interrupts */
    writereg_int(dev, intstatus_new, MOST_PCI_INTSTATUS_REG);

    measuring_int_end();

    return RTNRT_IRQ_HANDLED;
}

/**
 * The real-time interrupt service routine of a PCI driver.
 */
DECLARE_IRQ_PROXY(int_handler, handle_interrupt, struct most_dev);

/**
 * The probe function. This function is called if the PCI card was found, i.e.
 * normally on the insertion of the Kernel module for each PCI card that has
 * the right IDs.
 *
 * @param lpci_dev the pci_dev structure for the device
 * @param id  the id that was responsible for calling this function
 * @return 0 on success, any other error code on failure
 */
static int probe(struct pci_dev *lpci_dev, const struct pci_device_id *id)
{
    int                         err;
    struct most_dev             *dev;
    struct list_head            *cursor;
    struct most_high_driver     *high_driver;
    
    /* 
     * enable the device, this must be the first action because on some systems 
     * (e.g. PC-BUS the
     * addresses and IRQs get assigned only at this state 
     */
    if ((err = pci_enable_device(lpci_dev)) != 0) {
        rtnrt_err(PR "Unable to enable the PCI device\n");
        goto out;
    }
    
    /* 
     * enable bus mastering. This is required if pci_disable_device() was
     * called and the device should be enabled again without rebooting :-)
     */
    pci_set_master(lpci_dev);

    /* allocate the driver structure */
    dev = most_dev_new();
    if (unlikely(!dev)) {
        rtnrt_warn(PR "Allocation of private data structure failed\n");
        err = -ENOMEM;
        goto out_disable_dev;
    }

    /* if this would be the MOST_DEVICE_NUMBERth device, abort */
    if (MOST_DEV_CARDNUMBER(dev) >= MOST_DEVICE_NUMBER) {
        rtnrt_warn(PR "This would be the %dth device, but only %d "
                "devices are supported\n", 
                MOST_DEV_CARDNUMBER(dev), MOST_DEVICE_NUMBER);
        goto out_driver_structure;
    }
    
    /* request the regions of the PCI device */
    err = pci_request_regions(lpci_dev, dev->name);
    if (unlikely(err != 0)) {
        rtnrt_warn(PR "Could not allocate PCI regions.\n");
        goto out_driver_structure;
    }

    /* allocate the PCI device */
    dev->impl = (void *)kmalloc(sizeof(struct most_pci_device), GFP_KERNEL);
    if (unlikely(!dev->impl)) {
        rtnrt_warn(PR "Allocation of most_pci_device failed\n");
        err = -ENOMEM;
        goto out_region;
    }
    memset(PCI_DEV(dev), 0, sizeof(struct most_pci_device));

    /* fill the structure with function pointers */
    dev->manage_usage        = manage_usage;
    dev->ops.readreg         = readreg;
    dev->ops.writereg        = writereg;
    dev->ops.changereg       = changereg;
    dev->ops.readreg8104     = readreg_8104;
    dev->ops.writereg8104    = writereg_8104;
    dev->ops.intset          = intset;
    dev->ops.reset           = reset;
    dev->ops.intclear        = intclear;
    dev->ops.features        = features;
    dev->ops.dma_allocate    = dma_allocate;
    dev->ops.dma_deallocate  = dma_deallocate;
#ifdef RT_RTDM
    dev->rt_ops.readreg      = readreg;
    dev->rt_ops.writereg     = writereg;
#endif

    /* get the interrupt line */
    err = pci_read_config_byte(lpci_dev, PCI_INTERRUPT_LINE, 
            &PCI_DEV(dev)->interrupt_line);
    if (unlikely((err != 0) || (PCI_DEV(dev)->interrupt_line == 0))) {
        rtnrt_warn(PR "Card has no interrupt line assigned\n");
        goto out_region;
    }

    /* request the interrupt */
    err = rtnrt_register_interrupt_handler(&PCI_DEV(dev)->rt_irq_handle,
            PCI_DEV(dev)->interrupt_line, int_handler,
            (disable_shared_irq ? 0 : SA_SHIRQ), dev->name, dev);

    if (unlikely(err != 0)) {
        rtnrt_warn(PR "Interrupt line %d could not be assigned "
                "(error code = %d)\n", PCI_DEV(dev)->interrupt_line, err);
        goto out_region;
    }

    /* enable the interrupt */
    err = rtnrt_irq_enable(&PCI_DEV(dev)->rt_irq_handle);
    if (unlikely(err != 0)) {
        rtnrt_warn(PR "Could not enable real-time PCI interrupts");
        goto out_irq;
    }

    /* map the requested region in memory */
    PCI_DEV(dev)->mem = pci_iomap(lpci_dev, 0, 0);
    if (unlikely(! PCI_DEV(dev)->mem)) {
        rtnrt_warn(PR "Mapping of I/O memory to virtual address failed\n");
        err = -ENOMEM;
        goto out_irq;
    }

    /*  set the license information in the device */
    if (!get_license(dev)) {
        printk(KERN_WARNING PR "get_license failed\n");
        err = -ENODEV;
        goto out_irq;
    }
    revision(dev);

    /* turn off all interrupts */
    most_intset(dev, 0, 0x1ff, NULL);

    rtnrt_info(PR "Serial Number: %04X-%04X-%04X-%04X\n",
                      (unsigned int)(((dev->serial_number) >> 48) & 0xffff),
                      (unsigned int)(((dev->serial_number) >> 32) & 0xffff),
                      (unsigned int)(((dev->serial_number) >> 16) & 0xffff),
                      (unsigned int)(((dev->serial_number)      ) & 0xffff));

    /* 
     * add the device to the global list of devices 
     * increment the counter 
     * and set the private device data
     */
    down_write(&sema);
    devices[MOST_DEV_CARDNUMBER(dev)] = dev;
    up_write(&sema);
    pci_set_drvdata(lpci_dev, dev);
    
    /* set the device */
    PCI_DEV(dev)->lpci_dev = lpci_dev;

    /* now call the probe function of registered drivers */
    down_read(&most_base_high_drivers_sema.lock);
    list_for_each(cursor, &most_base_high_drivers_sema.list) {
        high_driver = list_entry(cursor, struct most_high_driver, sema_list);
        high_driver->probe(dev);
    }
    up_read(&most_base_high_drivers_sema.lock);
    
    rtnrt_info("MOST PCI (device %d) card found and enabled\n", dev->card_number);

	return 0;

out_irq:
    rtnrt_free_interrupt_handler(&PCI_DEV(dev)->rt_irq_handle, 
            PCI_DEV(dev)->interrupt_line, dev);
out_region:
    pci_release_regions(lpci_dev);
out_driver_structure:
    most_dev_free(dev);
out_disable_dev:
    pci_disable_device(lpci_dev);
out:

    return err;
}


/**
 * This function gets called if the PCI card was removed from the system.
 * Since normal PCs are not hot-pluggable, the function is called while
 * unloading the module.
 *
 * @param lpci_dev the pci_dev structure for the device that was removed from
 *        the system
 */
static void remove(struct pci_dev *lpci_dev)
{
    struct list_head            *cursor;
    struct most_high_driver     *high_driver;
    struct most_dev             *dev;

    /* get the driver data */
    dev = (struct most_dev *)pci_get_drvdata(lpci_dev);
    BUG_ON(!dev);

    /* unregister interrupts */
    rtnrt_free_interrupt_handler(&PCI_DEV(dev)->rt_irq_handle, 
            PCI_DEV(dev)->interrupt_line, dev);

    /* call the remove function for each registered driver */
    down_read(&most_base_high_drivers_sema.lock);
    list_for_each(cursor, &most_base_high_drivers_sema.list) {
        high_driver = list_entry(cursor, struct most_high_driver, sema_list);
        high_driver->remove(dev);
    }
    up_read(&most_base_high_drivers_sema.lock);

    /* unmap the I/O memory space */
    pci_iounmap(PCI_DEV(dev)->lpci_dev, PCI_DEV(dev)->mem);

    /* free the regions of the PCI device */
    pci_release_regions(PCI_DEV(dev)->lpci_dev);

    /* disables the device */
    pci_disable_device(PCI_DEV(dev)->lpci_dev);
    
    /* remove from the global list */
    down_write(&sema);
    devices[MOST_DEV_CARDNUMBER(dev)] = NULL;
    up_write(&sema);
    pci_set_drvdata(lpci_dev, NULL);

    rtnrt_info("PCI card removed (device %d)\n", dev->card_number);

    /* free the driver data */
    most_dev_free(dev);
}

/**
 * Reads a register.
 *
 * @param dev the most_dev stucture for the device
 * @param address the address to read from
 * @return the value that was read
 */
static u32 readreg(struct most_dev* dev, u32 address)
{
    return readreg_int(dev, address);
}

/**
 * Writes a register.
 *
 * @param dev the most_dev structure
 * @param value the value to write
 * @param address the address to write to
 */
static void writereg(struct most_dev* dev, u32 value, u32 address)
{
    writereg_int(dev, value, address);
}

/**
 * Sets the bits set in @p value which are set to one in @p mask.
 *
 * @param dev the most_dev structure
 * @param address the address to write to
 * @param value the value to write
 * @param mask the mask for value
 */
static void changereg(struct most_dev *dev, u32 address, u32 value, u32 mask)
{
    unsigned long   flags;
    u32             val;

    spin_lock_irqsave(&dev->lock, flags);

    val = readreg_int(dev, address);
    val = (val & ~mask) | value;
    writereg_int(dev, val, address);
    spin_unlock_irqrestore(&dev->lock, flags);
}

/**
 * Writes MAP. This is only a helper function. The dev->lock_cmdreg must be
 * held when calling this function.
 *
 * @param dev the device
 * @param map the address
 */
static inline void write_map_8104(struct most_dev      *dev, 
                                           unsigned char        map)
{
    int val;
    
    /* 
     *  - wait until EXEC bit is clear
     *  - set DATA bits         \
     *  - set CPOP[1:0] bits     > can be done with one write operation
     *  - set EXEC bit          /
     * see: page 26 of MOST PCI datasheet
     */
    val = loop_until_bit_is_clear(PCI_DEV(dev)->mem + MOST_PCI_CMD_REG,
            EXEC, NULL);

    val = map | EXEC;
    
    writereg_int(dev, val, MOST_PCI_CMD_REG);

    pr_devfunc_debug("Write MAP 0x%x\n", map);
}

/**
 * Writes data. This is only a help function. The dev->lock_cmdreg must be held
 * when calling this function.
 *
 * @param dev the device
 * @param data the data
 */
static inline void write_data_8104(struct most_dev* dev, unsigned char data)
{
    u32 val;
    
    /*
     *  - wait until EXEC bit is clear
     *  - set DATA bits         \
     *  - set CPOP[1:0] bits     > can be done with one write operation
     *  - set EXEC bit          /
     * see: page 26 of MOST PCI datasheet
     */
    val = loop_until_bit_is_clear(PCI_DEV(dev)->mem + MOST_PCI_CMD_REG, 
            EXEC, NULL);

    val = CPOP1 | data | EXEC;

    writereg_int(dev, val, MOST_PCI_CMD_REG);

    pr_devfunc_debug("Write data 0x%x\n", data);
}

/**
 * Reads data. This is only a helper function. The dev->lock_cmdreg must be
 * held when calling this function.
 *
 * @param dev the device
 */
static inline unsigned char read_data_8104(struct most_dev* dev)
{
    u32         val;
    
    /*
     *  - wait until EXEC bit is clear
     *  - set CPOP[1:0] bits     \   can be done with one
     *  - set EXEC bit           /   write operation to the register
     *  - wait until EXEC bit is clear
     *  - read DATA[7:0]
     * see: page 27 of MOST PCI datasheet
     */
    val = loop_until_bit_is_clear(PCI_DEV(dev)->mem + MOST_PCI_CMD_REG,
            EXEC, NULL);

    val = CPOP1 | CPOP0 | EXEC;
    writereg_int(dev, val, MOST_PCI_CMD_REG);

    val = 0;
    val = loop_until_bit_is_clear(PCI_DEV(dev)->mem + MOST_PCI_CMD_REG,
            EXEC, NULL);
    
    val &= VALUE_DATA;
    
    pr_devfunc_debug("read data 0x%x\n", val);

    return val;
}

/**
 * Performs a read of of one or more registers on the OS8104. For a detailled
 * description see the comment near the typedef write_os8104_func.
 *
 * @param dev the most_dev structure for the device (which must be a PCI
 *        structure in that case
 * @param dest the destination to copy (kernel address space)
 * @param len the number of bytes to read
 * @param addr the start address to read from
 * @return the number of bytes that have been read on success, a negative
 *         error code on failure
 */
static int readreg_8104(struct most_dev    *dev, 
                                 unsigned char      *dest, 
                                 size_t             len,
                                 u32                addr)
{
    unsigned long   flags;
    unsigned char   page = (addr >> 8) & 0x03;
    unsigned char   address = addr & 0xff;
    int             read = 0;
    
    spin_lock_irqsave(&dev->lock, flags);

    if (page != PCI_DEV(dev)->page) {
        write_map_8104(dev, MOST_IF_PAGE);
        write_data_8104(dev, page);
        PCI_DEV(dev)->page = page;
    }

    write_map_8104(dev, address);
    
    do {
        *dest++ = read_data_8104(dev);
        read++;
    } while (--len > 0);

    spin_unlock_irqrestore(&dev->lock, flags);
    
    return read;
}

/**
 * Performs a write of of one or more registers on the OS8104. For a detailled
 * description see the comment near the typedef read_os8104_func.
 *
 * @param dev the most_dev structure for the device (which must be a PCI
 *        structure in that case
 * @param src the source array to read from (kernel address space)
 * @param len the number of bytes to write
 * @param addr the start address to write to from
 * @return the number of bytes that have been written on success, a negative
 *         error code on failure
 */
static int writereg_8104(struct most_dev   *dev, 
                                  unsigned char     *src,
                                  size_t            len,
                                  u32               addr)
{
    unsigned long   flags;
    unsigned char   page    = (addr >> 8) & 0x03;
    unsigned char   address = addr & 0xff;
    int             written = 0;
    
    spin_lock_irqsave(&dev->lock, flags);

    if (page != PCI_DEV(dev)->page) {
        write_map_8104(dev, MOST_IF_PAGE);
        write_data_8104(dev, page);
        PCI_DEV(dev)->page = page;
    }

    write_map_8104(dev, address);

    do {
        write_data_8104(dev, *src++);
        written++;
    } while (--len > 0);
    
    spin_unlock_irqrestore(&dev->lock, flags);

    return written;
}

/**
 * Impements the interrupt disable method of a most_dev. 
 *
 * @param dev the most_dev which must be a PCI device here
 * @param interrupts value to set
 * @param mask the mask which must be applied to @p value before applying @p
 *        value to the interrupt mask register
 * @param oldmask if not @c NULL it will be set to the old value of the
 *        interrupt mask register (doesn't correspond to @p mask parameter)
 */
static void intset(struct most_dev    *dev, 
                            unsigned int       interrupts, 
                            unsigned int       mask,
                            unsigned int       *oldmask)
{
    unsigned long   flags;
    u32             val;

    spin_lock_irqsave(&dev->lock, flags);

    /* read the mask */
    val = readreg_int(dev, MOST_PCI_INTMASK_REG);

    /* fill the oldmask if necessary */
    if (oldmask) {
        *oldmask = val;
    }

    /* and write back the new, correct mask */
    val = (val & ~mask) | interrupts;
    writereg_int(dev, val, MOST_PCI_INTMASK_REG);

    pr_irq_debug(PR "intset, interrupts = 0x%x, mask = 0x%x => 0x%x\n",
            interrupts, mask, val);

    spin_unlock_irqrestore(&dev->lock, flags);
}

/**
 * Clears the given interrupt
 *
 * @param dev the MOST device
 * @param interrupts the interrupt mask to clear
 */
static void intclear(struct most_dev *dev, unsigned int interrupts)
{
    writereg_int(dev, interrupts, MOST_PCI_INTSTATUS_REG);
}

/**
 * Reset the MOST Transceiver
 * 
 * @param dev the device
 */
static void reset(struct most_dev *dev)
{
    unsigned long   flags;
    u32             tmp;

    spin_lock_irqsave(&dev->lock, flags);

    /* wait until commands are finished, from WinCE driver */
    tmp = loop_until_bit_is_clear(PCI_DEV(dev)->mem + MOST_PCI_CMD_REG, EXEC,
            NULL);

    /* perform the reset */
    writereg_int(dev, MRST, MOST_PCI_CMD_REG);
    spin_unlock_irqrestore(&dev->lock, flags);
}

/**
 * Allocates a DMA buffer.
 *
 * @param dev the MOST device
 * @param dma the DMA buffer
 * @return 0 on success, a negative error failure
 */
static int dma_allocate(struct most_dev        *dev,
                                struct dma_buffer *dma) 
{
    dma->addr_virt = dma_alloc_coherent(&PCI_DEV(dev)->lpci_dev->dev, 
            dma->size, &dma->addr_bus, GFP_KERNEL);
    pr_reg_access_debug(PR "Allocating %d bytes of DMA memory\n", 
            dma->size);
    if (!dma->addr_virt) {
        rtnrt_err(PR "Allocating of DMA memory failed\n");
        return -ENOMEM;
    }

    return 0;
}


/**
 * Deallocate DMA buffer.
 *
 * @param dev the MOST device
 * @param dma the DMA buffer
 * @return 0 on success, a negative error failure
 */
static void dma_deallocate(struct most_dev     *dev,
                                    struct dma_buffer   *dma)
{
    pr_reg_access_debug(PR "Deallocating %d bytes of DMA memory\n",
            dma->size);
    dma_free_coherent(&PCI_DEV(dev)->lpci_dev->dev, dma->size, dma->addr_virt,
            dma->addr_bus);
}

/**
 * The PCI driver structure for the PCI subsystem of the Kernel.  This driver does
 * not support power-management.
 */
static struct pci_driver pci_driver = {
	.name     = DRIVER_NAME,
	.id_table = ids,
	.probe    = probe,
	.remove   = remove
};

/**
 * The low driver which is registered in the most_base.ko module.
 */
static struct most_low_driver low_driver = {
    .name                       = "most-pci",
    .list                       = LIST_HEAD_INIT(low_driver.list),
    .high_driver_registered     = high_driver_registered,
    .high_driver_deregistered   = high_driver_deregistered,
    .proc_show                  = proc_show
};

/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_pci_init(void)
{
    rtnrt_info("Loading module %s, version %s\n", DRIVER_NAME, version);
    most_register_low_driver(&low_driver);

	return pci_register_driver(&pci_driver);
}

/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_pci_exit(void)
{
    rtnrt_info("Unloading module %s, version %s\n", DRIVER_NAME, version);

    most_deregister_low_driver(&low_driver);
	pci_unregister_driver(&pci_driver);
}

#ifndef DOXYGEN
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 639 $");
MODULE_DESCRIPTION("Base driver for the MOST PCI card from OASIS Silicon "
                   "systems.");
module_init(most_pci_init);
module_exit(most_pci_exit);
#endif

/* vim: set ts=4 et sw=4: */
