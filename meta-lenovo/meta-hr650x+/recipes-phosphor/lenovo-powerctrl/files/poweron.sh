# Copyright ? 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

#!/bin/bash

GPIO_BASE=$(cat /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/*/base)
GPIO_NUM=$(($GPIO_BASE + 64 + 0))

echo 1 > /sys/class/gpio/gpio${GPIO_NUM}/value
sleep 1
echo 0 > /sys/class/gpio/gpio${GPIO_NUM}/value

# It is a workaround for power on fail.
# The root cause is that op-wait-power-on could not detect correct PWR state.
# We should remove this workaround after we fix the incorrect PWR state issue.
systemctl stop op-wait-power-on@0.service

exit 0;
