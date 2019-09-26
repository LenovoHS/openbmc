#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit autotools systemd
inherit obmc-phosphor-systemd
inherit pkgconfig obmc-phosphor-ipmiprovider-symlink

S = "${WORKDIR}/"

SRC_URI = "file://include/fpga.h \
           file://include/Makefile.am \
           file://Makefile.am \
		   file://configure.ac \
		   file://bootstrap.sh \
		   file://fpga/Makefile.am \
		   file://fpga/fpga_control.c \
           file://fpga/fpga.pc.in \
          " 

DEPENDS = "systemd \
          autoconf-archive-native \
		  "

RDEPENDS_${PN} += "libsystemd"


SYSTEMD_PACKAGES = "${PN}"
#SYSTEMD_SERVICE_${PN} = "fpga.service"


IPMI_PROVIDER_LIBRARY += "libfpga.so"