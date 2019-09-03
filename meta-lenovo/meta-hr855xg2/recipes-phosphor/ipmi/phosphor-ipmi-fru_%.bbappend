# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr855xg2 = " hr855xg2-yaml-config"

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-Add-workaround-patch-to-fix-get-wrong-service-issue.patch \
           "

EXTRA_OECONF_append_hr855xg2 = " \
    YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-fru-read.yaml \
    PROP_YAML=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-extra-properties.yaml \
    "

EEPROM_NAMES = "motherboard cpu0 cpu1 cpu2 cpu3 \
                CPU0_DIMM_A1 CPU0_DIMM_A2 CPU0_DIMM_B1 CPU0_DIMM_B2 CPU0_DIMM_C1 CPU0_DIMM_C2 \
                CPU0_DIMM_D1 CPU0_DIMM_D2 CPU0_DIMM_E1 CPU0_DIMM_E2 CPU0_DIMM_F1 CPU0_DIMM_F2 \
                CPU1_DIMM_A1 CPU1_DIMM_A2 CPU1_DIMM_B1 CPU1_DIMM_B2 CPU1_DIMM_C1 CPU1_DIMM_C2 \
                CPU1_DIMM_D1 CPU1_DIMM_D2 CPU1_DIMM_E1 CPU1_DIMM_E2 CPU1_DIMM_F1 CPU1_DIMM_F2 \
                CPU2_DIMM_A1 CPU2_DIMM_A2 CPU2_DIMM_B1 CPU2_DIMM_B2 CPU2_DIMM_C1 CPU2_DIMM_C2 \
                CPU2_DIMM_D1 CPU2_DIMM_D2 CPU2_DIMM_E1 CPU2_DIMM_E2 CPU2_DIMM_F1 CPU2_DIMM_F2 \
                CPU3_DIMM_A1 CPU3_DIMM_A2 CPU3_DIMM_B1 CPU3_DIMM_B2 CPU3_DIMM_C1 CPU3_DIMM_C2 \
                CPU3_DIMM_D1 CPU3_DIMM_D2 CPU3_DIMM_E1 CPU3_DIMM_E2 CPU3_DIMM_F1 CPU3_DIMM_F2 \
               "

EEPROMFMT = "system/chassis/motherboard/{0}"
EEPROM_ESCAPEDFMT = "system-chassis-motherboard-{0}"
EEPROMS = "${@compose_list(d, 'EEPROMFMT', 'EEPROM_NAMES')}"
EEPROMS_ESCAPED = "${@compose_list(d, 'EEPROM_ESCAPEDFMT', 'EEPROM_NAMES')}"

ENVFMT = "obmc/eeproms/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN}_append_hr855xg2 := " ${@compose_list(d, 'ENVFMT', 'EEPROMS')}"

TMPL = "obmc-read-eeprom@.service"
TGT = "multi-user.target"
INSTFMT = "obmc-read-eeprom@{0}.service"
FMT = "../${TMPL}:${TGT}.wants/${INSTFMT}"

SYSTEMD_LINK_${PN}_append_hr855xg2 := " ${@compose_list(d, 'FMT', 'EEPROMS_ESCAPED')}"

