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
 * @file most-sync-rt-m.c
 * @ingroup rtsync
 *
 * @brief Implementation of the MOST Synchronous RTDM driver.
 */

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <linux/rwsem.h>
#include <asm/msr.h>
#include <asm/system.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>

#include "most-constants.h"
#include "most-base.h"
#include "most-sync-rt.h"
#include "most-common-rt.h"
#include "most-rxbuf.h"
#include "most-txbuf.h"
#include "serial-rt-debug.h"
#include "most-measurements.h"

/**
 * The name of the driver.
 */
#define DRIVER_NAME                     "most-sync-rt"

/**
 * The major version. Used as RTDM device version.
 */
#define DRIVER_MAJOR_VERSION            0

/**
 * The minor version. Used as RTDM device version.
 */
#define DRIVER_MINOR_VERSION            1

/**
 * The patchlevel. Used as RTDM device version.
 */
#define DRIVER_PATHLEVEL                0

/**
 * The prefix for printk outputs.
 */
#define PR                              DRIVER_NAME       ": "

/* should see the PR */
#include "most-sync-common.h"

/**
 * Variable that holds the version.
 */
static char *version = "$Rev: 170 $";

/**
 * Array for each device.
 */
struct most_sync_rt_dev *most_sync_rt_devices[MOST_DEVICE_NUMBER];

/* forward declarations ---------------------------------------------------- */
static int     most_sync_nrt_open  (struct rtdm_dev_context *, rtdm_user_info_t *, int);
static int     most_sync_nrt_close (struct rtdm_dev_context *, rtdm_user_info_t *);
static int     most_sync_nrt_ioctl (struct rtdm_dev_context *, rtdm_user_info_t *,
                                    int, void *);
static ssize_t most_sync_rt_read   (struct rtdm_dev_context *, rtdm_user_info_t *, 
                                    void *, size_t);
static ssize_t most_sync_rt_write  (struct rtdm_dev_context *, rtdm_user_info_t *, 
                                    const void *, size_t);

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

/**
 * Debugging to serial interfaces if SYNC_DEBUG is enabled.
 */
#ifdef SYNC_DEBUG
static serial_rt_t s_serial_fd;
#endif

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

/* general static data elements -------------------------------------------- */

/**
 * The template for the struct rtdm_device which is used on device registration.
 * This template is copied and modified then.
 */ static const struct rtdm_device __initdata device_templ = {
    .struct_version    = RTDM_DEVICE_STRUCT_VER,

    .device_flags      = RTDM_NAMED_DEVICE,
    .context_size      = sizeof(struct most_sync_rt_file),
    .device_name       = "",

    .open_rt           = NULL,
    .open_nrt          = most_sync_nrt_open,

    .ops               = {
        .close_rt      = NULL,
        .close_nrt     = most_sync_nrt_close,

        .ioctl_rt      = NULL,
        .ioctl_nrt     = most_sync_nrt_ioctl,

        .read_rt       = most_sync_rt_read,
        .read_nrt      = NULL,

        .write_rt      = most_sync_rt_write,
        .write_nrt     = NULL,
    },

    .device_class      = RTDM_CLASS_MOSTSYNC,
    .device_sub_class  = RTDM_SUBCLASS_MOSTSYNC_OASIS,
    .driver_name       = DRIVER_NAME,
    .driver_version    = RTDM_DRIVER_VER(DRIVER_MAJOR_VERSION,
                                        DRIVER_MINOR_VERSION,
                                        DRIVER_PATHLEVEL),
    .peripheral_name   = "MOST Synchronous Access for PCI Interface",
    .provider_name     = "Bernhard Walle",
};

/**
 * Stops reception (of the whole device)
 * See p. 33 of OS8604 specification
 *
 * @param[in,out] file the MOST synchonous file (rt)
 */
static inline void most_sync_nrt_stop_rx(struct most_sync_rt_file *file)
{
    most_sync_stop_rx_common(file->sync_dev, file);
}

/**
 * Stops transmission (of the whole device)
 * See p. 33 of OS8604 specification
 *
 * @param[in,out] file the MOST synchonous file (rt)
 */
