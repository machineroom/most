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

mode="666"

if [ "$UID" != 0 ]; then
    echo "You must run this script as super user!"
    echo "Exiting..."
    exit 1
fi

if [ "$MOST_MODULES_DIR" = "" ]; then
    echo "You must point the shell variable \$MOST_MODULES_DIR to the 
          directory where the most modules are located!"
    echo "Exiting..."
    exit 1
fi

if [ -z "$hw_tx_buffer_size" ] ; then
    hw_tx_buffer_size=1000
fi

if [ -z "$hw_rx_buffer_size" ] ; then
    hw_rx_buffer_size=1000
fi

if [ -z "$sw_tx_buffer_size" ] ; then
    sw_tx_buffer_size=44100
fi

if [ -z "$sw_rx_buffer_size" ] ; then
    sw_rx_buffer_size=44100
fi

if [ -z "$serial_irq" ] ; then
    serial_irq=3
fi

if [ -z "$serial_ioaddr" ] ; then
    serial_ioaddr=0x02f8
fi

#
# build the command line
MOST_SYNC_PARAMS=""
MOST_SYNC_PARAMS="$MOST_SYNC_PARAMS hw_tx_buffer_size=$hw_tx_buffer_size"
MOST_SYNC_PARAMS="$MOST_SYNC_PARAMS hw_rx_buffer_size=$hw_rx_buffer_size"
MOST_SYNC_PARAMS="$MOST_SYNC_PARAMS sw_tx_buffer_size=$sw_tx_buffer_size"
MOST_SYNC_PARAMS="$MOST_SYNC_PARAMS sw_rx_buffer_size=$sw_rx_buffer_size"

SERIAL_PARAMS=""
SERIAL_PARAMS="$SERIAL_PARAMS irq=$serial_irq"
SERIAL_PARAMS="$SERIAL_PARAMS ioaddr=$serial_ioaddr"

echo "Using following parameters:"
echo "       DMA transmit buffer size   : $hw_tx_buffer_size"
echo "       DMA receive buffer size    : $hw_rx_buffer_size"
echo "       Ring transmit buffer size  : $sw_tx_buffer_size"
echo "       Ring receive buffer size   : $sw_rx_buffer_size"
echo ""

sync; sync; sync

if [ "$1" == "Xenomai" -o "$1" == "RTAI" ] ; then
    SYNC_MOD=most-sync-rt
    SYNC_MOD_U=most_sync_rt
else
    SYNC_MOD=most-sync
    SYNC_MOD_U=most_sync
fi


# load the RTAI driver first because the base modules may contain RTAI symbols 
if [ "$1" == "RTAI" ] ; then
    
    RTAI_DIR=`rtai-config --module-dir`

    echo -n "Loading RTAI modules ["
    insmod $RTAI_DIR/rtai_hal.ko    && echo -n " rtai_hal "
    insmod $RTAI_DIR/rtai_lxrt.ko   && echo -n " rtai_lxrt "
    insmod $RTAI_DIR/rtai_sem.ko    && echo -n " rtai_sem "
    insmod $RTAI_DIR/rtai_rtdm.ko   && echo -n " rtai_rtdm "

    # for test programs, not required by the driver
    insmod $RTAI_DIR/rtai_fifos.ko  && echo -n " rtai_fifos "

    if [ -n "$use_serial" ] ; then
        insmod $RTAI_DIR/rtai_16550A.ko $SERIAL_PARAMS && echo -n " rtai_16550A "
    fi 

    echo "]"

fi

echo -n "Loading MOST modules ["
(insmod ${MOST_MODULES_DIR}/most-base.ko                     && echo -n " most-base ")
(insmod ${MOST_MODULES_DIR}/most-pci.ko                      && echo -n " most-pci ")
(insmod ${MOST_MODULES_DIR}/$SYNC_MOD.ko $MOST_SYNC_PARAMS   && echo -n " $SYNC_MOD ")
(insmod ${MOST_MODULES_DIR}/most-netservice.ko               && echo -n " most-netservice ")
if [ -f ${MOST_MODULES_DIR}/most-alsa.ko ] ; then
    (insmod ${MOST_MODULES_DIR}/most-alsa.ko                 && echo -n " most-alsa ")
fi
echo "]"

# remove old nodes
rm -f /dev/mostnets[0-7]

# get the major device number
major=$( awk '$2=="most-base" {print $1}' /proc/devices )

echo ""
echo Creating devices for MOST NetServices ...

if [ ! -r "/dev/mostnets0" ] ; then
    i=0
    while [ $i -lt 8 ] ; do
        mknod /dev/mostnets$i c $major $i
        echo "  /dev/mostnets$i [$major $i]"
        i=$[i+1]
    done
fi

chmod $mode /dev/mostnets[0-7]

if [ "$1" != "RTAI" -a "$1" != "Xenomai" ] ; then
    echo Creating devices for MOST Synchronous Data ...

    if [ ! -r "/dev/mostsync0" ] ; then
        i=0
        while [ $i -lt 8 ] ; do
            minor=$[i+8]
            mknod /dev/mostsync$i c $major $minor
            echo "  /dev/mostsync$i [$major $minor]"
            i=$[i+1]
        done
    fi

    chmod $mode /dev/mostsync[0-7]
fi

# vim: set ts=4 sw=4 et:

