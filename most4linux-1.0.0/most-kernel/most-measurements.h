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
#ifndef MOST_MEASUREMENTS_H
#define MOST_MEASUREMENTS_H

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifdef __KERNEL__
#  include "most-rxbuf.h"
#endif

/**
 * @file most-measurements.h
 * @ingroup common
 *
 * @brief Functionality that was added for measurements.
 *
 * Functionality that was added for measurements. The first group are memory
 * accessed that do nothing in reality but that were added to enable the PCI
 * tracing.
 */

/* print a compiler warning if measuring are turned on */
#if defined(MEASURING_PCI) || defined(MEASURING_SCHED)
#   warning "Driver is compiled with measurements turned on. This may give strange results!"
#endif

/**
 * High 16 bit word of the magic number that is used to identify the start of
 * data that is needed for measurements.
 *
 * @see MEASUREMENT_MAGIC_START_REC_ISR
 */
#define MEASUREMENT_MAGIC_START_HIGH        0x90520000

/**
 * High 16 bit word of the magic number that is used to identify the end of
 * data that is needed for measurements.
 *
 * @see MEASUREMENT_MAGIC_END_REC_ISR
 */
#define MEASUREMENT_MAGIC_END_HIGH          0x52900000

/**
 * Magic number that is used to determine the start of the time stamp which is
 * added in the receive interrupt service routine.  It's needed to measure the
 * scheduling latency.
 */
#define MEASUREMENT_MAGIC_START_REC_ISR                \
    (0x243A | MEASUREMENT_MAGIC_START_HIGH)

/**
 * Magic number that is used to determine the end of the time stamp which is
 * added in the receive interrupt service routine. It's needed to measure the
 * scheduling latency.
 */
#define MEASUREMENT_MAGIC_END_REC_ISR                  \
    (0x3A24 | MEASUREMENT_MAGIC_END_HIGH)

#if defined(RT_RTDM) || defined(DOXYGEN)
/**
 * The constant that is used for measuring the interrupt latency. This differs
 * for RT and NRT because of compatibility reasons.
 *
 *   - NRT: MOST_PCI_RESERVED_4 (0x74)
 *   - RT:  MOST_PCI_RESERVED_5 (0x78)
 */
#define MEASUREMENT_INT_CONSTANT                       \
    MOST_PCI_RESERVED_5
#else
#define MEASUREMENT_INT_CONSTANT                       \
    MOST_PCI_RESERVED_4
#endif

/**
 * Macro to combine a low and a high byte value
 */
#define MOST_LH(low, high)                             \
    ((((high) & 0xFFFF) << 16) | ((low) & 0xFFFF))

/**
 * Macro to get the low value.
 */
#define MOST_LOW(value)                                \
    ((value) & 0xFFFF)

/**
 * Macro to get the high value
 */
#define MOST_HIGH(value)                               \
    (((value) & 0xFFFF0000) > 16)


#ifdef __KERNEL__

#if defined(MEASURING_PCI) || defined(DOXYGEN) 
/**
 * Accesses the MOST_PCI_RESERVED_4 (NRT) or MOST_PCI_RESERVED_5 (RT) register
 * at the begin of a real-time interrupt service routine if @c MEASURING_PCI is
 * turned on.
 */
#define measuring_int_begin()                                               \
        do {                                                                \
            readreg_int(dev, MEASUREMENT_INT_CONSTANT);                     \
        } while (0)

/**
 * Accesses the MOST_PCI_RESERVED_6 register at the end of the ISR if @c
 * MEASURING_PCI is turned on.
 */
#define measuring_int_end()                                                 \
        do {                                                                \
            udelay(1);                                                      \
            readreg_int(dev, MOST_PCI_RESERVED_6);                          \
        } while (0)

/**
 * Prints an error message in case @c MEASURING_PCI was defined. This is used
 * if the ISR detects that it is not responsible for the specific interrupt.
 * IRQ sharing doesn't work with the measuring method.
 */
#define measuring_int_error_sharing()                                       \
        rtnrt_warn("IRQ sharing is not supported with measurements\n")

#else
#define measuring_int_begin()                                               \
        do { } while (0)
#define measuring_int_end()                                                 \
        do { } while (0)
#define measuring_int_error_sharing()                                       \
        do { } while (0)
#endif

#endif /* __KERNEL__ */

/**
 * Data structure which represents the field that is inserted
 * when measuring the scheduling latency in the interrupt service
 * routine.
 */
struct measuring_schedlat_data {
    unsigned int        magic_start;    /**< the magic start field */
    unsigned int        counter;        /**< a counter that prevents
                                             overruns */
    unsigned long long  tsc;            /**< the time stamp */
    unsigned int        magic_end;      /**< the magic end field */
} __attribute__((packed));


#ifdef __KERNEL__
#if defined(MEASURING_SCHED) || defined(DOXYGEN)

/**
 * Copy of the receive buffer.
 *
 * @see measuring_receive_isr_start()
 */
static struct rx_buffer rxbuf_copy;

/**
 * Must be called before modifying the buffer in the ISR of the MOST
 * synchronous driver. It simply copies the write pointer of the buffer
 * so that the time stamp can be added @e after the modification taked
 * place.
 *
 * @param[in] buffer a pointer to the receive buffer which is modified afterwards
 */
static inline void measuring_receive_isr_start(struct rx_buffer *buffer)
{
    rxbuf_copy = *buffer;
}

/**
 * Adds the timestamp to the receive interrupt service routine. This function
 * expands to nothing if @c MEASURING_SCHED is not defined. See also the
 * diploma thesis, chapter "Evaluation" and the @c README of 
 * measuring/3_sched_latency.
 */
static inline void measuring_receive_isr_wakeup(void)
{
    static unsigned int                cnt = 0;
    struct measuring_schedlat_data     data;

    data.magic_start = MEASUREMENT_MAGIC_START_REC_ISR;
    data.counter = cnt++;
    data.magic_end = MEASUREMENT_MAGIC_END_REC_ISR;
    rdtscll(data.tsc);
    rxbuf_put(&rxbuf_copy, (void *)&data, sizeof(data));
}
#else
static inline void measuring_receive_isr_start(struct rx_buffer *buffer)
{}
static inline void measuring_receive_isr_wakeup(void)
{}
#endif
#endif /* __KERNEL__ */


#if defined(MEASURING_SCHED) || defined(MEASURING_PCI) || defined(DOXYGEN)
/**
 * Prints a warning if the module is loaded (i.e. the function is called) if the 
 * driver was compiled with measurings turned on. Does nothing otherwise.
 */
static inline void print_measuring_warning(void)
{
	rtnrt_warn("Measuring turned on. This may give strange "
            "results in normal operation!\n");
}
#else
static inline void print_measuring_warning(void)
{}
#endif

#endif /* MOST_MEASUREMENTS_H */