static inline void most_sync_nrt_stop_tx(struct most_sync_rt_file *file)
{
    most_sync_stop_tx_common(file->sync_dev, file);
}

/**
 * Must be executed at the beginning of most_sync_nrt_setup_rx() and 
 * most_sync_nrt_setup_tx().
 *
 * Aquires the lock, i.e. synchronisation between NRT and RT. This means
 * that the RT part must not be in most_sync_rt_read() or most_sync_rt_write().
 * If it's in one of these functions, it waits. 
 *
 * @param sync the synchronisation structure (rx or tx)
 * @return 0 on sucess, a negative error code on failure (interrupted, -EINTR)
 */
static inline int most_sync_nrt_reconfigure_begin(struct most_conf_sync *sync)
{
    int         err;
    bool        ok_to_continue = false;

    while (true) {
        RTDM_EXECUTE_ATOMICALLY(
            ok_to_continue = !sync->reconfigure_flag && sync->counter == 0;
            if (ok_to_continue)
                sync->reconfigure_flag = true;
        );

        if (ok_to_continue) {
            break;
        }

        /* if not ok, wait here */
        err = wait_event_interruptible(sync->wait_reconfigure,
                !sync->reconfigure_flag && sync->counter == 0);
        if (err < 0) {
            return err;
        }
    }

    return 0;
}

/**
 * Must be executed at the end of most_sync_nrt_setup_rx() and 
 * most_sync_nrt_setup_tx().
 *
 * Releases the lock and wakes up waiting NRT and RT tasks.
 *
 * @param sync the synchronisation structure (rx or tx)
 */
static inline void most_sync_nrt_reconfigure_end(struct most_conf_sync *sync)
{
    RTDM_EXECUTE_ATOMICALLY(
        sync->reconfigure_flag = false;

        /* wake up RT tasks waiting until reconfigure_flags becomes false */
        rtdm_event_pulse(&sync->wait_read_write);

        /* wake up other NRT tasks waiting until reconfigure_flags becomes false */
        wake_up_interruptible(&sync->wait_reconfigure);
    );
}

/**
 * Must be executed at the beginning of most_sync_rt_read() and
 * most_sync_rt_write().
 *
 * Checks if any NRT task is in configuratation period and waits until this is
 * finished. 
 *
 * @b WARNING: This breaks real-time predictability since RT is waiting until
 *             NRT performs some tasks. You have to ensure in your application
 *             that no IOCTL call happens inside RT-critical code paths.
 *
 * @param sync the synchronisation structure (rx or tx)
 * @return 0 on success, a negative error value on failure
 */
static inline int most_sync_rt_read_write_begin(struct most_conf_sync *sync)
{
    int err = 0;

    return_value_if_fails_dbg(sync != NULL, -EINVAL);
    assert(sync->counter >= 0);

    /* wait until reconfigure_flag is zero */
    RTDM_EXECUTE_ATOMICALLY(
        if (sync->reconfigure_flag)
            err = rtdm_event_wait(&sync->wait_read_write);

        /* 
         * increase the read/write counter => no reconfigure ioctl may take
         * place 
         */
        if (err >= 0)
            sync->counter++;
    );

    return err;
}

/**
 * Must be executed at the end of most_sync_rt_read() and
 * most_sync_rt_write().
 *
 * Decreases the write counter and wakes up waiting Linux tasks if necessary.
 *
 * @param sync the synchronisation structure (rx or tx)
 */
static inline void most_sync_rt_read_write_end(struct most_conf_sync *sync)
{
    /* 
     * use RTDM_EXECUTE_ATOMICALLY because this syncs between NRT and RT as
     * well
     */
    RTDM_EXECUTE_ATOMICALLY(
        /* wake up Linux processes if necessary */
        if (--sync->counter == 0)
            wake_up_interruptible(&sync->wait_reconfigure);
    );

    assert(sync->counter >= 0);
}

