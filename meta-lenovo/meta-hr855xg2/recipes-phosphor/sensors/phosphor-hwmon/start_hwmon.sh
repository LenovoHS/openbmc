#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash

action=$1
devpath=$2
of_fullname=$3

#Use of_fullname if it's there, otherwise use devpath.

path=$of_fullname
if [ -z "$path" ]
then
    path=$devpath

    if [[ "$path" =~ (.*)/hwmon/hwmon[0-9]+$ ]];
    then
        path=${BASH_REMATCH[1]}
    fi
fi

path="${path//:/--}"
path="${path//-/\\x2d}"

systemctl $action 'xyz.openbmc_project.Hwmon@'$path'.service'

# Temporary solution for tuning default PWM values. Start
# Remove the solution after fan control is done. 
if [[ $path == *"pwm"* ]] ; then
    for pwm_loop in {1..7}
    do
        echo "153" > /sys/class/hwmon/hwmon0/pwm$pwm_loop
    done
    echo  "127" > /sys/class/hwmon/hwmon0/pwm8
fi

# Temporary solution for tuning default PWM values. End
