#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# P3V3 Riser2
# dump SIC456 status

run_dump() {
    # Check if Riser is presence
    command="busctl call org.openbmc.control.fpga /org/openbmc/control/fpga org.openbmc.control.fpga read_fpga yy 12 10"
    value=$(eval $command | awk '{print $2}')
    if [ $((value&2)) -gt 0 ]; then
        echo "Riser2 is not presence."
    else
        date +"%Y-%m-%d %H:%M:%S"
        echo
        echo "Riser2 P3V3 done."
    fi
}

run_dump > /tmp/P3V3_Riser2_VR_Fault.log

exit 0;

# EOF