/**
 * Gets called if the MOST Synchronous RT device gets opened.
 * Callable only from NRT context
 *
 * Does following tasks:
 *   - mages the device counter
 *   - initialises the context structure
 *   - puts the file in the list of openend files in the device
 *
 * @param context Context structure associated with opened device instance
 * @param user_info Opaque pointer to information about user mode caller, 
 *        NULL if kernel mode call
 * @param oflag Open flags as passed by the user
 */
static int most_sync_nrt_open(struct rtdm_dev_context      *context, 
                              rtdm_user_info_t             *user_info,
                              int                          oflag)
{
    struct most_sync_rt_dev     *sync_dev;
    struct most_sync_rt_file    *file = (void *)context->dev_private;

    sync_dev = container_of((struct rtdm_device *)context->device, 
                            struct most_sync_rt_dev, rtdm_dev);
    most_manage_usage(sync_dev->most_dev, 1);

    pr_sync_debug(PR "most_sync_open called for PCI card %d\n",
                  MOST_DEV_CARDNUMBER(sync_dev->most_dev));

    /* initialize members of the private data structure */
    memset(file, 0, sizeof(struct most_sync_rt_file));
    INIT_LIST_HEAD(&file->list);
    file->sync_dev = sync_dev;

    /* check and increase the counter */
    if (atomic_inc_and_test(&sync_dev->open_count)) {
        rtnrt_err(PR "Too much open (%d) for a MOST device,"
                " only %d allowed\n", atomic_read(&sync_dev->open_count),
                MOST_SYNC_OPENS);
        goto out_dec;
    }

    /* add the file to the list */
    rtdm_lock_get(&sync_dev->lock);
    list_add_tail(&file->list, &sync_dev->file_list);
    rtdm_lock_put(&sync_dev->lock);

    return 0;

out_dec:
    atomic_dec(&sync_dev->open_count);
    most_manage_usage(sync_dev->most_dev, -1);
    return -EBUSY;
}

/**
 * Gets called if the MOST Synchronous RT device should be closed.  Callable
 * only from NRT context.
 *
 * @param context Context structure associated with opened device instance
 * @param user_info Opaque pointer to information about user mode caller, NULL if
 *        kernel mode call
 */
static int most_sync_nrt_close(struct rtdm_dev_context     *context,
                               rtdm_user_info_t            *user_info)
{
    struct most_sync_rt_file    *file = (void *)context->dev_private;
    struct most_sync_rt_dev     *sync_dev = file->sync_dev;
    int                         err = 0, count = 0;

    pr_sync_debug(PR "most_sync_rt_close called for PCI card %d\n",
            MOST_DEV_CARDNUMBER(sync_dev->most_dev));;

    /* remove the device from the list and check if it's the last close */
    rtdm_lock_get(&sync_dev->lock);
    list_del(&file->list);
    rtdm_lock_put(&sync_dev->lock);

    /* check if it's the last reader */
    do {
        if (file->rx_running && atomic_dec_and_test(&sync_dev->receiver_count)) {
            pr_sync_debug(PR "Last Reader\n");
            err = most_sync_nrt_reconfigure_begin(&sync_dev->rx_sync);
            if (err >= 0) {
                most_sync_last_closed_rx(sync_dev, file, most_sync_nrt_stop_rx);
                most_sync_nrt_reconfigure_end(&sync_dev->rx_sync);
            }
        }

        /* check if it's the last writer */
        if (file->tx_running && atomic_dec_and_test(&sync_dev->transmitter_count)) {
            pr_sync_debug(PR "Last Transmitter\n");
            err = most_sync_nrt_reconfigure_begin(&sync_dev->tx_sync);
            if (err >= 0) {
                most_sync_last_closed_tx(sync_dev, file, most_sync_nrt_stop_tx);
                most_sync_nrt_reconfigure_end(&sync_dev->tx_sync);
            }
        }
    } while (err != 0 && count++ < MAX_RETRIES);

    if (count != 0) {
        rtnrt_warn(PR "Aquiring the lock failed %d times, err = %d\n", count, err);
    }

    most_manage_usage(sync_dev->most_dev, -1);
    atomic_dec(&sync_dev->open_count);

    return 0;
}

