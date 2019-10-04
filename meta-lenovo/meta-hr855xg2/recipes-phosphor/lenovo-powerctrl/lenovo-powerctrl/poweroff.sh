#!/bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

# power off
status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

if [ $status == "1" ]; then
   phosphor-gpio-util --gpio=64 --path=/dev/gpiochip0 --delay=6000 --action=low_high
fi

exit 0
