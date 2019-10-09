#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

# Prevent watchdog enable when user send power on command twice
inst=$1
if [ -f "/tmp/wdt_en" ]; then
    if [ -f "/tmp/poweron_done" ]; then
        # Set the watchdog setting to default
        ipmitool raw 0x06 0x24 0x45 0x20 0x0 0x08 0x2c 0x01
        ipmitool mc watchdog reset
        #busctl call `mapper get-service /xyz/openbmc_project/watchdog/host${inst}` /xyz/openbmc_project/watchdog/host${inst} org.freedesktop.DBus.Properties Set ssv xyz.openbmc_project.State.Watchdog Enabled b true
    fi
else
    touch /tmp/wdt_en
fi
