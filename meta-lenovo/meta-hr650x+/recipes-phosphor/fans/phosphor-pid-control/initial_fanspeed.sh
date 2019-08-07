#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!bin/bash

#Initialize pwm values for fan speed.

# For system fan PWM.
for pwm_val in {1..6} ; do 
    echo 153 > /sys/class/hwmon/*/pwm${pwm_val} 
done
