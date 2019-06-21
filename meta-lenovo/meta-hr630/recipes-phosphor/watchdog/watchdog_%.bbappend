# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/files:"

inherit systemd
inherit obmc-phosphor-systemd

SRC_URI += "file://watchdog.service \
            file://wdt-setting.sh \
           " 
SYSTEMD_AUTO_ENABLE = "enable"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "watchdog.service"

do_install_append () {
	install -d ${D}/usr/sbin
        install -m 0755 ${WORKDIR}/wdt-setting.sh ${D}/${sbindir}/
}

