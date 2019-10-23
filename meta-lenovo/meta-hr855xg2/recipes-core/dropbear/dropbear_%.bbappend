# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += " file://dropbear.default \
           "

do_install_append() {
        install -d ${D}${sysconfdir}
        install -m 0644 ${S}/debian/dropbear.default \
            ${D}${sysconfdir}/default/dropbear
}
