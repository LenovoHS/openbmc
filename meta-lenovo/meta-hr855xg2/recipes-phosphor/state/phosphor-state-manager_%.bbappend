#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += " file://sync-power.sh \
           "

FILES_${PN}-discover += " ${sbindir} \
                        "

do_install_append() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/sync-power.sh \
            ${D}${sbindir}/sync-power.sh
}
