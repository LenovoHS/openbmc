#"Copyright ? 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
EXTRA_OECONF_append_hr650x+ = " --enable-negative-errno-on-fail"
NAMES = " bus@1e78a000/i2c-bus@40/tmp75@4e   \
          bus@1e78a000/i2c-bus@80/tmp75@4d   \
          bus@1e78a000/i2c-bus@c0/pxm1310@60 \
          bus@1e78a000/i2c-bus@c0/pxm1310@62 \
          bus@1e78a000/i2c-bus@c0/pxm1310@70 \
          bus@1e78a000/i2c-bus@c0/pxm1310@72 \ 
          bus@1e78a000/i2c-bus@c0/pxe1610@74 \
          bus@1e78a000/i2c-bus@c0/pxe1610@76 \
          bus@1e78a000/i2c-bus@140/pmbus@68  \
          bus@1e78a000/i2c-bus@140/pmbus@69  \
          bus@1e78a000/i2c-bus@300/i2c-switch@76/i2c@3/tmp421@1f \
          pwm-tacho-controller@1e786000      \
        "
ITEMSFMT = "ahb/apb/{0}.conf"

ITEMS = "${@compose_list(d, 'ITEMSFMT', 'NAMES')}"
ITEMS += "iio-hwmon.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN} += "${@compose_list(d, 'ENVS', 'ITEMS')}"
