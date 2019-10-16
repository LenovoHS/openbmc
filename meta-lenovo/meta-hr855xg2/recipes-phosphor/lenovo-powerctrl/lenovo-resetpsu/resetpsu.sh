#!/bin/sh  

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."               
  
GPIO_BASE=$(cat /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/*/base)                                                            
PWRGD_SYS_PWROK_BMC=$(($GPIO_BASE + 63))                                                                                                
FM_PCH_PWRBTN_N=$(($GPIO_BASE + 65))                                                                                                    
PDB_RESTART_N=$(($GPIO_BASE + 202))                                                                                                     

if [ ! -L "/sys/class/gpio/gpio345" ];then
  echo $FM_PCH_PWRBTN_N > /sys/class/gpio/export
fi

if [ ! -L "/sys/class/gpio/gpio482" ];then
  echo $PDB_RESTART_N > /sys/class/gpio/export
fi

source /run/psu_timedelay

sleep $PSU_HARDRESET_DELAY                                                                                                                       

#VALUE=$(cat /sys/class/gpio/gpio${PWRGD_SYS_PWROK_BMC}/value)  

status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`
                                                                         
# power get
if [ $status == "1"  ];then                                                                                                             
  echo "low" > /sys/class/gpio/gpio${FM_PCH_PWRBTN_N}/direction                                                                     
  sleep 6                                                                                                                               
  echo 1 > /sys/class/gpio/gpio${FM_PCH_PWRBTN_N}/value 

fi

echo "low" > /sys/class/gpio/gpio${PDB_RESTART_N}/direction

exit 0
