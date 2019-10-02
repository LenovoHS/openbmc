# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SYSTEMD_AUTO_ENABLE = "enable"

SRC_URI += " file://discover-state.sh \
           "

FILES_${PN}-discover += " ${sbindir} \
                        "

do_install_append() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/discover-state.sh \
    ${D}${sbindir}/discover-state.sh
}