/**
 * See documentation of MOST_SYNC_RT_SETUP_RX.
 *
 * @param file the most_syncrt__file structure 
 * @param user_info Opaque pointer to information about user mode caller, 
 *        NULL if kernel mode call
 * @param ioctl_arg the already checked ioctl argument
 */
static inline int most_sync_nrt_setup_rx(struct most_sync_rt_file *file,
                                         rtdm_user_info_t         *user_info,
                                         void                     *ioctl_arg)
{
    struct frame_part           *arg = (struct frame_part *)ioctl_arg;
    struct most_sync_rt_dev     *sync_dev = file->sync_dev;
    int                         err = 0;
    struct frame_part           param;
    
    /* get the argument */
    copy_from_user_or_kernel(err, user_info, &param, arg, sizeof(struct frame_part));
    if (unlikely(err != 0)) {
        return -EFAULT;
    }

    pr_sync_debug(PR "size = %d, offset = %d\n", param.count,
            param.offset);

    /* on error, the task was interrupted */
    err = most_sync_nrt_reconfigure_begin(&sync_dev->rx_sync);
    if (err < 0) {
        return err;
    }

    /* if it's running, stop it*/
    if (file->rx_running) {
        most_sync_nrt_stop_rx(file);
    }

    most_sync_setup_rx_common(param, file, sync_dev, hw_rx_buffer_size,
            sw_rx_buffer_size, err, most_sync_rt_file);

    most_sync_nrt_reconfigure_end(&sync_dev->rx_sync);

    return err;
}
            
/**
 * See documentation of MOST_SYNC_RT_SETUP_TX.
 *
 * @param file the most_sync_rt_file structure
 * @param user_info Opaque pointer to information about user mode caller, 
 *        NULL if kernel mode call
 * @param ioctl_arg the already checked ioctl argument
 */
static inline int most_sync_nrt_setup_tx(struct most_sync_rt_file *file, 
                                         rtdm_user_info_t         *user_info, 
                                         void                     *ioctl_arg)
{
    struct frame_part           *arg = (struct frame_part *)ioctl_arg;
    struct most_sync_rt_dev     *sync_dev = file->sync_dev;
    int                         err = 0;
    struct frame_part           param;
    
    /* get the argument */
    copy_from_user_or_kernel(err, user_info, &param, arg, sizeof(struct frame_part));
    if (unlikely(err != 0)) {
        return -EFAULT;
    }

    /* on error, the task was interrupted */
    err = most_sync_nrt_reconfigure_begin(&sync_dev->tx_sync);
    if (err < 0) {
        return err;
    }

    /* if it's running, stop it*/
    if (file->tx_running) {
        most_sync_nrt_stop_rx(file);
    }

    most_sync_setup_tx_common(param, file, sync_dev, hw_tx_buffer_size,
            sw_tx_buffer_size, err, most_sync_rt_file);
    most_sync_nrt_reconfigure_end(&sync_dev->tx_sync);

    return err;
}

/**
 * Gets called if the MOST Synchronous RT device should be configured
 * Callable only from NRT context.
 * 
 * @param context Context structure associated with opened device instance
 * @param user_info Opaque pointer to information about user mode caller, NULL
 *        if kernel mode call
 * @param request Request number as passed by the user
 * @param arg Request argument as passed by the user
 */
static int most_sync_nrt_ioctl(struct rtdm_dev_context *context,
                               rtdm_user_info_t        *user_info, 
                               int                     request, 
                               void                    *arg)
{
    struct most_sync_rt_file    *file = (void *)context->dev_private;
    int                         err = 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (unlikely((_IOC_TYPE(request) != MOST_SYNC_RT_IOCTL_MAGIC))) {
        return -ENOTTY;
    }

    /* if ioctl() is called from userspace, check the arguments */
    if (user_info) {
        if (_IOC_DIR(request) & _IOC_READ) {
            err = !rtdm_read_user_ok(user_info, arg, _IOC_SIZE(request));
        } else if (_IOC_DIR(request) & _IOC_WRITE) {
            err = !rtdm_rw_user_ok(user_info, arg, _IOC_SIZE(request));
        }

        if (err) {
            return -EFAULT;
        }
    }

    /* now do the command */
    switch (request) {
        case MOST_SYNC_RT_SETUP_RX:
            return most_sync_nrt_setup_rx(file, user_info, arg);

        case MOST_SYNC_RT_SETUP_TX:
            return most_sync_nrt_setup_tx(file, user_info, arg);

        default:
            return -ENOTTY;
    }

    return 0;
}

