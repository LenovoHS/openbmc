FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://pwr-seq.c \
           file://Makefile \
           file://lenovo-pwr-sequence.service \
          " 

DEPENDS = "systemd \
           i2c-tools \
          "

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "lenovo-pwr-sequence.service"

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 pwr-seq ${D}/${sbindir}/
}
