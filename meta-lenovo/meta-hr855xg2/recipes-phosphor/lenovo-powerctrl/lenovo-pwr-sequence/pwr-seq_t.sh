# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/bash

# power get

status=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

if [ $status == "0" ]; then
   pwr-seq
fi 

exit 0
