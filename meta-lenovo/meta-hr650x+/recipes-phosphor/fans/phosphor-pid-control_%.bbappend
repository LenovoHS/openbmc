# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd
SYSTEMD_SERVICE_${PN} = "phosphor-pid-control.service \
                         initial_fanspeed.service \
                        "
 
#EXTRA_OECONF = "--enable-configure-dbus=yes"

SRC_URI += "file://fan-info.json \
            file://0001_AddSetPWMOEM.patch \
            file://initial_fanspeed.sh \
            file://start_phosphor-pid-control.sh \
           "
FILES_${PN} += "${datadir}/swampd/ \
                ${sbindir} \
               "

do_install_append() {
   install -d ${D}${datadir}/swampd
   install -m 0644 ${WORKDIR}/fan-info.json ${D}${datadir}/swampd/config.json
   install -d ${D}/${sbindir}
   install -m 0755 ${WORKDIR}/*.sh ${D}/${sbindir}/
}
