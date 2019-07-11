#!/bin/sh


# Get current state

uid_on()
{
    write_cmd=`fpga -w -b 13 -o 14 -l 1 2`
}
uid_off()
{
    write_cmd=`fpga -w -b 13 -o 14 -l 1 0`
}

#read_cmd=`fpga -r -b 13 -o 14 -l 1 |grep receive`
#read_cmd=${read_cmd#*:}

if [ $1 == "on" ]; then
    uid_on
else
    uid_off
fi
