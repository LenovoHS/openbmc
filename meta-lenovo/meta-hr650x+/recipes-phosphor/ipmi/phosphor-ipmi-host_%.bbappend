#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr650x+ = " hr650x+-yaml-config"

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-fru-workaround.patch \
           "

EXTRA_OECONF_append_hr650x+ = " \
    SENSOR_YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr650x+-yaml-config/ipmi-sensors.yaml \
    FRU_YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr650x+-yaml-config/ipmi-fru-read.yaml \
    "
