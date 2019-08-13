SUMMARY = "Lenovo Fan Services"
DESCRIPTION = "Lenovo Fan Services for specific purpose"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"
#PR = "r1"
#PV = "0.1+git"

inherit cmake systemd obmc-phosphor-systemd

S = "${WORKDIR}"

SRC_URI = "file://src/fan_switch.cpp \
           file://CMakeLists.txt \
           file://service_files/lenovo-fan.service \
           "

DEPENDS = "systemd"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "lenovo-fan.service"
