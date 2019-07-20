#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit systemd
inherit obmc-phosphor-systemd
inherit pkgconfig
#inherit obmc-phosphor-sdbus-service
inherit obmc-phosphor-c-daemon

S = "${WORKDIR}/"

SRC_URI = "file://fpga.c \
           file://fpga.h \
           file://Makefile \
           file://fpga_control.c \
           file://fpga.service \
          " 

DEPENDS = "systemd \
          "
RDEPENDS_${PN} += "libsystemd"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "fpga.service"



do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 fpga ${D}/${sbindir}/
	install -m 0755 fpga_control ${D}/${sbindir}/
}