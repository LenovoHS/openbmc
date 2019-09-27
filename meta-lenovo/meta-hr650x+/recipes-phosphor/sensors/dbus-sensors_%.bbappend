# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.cpusensor.service"
EXTRA_OECMAKE_append_hr650x+ = " -DDISABLE_ADC=1 -DDISABLE_EXIT_AIR=1 -DDISABLE_FAN=1 -DDISABLE_HWMON_TEMP=1 -DDISABLE_INTRUSION=1 -DDISABLE_IPMB=1 -DDISABLE_MCUTEMP=1 -DDISABLE_PSU=1"
