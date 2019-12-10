# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."


DEPENDS_append_hr855xg2 = " hr855xg2-yaml-config"

FILESEXTRAPATHS_append_hr855xg2 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-fru-workaround.patch \
            file://0006-pwm-sensor.patch \
            file://0007-fix-assertion-type-sensor-disable.patch \
           "

EXTRA_OECONF_append_hr855xg2 = " \
    SENSOR_YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-sensors.yaml \
    FRU_YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-fru-read.yaml \
     --enable-update-functional-on-fail \
    "
