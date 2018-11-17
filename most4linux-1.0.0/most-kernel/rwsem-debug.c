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
#include <linux/rwsem.h>

/* ------------------------------------------------------------------------- */
void init_rwsem_orig(struct rw_semaphore *sem)
{
    init_rwsem(sem);
}

/* ------------------------------------------------------------------------- */
void down_read_orig(struct rw_semaphore *sem)
{
    down_read(sem);
}

/* --------------------------------------------------------------------------*/
int down_read_trylock_orig(struct rw_semaphore *sem)
{
    return down_read_trylock(sem);
}

/* ------------------------------------------------------------------------- */
void up_read_orig(struct rw_semaphore *sem)
{
    up_read(sem);
}

/* ------------------------------------------------------------------------- */
void down_write_orig(struct rw_semaphore *sem)
{
    down_write(sem);
}

/* ------------------------------------------------------------------------- */
int down_write_trylock_orig(struct rw_semaphore *sem)
{
    return down_write_trylock(sem);
}

/* ------------------------------------------------------------------------- */
void up_write_orig(struct rw_semaphore *sem)
{
    up_write(sem);
}

/* ------------------------------------------------------------------------- */
void downgrade_wrte_orig(struct rw_semaphore *sem)
{
    downgrade_write(sem);
}

