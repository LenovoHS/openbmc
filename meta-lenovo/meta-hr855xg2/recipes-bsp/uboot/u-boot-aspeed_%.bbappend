# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.


FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"
SRC_URI += " file://0001-Disable-SPI-clock-max-speed-detection-and-limit-SPI-.patch \
             file://0001-aspeed-Disable-unnecessary-features.patch \
             file://0002-ENABLE-DEDOCE.patch \
           "

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

