# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"


inherit systemd
inherit obmc-phosphor-systemd
inherit systemd

S = "${WORKDIR}/"

SRC_URI += " \
        file://bios-verify.service \
        file://bios-verify.sh \
        file://bios-update.service \
        file://bios-update.sh \
        file://config-bios.json \
       "

DEPENDS += "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "bios-verify.service bios-update.service"

do_install() {
    install -d ${D}/${sbindir} 
    install -d ${D}/${datadir}/phosphor-ipmi-flash/
    install -m 0755 ${S}/bios-verify.sh ${D}/${sbindir}/
    install -m 0755 ${S}/bios-update.sh ${D}/${sbindir}/
    install -m 0644 ${S}/config-bios.json ${D}/${datadir}/phosphor-ipmi-flash/
}

FILES_${PN} += " \
    /usr/share/phosphor-ipmi-flash/config-bios.json \
    "


