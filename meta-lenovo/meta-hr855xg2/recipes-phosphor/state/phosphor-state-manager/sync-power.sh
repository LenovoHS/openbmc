#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

echo "phosphor-discover-system-state post exec"

# Get Power Policy Setting
# Always-on:  0
# Always-off: 1
# Previous:   2
policy=`cat /run/initramfs/rw/cow/var/lib/phosphor-settings-manager/settings/xyz/openbmc_project/control/host0/power_restore_policy__ | grep -i value | xargs | awk '{print $4}'`

if [ $policy != 1 ]; then
   exit
fi

# Get PWRGOOD state
state=`busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | awk '{print $2}'`

# Sync Power State to state manager
if [ $state == "1" ]; then
   busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState s xyz.openbmc_project.State.Chassis.PowerState.On
fi
