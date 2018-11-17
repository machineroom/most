/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Steve Kreyer   <steve.kreyer@web.de>
 *                           Bernhard Walle <bernhard.walle@gmx.de>
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

#define XCR_MASTER 0x80
#define XCR_SLAVE 0x00
#define CTRL_DATA_LENGTH 17

typedef unsigned char byte;

typedef struct{
    byte prio;
    byte msg_type;
    byte tgt_addr_high;
    byte tgt_addr_low;
    byte ctrl_data[CTRL_DATA_LENGTH];
} ctrl_data_t;

/* global variable --------------------------------------------------------- */
int                     InstID = 0;
volatile sig_atomic_t   g_stop = false;
int                     g_nets_fd = 0;
int                     g_mode = XCR_SLAVE;



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

void ctrl_transmit() {
    ctrl_data_t t;
    byte transmit_status = 0;

    bzero(&t, sizeof(ctrl_data_t));

    t.prio = 0x0;
    t.msg_type = 0x0;       /* message is a normal message */
    t.tgt_addr_high = 0xf;  /* most significant byte of target node address */
    t.tgt_addr_low = 0xff;  /* lowes significant byte of target node address */

    t.ctrl_data[0] = 0x3F;	// FBlock
    t.ctrl_data[1] = 0x00;	// Inst
    t.ctrl_data[2] = 0x20;	// Func
    t.ctrl_data[3] = 0x00;	// Func + OpType
    t.ctrl_data[4] = 0x01;	// Length
    t.ctrl_data[5] = 0x04;
    
    /* write control message to control message buffer */
    regwriteblock(g_nets_fd, 
                  MOSTREG_XCMB /* address 0x80 */, 
                  sizeof(ctrl_data_t), 
                  (byte*) &t);

    /* now that data is in the buffer => send it! */
    regwrite(g_nets_fd, MOSTREG_MSGC /* 0x85 */, 0x80);
    /* wait a little before asking status */
    sleep(1);
    /* now check if the message was transmitted without an error */
    transmit_status = regread(g_nets_fd, MOSTREG_XTS /* 0xd5 */);
    if(transmit_status == 0x10)
        printf("Control message succesfully transmitted to node with node address 0x%x%x\n", 
               t.tgt_addr_high, 
               t.tgt_addr_low);
    else
        fprintf(stderr, "Error: Sending control message to node with node address 0x%x%x failed! Error code=0x%x\n", 
                transmit_status,
                t.tgt_addr_high,
                t.tgt_addr_low);


}


bool init8104(void)
{
    
    char            buffer[32];
    byte pll_input_select = 0;
    
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

    /* Transceiver Status Register */
    regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

    /* Transceiver Control Register */
    regwrite(g_nets_fd, MOSTREG_XCR /* 0x80 */, 0x20 | g_mode /* ->setting master or slave mode */);

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
    regwrite(g_nets_fd, MOSTREG_CM4 /* 0x93 */, 0xc0); /* 0xc0 */

    /* Transceiver Status Register */
    sleep(1);
    regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

    /* Transceiver Control Register */
    regwrite(g_nets_fd, MOSTREG_XCR /* 0x80 */, 0x62 | g_mode /* ->setting master or slave mode */);

    /* Synchronous Bandwidth Control Register */
    regwrite(g_nets_fd, MOSTREG_SBC /* 0x96 */, 0xf); /* new */

    /* set node address */
    regwrite(g_nets_fd, MOSTREG_NAH /* 0x8a */, 0xab);
    regwrite(g_nets_fd, MOSTREG_NAL /* 0x8b */, 0xcd);

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
    struct sigaction    action;

    /* register signal handlers */
    action.sa_handler = sighandler_exit;
    action.sa_flags = 0;
    sigfillset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    
    parse_command_line(argc, argv);

    printf("Initializing the MOST controller...");
    fflush(stdout);
    if (!init8104()) {
        return 1;
    }
    printf(" done\n");
    printf("Sending control data...\n");
    ctrl_transmit();
out:
    printf("Quit.\n");
    close_8104();

    return 0;
}

/* vim: set ts=4 et sw=4: */
