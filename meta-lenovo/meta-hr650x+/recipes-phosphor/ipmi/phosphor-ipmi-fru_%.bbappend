#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr650x+ = " hr650x+-yaml-config"

FILESEXTRAPATHS_append := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-Add-workaround-patch-to-fix-get-wrong-service-issue.patch \
           "

EXTRA_OECONF_append_hr650x+ = " \
    YAML_GEN=${STAGING_DIR_HOST}${datadir}/hr650x+-yaml-config/ipmi-fru-read.yaml \
    PROP_YAML=${STAGING_DIR_HOST}${datadir}/hr650x+-yaml-config/ipmi-extra-properties.yaml \
    "
