# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend_hr650x+ := "${THISDIR}/${PN}:"

ALT_RMCPP_IFACE = "eth1"
SYSTEMD_SERVICE_${PN} += " \
    ${PN}@${ALT_RMCPP_IFACE}.service \
    ${PN}@${ALT_RMCPP_IFACE}.socket \
    "
