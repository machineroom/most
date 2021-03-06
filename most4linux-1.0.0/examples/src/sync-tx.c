/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Steve Kreyer   <steve.kreyer@web.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *                           All rights reserved.
 *
 * ----------------------------------------------------------------------------
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Siemens code.
 * 
 * The Initial Developer of the Original Code is Siemens AG.
 * Portions created by the Initial Developer are Copyright (C) 2005-06
 * the Initial Developer. All Rights Reserved.
 * ----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/param.h>

#include "most-nets-usp/par_cp.h"
#include "most-nets-usp/registers.h"
#include "most-kernel/most-sync.h"

#define BUFFER_SIZE             (4*44100)

#define XCR_MASTER 0x80
#define XCR_SLAVE 0x00

/* global variable --------------------------------------------------------- */
int                     InstID = 0;
volatile sig_atomic_t   g_stop = false;
int                     g_nets_fd = 0;
int                     g_mode = XCR_SLAVE;


struct thread_arg {
    int count;
    int offset;
    int number;
};


void sighandler_exit(int signo)
{
    fprintf(stderr, "!\n");
    g_stop = true;
}

void print_help(void)
{
    fprintf(stderr, "Usage: sync-rx [2] [-h] [-m] [-i #]\n\n");
    fprintf(stderr, "    -h   : Print help\n"
                    "    -m   : Master mode\n"
                    "    -i # : Card id\n");
}

void parse_command_line(int argc, char *argv[])
{
    int opt, num;

    while ((opt = getopt(argc, argv, "hmi:")) != EOF) {
        switch (opt) {
            case 'm':
                g_mode = XCR_MASTER;
                break;

            case 'i':
                InstID = atoi(optarg);
                break;

            case 'h':
                print_help();
                exit(0);
                break;

            default:
                print_help();
                exit(1);
                break;
        }
    }

}

void *sync_transmit_thread(void *cookie)
{
    struct thread_arg   *th_arg = (struct thread_arg *)cookie;
    int                 i = 0;
    int                 n = 0;
    int                 fd;
    int                 err;
    time_t              end;
    struct frame_part   arg;
    unsigned char       buf[BUFFER_SIZE];
    char                stringbuffer[PATH_MAX];

    printf("Sync transmit\n");

    snprintf(stringbuffer, PATH_MAX, "/dev/mostsync%d", InstID);
    fd = open(stringbuffer, O_WRONLY);
    if (fd < 0) {
        perror("Couldn't open device for writing");
        return NULL;
    }

    arg.count = th_arg->count;
    arg.offset = th_arg->offset;
    printf("count=%d, offset=%d\n", arg.count, arg.offset);
    err = ioctl(fd, MOST_SYNC_SETUP_TX, &arg);
    if (err < 0) {
        perror("MOST_SYNC_SETUP_TX failed");
        goto out;
    }

    while (!g_stop) {
        for (n = 0; n < BUFFER_SIZE; n += 4) {
            buf[n] = i;
            buf[n+1] = 255 - i % 256;
            buf[n+2] = i;
            buf[n+3] = 255 - i % 256;

            i = (i + 1) % 256;
        }

        int x = write(fd, buf, BUFFER_SIZE);
        printf("[%d] wrote %d bytes\n", th_arg->number, x);
        if (x < 0) {
            break;
        }
    }

out:
    close(fd);
    return NULL;
}


