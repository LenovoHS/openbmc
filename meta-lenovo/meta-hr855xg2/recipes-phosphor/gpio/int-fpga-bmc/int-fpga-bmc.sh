#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# CPU Status
command="busctl call org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga read_fpga yy 14 1"
value=$(eval $command | awk '{print $2}')

MAX_CPU=4
for ((bit=0; bit<$MAX_CPU; bit++))
do
    if [ $((value&$((1<<bit)))) -gt 0 ]; then
       snr_num=$((bit+0x91))
       ipmitool raw 0x0A 0x44 0x00 0x00 0x02 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x07 ${snr_num} 0x6F 0x01 0xFF 0xFF
       busctl set-property --no-page xyz.openbmc_project.CPUSensor /xyz/openbmc_project/inventory/system/chassis/motherboard/CPU${bit}_Status xyz.openbmc_project.Inventory.Item.Cpu Thermal_Trip b
    fi
done

exit 0;
