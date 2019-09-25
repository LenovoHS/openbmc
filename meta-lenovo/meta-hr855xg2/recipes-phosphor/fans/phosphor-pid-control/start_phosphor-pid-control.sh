#!bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

# phosphor-pid-control flow
PID_CONTROL="/usr/bin/swampd -l /tmp/"
HW_CONFIG="/usr/sbin/hw_config.sh"
BIOS_READY="128"

# Check if BIOS is ready or not, if not, then break, restart phosphor-pid-service
# If BIOS is ready, then start PID_CONTROL.
bios_sta=`busctl call --no-page org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga read_fpga yy 0x02 0x08 | awk '{print $2}'`
if [ $bios_sta == $BIOS_READY ]; then
    #${HW_CONFIG}
    #if [ $? == "0" ] ; then
        ${PID_CONTROL}
    #fi
fi