void sync_in_connect(void)
{
    static unsigned char buffer[128] = {
        /*  0 */   0x47, 0x46, 0x45, 0x44, 0x04, 0x05, 0x06, 0x07,
        /*  8 */   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        /* 10 */   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        /* 18 */   0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        /* 20 */   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        /* 28 */   0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        /* 30 */   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /* 38 */   0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        /* 40 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 48 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 50 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 58 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 60 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 68 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 70 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
        /* 78 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8  };

    regwriteblock(g_nets_fd, 0, 128, buffer);
}


bool init8104(void)
{
    
    char            buffer[32];
    unsigned char pll_input_select = 0;
    unsigned char   block[] = {
        0x00, 0x04, 0x04, 0x00, 0x00, 0x7f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00 };
    
    snprintf(buffer, sizeof(buffer), "/dev/mostnets%d", InstID);
    g_nets_fd = open(buffer, O_RDWR);
    if (g_nets_fd < 0) {
        perror("Could not open NetServices");
        return false;
    }

    /* if device mode is configured to work in slave mode
     * the PLL input select (CM1.MX[0:1] bits) must be set to 00 */
    if(g_mode == XCR_SLAVE) 
        pll_input_select = 0x00;
    /* otherwise set PLL input select to 0x2 */
    else
        pll_input_select = 0x2;

    /* Interrupt Enable register -- no interrupts */
    regwrite(g_nets_fd, MOSTREG_IE /* 0x88 */, 0xff);
    
    /* Node Address Low Register */
    regwrite(g_nets_fd, MOSTREG_NAL /* 0x8b */, 0xfe);

    /* Transceiver Status Register */
    sleep(1);
    regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

    /* Transceiver Control Register */
    regwrite(g_nets_fd, MOSTREG_XCR /* 0x80 */, 0x62 | g_mode /* ->setting master or slave mode */);

    /* Transceiver Status Register */
    regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

    /* Transceiver Status Register2 */
    regwrite(g_nets_fd, MOSTREG_XSR2 /* 0x97 */, 0x02);

    /* Source Data Control Register 1 */
    regwrite(g_nets_fd, MOSTREG_SDC1 /* 0x82 */, 0x3);

    /* Packet Priority Register 1 */
    regwrite(g_nets_fd, MOSTREG_PPI /* 0xf2 */, 0x1);

    /* Clock Manager Register */
    regwrite(g_nets_fd, MOSTREG_CM1 /* 0x83 */, 0x54 | pll_input_select);
   
    /* source data control register 2 */
    regwrite(g_nets_fd, MOSTREG_SDC2 /* 0x8c */, 0xa8);

    /* source data control register 3 */
    regwrite(g_nets_fd, MOSTREG_SDC3 /* 0x8d */, 0x0);

    /* clock manager 4 register */
    regwrite(g_nets_fd, MOSTREG_CM4 /* 0x93 */, 0xc3); /* 0xc0 */

    /* Synchronous Bandwidth Control Register */
    regwrite(g_nets_fd, MOSTREG_SBC /* 0x96 */, 0x04); /* new */

    /* Transceiver Status Register */
    sleep(1);
    regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

    /* Synchronous Bandwidth Control Register */
    regwrite(g_nets_fd, MOSTREG_SBC /* 0x96 */, 0xf); /* new */

    /* deallocate all */
    regwrite(g_nets_fd, 0x3d6, 0x7f);
    regwrite(g_nets_fd, 0x3d2, 0x1);

    /* writeblock */
    regwriteblock(g_nets_fd, 0xc0, 21, block);

    /* set node address */
    regwrite(g_nets_fd, MOSTREG_NAH /* 0x8a */, 0xab);
    regwrite(g_nets_fd, MOSTREG_NAL /* 0x8b */, 0xcd);

    /* deallocate all */
    regwrite(g_nets_fd, 0x3d6, 0x7f);
    regwrite(g_nets_fd, 0x3d2, 0x1);

    /* Message control Register */
    regwrite(g_nets_fd, MOSTREG_MSGC /* 0x85 */, 0x80);

    /* wait */
    sleep(1);

    return true;
}

void close_8104(void)
{
    close(g_nets_fd);
}

int main(int argc, char *argv[])
{
    unsigned char       channel_id[8];
    struct sigaction    action;
    short               ret;
    int                 i, fr;
    struct thread_arg   thread_arg;
    int                 maxframe, max;

    /* register signal handlers */
    action.sa_handler = sighandler_exit;
    action.sa_flags = 0;
    sigfillset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    
    parse_command_line(argc, argv);

    printf("sync_tx: Hit ctrl-c to abort! ***\n");
    printf("Initializing the MOST controller...");
    fflush(stdout);
    if (!init8104()) {
        return 1;
    }
    printf(" done.\n");
    sync_in_connect();

    /* wait */
    sleep(1);

    /* receive synchronous frames */
    thread_arg.offset = 0;
    thread_arg.count = 4;
    thread_arg.number = 0;
    sync_transmit_thread(&thread_arg);

out:
    printf("Quit.\n");
    close_8104();

    return 0;
}

/* vim: set ts=4 et sw=4: */
