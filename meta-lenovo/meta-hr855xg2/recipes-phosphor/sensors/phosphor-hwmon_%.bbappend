# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"

# Temporarily enable the configure "enable-negative-errno-on-fail", 
# without the check, FAN readings become unavaliable.
# See https://gerrit.openbmc-project.xyz/c/openbmc/meta-lenovo/+/23002/
# and https://gerrit.openbmc-project.xyz/c/openbmc/phosphor-host-ipmid/+/22780 for details.
# Once ipmi-host is fixed, the below configure can be removed. 
EXTRA_OECONF_append_hr855xg2 = " --enable-negative-errno-on-fail"

CHIPS = " bus@1e78a000/i2c-bus@1c0/tmp75@49  \
          bus@1e78a000/i2c-bus@1c0/tmp75@4d  \
          bus@1e78a000/i2c-bus@80/HotSwap@10 \
          bus@1e78a000/i2c-bus@80/VR@45      \
          pwm-tacho-controller@1e786000      \
          bus@1e78a000/i2c-bus@180/CPU0_VCCIN@60   \
          bus@1e78a000/i2c-bus@180/CPU1_VCCIN@62   \
          bus@1e78a000/i2c-bus@180/CPU2_VCCIN@64   \
          bus@1e78a000/i2c-bus@180/CPU3_VCCIN@66   \
          bus@1e78a000/i2c-bus@480/CPU0_VDDQ_ABC@58  \
          bus@1e78a000/i2c-bus@480/CPU0_VDDQ_DEF@5a   \
          bus@1e78a000/i2c-bus@480/CPU1_VDDQ_ABC@5c   \
          bus@1e78a000/i2c-bus@480/CPU1_VDDQ_DEF@5e   \
          bus@1e78a000/i2c-bus@480/CPU2_VDDQ_ABC@68   \
          bus@1e78a000/i2c-bus@480/CPU2_VDDQ_DEF@6a   \
          bus@1e78a000/i2c-bus@480/CPU3_VDDQ_ABC@6c   \
          bus@1e78a000/i2c-bus@480/CPU3_VDDQ_DEF@6e   \
        "
ITEMSFMT = "ahb/apb/{0}.conf"

HR855XG2_ITEMS = "${@compose_list(d, 'ITEMSFMT', 'CHIPS')}"
HR855XG2_ITEMS += "iio-hwmon.conf"

HR855XG2_ITEMS += "iio-hwmon-battery.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN}_append_hr855xg2 := "${@compose_list(d, 'ENVS', 'HR855XG2_ITEMS')}"
