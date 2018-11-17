#!/bin/sh
#
# -----------------------------------------------------------------------------
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as 
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# -----------------------------------------------------------------------------

if [ "$UID" != 0 ]; then
    echo "You must run this script as super user!"
    echo "Exiting..."
    exit 1
fi

rm -f /dev/mostnets[0-7]
if [ "$1" != "RTAI" -a "$1" != "Xenomai" ] ; then
    rm -f /dev/mostsync[0-7]
fi

if [ "$1" == "RTAI" -o "$1" == "Xenomai" ] ; then
    SYNC_MOD=most_sync_rt
else
    SYNC_MOD=most_sync
fi


echo -n "Unloading MOST modules ["

if lsmod | grep most_alsa >> /dev/null 2>&1 ; then
    rmmod most_alsa				&& echo -n " most_alsa "
fi

rmmod $SYNC_MOD                   && echo -n " $SYNC_MOD "
rmmod most_netservice             && echo -n " most_netservice "
rmmod most_pci                    && echo -n " most_pci "
rmmod most_base                   && echo -n " most_base "

echo "]"


if [ "$1" == "RTAI" ] ; then

    echo -n "Unloading RTAI modules ["
    rmmod rtai_fifos              && echo -n " rtai_fifos "
    if lsmod | grep rtai_16550A >> /dev/null ; then
        rmmod rtai_16550A             && echo -n " rtai_16550A "
    fi
    rmmod rtai_rtdm               && echo -n " rtai_rtdm "
    rmmod rtai_sem                && echo -n " rtai_sem "
    rmmod rtai_lxrt               && echo -n " rtai_lxrt "
    rmmod rtai_hal                && echo -n " rtai_hal "
    echo "]"
fi

