# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

#inherit systemd
#inherit obmc-phosphor-systemd

#SRC_URI += "file://xyz.openbmc_project.State.Host.service \
#           " 

SYSTEMD_AUTO_ENABLE = "enable"

#SYSTEMD_PACKAGES = "${PN}"
#SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.State.Host.service"
