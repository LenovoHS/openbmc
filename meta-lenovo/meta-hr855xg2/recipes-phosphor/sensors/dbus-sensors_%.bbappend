#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-add-feature-to-get-cpu-present-by-FPGA.patch \
            file://0002-update-cpu-sensor-to-match-spec.patch \
           "

