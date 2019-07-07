#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"
EXTRA_OECONF_append_hr855xg2 = " --enable-negative-errno-on-fail"
NAMES = " bus@1e78a000/i2c-bus@1c0/tmp75@49  bus@1e78a000/i2c-bus@1c0/tmp75@4d bus@1e78a000/i2c-bus@80/HotSwap@10 bus@1e78a000/i2c-bus@80/VR@45  pwm-tacho-controller@1e786000 "
ITEMSFMT = "ahb/apb/{0}.conf"

ITEMS = "${@compose_list(d, 'ITEMSFMT', 'NAMES')}"
ITEMS += "iio-hwmon.conf"

ITEMS += "iio-hwmon-battery.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN}_append_hr855xg2 := "${@compose_list(d, 'ENVS', 'ITEMS')}"
