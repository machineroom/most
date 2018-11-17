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
#include <rtdm/rtdm_driver.h>

#include "serial-rt-debug.h"
#include "most-common.h"

/**
 * @file serial-rt-debug.c
 * @ingroup rtcommon
 *
 * @brief Implementation to output debug information on serial line.
 *
 * This implementation uses the 16550A driver of RTAI/Xenomai.
 */

/**
 * Prefix used for printk() outputs.
 */
#define PR                   "serial-rt-debug: "


/**
 * The configuration used for the serial communication
 */
static const struct rtser_config serial_configuration = {
    .config_mask        = RTSER_SET_BAUD | RTSER_SET_PARITY | 
                          RTSER_SET_DATA_BITS | RTSER_SET_STOP_BITS |
                          RTSER_SET_HANDSHAKE,
    .baud_rate          = SERIAL_RT_BAUDRATE,
    .parity             = SERIAL_RT_PARITY,
    .data_bits          = SERIAL_RT_DATABITS,
    .stop_bits          = SERIAL_RT_STOPBITS,
    .handshake          = SERIAL_RT_HANDSHAKE
};

#ifdef SERIAL_RT_DEBUG

/**
 * File descriptor
 */
static int s_fd;

/*
 * Documentation: see header
 */
int serial_rt_debug_init(void)
{
    int       err;

    return_value_if_fails(s_fd <= 0, 0);

    s_fd = rtdm_open(SERIAL_RT_DEVICE, 0);
    if (unlikely(s_fd < 0)) {
        pr_warning(PR "rtdm_open failed with %d\n", s_fd);
        goto out;
    }

    err = rtdm_ioctl(s_fd, RTSER_RTIOC_SET_CONFIG, &serial_configuration);
    if (unlikely(err < 0)) {
        pr_warning(PR "rtdm_ioctl failed with %d\n", err);
        goto out_close;
    }

    return 0;

out_close:
    rtdm_close(s_fd);
out:
    return s_fd;
}

/*
 * Documentation: see header
 */
int serial_rt_debug_write(const char *format, ...)
{
    va_list         args;
    int             ret;
    int             err;
    static char     buffer[1024];

    return_value_if_fails_dbg(s_fd >= 0, -EINVAL);

    va_start(args, format);
    ret = vsnprintf(buffer, 1024, format, args);
    va_end(args);

    if (unlikely(ret < 0)) {
        pr_warning(PR "vsnprintf failed with %d\n", ret);
        return ret;
    }
    buffer[1023] = 0;

    /* write to the hardware */
    err = rtdm_write(s_fd, buffer, strlen(buffer));
    if (unlikely(err < 0)) {
        pr_warning(PR "rtdm_write failed with %d\n", err);
        return err;
    }

    return ret;
}

/*
 * Documentation: see header
 */
void serial_rt_debug_finish(void)
{
    return_if_fails(s_fd >= 0);

    rtdm_close(s_fd);
    s_fd = 0;
}

#endif /* SERIAL_RT_DEBUG */

/* vim: set ts=4 et sw=4: */
