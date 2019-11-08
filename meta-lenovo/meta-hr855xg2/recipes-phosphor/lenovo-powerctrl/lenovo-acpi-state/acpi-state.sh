#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

init_done=false
last_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

while :;
do
   # Get PWRGOOD state
   curr_state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`
   ret=$?
   if [ $ret != "0" ]; then
      echo "Get PGOOD Failed !!"
      sleep 1
      continue
   fi

   # Init Once
   if [ $init_done != true ]; then
      if [ $curr_state == "0" ]; then
         busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.Off
      else
         busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
      fi

      ret=$?
      if [ $ret == "0" ]; then
         if [ $curr_state == "1" ]; then
            # Set ACPI status
            busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Control.Power.ACPIPowerState SysACPIStatus s xyz.openbmc_project.Control.Power.ACPIPowerState.ACPI.S0_G0_D0
            ret=$?
            if [ $ret != "0" ]; then
               echo "Set ACPI State Failed !!"
               sleep 1
               continue
            fi
            # Add sel
            ipmitool raw 0x0a 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x22 0xf0 0x6f 0x00 0xff 0xff
         else
            # Set ACPI status
            busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Control.Power.ACPIPowerState SysACPIStatus s xyz.openbmc_project.Control.Power.ACPIPowerState.ACPI.S5_G2
            ret=$?
            if [ $ret != "0" ]; then
               echo "Set ACPI State Failed !!"
               sleep 1
               continue
            fi
         fi

         last_state=$curr_state
         echo "ACPI state Init ...Done"
         init_done=true
      else
         echo "Set Power State Failed !!"
         sleep 1
         continue
      fi
   fi

   if [ $last_state != $curr_state ]; then
      # Sync Power State to state manager
      if [ $curr_state == "1" ]; then
          busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
          # Set ACPI status
          busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Control.Power.ACPIPowerState SysACPIStatus s xyz.openbmc_project.Control.Power.ACPIPowerState.ACPI.S0_G0_D0
          # Add sel
          ipmitool raw 0x0a 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x22 0xf0 0x6f 0x00 0xff 0xff
      else
         busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.Off
         # Set ACPI status
         busctl set-property xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/ACPI_State xyz.openbmc_project.Control.Power.ACPIPowerState SysACPIStatus s xyz.openbmc_project.Control.Power.ACPIPowerState.ACPI.S5_G2
         # Add sel
         ipmitool raw 0x0a 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x22 0xf0 0x6f 0x05 0xff 0xff
      fi

      ret=$?
      if [ $ret == "0" ]; then
         last_state=$curr_state
      else
         sleep 1
         continue
      fi
   fi

   sleep 1
done
