#!/bin/sh

# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

mknod -m 660 /dev/mem c 1 1
devmem 0x1E78909C 32 0x00

unlink /dev/mem
