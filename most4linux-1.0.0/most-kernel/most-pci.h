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
#ifndef MOST_PCI_H
#define MOST_PCI_H

/**
 * @file most-pci.h
 * @ingroup pci
 *
 * @brief Declarations for the MOST PCI driver
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#ifdef RT_RTDM
#   include <rtdm/rtdm_driver.h>
#endif

#include "most-common.h"

/**
 * Private data for the MOST PCI driver. This information is PCI-card specific.
 */
struct most_pci_device
{
    void __iomem       *mem;                   /**< the memory region that was mapped with
                                                    ioremap() */
    struct pci_dev     *lpci_dev;              /**< the corresponding PCI device */
    u8                 interrupt_line;         /**< the interrupt line. This way we don't have
                                                    to read the value from the configuration
                                                    address space each time */
    short              fpga_revision;          /**< the revision number of the FPGA */
    unsigned int       fpga_features;          /**< the features of the FPGA */
    unsigned char      page;                   /**< the current page (valid are 0..3) */
#ifdef RT_RTDM
    rtdm_irq_t         rt_irq_handle;          /**< the IRQ handle for RTDM */
#endif
};


/**
 * Returns the most_pci_device in the struct most_dev by casting the impl pointer.
 */
#define PCI_DEV(most_device)                                                        \
    ((struct most_pci_device *)((most_device)->impl))

#endif /* MOST_PCI_H */


/* vim: set ts=4 et sw=4: */
