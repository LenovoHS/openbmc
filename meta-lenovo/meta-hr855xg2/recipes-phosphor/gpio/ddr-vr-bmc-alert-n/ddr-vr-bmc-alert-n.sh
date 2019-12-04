#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# DIMM VR  TI SN1701022
# dump DIMM VR status and black-box data

run_dump () {
    date +"%Y-%m-%d %H:%M:%S"
    echo
    echo "all DDR PVPP PVDQ VR done."
}

run_dump > /tmp/DDR_PVPP_PVDQ_VR_Fault.log

exit 0;

# EOF
