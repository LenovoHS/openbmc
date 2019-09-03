# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

PACKAGECONFIG_append_hr855xg2 = " static-bmc"
PACKAGECONFIG_append_hr855xg2 = " reboot-update"
PACKAGECONFIG_append_hr855xg2 = " host-bios"
PACKAGECONFIG_append_hr855xg2 = " aspeed-lpc"
IPMI_FLASH_BMC_ADDRESS_hr855xg2 = "0xFE0B0000"

do_install_append_hr855xg2() {
    rm -f ${D}/usr/share/phosphor-ipmi-flash/config-bios.json
}