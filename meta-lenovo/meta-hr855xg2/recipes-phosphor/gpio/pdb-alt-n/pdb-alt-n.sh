#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

# PDB 48V D2D
# dump RAA228006(PWM)/RAA226054(Driver)X10 status & black box

run_dump () {
    date +"%Y-%m-%d %H:%M:%S"
    echo
    echo "PDB 48V D2D done."
}

run_dump > /tmp/PDB_48V_D2D_VR_Fault.log

exit 0;

# EOF
