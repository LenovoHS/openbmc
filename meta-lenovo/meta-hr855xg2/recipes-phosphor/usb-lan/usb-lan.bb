FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://usb-lan.service \
           file://usb_network.sh \
           file://00-bmc-usb0.network \
          " 

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "usb-lan.service"

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 ${S}usb_network.sh ${D}/${sbindir}/
    #install -m 644 ${WORKDIR}/00-bmc-usb0.network ${D}${systemd_unitdir}/network/    
}
