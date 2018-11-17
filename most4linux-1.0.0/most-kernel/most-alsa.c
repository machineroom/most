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
#include <linux/init.h>

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>

#include "most-constants.h"
#include "most-base.h"
#include "most-sync-common.h"
#include "most-sync.h"
#include "most-alsa.h"


/**
 * @file most-alsa.c
 * @ingroup alsa
 *
 * @brief ALSA Driver for MOST
 */

/* constants {{{ ----------------------------------------------------------- */

/**
 * The name of the driver.
 */
#define DRIVER_NAME                     "most-alsa"

/**
 * The prefix for printk outputs.
 */
#define PR                              DRIVER_NAME       ": "

/**
 * Template for a MOST synchronous file
 */
#define MOST_SYNC_DEV_TEMPLATE          "/dev/mostsync%d"

/* }}} */

/* general static data elements {{{ ---------------------------------------- */

/**
 * Variable that holds the version.
 */
static char *version = "$Rev: 639 $";

/**
 * Array for each device.
 */
struct most_alsa_dev *most_alsa_devices[MOST_DEVICE_NUMBER];

/* }}} */

/* module parameters {{{ --------------------------------------------------- */

/**
 * Index value for the MOST sound device.
 */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;

/**
 * IDs for the MOST sound card.
 */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;

/**
 * Whether the card should be enabled or not.
 */
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

/**
 * The offset in bytes for accessing the buffer for the synchronous 
 * transfer in transmit direction.
 *
 * @see MOST_SYNC_SETUP_TX
 * @see struct frame_part
 */
static int playback_offset[SNDRV_CARDS] = { [0 ... SNDRV_CARDS-1] = 0 };

/**
 * The offset in bytes for accessing the buffer for the synchronous 
 * transfer in receive direction.
 *
 * @see MOST_SYNC_SETUP_RX
 * @see struct frame_part
 */
static int capture_offset[SNDRV_CARDS] = { [0 ... SNDRV_CARDS-1] = 0 };

#ifndef DOXYGEN
module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for MOST soundcard.");

module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for CS4281 soundcard.");

module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable CS4281 soundcard.");

module_param_array(playback_offset, int, NULL, 0444);
MODULE_PARM_DESC(playback_offset, 
        "Offset for accessing the synchronous data in playback direction");

module_param_array(capture_offset, int, NULL, 0444);
MODULE_PARM_DESC(capture_offset, 
        "Offset for accessing the synchronous data in capture direction");
#endif

/* }}} */

/* PCM driver {{{   -------------------------------------------------------- */

/* Playback {{{ ------------------------------------------------------------ */


/**
 * Sets up the MOST synchronous transmission for playback.
 *
 * @param[in] alsa_dev the MOST ALSA device
 * @param[out] filp the struct file that is set to the newly created file 
 *             pointer
 */
static int snd_most_playback_setup_sync_file(struct most_alsa_dev   *alsa_dev,
                                             struct file            **filp)
{
    struct frame_part           frame_part;
    struct file                 *most_sync_filp;
    char                        syncdev[32];
    int                         err;

    snd_assert(filp != NULL);

    /* create the MOST synchronous device */
    snprintf(syncdev, PATH_MAX, MOST_SYNC_DEV_TEMPLATE, 
            MOST_DEV_CARDNUMBER(alsa_dev->most_dev));
    most_sync_filp = filp_open(syncdev, O_WRONLY, 0);
    if (!unlikely(most_sync_filp)) {
        rtnrt_warn(PR "Couldn't open %s in kernelspace\n", syncdev);
        return -EIO;
    }

    frame_part.count = 4;
    frame_part.offset = playback_offset[alsa_dev->card->number];

    err = most_sync_setup_tx(most_sync_filp, &frame_part);
    if (unlikely(err != 0)) {
        rtnrt_warn(PR "most_sync_setup_tx failed with %d\n", err);
        goto out;
    }

    *filp = most_sync_filp;

    return 0;
out:
    filp_close(most_sync_filp, current->files);
    return err;
}

/**
 * Kernel thread that read the audio data from the ALSA buffer and
 * calls most_sync_write(). It gets triggered from the interrupt 
 * handler
 *
 * @param[in] data the struct most_alsa_dev
 */
