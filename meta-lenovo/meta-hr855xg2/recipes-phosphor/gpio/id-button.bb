# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

SUMMARY = "Lenovo ID Button pressed application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://toggle_identify_led.sh \
            file://uidon.service \
            file://uidoff.service \
            file://uid.sh \
            "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/toggle_identify_led.sh \
            ${D}${sbindir}/toggle_identify_led.sh
        install -m 0755 ${WORKDIR}/uid.sh \
            ${D}${sbindir}/uid.sh
}

SYSTEMD_SERVICE_${PN} += " \
                        uidon.service\
                        uidoff.service \
                        "

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/id_button"

ID_BUTTON_SERVICE = "id_button"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "id-button-pressed.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'ID_BUTTON_SERVICE')}"
