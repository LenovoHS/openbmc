# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/files:"

SRC_URI += "file://watchdog.conf \
           "
do_install_append() {
    install -Dm 0644 ${WORKDIR}/watchdog.conf ${D}${sysconfdir}/watchdog.conf
}
