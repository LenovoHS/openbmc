# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_append := "${THISDIR}/files:"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit autotools pkgconfig
inherit obmc-phosphor-ipmiprovider-symlink

DEPENDS += "autoconf-archive-native"
DEPENDS += "phosphor-ipmi-host"
DEPENDS += "fpga"

S = "${WORKDIR}"

SRC_URI = "file://Makefile.am \
           file://src/fpga-util.cpp \
           file://src/argument.cpp \
           file://include/argument.hpp \
           file://include/fpga-util.hpp \
           file://src/Makefile.am \
           file://configure.ac \
           file://bootstrap.sh \
          "



