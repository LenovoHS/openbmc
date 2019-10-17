#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_append_hr855xg2 := "${THISDIR}/${PN}:"

# Package configuration

SYSTEMD_SERVICE_${PN}-presence-tach_remove_hr855xg2 = "${TMPL_TACH}"
SYSTEMD_LINK_${PN}-presence-tach_remove_hr855xg2 = "${@compose_list(d, 'FMT_TACH', 'OBMC_CHASSIS_INSTANCES')}"
 
SYSTEMD_SERVICE_${PN}-control_remove_hr855xg2 = "${TMPL_CONTROL} ${TMPL_CONTROL_INIT}"
SYSTEMD_LINK_${PN}-control_remove_hr855xg2 = "${@compose_list(d, 'FMT_CONTROL_INIT', 'OBMC_CHASSIS_INSTANCES')}"
SYSTEMD_LINK_${PN}-control_remove_hr855xg2 = "${@compose_list(d, 'FMT_CONTROL', 'OBMC_CHASSIS_INSTANCES')}"

SYSTEMD_SERVICE_${PN}-monitor_remove_hr855xg2 = "${TMPL_MONITOR} ${TMPL_MONITOR_INIT}"
SYSTEMD_LINK_${PN}-monitor_remove_hr855xg2 = "${@compose_list(d, 'FMT_MONITOR_INIT', 'OBMC_CHASSIS_INSTANCES')}"
SYSTEMD_LINK_${PN}-monitor_remove_hr855xg2 = "${@compose_list(d, 'FMT_MONITOR', 'OBMC_CHASSIS_INSTANCES')}"
