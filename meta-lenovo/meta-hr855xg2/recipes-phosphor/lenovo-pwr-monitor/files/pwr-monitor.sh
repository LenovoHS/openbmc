#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash

echo "power monitor starting ..."

GPIO_BASE=$(cat /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/*/base)
PWR_GOOD=$(($GPIO_BASE + 63))

changeon=0
changeoff=0

while [ 1 ]
do
   st=$(cat /sys/class/gpio/gpio${PWR_GOOD}/value)
   status=`busctl get-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState | awk '{print $2}'`
   if [ "$st" == "0" ]; then
         changeon=0
         if [ $status == "\"xyz.openbmc_project.State.Chassis.PowerState.On\"" ]; then
             let "changeoff+=1"
         else
             changeoff=0
         fi
   else
         changeoff=0 
         if [ $status == "\"xyz.openbmc_project.State.Chassis.PowerState.Off\"" ]; then
             let "changeon+=1"
         else
             changeon=0
         fi
   fi

   if [ $changeon -eq 2 ]; then
	changeon=0
        echo "WHT successfuly found, set on"
        busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
   fi

   if [ $changeoff -eq 2 ]; then
	changeoff=0
        echo "WHT successfuly found, set off"
        busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.Off
   fi

   sleep 1
done

echo "power monitor exit ..."

exit 0
