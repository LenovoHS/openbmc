# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append_hr855xg2 := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://uart-bios.sh \
           file://uart-bmc.sh \
           file://uart-switch.service"

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "uart-switch.service"

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 ${S}uart-bios.sh ${D}/${sbindir}/
    install -m 0755 ${S}uart-bmc.sh ${D}/${sbindir}/
}
