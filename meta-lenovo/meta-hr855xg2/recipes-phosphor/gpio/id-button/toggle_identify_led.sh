# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

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

read_cmd=`fpga -r -b 13 -o 14 -l 1 |grep receive`
read_cmd=${read_cmd#*:}

if [ $read_cmd -eq 0 ]; then
    uid_on
else
    uid_off
fi
