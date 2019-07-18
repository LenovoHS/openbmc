#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash
GPIO_BASE=$(cat /sys/class/gpio/gpio*/base)
GPIO_NUM=$(($GPIO_BASE + 64))
PWR_GOOD=$(($GPIO_BASE + 63))

st=$(cat /sys/class/gpio/gpio${PWR_GOOD}/value)

if [ "$st" == "0" ]; then
echo 1 > /sys/class/gpio/gpio${GPIO_NUM}/value
sleep 1
echo 0 > /sys/class/gpio/gpio${GPIO_NUM}/value
fi

# It is a workaround for power on fail.
# The root cause is that op-wait-power-on could not detect correct PWR state.
# We should remove this workaround after we fix the incorrect PWR state issue.
systemctl stop op-wait-power-on@0.service

exit 0;
