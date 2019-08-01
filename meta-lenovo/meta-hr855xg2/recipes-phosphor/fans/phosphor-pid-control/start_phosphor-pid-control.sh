#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!bin/bash

# phosphor-pid-control flow

GPIO_BASE=$(cat /sys/class/gpio/gpio*/base)
PWGOOD=$(($GPIO_BASE+63))
PWGOOD_PATH="/sys/class/gpio/gpio"${PWGOOD}"/value"
PID_CONTROL="/usr/bin/swampd -l /tmp/"


# Check if system is power on or not, if not, then sleep 
# and wait for power on completion and start PID_CONTROL daemon. 
if [[ -f ${PWGOOD_PATH} ]] ; then
    while [ 1 ] ; do
        st=`cat ${PWGOOD_PATH}`
        if [ "$st" == "1" ]; then
            # Wait here until BIOS is ready, so the sensors are available on DBUS.
            # TODO: query current BIOS status, if it's ready, then start pid-control.
            sleep 10
            ${PID_CONTROL}
        else
            echo "System is not power on, wait for power-on"
        fi
        sleep 1
    done
else
    echo "PWGOOD PIN, ${PWGOOD_PATH} is not defined!!" 
    exit 1
fi
 