static int snd_most_playback_thread(void *data)
{
    struct rtnrt_memcopy_desc   copy = { rtnrt_memmove, NULL };
    struct most_alsa_dev        *alsa_dev = (struct most_alsa_dev *)data;
    struct snd_pcm_runtime      *runtime = alsa_dev->p_substream->runtime;
    unsigned int                period_bytes, initial_fill, i;
    unsigned char               *zero_buffer = NULL;
    int                         err;
    struct file                 *most_sync_filp = NULL;
    void                        *dma_buf;

    pr_alsa_debug(PR "snd_most_playback_thread started\n");
    daemonize("most_alsa_playback");
    allow_signal(SIGTERM);
    up(&alsa_dev->p_thread_sema);

    /* setup the MOST device */
    err = snd_most_playback_setup_sync_file(alsa_dev, &most_sync_filp);
    if (unlikely(err != 0)) {
        goto out;
    }

    /* wait until the buffer was setup */
    err = down_interruptible(&alsa_dev->p_buf_setup_sema);
    if (unlikely(err < 0)) {
        pr_alsa_debug(PR "snd_most_playback_thread: wait_event_interruptible "
                "returned with %d", err);
        goto out_close;
    }

    /* init some data */
    period_bytes = frames_to_bytes(runtime, runtime->period_size);
    initial_fill = max(2U, runtime->periods/2);

    /* allocate zero buffer */
    zero_buffer = kmalloc(period_bytes, GFP_KERNEL);
    if (unlikely(!zero_buffer)) {
        rtnrt_warn(PR "snd_most_playback_thread: alloc zero buffer failed\n");
        goto out_close;
    }
    memset(zero_buffer, 0, period_bytes);

    /* fill the buffer initially */
    for (i = 0; i < initial_fill; i++) {
        err = most_sync_write(most_sync_filp, zero_buffer, period_bytes, &copy);
        if (unlikely(err < 0)) {
            pr_alsa_debug(PR "snd_most_playback_thread, break write\n");
            break;
        }
        atomic_inc(&alsa_dev->p_cur_period);
    }

    /* zero the DMA buffer initially */
    err = down_interruptible(&alsa_dev->p_buffer_mutex);
    if (unlikely(err != 0)) {
        up(&alsa_dev->p_buffer_mutex);
        goto out_close;
    }

    if (!runtime->dma_buffer_p) {
        up(&alsa_dev->p_buffer_mutex);
        goto out_close;
    }
    dma_buf = runtime->dma_buffer_p->area;

    memset(dma_buf, 0, period_bytes * runtime->periods);
    up(&alsa_dev->p_buffer_mutex);

    while (!signal_pending(current)) {
        unsigned long   offset;

        err = down_interruptible(&alsa_dev->p_event);
        if (unlikely(err != 0)) {
            break;
        }

        err = down_interruptible(&alsa_dev->p_buffer_mutex);
        if (unlikely(err != 0)) {
            break;
        }

        offset = (atomic_read(&alsa_dev->p_cur_period) % runtime->periods) 
            * period_bytes;

        if (!alsa_dev->p_silent) {
            
            if (!runtime->dma_buffer_p) {
                up(&alsa_dev->p_buffer_mutex);
                break;
            }
            dma_buf = runtime->dma_buffer_p->area;

            /* swap bytes if necessary, we always need big endian */
            if (snd_pcm_format_little_endian(runtime->format)) {
                swap_bytes(dma_buf + offset, period_bytes);
            }
            err = most_sync_write(most_sync_filp, dma_buf + offset, 
                    period_bytes, &copy);
            if (unlikely(err < 0)) {
                up(&alsa_dev->p_buffer_mutex);
                break;
            }

            atomic_inc(&alsa_dev->p_cur_period);
            snd_pcm_period_elapsed(alsa_dev->p_substream);
        } else {
            most_sync_write(most_sync_filp, zero_buffer, period_bytes, &copy);
        }

        up(&alsa_dev->p_buffer_mutex);
    }

out_close:
    filp_close(most_sync_filp, current->files);
out:
    kfree(zero_buffer);
    alsa_dev->p_thread_id = 0;
    pr_alsa_debug(PR "snd_most_playback_thread finished\n");
    complete_and_exit(&alsa_dev->p_completion, 0);
}

/**
 * Hardware parameters for the playback device
 */
static struct snd_pcm_hardware snd_most_playback_hw = {
    .info               = (SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP),
    .formats            = SNDRV_PCM_FMTBIT_S16_LE,
    .rates              = SNDRV_PCM_RATE_44100,
    .rate_min           = 44100,
    .rate_max           = 44100,
    .channels_min       = 2,
    .channels_max       = 2,
    .periods_min        = 2,
    .periods_max        = 8
};

