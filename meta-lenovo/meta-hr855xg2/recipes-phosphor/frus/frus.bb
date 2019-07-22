#"Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SUMMARY = "hr855xg2 default FRU for CPU and DIMM"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

S = "${WORKDIR}/"

SRC_URI = "file://fru_dimm.bin \
           file://fru_cpu.bin \
          "

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 ${S}fru_dimm.bin ${D}/${sbindir}/
    install -m 0755 ${S}fru_cpu.bin ${D}/${sbindir}/
}
