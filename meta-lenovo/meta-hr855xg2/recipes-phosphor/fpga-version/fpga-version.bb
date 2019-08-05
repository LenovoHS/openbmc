#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd



S = "${WORKDIR}/"

SRC_URI = "file://version.service \
           file://cpld0.version \
           file://version.sh \
          " 

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "version.service"



do_install() {
    install -d ${D}/var
	install -m 0755 cpld0.version ${D}/var/
	install -d ${D}/usr/sbin
    install -m 0755 version.sh ${D}/${sbindir}/
}