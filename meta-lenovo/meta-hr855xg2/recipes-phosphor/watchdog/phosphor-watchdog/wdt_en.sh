#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

# Prevent watchdog enable after BMC reboot
if [ -f "/tmp/wdt_en" ]; then
    busctl call `mapper get-service /xyz/openbmc_project/watchdog/host%i` /xyz/openbmc_project/watchdog/host%i org.freedesktop.DBus.Properties Set ssv xyz.openbmc_project.State.Watchdog Enabled b true
else
    touch /tmp/wdt_en
fi