/**
 * Opens the playback stream. Initialises the snd_most_playback_hw structure
 * and assigns it to the @c hw element of the runtime. Creates the kernel
 * thread.
 *
 * @param[in,out] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_open(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    struct snd_pcm_runtime  *runtime = substream->runtime;
    int                     err;

    pr_alsa_debug(PR "snd_most_playback_open\n");

    /* initialise some members at runtime ... */
    snd_most_playback_hw.buffer_bytes_max = (hw_tx_buffer_size * 4 * 8);
    snd_most_playback_hw.period_bytes_min = (hw_tx_buffer_size * 4);
    snd_most_playback_hw.period_bytes_max = (hw_tx_buffer_size * 4);
    snd_most_playback_hw.periods_max = min(snd_most_playback_hw.periods_max,
            (unsigned int)(sw_tx_buffer_size / hw_tx_buffer_size));

    /* ... and finally assign it */
    runtime->hw = snd_most_playback_hw;

    /* setup members of struct most_alsa_dev for playback */
    alsa_dev->p_substream = substream;
    alsa_dev->p_thread_id = 0;
    atomic_set(&alsa_dev->p_cur_period, 0);
    init_MUTEX_LOCKED(&alsa_dev->p_event); /* used as sempahore */
    init_MUTEX_LOCKED(&alsa_dev->p_thread_sema);
    init_MUTEX(&alsa_dev->p_buffer_mutex); /* used as mutex */
    init_MUTEX_LOCKED(&alsa_dev->p_buf_setup_sema);
    init_completion(&alsa_dev->p_completion);
    alsa_dev->p_silent = true;

    /* setup synchronous transmission */
    alsa_dev->p_thread_id = kernel_thread(snd_most_playback_thread, 
            alsa_dev, CLONE_KERNEL);
    if (unlikely(alsa_dev->p_thread_id == 0)) {
        rtnrt_warn(PR "playback kernel_thread creation failed\n");
        err = -EIO;
        goto out;
    }

    /* wait until the thread has finished */
    err = down_interruptible(&alsa_dev->p_thread_sema);
    if (unlikely(err != 0)) {
        pr_alsa_debug(PR "snd_most_playback_open interrupted %d\n", err);
        goto out_proc;
    }

    return 0;
out_proc:
    kill_proc(alsa_dev->p_thread_id, SIGTERM, 1);
    wait_for_completion(&alsa_dev->p_completion);
out:
    return err;
}

/**
 * Closes the substream. Kills the kernel thread.
 * 
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_close(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);

    pr_alsa_debug(PR "snd_most_playback_close\n");

    /* wait until the kernel thread has finished */
    if (likely(alsa_dev->p_thread_id != 0)) {
        kill_proc(alsa_dev->p_thread_id, SIGTERM, 1);
        wait_for_completion(&alsa_dev->p_completion);
    }
    alsa_dev->p_substream = NULL;

    return 0;
}

/**
 * Sets up the hardware. Triggers the mutex that signalises the
 * kernel thread that the buffer has been setup initially.
 *
 * @param[in] substream the ALSA substream
 * @param[in] hw_params the hardware parameters
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_hw_params(struct snd_pcm_substream  *substream,
                                       struct snd_pcm_hw_params  *hw_params)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    int                     ret;

    BUG_ON(alsa_dev->most_dev == NULL);
    pr_alsa_debug(PR "snd_most_pcm_hw_params\n");

    ret = down_interruptible(&alsa_dev->p_buffer_mutex);
    if (unlikely(ret != 0)) {
        rtnrt_warn(PR "snd_most_pcm_hw_params: down_interruptible failed\n");
        return ret;
    }

    ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
    up(&alsa_dev->p_buffer_mutex);

    if (unlikely(ret < 0)) {
        rtnrt_warn(PR "snd_most_pcm_hw_params: snd_pcm_lib_malloc_pages "
                "failed with %d\n", ret);
        return ret;
    }

    alsa_dev->p_silent = false;
    up(&alsa_dev->p_buf_setup_sema);

    return 0;
}

/**
 * Reverts the setup of the hardware. Frees the buffer.
 *
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_hw_free(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);

    pr_alsa_debug(PR "snd_most_playback_hw_free\n");

    alsa_dev->p_silent = true;

    return snd_pcm_lib_free_pages(substream);
}

/**
 * Prepare handler. Only modifies the position pointer for the buffer.
 *
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_prepare(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev      *alsa_dev = snd_pcm_substream_chip(substream);

    atomic_set(&alsa_dev->p_cur_period, 0);

    pr_alsa_debug(PR "snd_most_pcm_prepare\n");
    
#if 0
    pr_info("rate = %d, channels = %d\n",
            substream->runtime->rate, substream->runtime->channels);
    pr_info("period_size = %lu, periods = %d\n",
            substream->runtime->period_size, substream->runtime->periods);
    pr_info("buffer_size = %lu, tick_time = %d\n",
            substream->runtime->buffer_size, substream->runtime->tick_time);
    pr_info("frame_bits = %d, sample_bits = %d\n",
            substream->runtime->frame_bits, substream->runtime->sample_bits);
#endif

    return 0;
}

/**
 * Trigger handler. Gets called on start and stop. Modifies the 
 * @c playback_silent member of the struct most_alsa_dev.
 *
 * @param[in] substream the ALSA substream
 * @param[in] cmd the command -- either SNDRV_PCM_TRIGGER_START or
 *            SNDRV_PCM_TRIGGER_START
 * @return 0 on success, an error code on failure
 */
