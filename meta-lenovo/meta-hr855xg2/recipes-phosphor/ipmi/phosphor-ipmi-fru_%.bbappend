# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr855xg2 = " hr855xg2-yaml-config"

FILESEXTRAPATHS_append_hr855xg2 := "${THISDIR}/${PN}:"
SRC_URI += "file://0002-Disable-fru-write-command-in-fru-parser.patch \
           "

EXTRA_OECONF_append_hr855xg2 = " \
    YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-fru-read.yaml \
    PROP_YAML=${STAGING_DIR_HOST}${datadir}/hr855xg2-yaml-config/ipmi-extra-properties.yaml \
    "

