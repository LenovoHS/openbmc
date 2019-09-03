# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://fpga_recovery \
           file://version-in-bmc \
           file://fpga-recovery.sh \
           file://fpga-recovery.service \
          "

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "fpga-recovery.service"

INSANE_SKIP_${PN} = "installed-vs-shipped "

do_install() {
	install -d ${D}/var
	install -m 0755 ${S}fpga_recovery ${D}/var/
    install -m 0444 ${S}version-in-bmc ${D}/var/
	install -d ${D}/usr/sbin
    install -m 0755 ${S}fpga-recovery.sh ${D}/${sbindir}/
}
