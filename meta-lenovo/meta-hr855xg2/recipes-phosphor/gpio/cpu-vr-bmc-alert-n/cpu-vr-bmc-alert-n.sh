#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# CPU VCCIN Renesas ISL69259
# dump CPU VCCIN VR status and black-box data

run_dump () {
    date +"%Y-%m-%d %H:%M:%S"
    echo
    echo "all CPU PVCCIO PVCCSA done."
}

run_dump > /tmp/CPU_PVCCSA_VR_Fault.log

# EOF
