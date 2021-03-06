# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

#!bin/bash

# phosphor-pid-control flow

PID_CONTROL="/usr/bin/swampd -l /tmp/"

# Check if system is power on or not, if not, then sleep 
# and wait for power on completion and start PID_CONTROL daemon. 
while [ 1 ] ; do
    st=`gpioget gpiochip0 201`
    if [ "$st" == "1" ]; then
        # Wait here until BIOS is ready, so the sensors are available on DBUS.
        # TODO: query current BIOS status, if it's ready, then start pid-control.
        sleep 10
        ${PID_CONTROL}
    fi
    sleep 1
done
 
