# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo PDB Alert application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://fm-pvccin-cpu3-pwr-in-alert-n.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/fm-pvccin-cpu3-pwr-in-alert-n.sh \
            ${D}${sbindir}/fm-pvccin-cpu3-pwr-in-alert-n.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/fm_pvccin_cpu3_pwr_in_alert_n"

FM_PVCCIN_CPU3_PWR_IN_ALERT_N_SERVICE = "fm_pvccin_cpu3_pwr_in_alert_n"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "fm-pvccin-cpu3-pwr-in-alert-n.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'FM_PVCCIN_CPU3_PWR_IN_ALERT_N_SERVICE')}"
