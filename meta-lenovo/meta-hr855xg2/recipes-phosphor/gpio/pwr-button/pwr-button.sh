#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# Add Power Button Press SEL
ipmitool raw 0x0a 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x14 0xf2 0x6f 0x00 0xff 0xff

# Get PWRGOOD state
last_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

count=1
while :;
do
   curr_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

   if [ $last_state != $curr_state ]; then
      break
   fi

   count=$(($count+1))
   if [ $count -eq 11 ]; then
      break
   fi
   sleep 1
done
state=$curr_state

# Sync Power State to state manager
if [ $state == "1" ]; then
   busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
else
   busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.Off
fi
