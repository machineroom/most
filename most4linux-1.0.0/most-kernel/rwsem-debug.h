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
#ifndef RWSEM_DEBUG_H
#define RWSEM_DEBUG_H

#ifdef HAVE_CONFIG_H
#include "config/config.h"
#endif
#ifdef init_rwsem
#   error  "Don't include <linux/rwsem.h> if you include this file!"
#endif

struct rw_semaphore;

void init_rwsem_orig(struct rw_semaphore *sem);
void down_read_orig(struct rw_semaphore *sem);
int down_read_trylock_orig(struct rw_semaphore *sem);
void up_read_orig(struct rw_semaphore *sem);
void down_write_orig(struct rw_semaphore *sem);
int down_write_trylock_orig(struct rw_semaphore *sem);
void up_write_orig(struct rw_semaphore *sem);
void downgrade_wrte_orig(struct rw_semaphore *sem);

#define init_rwsem(sem)                                             \
    do {                                                            \
        pr_debugm("%s:%d: init_rwsem\n", __FILE__, __LINE__);       \
        init_rwsem_orig(sem);                                       \
    } while (0);

#define down_read(sem)                                              \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        down_read_orig(sem);                                        \
    } while (0);

#define down_read_trylock(sem)                                      \
    do {                                                            \
        pr_debugm("%s:%d: down_read_trylock\n", __FILE__, __LINE__);\
        down_read_orig(sem);                                        \
    } while (0);

#define up_read(sem)                                                \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        up_read_orig(sem);                                          \
    } while (0);

#define down_write(sem)                                             \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        down_write_orig(sem);                                       \
    } while (0);

#define down_write_trylock(sem)                                     \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        down_write_trylock_orig(sem);                               \
    } while (0);

#define up_write(sem)                                               \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        up_write_orig(sem);                                         \
    } while (0);

#define downgrade_wrte(sem)                                         \
    do {                                                            \
        pr_debugm("%s:%d: down_read\n", __FILE__, __LINE__);        \
        downgrade_wrte_orig(sem);                                   \
    } while (0);

#endif /* RWSEM_DEBUG_H */
