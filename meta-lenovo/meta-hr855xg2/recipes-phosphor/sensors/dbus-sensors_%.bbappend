# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"

SRC_URI += "file://0003-modify-CPU-DIMM-sensors-name.patch \
            file://0005-use-DIMM-thresholds-from-configs.patch \
            file://0006-not-update-DIMM-thresholds-from-attr.patch \
           "

# Overwrite the SYSTEMD_SERVICE_${PN} to select the services we want to use
SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.cpusensor.service"

# Disable the unnecessary package 
EXTRA_OECMAKE_append_hr855xg2 = " -DDISABLE_ADC=1 \
                                  -DDISABLE_EXIT_AIR=1 \
                                  -DDISABLE_FAN=1 \
                                  -DDISABLE_HWMON_TEMP=1 \
                                  -DDISABLE_INTRUSION=1 \
                                  -DDISABLE_IPMB=1 \
                                  -DDISABLE_MCUTEMP=1 \
                                  -DDISABLE_PSU=1 \
                                "

