#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-add-lenovo-cpu-status-property.patch \
            file://0002-add-discrete-snr-interface-in-dbus.patch \
            file://0002-add-lenovo-pcie-device-property.patch \
            file://0003-add-lenovo-slot-conn-property.patch \
            file://0004-add-lenovo-cable-property.patch \
            file://0005-add-lenovo-sys-fw-property.patch \
            file://0006-add-sensor-memerr-property.patch \
           "