static int snd_most_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct most_alsa_dev      *alsa_dev = snd_pcm_substream_chip(substream);

    switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
            pr_alsa_debug(PR "SNDRV_PCM_TRIGGER_START\n");
            alsa_dev->p_silent = false;
            break;

        case SNDRV_PCM_TRIGGER_STOP:
            pr_alsa_debug(PR "SNDRV_PCM_TRIGGER_STOP\n");
            alsa_dev->p_silent = true;
    }

    return 0;
}

/**
 * Pointer handler. Returns the current position of the buffer access
 * in the kernel thread.
 *
 * @param[in] substream the ALSA substream
 * @return the current position
 */
static snd_pcm_uframes_t snd_most_playback_pointer(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    struct snd_pcm_runtime  *runtime = substream->runtime;

    return runtime->period_size * 
            (atomic_read(&alsa_dev->p_cur_period) % runtime->periods);
}

/* }}} */

/* Capture  {{{ ------------------------------------------------------------ */


/**
 * Sets up the MOST synchronous reception for capture.
 *
 * @param[in] alsa_dev the MOST ALSA device
 * @param[out] filp the struct file that is set to the newly created file 
 *             pointer
 */
static int snd_most_capture_setup_sync_file(struct most_alsa_dev   *alsa_dev,
                                             struct file            **filp)
{
    struct frame_part           frame_part;
    struct file                 *most_sync_filp;
    char                        syncdev[32];
    int                         err;

    snd_assert(filp != NULL);

    /* create the MOST synchronous device */
    snprintf(syncdev, PATH_MAX, MOST_SYNC_DEV_TEMPLATE, 
            MOST_DEV_CARDNUMBER(alsa_dev->most_dev));
    most_sync_filp = filp_open(syncdev, O_RDONLY, 0);
    if (!unlikely(most_sync_filp)) {
        rtnrt_warn(PR "Couldn't open %s in kernelspace\n", syncdev);
        return -EIO;
    }

    frame_part.count = 4;
    frame_part.offset = capture_offset[alsa_dev->card->number];

    err = most_sync_setup_rx(most_sync_filp, &frame_part);
    if (unlikely(err != 0)) {
        rtnrt_warn(PR "most_sync_setup_rx failed with %d\n", err);
        goto out;
    }

    *filp = most_sync_filp;

    return 0;
out:
    filp_close(most_sync_filp, current->files);
    return err;
}

/**
 * Kernel thread that read the audio data from the MOST buffer by
 * calling most_sync_read() and writes it into the ALSA buffer.
 * It gets triggered from the interrupt handler
 *
 * @param[in] data the struct most_alsa_dev
 */
