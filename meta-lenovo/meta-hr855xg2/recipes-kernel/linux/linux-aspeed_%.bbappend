#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."


FILESEXTRAPATHS_prepend_hr855xg2 := "${THISDIR}/${PN}:"
SRC_URI += "file://aspeed-bmc-lenovo-hr855xg2.dts \
            file://hr855xg2.cfg \
            file://0001-BMC-FPGACHIP-COMMUNICATION.patch \
            file://0001-support-ASIC.patch \
            file://0002-DO-NOT-MODIFY-ADDR-REG-FOR-2M-ROM.patch \
            "     
			

do_configure_append() {

	myfile="../aspeed-bmc-lenovo-hr855xg2.dts"
	
	if [ ! -f $myfile ]; then
		echo $myfile" is not exist"	
	else
		cp ../aspeed-bmc-lenovo-hr855xg2.dts  ./source/arch/arm/boot/dts/
	fi	
}


