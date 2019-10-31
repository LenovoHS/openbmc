# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI += "file://bmc-verify.service \
            file://bmc-verify.sh \
           "

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "bmc-verify.service "

do_install() {
    install -d ${D}/${sbindir}
    install -m 0755 ${S}/bmc-verify.sh ${D}/${sbindir}/    
}
