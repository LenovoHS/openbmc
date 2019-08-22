#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-add-lenovo-cpu-status-property.patch \
            file://0002-add-discrete-snr-interface-in-dbus.patch \
           "
