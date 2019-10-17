SUMMARY = "Lenovo OEM IPMI commands"
DESCRIPTION = "Lenovo OEM IPMI commands"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"
#PR = "r1"
#PV = "0.1+git"

inherit autotools pkgconfig
inherit obmc-phosphor-ipmiprovider-symlink

DEPENDS += "autoconf-archive-native"
DEPENDS += "phosphor-ipmi-host"


S = "${WORKDIR}/"

SRC_URI = "file://Makefile.am \
           file://configure.ac \
           file://bootstrap.sh \
           file://src/Makefile.am \
           file://src/oemcommands.cpp \
           file://include/oemcommands.hpp \
           file://src/storagecommands.cpp \
           file://include/storagecommands.hpp \
           file://include/sdrutils.hpp \
           file://include/selutility.hpp \
           file://include/commandutils.hpp \ 
          " 

# The following installs the OEM IPMI handler for the Lenovo OEM IPMI commands.
FILES_${PN}_append = " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/host-ipmid/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/net-ipmid/lib*${SOLIBS}"
FILES_${PN}-dev_append = " ${libdir}/ipmid-providers/lib*${SOLIBSDEV} ${libdir}/ipmid-providers/*.la"

HOSTIPMI_PROVIDER_LIBRARY += "liboemcmds.so"

