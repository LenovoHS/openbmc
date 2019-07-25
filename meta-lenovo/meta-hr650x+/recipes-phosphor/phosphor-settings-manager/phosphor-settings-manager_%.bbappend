#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

DEPENDS_append_hr650x+ = " hr650x+-yaml-config"

EXTRA_OECONF_append_hr650x+ = "SETTINGS_YAML=${STAGING_DIR_HOST}${datadir}/hr650x+-yaml-config/defaults.yaml"