/**
 * Gets called if data should be written from the MOST Synchronous RT device.
 * Callable in RT and NRT context.
 *
 * @param[in] context Context structure associated with opened device instance
 * @param[in] user_info Opaque pointer to information about user mode caller, 
 *            NULL if kernel mode call
 * @param[out] buff Input buffer as passed by the user
 * @param[in] count Number of bytes the user requests to read
 */
static ssize_t most_sync_rt_read(struct rtdm_dev_context *context,
                                 rtdm_user_info_t        *user_info,
                                 void                    *buff,
                                 size_t                  count)
{
    struct most_sync_rt_file        *file = (void *)context->dev_private;
    struct most_sync_rt_dev         *sync_dev = file->sync_dev;
    struct rtnrt_memcopy_desc       copy = { rtnrt_copy_to_user_rt, (void *)user_info };
    ssize_t                         copied = 0;
    int                             err = 0;

    pr_sync_debug(PR "Entering most_sync_rt_read %d, c=%d\n",
            file->reader_index, count);

    /* optimisation */
    return_value_if_fails(count != 0, 0);

    /* check if we can read */
    if (!file->rx_running) {
        rtnrt_err(PR "Cannot read at this time\n");
        return -EBUSY;
    }

    /* aquire the lock */
    err = most_sync_rt_read_write_begin(&sync_dev->rx_sync);
    if (err < 0) {
        pr_sync_debug(PR "most_sync_rt_read_write_begin "
                "returned %d\n", err);
        return err;
    }

    while (copied == 0) {
        copied = rxbuf_get(sync_dev->sw_receive_buf, file->reader_index, 
                    file->part_rx, buff, count, &copy);

        if (unlikely(copied < 0)) {
            rtnrt_err(PR "Error in rxbuf_get: %d\n", copied);
            goto out_read;
        }

        if (copied == 0) {
            RTDM_EXECUTE_ATOMICALLY(
                if (rxbuf_is_empty(sync_dev->sw_receive_buf, file->reader_index))
                    err = rtdm_event_wait(&sync_dev->rx_wait);
            );

            if (unlikely(err != 0)) {
                pr_sync_debug(PR "rtdm_event_wait returned with %d\n", err);
                copied = err;
                goto out_read;
            }
        }
    }

out_read:
    most_sync_rt_read_write_end(&sync_dev->rx_sync);

    pr_sync_debug(PR "Finishing most_sync_rt_read %d with %d\n",
                                file->reader_index, copied);
    return copied;
}

/**
 * Gets called if data should be written to the MOST Synchronous RT device.
 * Callable in RT and NRT context.
 *
 * @param[in] context Context structure associated with opened device instance
 * @param[in] user_info Opaque pointer to information about user mode caller, NULL 
 *            if kernel mode call
 * @param[in] buff Output buffer as passed by the user
 * @param[in] count Number of bytes the user requests to write
 */
