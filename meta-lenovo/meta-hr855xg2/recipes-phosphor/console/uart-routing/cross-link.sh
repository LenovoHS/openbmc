#!/bin/sh

#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

stty -F /dev/ttyS0 115200

#Enable UART IO2 pin
mknod -m 660 /dev/mem c 1 1
devmem 0x1e6e2084 32 0xff00f000
unlink /dev/mem

