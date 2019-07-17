#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"
SRC_URI += "file://0001_InvenManager.patch \
            file://0002_FanDetect.patch \
           "

# Package configuration

TMPL_TACH = "phosphor-fan-presence-tach.service"
FMT_TACH = ""
SYSTEMD_LINK_${PN}-presence-tach = ""
 
FMT_CONTROL = ""
FMT_CONTROL_INIT = ""
SYSTEMD_SERVICE_${PN}-control = ""
SYSTEMD_LINK_${PN}-control = ""

FMT_MONITOR = ""
FMT_MONITOR_INIT = ""
SYSTEMD_SERVICE_${PN}-monitor = ""
SYSTEMD_LINK_${PN}-monitor = ""

TMPL_COOLING = "phosphor-cooling-type@.service"
INSTFMT_COOLING = "phosphor-cooling-type@{0}.service"
COOLING_TGT = "${SYSTEMD_DEFAULT_TARGET}"
FMT_COOLING = "../${TMPL_COOLING}:${COOLING_TGT}.requires/${INSTFMT_COOLING}"

FILES_phosphor-cooling-type = "${sbindir}/phosphor-cooling-type"
SYSTEMD_SERVICE_phosphor-cooling-type += "${TMPL_COOLING}"
SYSTEMD_LINK_phosphor-cooling-type += "${@compose_list(d, 'FMT_COOLING', 'OBMC_CHASSIS_INSTANCES')}"

COOLING_ENV_FMT = "obmc/phosphor-fan/phosphor-cooling-type-{0}.conf"

SYSTEMD_ENVIRONMENT_FILE_phosphor-cooling-type += "${@compose_list(d, 'COOLING_ENV_FMT', 'OBMC_CHASSIS_INSTANCES')}"
