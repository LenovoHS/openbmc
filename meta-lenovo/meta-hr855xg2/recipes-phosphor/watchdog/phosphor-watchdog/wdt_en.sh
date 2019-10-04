#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

# Prevent watchdog enable when user send power on command twice
inst=$1
if [ ! -f "/tmp/poweron_done" ]; then
    busctl call `mapper get-service /xyz/openbmc_project/watchdog/host${inst}` /xyz/openbmc_project/watchdog/host${inst} org.freedesktop.DBus.Properties Set ssv xyz.openbmc_project.State.Watchdog Enabled b false
fi