static int snd_most_capture_thread(void *data)
{
    struct rtnrt_memcopy_desc   copy = { rtnrt_memmove, NULL };
    struct most_alsa_dev        *alsa_dev = (struct most_alsa_dev *)data;
    struct snd_pcm_runtime      *runtime = alsa_dev->c_substream->runtime;
    unsigned int                period_bytes;
    int                         err;
    struct file                 *most_sync_filp = NULL;
    void                        *dma_buf;

    pr_alsa_debug(PR "snd_most_capture_thread started\n");
    daemonize("most_alsa_capture");
    allow_signal(SIGTERM);
    up(&alsa_dev->c_thread_sema);

    /* setup the MOST device */
    err = snd_most_capture_setup_sync_file(alsa_dev, &most_sync_filp);
    if (unlikely(err != 0)) {
        goto out;
    }

    /* wait until the buffer was setup */
    err = down_interruptible(&alsa_dev->c_buf_setup_sema);
    if (unlikely(err < 0)) {
        pr_alsa_debug(PR "snd_most_capture_thread: wait_event_interruptible "
                "returned with %d\n", err);
        goto out_close;
    }

    /* init some data */
    period_bytes = frames_to_bytes(runtime, runtime->period_size);

    /* fill the buffer initially */
    err = down_interruptible(&alsa_dev->c_buffer_mutex);
    if (unlikely(err != 0)) {
        up(&alsa_dev->c_buffer_mutex);
        goto out_close;
    }

    if (!runtime->dma_buffer_p) {
        up(&alsa_dev->c_buffer_mutex);
        goto out_close;
    }
    dma_buf = runtime->dma_buffer_p->area;

    memset(dma_buf, 0, period_bytes * runtime->periods);
    up(&alsa_dev->c_buffer_mutex);

    while (!signal_pending(current)) {
        unsigned long   offset;

        err = down_interruptible(&alsa_dev->c_buffer_mutex);
        if (unlikely(err != 0)) {
            break;
        }

        if (!runtime->dma_buffer_p) {
            up(&alsa_dev->c_buffer_mutex);
            break;
        }
        dma_buf = runtime->dma_buffer_p->area;
        offset = (atomic_read(&alsa_dev->c_cur_period) % runtime->periods) 
            * period_bytes;

        if (!alsa_dev->c_silent) {
            err = most_sync_read(most_sync_filp, dma_buf + offset, period_bytes, &copy);
            if (unlikely(err < 0)) {
                up(&alsa_dev->c_buffer_mutex);
                break;
            }
            snd_assert(err == period_bytes);

            /* swap bytes if necessary, we always need big endian */
            if (snd_pcm_format_little_endian(runtime->format)) {
                swap_bytes(dma_buf + offset, period_bytes);
            }
            atomic_inc(&alsa_dev->c_cur_period);
            snd_pcm_period_elapsed(alsa_dev->c_substream);
        } else {
            memset(dma_buf, 0, period_bytes * runtime->periods); 
        }

        up(&alsa_dev->c_buffer_mutex);
    }

out_close:
    filp_close(most_sync_filp, current->files);
out:
    alsa_dev->c_thread_id = 0;
    pr_alsa_debug(PR "snd_most_capture_thread finished\n");
    complete_and_exit(&alsa_dev->c_completion, 0);
}

/**
 * Hardware parameters for the capture device
 */
static struct snd_pcm_hardware snd_most_capture_hw = {
    .info               = (SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP),
    .formats            = SNDRV_PCM_FMTBIT_S16_LE,
    .rates              = SNDRV_PCM_RATE_44100,
    .rate_min           = 44100,
    .rate_max           = 44100,
    .channels_min       = 2,
    .channels_max       = 2,
    .periods_min        = 2,
    .periods_max        = 8
};

/**
 * Opens the capture stream. Initialises the snd_most_capture_hw structure
 * and assigns it to the @c hw element of the runtime. Creates the kernel
 * thread.
 *
 * @param[in,out] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_open(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    struct snd_pcm_runtime  *runtime = substream->runtime;
    int                     err;

    pr_alsa_debug(PR "snd_most_capture_open\n");

    /* initialise some members at runtime ... */
    snd_most_capture_hw.buffer_bytes_max = (hw_tx_buffer_size * 4 * 8);
    snd_most_capture_hw.period_bytes_min = (hw_tx_buffer_size * 4);
    snd_most_capture_hw.period_bytes_max = (hw_tx_buffer_size * 4);
    snd_most_capture_hw.periods_max = min(snd_most_capture_hw.periods_max,
            (unsigned int)(sw_tx_buffer_size / hw_tx_buffer_size));

    /* ... and finally assign it */
    runtime->hw = snd_most_capture_hw;

    /* setup members of struct most_alsa_dev for capture */
    alsa_dev->c_substream = substream;
    alsa_dev->c_thread_id = 0;
    atomic_set(&alsa_dev->c_cur_period, 0);
    init_MUTEX(&alsa_dev->c_buffer_mutex);
    init_MUTEX_LOCKED(&alsa_dev->c_thread_sema);
    init_MUTEX_LOCKED(&alsa_dev->c_buf_setup_sema);
    init_completion(&alsa_dev->c_completion);
    alsa_dev->c_silent = true;

    /* setup synchronous transmission */
    alsa_dev->c_thread_id = kernel_thread(snd_most_capture_thread, 
            alsa_dev, CLONE_KERNEL);
    if (unlikely(alsa_dev->c_thread_id == 0)) {
        rtnrt_warn(PR "capture kernel_thread creation failed\n");
        err = -EIO;
        goto out;
    }

    /* wait until the thread has finished */
    err = down_interruptible(&alsa_dev->c_thread_sema);
    if (unlikely(err != 0)) {
        pr_alsa_debug(PR "snd_most_capture_open interrupted %d\n", err);
        goto out_proc;
    }

    return 0;
