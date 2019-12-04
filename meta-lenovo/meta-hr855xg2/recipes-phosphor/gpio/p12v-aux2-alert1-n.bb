# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo P12V AUX2 Alert1 application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://p12v-aux2-alert1-n.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/p12v-aux2-alert1-n.sh \
            ${D}${sbindir}/p12v-aux2-alert1-n.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/p12v_aux2_alert1_n"

P12V_AUX2_ALERT1_N_SERVICE = "p12v_aux2_alert1_n"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "p12v-aux2-alert1-n.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'P12V_AUX2_ALERT1_N_SERVICE')}"
