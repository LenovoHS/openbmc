#"Copyright ? 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append_hr650x+ := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Enable-debug-mode.patch \
            file://HR650X+.json "

do_install_append () {
        install -m 644 ${WORKDIR}/HR650X+.json ${D}/usr/share/entity-manager/configurations
}