static ssize_t most_sync_rt_write(struct rtdm_dev_context *context, 
                                  rtdm_user_info_t        *user_info, 
                                  const void              *buff, 
                                  size_t                  count)
{
    struct most_sync_rt_file    *file = (void *)context->dev_private;
    struct most_sync_rt_dev     *sync_dev = file->sync_dev;
    struct rtnrt_memcopy_desc   copy = { rtnrt_copy_from_user_rt, (void *)user_info };
    ssize_t                     copied = 0;
    int                         err = 0;

    pr_sync_debug(PR "Write called, count = %d\n", count);

    /* optimisation */
    return_value_if_fails(count != 0, 0);

    /* check if we can write */
    if (!file->tx_running) {
        rtnrt_err(PR "Cannot write at this time\n");
        return -EBUSY;
    }

    /* aquire the lock */
    err = most_sync_rt_read_write_begin(&sync_dev->tx_sync);
    if (err < 0) {
        pr_sync_debug(PR "most_sync_rt_read_write_begin returned %d", err);
        return err;
    }

    while (copied != count) {
        err = txbuf_put(sync_dev->sw_transmit_buf, file->writer_index,
                    file->part_tx, buff + copied, count - copied, &copy);
        if (unlikely(err < 0)) {
            rtnrt_err(PR "Error in txbuf_put: %d\n", err);
            goto out_write;
        }

        copied += err;
        if (err == 0) {
            RTDM_EXECUTE_ATOMICALLY(
                if (txbuf_is_full(sync_dev->sw_transmit_buf, file->writer_index)) 
                    err = rtdm_event_wait(&sync_dev->tx_wait);
            );

            if (unlikely(err != 0)) {
                pr_sync_debug(PR "rtdm_event_wait returned with %d\n", err);
                copied = err;
                goto out_write;
            }
        }
    }

out_write:
    most_sync_rt_read_write_end(&sync_dev->tx_sync);
    pr_sync_debug(PR "Finishing most_sync_write %d with %d\n",
                                file->writer_index, copied);
    return copied;
}

/**
 * Gets called by the MOST driver when a new MOST device was discovered.
 *
 * @param most_dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int most_sync_rt_probe(struct most_dev *most_dev)
{
    int                         number = MOST_DEV_CARDNUMBER(most_dev);
    int                         err;
    struct most_sync_rt_dev     *sync_dev;

    return_value_if_fails_dbg(number < MOST_DEVICE_NUMBER, -EINVAL);
    pr_sync_debug(PR "most_sync_rt_probe called, registering %d\n", number);

    /* create a new device structure */
    sync_dev = kmalloc(sizeof(struct most_sync_rt_dev), GFP_KERNEL);
    if (unlikely(sync_dev == NULL)) {
        rtnrt_warn(PR "Allocation of private data structure failed\n");
        err = -ENOMEM;
        goto out;
    }
    memset(sync_dev, 0, sizeof(struct most_sync_rt_dev));

    /* initialize some members */
    sync_dev->most_dev = most_dev;
    INIT_LIST_HEAD(&sync_dev->file_list);
    rtdm_lock_init(&sync_dev->lock);
    rtdm_event_init(&sync_dev->rx_wait, 0);
    rtdm_event_init(&sync_dev->tx_wait, 0);
    most_conf_sync_init(&sync_dev->tx_sync);
    most_conf_sync_init(&sync_dev->rx_sync);
    atomic_set(&sync_dev->receiver_count, 0);
    atomic_set(&sync_dev->transmitter_count, 0);

    /* create the new RTDM device */
    memcpy(&sync_dev->rtdm_dev, &device_templ, sizeof(struct rtdm_device));
    snprintf(sync_dev->rtdm_dev.device_name, RTDM_MAX_DEVNAME_LEN,
            "mostsync%d", number);
    sync_dev->rtdm_dev.device_id = number;
    sync_dev->rtdm_dev.proc_name = sync_dev->rtdm_dev.device_name;

    /* register the new RTDM device */
    err = rtdm_dev_register(&sync_dev->rtdm_dev);
    if (unlikely(err < 0)) {
        goto out_sync_dev;
    }

    /* put the device in the global list of devices */
    most_sync_rt_devices[number] = sync_dev;

    return 0;

out_sync_dev:
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
static int most_sync_rt_remove(struct most_dev *dev)
{
    int                         number = MOST_DEV_CARDNUMBER(dev);
    struct most_sync_rt_dev     *sync_dev = most_sync_rt_devices[number];

    pr_sync_debug(PR "most_sync_rt_remove called, unregistering %d\n", number);

    /* delete device */
    rtdm_dev_unregister(&sync_dev->rtdm_dev, 1000);

    /* free memory and remove from the global device list */
    most_sync_rt_devices[number] = NULL;
    rtdm_event_destroy(&sync_dev->rx_wait);
    rtdm_event_destroy(&sync_dev->tx_wait);
    most_conf_sync_destroy(&sync_dev->rx_sync);
    most_conf_sync_destroy(&sync_dev->tx_sync);

    kfree(sync_dev);

    return 0;
}

