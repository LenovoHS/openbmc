#"Copyright ? 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
EXTRA_OECONF_append_hr650x+ = " --enable-negative-errno-on-fail"
NAMES = " bus@1e78a000/i2c-bus@40/tmp75@4e  bus@1e78a000/i2c-bus@80/tmp75@4d bus@1e78a000/i2c-bus@140/pmbus@68 bus@1e78a000/i2c-bus@140/pmbus@69 pwm-tacho-controller@1e786000 "
ITEMSFMT = "ahb/apb/{0}.conf"

ITEMS = "${@compose_list(d, 'ITEMSFMT', 'NAMES')}"
ITEMS += "iio-hwmon.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN} += "${@compose_list(d, 'ENVS', 'ITEMS')}"