out_proc:
    kill_proc(alsa_dev->c_thread_id, SIGTERM, 1);
    wait_for_completion(&alsa_dev->c_completion);
out:
    return err;
}

/**
 * Closes the substream. Closes the MOST synchronous device.
 * 
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_close(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);

    pr_alsa_debug(PR "snd_most_capture_close\n");

    /* wait until the kernel thread has finished */
    if (likely(alsa_dev->c_thread_id != 0)) {
        kill_proc(alsa_dev->c_thread_id, SIGTERM, 1);
        wait_for_completion(&alsa_dev->c_completion);
    }
    alsa_dev->c_substream = NULL;

    return 0;
}

/**
 * Sets up the hardware. Triggers the mutex that signalises the
 * kernel thread that the buffer has been setup initially.
 *
 * @param[in] substream the ALSA substream
 * @param[in] hw_params the hardware parameters
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_hw_params(struct snd_pcm_substream  *substream,
                                      struct snd_pcm_hw_params  *hw_params)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    int                     ret;

    BUG_ON(alsa_dev->most_dev == NULL);
    pr_alsa_debug(PR "snd_most_pcm_hw_params\n");

    ret = down_interruptible(&alsa_dev->c_buffer_mutex);
    if (unlikely(ret != 0)) {
        rtnrt_warn(PR "snd_most_pcm_hw_params: down_interruptible failed\n");
        return ret;
    }

    ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
    up(&alsa_dev->c_buffer_mutex);

    if (unlikely(ret < 0)) {
        rtnrt_warn(PR "snd_most_pcm_hw_params: snd_pcm_lib_malloc_pages "
                "failed with %d\n", ret);
        return ret;
    }

    alsa_dev->c_silent = false;
    up(&alsa_dev->c_buf_setup_sema);

    return 0;
}

/**
 * Reverts the setup of the hardware. Frees the buffer.
 *
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_hw_free(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);

    pr_alsa_debug(PR "snd_most_capture_hw_free\n");

    alsa_dev->c_silent = true;

    return snd_pcm_lib_free_pages(substream);
}

/**
 * Prepare handler. Only modifies the position pointer for the buffer.
 *
 * @param[in] substream the ALSA substream
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_prepare(struct snd_pcm_substream *substream)
{
    struct most_alsa_dev      *alsa_dev = snd_pcm_substream_chip(substream);

    atomic_set(&alsa_dev->c_cur_period, 0);

    pr_alsa_debug(PR "snd_most_pcm_prepare\n");
    
#if 0
    pr_alsa_debug(PR "rate = %d, channels = %d\n",
            substream->runtime->rate, substream->runtime->channels);
    pr_alsa_debug(PR p"eriod_size = %lu, periods = %d\n",
            substream->runtime->period_size, substream->runtime->periods);
    pr_alsa_debug(PR "buffer_size = %lu, tick_time = %d\n",
            substream->runtime->buffer_size, substream->runtime->tick_time);
    pr_alsa_debug(PR "frame_bits = %d, sample_bits = %d\n",
            substream->runtime->frame_bits, substream->runtime->sample_bits);
#endif

    return 0;
}

/**
 * Trigger handler. Gets called on start and stop. Modifies the 
 * @c capture_silent member of the struct most_alsa_dev.
 *
 * @param[in] substream the ALSA substream
 * @param[in] cmd the command -- either SNDRV_PCM_TRIGGER_START or
 *            SNDRV_PCM_TRIGGER_START
 * @return 0 on success, an error code on failure
 */
