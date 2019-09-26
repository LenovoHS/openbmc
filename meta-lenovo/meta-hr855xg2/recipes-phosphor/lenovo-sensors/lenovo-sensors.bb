#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."
SUMMARY = "Lenovo Sensors"
DESCRIPTION = "Lenovo Sensors"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

SRC_URI = "file://CMakeLists.txt \
           file://include/FPGASensor.hpp \
           file://include/OEMSensor.hpp \
           file://include/Utils.hpp \
           file://include/VariantVisitors.hpp \
           file://src/FPGASensorMain.cpp \
           file://src/OEMSensorMain.cpp \
           file://src/FPGASensor.cpp \
           file://src/Utils.cpp \
           file://service_files/xyz.openbmc_project.fpgasensor.service \
           file://service_files/xyz.openbmc_project.oemsensor.service \
           file://cmake/HunterGate.cmake \
           file://cmake/Finddbus.cmake \
          "

SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.fpgasensor.service \
                         xyz.openbmc_project.oemsensor.service \
                        "


DEPENDS = "boost nlohmann-json sdbusplus i2c-tools libgpiod fpga"
inherit cmake systemd 

S = "${WORKDIR}/"

# linux-libc-headers guides this way to include custom uapi headers
CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include"
do_configure[depends] += "virtual/kernel:do_shared_workdir"
EXTRA_OECMAKE = "-DYOCTO=1"
