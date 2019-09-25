#!bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

# Feed FPGA watchdog for abnormal fan control.
# If FPGA does not receive kicks from BMC, then trigger fan PWMs to 255.

# Pull CPU_PROC_HOT events from FPGA, if PROC_HOT event is trigger, then driver 
# fan PWMs to 255. 

fpga_platform2_blk=13
fpga_fan_watchdog_offset=0x0D
timeout=5

fpga_ras_blk=14
fpga_throttle_offset=0x03

fpga_fan_watchdog_kick_command="busctl call org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga write_fpga yyy $fpga_platform2_blk $fpga_fan_watchdog_offset $timeout"

fpga_prochot_command=`busctl call org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga read_fpga yy $fpga_ras_blk $fpga_throttle_offset | awk '{print $2}'`

while true ; do 
    $fpga_fan_watchdog_kick_command > /dev/null
    ret=$((fpga_prochot_command))
    # Check if any CPU is triggering PROC_HOT event, if yes, then trigger PWM to full throttle. 
    if [ $ret -gt 0 -a $ret -lt 4 ]; then
        systemctl stop phosphor-pid-control.service
    else
        # Check if phosphor-pid-control.service is running or not, if not 
        # then restart phosphor-pid-control.service. 
        systemctl status --no-page -l phosphor-pid-control.service | egrep 'Stop'
        if [ $? == "1" ] ; then
            systemctl start phosphor-pid-control.service
        fi
    fi
    sleep $timeout
done 
