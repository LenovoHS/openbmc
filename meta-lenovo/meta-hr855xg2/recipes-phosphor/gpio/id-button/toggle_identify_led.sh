#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# Get current state
SERVICE="org.openbmc.control.fpga"
OBJECT="/org/openbmc/control/fpga"
INTERFACE="org.openbmc.control.fpga"
READ_METHOD="read_fpga"
WRITE_METHOD="write_fpga"

uid_on() 
{
    #write_cmd=`fpga -w -b 13 -o 14 -l 1 2` 
    write_cmd=$(busctl call $SERVICE $OBJECT $INTERFACE $WRITE_METHOD yyy 13 14 2)
}
uid_off()
{
    #write_cmd=`fpga -w -b 13 -o 14 -l 1 0`
    write_cmd=$(busctl call $SERVICE $OBJECT $INTERFACE $WRITE_METHOD yyy 13 14 0)
}

#read_cmd=`fpga -r -b 13 -o 14 -l 1 |grep receive`
#read_cmd=${read_cmd#*:}
read_cmd=$(busctl call $SERVICE $OBJECT $INTERFACE $READ_METHOD yy 13 14 | awk '{print $2}')

if [ $read_cmd -eq 0 ]; then
    uid_on
else
    uid_off
fi
