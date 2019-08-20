#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash
# Set all output GPIOs as such and drive them with reasonable values.
#function set_gpio_active_low() {
#  if [ $# -ne 2 ]; then
#    echo "WHT <init.sh> set_gpio_active_low: need both GPIO# and initial level";
#    return;
#  fi
#
#  if [ -e /sys/class/gpio/gpio$1 ]; then
#	echo "WHT <init.sh> /sys/class/gpio/gpio$1 is exist";
#     echo $2 > /sys/class/gpio/gpio$1/direction
#  else
#	echo "WHT <init.sh> /sys/class/gpio/gpio$1 set_gpio_active_low()";
#     echo $1 > /sys/class/gpio/export
#     echo $2 > /sys/class/gpio/gpio$1/direction
#  fi
#  
#  
#}

# GPIO_BASE=$(cat /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/*/base)

# FP_PWR_BTN_PASS_R_N, GPIO I0, active low
# set_gpio_active_low $((${GPIO_BASE} + 64 )) high
# echo 1 > /sys/class/gpio/gpio$((${GPIO_BASE} + 64))/active_low

# PWR_GOOD, GPIO H7 , active low
# set_gpio_active_low $((${GPIO_BASE} + 63)) in

exit 0
