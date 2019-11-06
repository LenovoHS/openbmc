# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo Power Button pressed application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://int-fpga-bmc.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/int-fpga-bmc.sh \
            ${D}${sbindir}/int-fpga-bmc.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/int_fpga_bmc"

INT_FPGA_BMC_SERVICE = "int_fpga_bmc"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "int-fpga-bmc.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'INT_FPGA_BMC_SERVICE')}"
