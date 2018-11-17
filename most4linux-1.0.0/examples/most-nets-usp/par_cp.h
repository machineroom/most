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

#ifndef PAR_CP_H
#define PAR_CP_H

#include <sys/ioctl.h>
#include <fcntl.h>

#include "most-kernel/most-netservice.h"

static inline void regwrite(int fd, int map, unsigned char value)
{
    struct single_transfer_arg  arg;
    int                         ret;

    arg.address = map;
    arg.value   = value;

    ret = ioctl(fd, MOST_NETS_WRITEREG, &arg);
    if (ret < 0) {
        perror("Writing a register value failed");
    }
}

static inline unsigned char regread(int fd, int map)
{
    struct single_transfer_arg  arg;
    int                         ret;

    arg.address = map;

    ret = ioctl(fd, MOST_NETS_READREG, &arg);
    if (ret < 0) {
        perror("Writing a register value failed");
    }
    return arg.value;
}


static inline void regwriteblock(int fd, int map, int num, unsigned char *data)
{
    struct block_transfer_arg   arg;
    int                         ret;

    arg.address = map;
    arg.count   = num;
    arg.data    = data;

    ret = ioctl(fd, MOST_NETS_WRITEREG_BLOCK, &arg);
    if (ret < 0) {
        perror("Writing a block of register value failed");
    }
}


#endif /* PAR_CP_H */

/* vim: set sw=4 ts=4 et: */
