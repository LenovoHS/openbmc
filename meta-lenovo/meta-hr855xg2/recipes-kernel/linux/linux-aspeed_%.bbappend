#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"
SRC_URI += "file://aspeed-bmc-lenovo-hr855xg2.dts \
            file://hr855xg2.cfg \
            file://0001-BMC-FPGACHIP-COMMUNICATION.patch \
            file://0001-support-ASIC.patch \
            file://0002-DO-NOT-MODIFY-ADDR-REG-FOR-2M-ROM.patch \
            file://0003-MODIFY-READ-LEN-AST-SPI.patch \
            file://0004-ENABLE-RAA228006.patch \
            file://0005-ENABLE-SB1701022.patch  \
            file://0005-FIX-REMOVE-DRIVER-KERNEL-PANIC.patch \
            file://0006-REMOVE-SPI-DEBUG-MESSAGE.patch \
            file://0007-ASPEED-VUART.patch \
            file://0008-ENABLE-IR38164.patch \
            "

do_configure_append() {

    myfile="../aspeed-bmc-lenovo-hr855xg2.dts"

    if [ ! -f $myfile ]; then
        echo $myfile" is not exist"
    else
        cp ../aspeed-bmc-lenovo-hr855xg2.dts  ./source/arch/arm/boot/dts/
    fi
}

