# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo Riser2 VR Alert application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://riser2-vr-al-r.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/riser2-vr-al-r.sh \
            ${D}${sbindir}/riser2-vr-al-r.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/riser2_vr_al_r"

RISER2_VR_ALT_SERVICE = "riser2_vr_al_r"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "riser2-vr-al-r.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'RISER2_VR_ALT_SERVICE')}"
