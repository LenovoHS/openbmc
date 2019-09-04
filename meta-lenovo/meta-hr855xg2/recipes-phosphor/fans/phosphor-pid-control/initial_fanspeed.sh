#!bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#Initialize pwm values for fan speed.
# For system fan PWM.
for pwm_val in {1..7} ; do 
    echo 128 > /sys/class/hwmon/*/pwm${pwm_val} 
done

# For PDB fan PWM.
echo 77 > /sys/class/hwmon/*/pwm8

