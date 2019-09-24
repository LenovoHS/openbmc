# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."


#!/bin/bash

# Check power good
status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

if [ $status == "1" ]; then
    GPIO_BASE=$(cat /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/*/base)
    GPIO_NUM=$(($GPIO_BASE + 64 + 0))

    echo 1 > /sys/class/gpio/gpio${GPIO_NUM}/value
    sleep 8
    echo 0 > /sys/class/gpio/gpio${GPIO_NUM}/value
fi

exit 0;
