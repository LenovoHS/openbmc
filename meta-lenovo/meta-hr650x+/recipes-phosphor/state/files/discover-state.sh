#!/bin/sh

# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

# Workaround: Only discover system state once
if [ -f "/tmp/state_discovered" ]; then
    echo "Already discover the system state"
else
    touch /tmp/state_discovered
    /usr/bin/env phosphor-discover-system-state --host $1
fi

exit 0
