#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# P12V AUX EFUSE
# dump ADM1278 status and black-box data

run_dump () {
    date +"%Y-%m-%d %H:%M:%S"
    echo
    echo "MB P12V_AUX2_EFUSE done."
}

run_dump > /tmp/P12V_AUX2_VR_Fault.log

exit 0;

# EOF
