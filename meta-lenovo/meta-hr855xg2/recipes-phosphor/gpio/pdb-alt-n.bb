# "Copyright (c) 2019-present Lenovo

SUMMARY = "Lenovo PDB Alert application"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://pdb-alt-n.sh \
           "

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/pdb-alt-n.sh \
            ${D}${sbindir}/pdb-alt-n.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/pdb_alt_n"

PDB_ALT_N_SERVICE = "pdb_alt_n"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "multi-user.target"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "pdb-alt-n.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'PDB_ALT_N_SERVICE')}"
