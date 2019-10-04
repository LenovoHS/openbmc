#!/bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

# power on
status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

if [ $status == "0" ]; then
   phosphor-gpio-util --gpio=64 --path=/dev/gpiochip0 --delay=1000 --action=low_high
   if [ ! -f "/tmp/poweron_done" ]; then
       touch /tmp/poweron_done
   fi
else
   if [ -f "/tmp/poweron_done" ]; then
       rm -rf /tmp/poweron_done
   fi
fi

exit 0
