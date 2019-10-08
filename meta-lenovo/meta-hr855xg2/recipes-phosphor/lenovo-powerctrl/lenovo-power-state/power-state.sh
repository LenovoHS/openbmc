#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

cmd=`echo 475 > /sys/class/gpio/export`

if [[ $? -ne 0 ]]; then
  exit 1
fi

last_state=`cat /sys/class/gpio/gpio475/value`
if [[ $last_state -eq 0 ]]; then
  echo "Ivan power off script out"
  rm /tmp/pwr_on
else
  echo "Ivan power on script out"
  touch /tmp/pwr_on
fi

while :;
do
  # Get PWRGOOD state
  curr_state=`cat /sys/class/gpio/gpio475/value`

  if [[ $last_state -ne $curr_state ]]; then
    if [[ $curr_state -eq 0 ]]; then
      echo "Ivan power off script"
      rm /tmp/pwr_on
    else
      echo "Ivan power on script"
      touch /tmp/pwr_on
    fi
    last_state=$curr_state
    echo $last_state 
  fi

  sleep 0.5
done

