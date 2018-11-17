/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
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
#include <errno.h>
#include <sys/ioctl.h>

#include "adjust.h"
#include "mostns1.h"
#include "mostnetsdll.h"
#include "par_cp.h"
#include "global.h"
#include "most-netservice.h"

/* -------------------------------------------------------------------------- */
void ParWrite(tMostMap map, byte value)
{
    struct single_transfer_arg  arg;
    int                         ret;

    CHECK_FILEDESCRIPTOR_RET();
    
    arg.address = map;
    arg.value   = value;

    ret = ioctl(g_control_fd, MOST_NETS_WRITEREG, &arg);
    if (ret < 0) {
        PERR_DEBUG("Writing a register value failed");
    }

    PRINT_TRACE("Writing register 0x%x, value 0x%x", map, value);
}

/* -------------------------------------------------------------------------- */
byte ParRead(tMostMap map)
{
    struct single_transfer_arg  arg;
    int                         ret;

    CHECK_FILEDESCRIPTOR_RET_VAL(-1);

    arg.address = map;

    ret = ioctl(g_control_fd, MOST_NETS_READREG, &arg);
    if (ret < 0) {
        PERR_DEBUG("Reading a register value failed");
    }

    PRINT_TRACE("Reading register 0x%x, value 0x%x", map, arg.value);
    
    return arg.value;
}


/* -------------------------------------------------------------------------- */
void ParWriteBlock(tMostMap map, byte num, byte *data)
{
    struct block_transfer_arg   arg;
    int                         ret;

    CHECK_FILEDESCRIPTOR_RET();

    arg.address = map;
    arg.count   = num;
    arg.data    = data;

    ret = ioctl(g_control_fd, MOST_NETS_WRITEREG_BLOCK, &arg);
    if (ret < 0) {
        PERR_DEBUG("Writing a block of register value failed");
    }

    PRINT_TRACE("Writing block of registers 0x%x, number = %d", map, num);
}

/* -------------------------------------------------------------------------- */
void ParReadBlock(tMostMap map, byte num, byte *data)
{
    struct block_transfer_arg   arg;
    int                         ret;

    CHECK_FILEDESCRIPTOR_RET();

    arg.address = map;
    arg.count   = num;
    arg.data    = data;

    ret = ioctl(g_control_fd, MOST_NETS_READREG_BLOCK, &arg);
    if (ret < 0) {
        PERR_DEBUG("Reading a block of register value failed");
    }

    PRINT_TRACE("Reading block of registers 0x%x, number = %d", map, num);
}

/* -------------------------------------------------------------------------- */
void Most_Reset(void)
{
    int ret;

    PRINT_TRACE();
    
    printf("-- Reset --\n");

    CHECK_FILEDESCRIPTOR_RET();
    
    ret = ioctl(g_control_fd, MOST_NETS_RESET);
    if (ret < 0) {
        PERR_DEBUG("MOST_NETS_RESET failed");
    }
}

/* vim: set ts=4 et sw=4: */
