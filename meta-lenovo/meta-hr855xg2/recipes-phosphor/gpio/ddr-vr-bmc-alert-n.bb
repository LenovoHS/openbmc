# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo DDR VR Alert application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://ddr-vr-bmc-alert-n.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/ddr-vr-bmc-alert-n.sh \
            ${D}${sbindir}/ddr-vr-bmc-alert-n.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/ddr_vr_bmc_alert_n"

DDR_VR_BMC_ALERT_N_SERVICE = "ddr_vr_bmc_alert_n"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "ddr-vr-bmc-alert-n.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'DDR_VR_BMC_ALERT_N_SERVICE')}"