static int snd_most_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct most_alsa_dev      *alsa_dev = snd_pcm_substream_chip(substream);

    switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
            pr_alsa_debug(PR "SNDRV_PCM_TRIGGER_START\n");
            alsa_dev->c_silent = false;
            break;

        case SNDRV_PCM_TRIGGER_STOP:
            pr_alsa_debug(PR "SNDRV_PCM_TRIGGER_STOP\n");
            alsa_dev->c_silent = true;
    }

    return 0;
}

/**
 * Pointer handler. Returns the current position of the buffer access
 * in the kernel thread.
 *
 * @param[in] substream the ALSA substream
 * @return the current position
 */
static snd_pcm_uframes_t snd_most_capture_pointer(
        struct snd_pcm_substream *substream)
{
    struct most_alsa_dev    *alsa_dev = snd_pcm_substream_chip(substream);
    struct snd_pcm_runtime  *runtime = substream->runtime;

    return runtime->period_size * 
            (atomic_read(&alsa_dev->c_cur_period) % runtime->periods);
}

/* }}} */

/* General  {{{ ------------------------------------------------------------ */

/**
 * Playback operations for the MOST PCM device
 */
static struct snd_pcm_ops snd_most_playback_ops = {
    .open       = snd_most_playback_open,
    .close      = snd_most_playback_close,
    .ioctl      = snd_pcm_lib_ioctl,
    .hw_params  = snd_most_playback_hw_params,
    .hw_free    = snd_most_playback_hw_free,
    .prepare    = snd_most_playback_prepare,
    .trigger    = snd_most_playback_trigger,
    .pointer    = snd_most_playback_pointer,
};

/**
 * Capture operations for the MOST PCM device
 */
static struct snd_pcm_ops snd_most_capture_ops = {
    .open       = snd_most_capture_open,
    .close      = snd_most_capture_close,
    .ioctl      = snd_pcm_lib_ioctl,
    .hw_params  = snd_most_capture_hw_params,
    .hw_free    = snd_most_capture_hw_free,
    .prepare    = snd_most_capture_prepare,
    .trigger    = snd_most_capture_trigger,
    .pointer    = snd_most_capture_pointer,
};


/**
 * Sets up the PCM device in ALSA. 
 *
 * @param[in,out] alsa_dev the most_alsa_dev structure
 * @return 0 on success, an error code on failure
 */
static int __devinit snd_most_new_pcm(struct most_alsa_dev *alsa_dev)
{
    struct snd_pcm  *pcm;
    int             err;

    err = snd_pcm_new(alsa_dev->card, "MOST PCM", 0, 1, 1, &pcm);
    if (unlikely(err != 0)) {
        return err;
    }

    pcm->private_data = alsa_dev;
    strcpy(pcm->name, "MOST PCM");
    alsa_dev->pcm = pcm;

    /* set operators */
    snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_most_playback_ops);
    snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_most_capture_ops);

    /* pre-allocation of buffers, may fail */
    err = snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
            snd_dma_continuous_data(GFP_KERNEL), 
            max(hw_tx_buffer_size, hw_rx_buffer_size) * 8 * 4, 
            max(hw_tx_buffer_size, hw_rx_buffer_size) * 8 * 4);
    if (unlikely(err != 0)) {
        rtnrt_warn(PR "snd_pcm_lib_preallocate_pages_for_all failed "
                "with %d\n", err);
        goto err_pcm;
    }

    return 0;

err_pcm:
    return err;
}

/* }}} */

/* }}} */

/* ALSA Sound Card   {{{ --------------------------------------------------- */

/**
 * Deinitialises the struct most_alsa_dev elements 
 *
 * @param card the sound card
 */
static void snd_most_free(struct snd_card *card)
{
    /* Deinitialise the struct most_alsa_dev elements */

}

/**
 * Creates the ALSA MOST Device and stores the data.
 *
 * @param card the soundcard
 * @param most_dev the MOST device
 */
static int __devinit snd_most_create(struct snd_card    *card, 
                                     struct most_dev    *most_dev)
{
    struct most_alsa_dev *alsa_dev = ALSA_DEV(card);

    card->private_free = snd_most_free;

    /* initialise the struct most_alsa_dev elements */
    memset(alsa_dev, 0, sizeof(struct most_alsa_dev));
    alsa_dev->most_dev = most_dev;
    alsa_dev->card = card;

    return 0;
}

