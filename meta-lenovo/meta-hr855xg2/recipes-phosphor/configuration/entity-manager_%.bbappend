#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append_hr855xg2 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Enable-debug-mode.patch \
            file://0002-Add-internal-length-check.patch \
            file://HR855XG2.json "

do_install_append () {
        install -m 644 ${WORKDIR}/HR855XG2.json ${D}/usr/share/entity-manager/configurations
}

