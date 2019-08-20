#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash

# power on

status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`


if [ $status == "0" ]; then
   phosphor-gpio-util --gpio=64 --path=/dev/gpiochip0 --delay=1000 --action=low_high
fi

sleep 4

# ipmitool mc watchdog set action=none

exit 0
