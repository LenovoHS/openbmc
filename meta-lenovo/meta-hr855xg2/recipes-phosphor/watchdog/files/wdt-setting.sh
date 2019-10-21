#!/bin/sh

# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

# Make sure watchdog service is ready to avoid script setting be written
while [ ! -f /var/run/watchdog.pid ]
do
  sleep 1
done

# TODO: /dev/mem will be disabled in the future
mknod -m 660 /dev/mem c 1 1

# Enable WDT and sent EXTRST after timeout
devmem 0x1E78502C 32 0x1B

# Disable reset GPIO controller
devmem 0x1e78503C 32 0x021FFFF3

unlink /dev/mem

