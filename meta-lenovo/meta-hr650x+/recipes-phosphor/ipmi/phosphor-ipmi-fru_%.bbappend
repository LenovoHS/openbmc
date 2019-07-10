inherit obmc-phosphor-systemd

FILESEXTRAPATHS_prepend_hr650x+ := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-workaround-patch-to-fix-get-wrong-service-issue.patch \
           "

EEPROM_NAMES = "motherboard"

EEPROMFMT = "system/chassis/{0}"
EEPROM_ESCAPEDFMT = "system-chassis-{0}"
EEPROMS = "${@compose_list(d, 'EEPROMFMT', 'EEPROM_NAMES')}"
EEPROMS_ESCAPED = "${@compose_list(d, 'EEPROM_ESCAPEDFMT', 'EEPROM_NAMES')}"

ENVFMT = "obmc/eeproms/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN}_append_hr650x+ := " ${@compose_list(d, 'ENVFMT', 'EEPROMS')}"

TMPL = "obmc-read-eeprom@.service"
TGT = "multi-user.target"
INSTFMT = "obmc-read-eeprom@{0}.service"
FMT = "../${TMPL}:${TGT}.wants/${INSTFMT}"

SYSTEMD_LINK_${PN}_append_hr650x+ := " ${@compose_list(d, 'FMT', 'EEPROMS_ESCAPED')}"
