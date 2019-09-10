#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/sh
# BMC use one UART3 controller to cross link with Host System COM port (UART 1) to function as SOL
stty -F /dev/ttyS0 115200
mknod -m 660 /dev/mem c 1 1
devmem 0x1E78909C 32 0x01450000
devmem 0x1E789098 32 0x00000A30
unlink /dev/mem
