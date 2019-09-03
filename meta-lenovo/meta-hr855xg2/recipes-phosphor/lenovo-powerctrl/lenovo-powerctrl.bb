# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://poweroff.sh \
           file://poweron.sh \
           file://host-poweroff.service \
           file://host-poweron.service \
           "

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "host-poweron.service host-poweroff.service"

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 ${S}poweroff.sh ${D}/${sbindir}/
    install -m 0755 ${S}poweron.sh ${D}/${sbindir}/
}