/**
 * Gets called by the MOST Base driver when a MOST device was removed.
 *
 * @param most_dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int most_alsa_remove(struct most_dev *most_dev)
{
    int                     number = MOST_DEV_CARDNUMBER(most_dev);
    struct most_alsa_dev    *alsa_dev = most_alsa_devices[number];

    pr_alsa_debug(PR "alsa_remove called, number = %d\n", number);

    most_alsa_devices[number] = NULL;
    snd_card_free(alsa_dev->card);

    return 0;
}

/**
 * Gets called by the MOST driver when a new MOST device was discovered.
 *
 * @param[in,out] most_dev the most_dev that was discovered
 * @return @c 0 on success, an error code on failure
 */
static int most_alsa_probe(struct most_dev *most_dev)
{
    struct snd_card         *card;
    int                     err;
    int                     number = MOST_DEV_CARDNUMBER(most_dev);

    pr_alsa_debug(PR "alsa_probe called, number = %d\n", number);

    if (most_dev->card_number >= SNDRV_CARDS) {
        return -ENODEV;
    }
    if (!enable[most_dev->card_number]) {
        return -ENOENT;
    }

    card = snd_card_new(index[number], id[number],
            THIS_MODULE, sizeof(struct most_alsa_dev));
    if (unlikely(!card)) {
        return -ENOMEM;
    }

    err = snd_most_create(card, most_dev);
    if (unlikely(err < 0)) {
        goto out_snd_most;
    }

    strcpy(card->driver, "MOST");
    strcpy(card->shortname, "OASIS MOST PCI device");
    sprintf(card->longname, "%s number %d", card->shortname, most_dev->card_number);

    err = snd_most_new_pcm(ALSA_DEV(card));
    if (unlikely(err < 0)) {
        goto out_snd_most;
    }

    err = snd_card_register(card);
    if (unlikely(err < 0)) {
        goto out_snd_most;
    }

    most_alsa_devices[number] = ALSA_DEV(card);

    return 0;

out_snd_most:
    snd_card_free(card);
    return err;
}

/**
 * Interrupt handler. Wakes up the kernel thread. The reason why not the blocking
 * facility of most_sync_write() is used is that the buffer is not filled
 * entirely and the latency is kept small. 
 *
 * @param[in] dev the MOST device
 * @param[in] intstatus the interrupt status which is used to discriminiate
 *            between receive and transmit interrupts
 */
static void most_alsa_int_handler(struct most_dev        *dev,
                                  unsigned int           intstatus)
{
    int                     number = MOST_DEV_CARDNUMBER(dev);
    struct most_alsa_dev    *alsa_dev = most_alsa_devices[number];

    BUG_ON(dev == NULL);
    if (alsa_dev) {
        if ((intstatus & IESTX) && alsa_dev->p_thread_id != 0) {
            up(&alsa_dev->p_event);
        }
    }
}

/**
 * The structure for the MOST High driver that is registered by the MOST PCI
 * driver. No interrupt handlers are needed in this driver.
 */
static struct most_high_driver most_alsa_high_driver = {
    .name               = "most-alsa",
    .sema_list          = LIST_HEAD_INIT(most_alsa_high_driver.sema_list),
    .spin_list          = LIST_HEAD_INIT(most_alsa_high_driver.spin_list),
    .probe              = most_alsa_probe,
    .remove             = most_alsa_remove,
    .int_handler        = most_alsa_int_handler,
    .interrupt_mask     = (IESTX | IESRX)
};

/* }}} */

/* Module init/exit  {{{ --------------------------------------------------- */

/**
 * This function gets called if the kernel loads this module.
 *
 * @return 0 on success, an error code on failure
 */
static int __init most_alsa_init(void)
{
    int err;

    rtnrt_info(PR "Loading module %s, version %s\n", DRIVER_NAME, version);

    /* register driver */
    err = most_register_high_driver(&most_alsa_high_driver);
    if (unlikely(err != 0)) {
        return err;
    }

    return 0;
}

/**
 * This function gets called if the Kernel removes this module.
 */
static void __exit most_alsa_exit(void)
{
    most_deregister_high_driver(&most_alsa_high_driver);

    rtnrt_info(PR "Unloading module %s, version %s\n", DRIVER_NAME, version);
}

/* }}} */


#ifndef DOXYGEN
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Walle");
MODULE_VERSION("$Rev: 639 $");
MODULE_DESCRIPTION("ALSA Driver for MOST Synchronous data.");
module_init(most_alsa_init);
module_exit(most_alsa_exit);
#endif

/* vim: set ts=4 et sw=4 foldmethod=marker: */
