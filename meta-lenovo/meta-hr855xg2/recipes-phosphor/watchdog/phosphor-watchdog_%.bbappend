# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-add-BIST-event-for-CPU_Status-sensor.patch \
            file://wdt_en.sh \
           "

do_install_append() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/wdt_en.sh \
            ${D}${sbindir}/wdt_en.sh
}
