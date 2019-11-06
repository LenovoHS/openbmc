#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

command="busctl call org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga read_fpga yy 14 7"
value=$(eval $command | awk '{print $2}')

if [ $((value&1)) -gt 0 ]; then
   ipmitool raw 0x0A 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x07 0x99 0x03 0x01 0xFF 0xFF
   busctl set-property --no-page xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/CPU_IERR xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_1 b true
elif [ $((value&16)) -gt 0 ]; then
   ipmitool raw 0x0A 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x07 0x9A 0x03 0x01 0xFF 0xFF
   busctl set-property --no-page xyz.openbmc_project.OEMSensor /xyz/openbmc_project/OEMSensor/CPU_MCERR xyz.openbmc_project.Sensor.Discrete.SpecificOffset Offset_1 b true
fi

exit 0;