/**
 * Interrupt handler of a synchronous driver.
 *
 * @param[in] dev the MOST device
 * @param[in] intstatus the interrupt status register content
 */
static void most_sync_rt_interrupt_handler(struct most_dev      *dev, 
                                           unsigned int         intstatus)
{
    int                         card = MOST_DEV_CARDNUMBER(dev);
    struct most_sync_rt_dev     *sync_dev = most_sync_rt_devices[card];
    u32                         val;
    int                         current_page;

    return_if_fails_dbg(sync_dev != NULL);

    if (intstatus & ISSRX) {
        void          *dma_start;
        size_t        siz;
        
        pr_rt_irq_debug(PR "RT RX INT\n");

        val = most_readreg_rt(sync_dev->most_dev, MOST_PCI_SRXCTRL_REG);
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
        rxbuf_put(sync_dev->sw_receive_buf, dma_start, siz);
        measuring_receive_isr_wakeup();
        rtdm_event_pulse(&sync_dev->rx_wait);

        current_page = (val & SRXPP) ? 1 : 2;
        if ((sync_dev->rx_current_page != 0) && 
                (sync_dev->rx_current_page == current_page)) {
            rtnrt_warn(PR "RT-RX: sync_dev->rx_current_page == current_page\n");
        }
        sync_dev->rx_current_page = current_page;
    }
    
    if (intstatus & ISSTX) {
        void          *dma_start;
        ssize_t       read;
        size_t        siz;
        unsigned int  *ptr;

        pr_rt_irq_debug(PR "TX RT INT\n");

        val = most_readreg_rt(sync_dev->most_dev, MOST_PCI_STXCTRL_REG);
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

        read = txbuf_get(sync_dev->sw_transmit_buf, dma_start, siz);
        rtdm_event_pulse(&sync_dev->tx_wait);

        current_page = (val & STXPP) ? 1 : 2;
        if ((sync_dev->tx_current_page != 0) && 
                (sync_dev->tx_current_page == current_page))
        {
            rtnrt_warn(PR "RT-TX: sync_dev->tx_current_page == current_page\n");
        }
        sync_dev->tx_current_page = current_page;
    }
}

/**
 * The structure for the MOST High driver that is registered by the MOST PCI
 * driver
 */
static struct most_high_driver most_sync_rt_high_driver = {
    .name                   = "most-sync-rt",
    .sema_list              = LIST_HEAD_INIT(most_sync_rt_high_driver.sema_list),
    .spin_list              = LIST_HEAD_INIT(most_sync_rt_high_driver.spin_list),
    .probe                  = most_sync_rt_probe,
    .remove                 = most_sync_rt_remove,
    .interrupt_mask         = (IESTX | IESRX),
    .int_handler            = most_sync_rt_interrupt_handler
};

/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_sync_rt_init(void)
{
    int err;

    rtnrt_info("Loading module %s, version %s\n", DRIVER_NAME, version);
    print_measuring_warning();

    serial_rt_debug_init();

    /* register driver */
    err = most_register_high_driver(&most_sync_rt_high_driver);
    if (unlikely(err != 0)) {
        return err;
    }

    return 0;
}

/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_sync_rt_exit(void)
{
    most_deregister_high_driver(&most_sync_rt_high_driver);
    serial_rt_debug_finish();

    rtnrt_info("Unloading module %s, version %s\n", DRIVER_NAME, version);
}

#ifndef DOXYGEN
EXPORT_SYMBOL(hw_tx_buffer_size);
EXPORT_SYMBOL(hw_rx_buffer_size);
EXPORT_SYMBOL(sw_tx_buffer_size);
EXPORT_SYMBOL(sw_rx_buffer_size);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 170 $");
MODULE_DESCRIPTION("Real-time (RTDM) Driver for MOST Synchronous data.");
module_init(most_sync_rt_init);
module_exit(most_sync_rt_exit);
#endif

/* vim: set ts=4 et sw=4: */
