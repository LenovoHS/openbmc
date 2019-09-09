#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-implement-CPU_Status-and-DIMM_Status-sensor.patch \
            file://0002-add-service-dependency.patch \
            file://0003-add-fpgasensor-service.patch \
            file://0004-add-oemsensor-service.patch \
           "

SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.fpgasensor.service \
                          xyz.openbmc_project.oemsensor.service \
                         "

