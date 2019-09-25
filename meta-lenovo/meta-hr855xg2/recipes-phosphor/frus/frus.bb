# "Copyright (c) 2019-present Lenovo"

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/files:"
SUMMARY = "hr855xg2 default FRU for CPU and DIMM"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

S = "${WORKDIR}/"

SRC_URI = "file://fru_dimm.bin \
           file://fru_cpu.bin \
           file://fru_cpu0.bin \
           file://fru_cpu1.bin \
           file://fru_cpu2.bin \
           file://fru_cpu3.bin \
           file://fru_dimm0.bin \
           file://fru_dimm1.bin \ 
           file://fru_dimm2.bin \
           file://fru_dimm3.bin \
           file://fru_dimm4.bin \
           file://fru_dimm5.bin \
           file://fru_dimm6.bin \
           file://fru_dimm7.bin \
           file://fru_dimm8.bin \
           file://fru_dimm9.bin \
           file://fru_dimm10.bin \
           file://fru_dimm11.bin \
           file://fru_dimm12.bin \
           file://fru_dimm13.bin \
           file://fru_dimm14.bin \
           file://fru_dimm15.bin \
           file://fru_dimm16.bin \
           file://fru_dimm17.bin \
           file://fru_dimm18.bin \
           file://fru_dimm19.bin \
           file://fru_dimm20.bin \
           file://fru_dimm21.bin \
           file://fru_dimm22.bin \
           file://fru_dimm23.bin \
           file://fru_dimm24.bin \
           file://fru_dimm25.bin \
           file://fru_dimm26.bin \
           file://fru_dimm27.bin \
           file://fru_dimm28.bin \
           file://fru_dimm29.bin \
           file://fru_dimm30.bin \
           file://fru_dimm31.bin \
           file://fru_dimm32.bin \
           file://fru_dimm33.bin \
           file://fru_dimm34.bin \
           file://fru_dimm35.bin \
           file://fru_dimm36.bin \
           file://fru_dimm37.bin \
           file://fru_dimm38.bin \
           file://fru_dimm39.bin \
           file://fru_dimm40.bin \
           file://fru_dimm41.bin \
           file://fru_dimm42.bin \
           file://fru_dimm43.bin \
           file://fru_dimm44.bin \
           file://fru_dimm45.bin \
           file://fru_dimm46.bin \
           file://fru_dimm47.bin \
          "

do_install() {
    install -d 0755 ${D}/var/frus
    install -d ${D}/usr/sbin
    install -m 0755 ${S}fru_dimm.bin ${D}/${sbindir}/
    install -m 0755 ${S}fru_cpu.bin ${D}/${sbindir}/
    install -m 0755 ${S}fru_cpu0.bin ${D}/var/frus/
    install -m 0755 ${S}fru_cpu1.bin ${D}/var/frus/
    install -m 0755 ${S}fru_cpu2.bin ${D}/var/frus/
    install -m 0755 ${S}fru_cpu3.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm0.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm1.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm2.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm3.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm4.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm5.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm6.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm7.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm8.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm9.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm10.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm11.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm12.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm13.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm14.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm15.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm16.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm17.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm18.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm19.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm20.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm21.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm22.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm23.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm24.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm25.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm26.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm27.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm28.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm29.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm30.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm31.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm32.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm33.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm34.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm35.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm36.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm37.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm38.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm39.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm40.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm41.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm42.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm43.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm44.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm45.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm46.bin ${D}/var/frus/
    install -m 0755 ${S}fru_dimm47.bin ${D}/var/frus/
}
