#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr855xg2 = " hr855xg2-yaml-config"

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-Add-workaround-patch-to-fix-get-wrong-service-issue.patch \
           "

EXTRA_OECONF_append_hr855xg2 = " \
    YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-fru-read.yaml \
    PROP_YAML=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-extra-properties.yaml \
    "

EEPROM_NAMES = "motherboard cpu0 cpu1 cpu2 cpu3 \
                CPU0_DIMMA1 CPU0_DIMMA2 CPU0_DIMMB1 CPU0_DIMMB2 CPU0_DIMMC1 CPU0_DIMMC2 \
                CPU0_DIMMD1 CPU0_DIMMD2 CPU0_DIMME1 CPU0_DIMME2 CPU0_DIMMF1 CPU0_DIMMF2 \
                CPU1_DIMMA1 CPU1_DIMMA2 CPU1_DIMMB1 CPU1_DIMMB2 CPU1_DIMMC1 CPU1_DIMMC2 \
                CPU1_DIMMD1 CPU1_DIMMD2 CPU1_DIMME1 CPU1_DIMME2 CPU1_DIMMF1 CPU1_DIMMF2 \
                CPU2_DIMMA1 CPU2_DIMMA2 CPU2_DIMMB1 CPU2_DIMMB2 CPU2_DIMMC1 CPU2_DIMMC2 \
                CPU2_DIMMD1 CPU2_DIMMD2 CPU2_DIMME1 CPU2_DIMME2 CPU2_DIMMF1 CPU2_DIMMF2 \
                CPU3_DIMMA1 CPU3_DIMMA2 CPU3_DIMMB1 CPU3_DIMMB2 CPU3_DIMMC1 CPU3_DIMMC2 \
                CPU3_DIMMD1 CPU3_DIMMD2 CPU3_DIMME1 CPU3_DIMME2 CPU3_DIMMF1 CPU3_DIMMF2 \
               "

EEPROMFMT = "system/chassis/{0}"
EEPROM_ESCAPEDFMT = "system-chassis-{0}"
EEPROMS = "${@compose_list(d, 'EEPROMFMT', 'EEPROM_NAMES')}"
EEPROMS_ESCAPED = "${@compose_list(d, 'EEPROM_ESCAPEDFMT', 'EEPROM_NAMES')}"

ENVFMT = "obmc/eeproms/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN}_append_hr855xg2 := " ${@compose_list(d, 'ENVFMT', 'EEPROMS')}"

TMPL = "obmc-read-eeprom@.service"
TGT = "multi-user.target"
INSTFMT = "obmc-read-eeprom@{0}.service"
FMT = "../${TMPL}:${TGT}.wants/${INSTFMT}"

SYSTEMD_LINK_${PN}_append_hr855xg2 := " ${@compose_list(d, 'FMT', 'EEPROMS_ESCAPED')}"

