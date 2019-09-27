#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

last_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

while :;
do
   # Get PWRGOOD state
   curr_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

   if [ $last_state != $curr_state ]; then
      # Sync Power State to state manager
      if [ $curr_state == "1" ]; then
          busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
          # Set ACPI status
          busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_0 b true
          busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_5 b false
          # Add sel
          ipmitool raw 0x0a 0x44 0x00 0x00 0x03 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x22 0xf0 0x6f 0x00 0xff 0xff
      else
         busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.Off
         # Set ACPI status
         busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_0 b false
         busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_5 b true
         # Add sel
         ipmitool raw 0x0a 0x44 0x00 0x00 0x03 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x22 0xf0 0x6f 0x05 0xff 0xff
      fi
      last_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`
   fi
   
   sleep 1
done
