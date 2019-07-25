#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd
SYSTEMD_SERVICE_${PN} = "phosphor-pid-control.service"
#EXTRA_OECONF = "--enable-configure-dbus=yes"

SRC_URI += "file://fan-info.json \
            file://fan-info.json_mod \
            file://0001_AddSetPWMOEM.patch \
           "
FILES_${PN} += "${datadir}/swampd/"

do_install_append() {
   install -d ${D}${datadir}/swampd
   install -m 0644 ${WORKDIR}/fan-info.json ${D}${datadir}/swampd/config.json
   install -m 0644 ${WORKDIR}/fan-info.json_mod ${D}${datadir}/swampd/config.json_mod
}
