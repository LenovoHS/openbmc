SUMMARY = "Lenovo OEM LED Manager"
DESCRIPTION = "Lenovo OEM LED Manager"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit cmake systemd
S = "${WORKDIR}/"


SRC_URI = "file://include/led-manager.hpp \
           file://src/led-manager.cpp \
           file://CMakeLists.txt \
           file://service_files/led-manager.service \
           "


DEPENDS += "systemd sdbusplus boost phosphor-ipmi-host"

SYSTEMD_SERVICE_${PN} += "led-manager.service"