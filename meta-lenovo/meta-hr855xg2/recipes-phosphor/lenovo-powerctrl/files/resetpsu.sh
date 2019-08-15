#!/bin/bash                                                                                                                             
                                                                                                                                        
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
                                                                                                                                        
VALUE=$(cat /sys/class/gpio/gpio${PWRGD_SYS_PWROK_BMC}/value)                                                                           
                                                                                                                                        
if [ $VALUE -eq 49 ];then                                                                                                             
  echo "low" > /sys/class/gpio/gpio${FM_PCH_PWRBTN_N}/direction                                                                     
  sleep 6                                                                                                                               
  echo 1 > /sys/class/gpio/gpio${FM_PCH_PWRBTN_N}/value 
  sleep 10
fi

echo "low" > /sys/class/gpio/gpio${PDB_RESTART_N}/direction

exit 0;
