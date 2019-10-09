# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-add-BIST-event-for-CPU_Status-sensor.patch \
            file://0002-add-watchdog2-sensor-event.patch \
            file://poweron.conf \
            file://wdt_en.sh \
           "
WATCHDOG_FMT = "../${WATCHDOG_TMPL}:phosphor-ipmi-host.service.wants/${WATCHDOG_TGTFMT}"

do_install_append() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/wdt_en.sh \
            ${D}${sbindir}/wdt_en.sh
}
