# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.


FILESEXTRAPATHS_prepend_hr650x+ := "${THISDIR}/${PN}:"
SRC_URI += " file://0001-Disable-SPI-clock-max-speed-detection-and-limit-SPI-.patch \
           "

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

