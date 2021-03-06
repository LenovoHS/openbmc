# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo CATERR event application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://caterr-event.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/caterr-event.sh \
            ${D}${sbindir}/caterr-event.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/cpu_caterr"

CATERR_SERVICE = "cpu_caterr"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "caterr-event.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'CATERR_SERVICE')}"
