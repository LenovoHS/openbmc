# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/sh


switch_spidev_gpio()
{
	if [ ! -d /sys/class/gpio/gpio332 ]; then
		cd /sys/class/gpio
		echo 332 >export
		cd gpio332
	else
		cd /sys/class/gpio/gpio332
	fi
	direc=`cat direction`
	if [ $direc == "in" ]; then
		echo "out" >direction
	fi
	data=`cat value`
	echo $data
	if [ "$data" == "1" ]; then
		echo 0 >value
	fi
	return 0
}
switch_mtd_gpio()
{
	if [ ! -d /sys/class/gpio/gpio332 ]; then
		cd /sys/class/gpio
		echo 332 >export
		cd gpio332
	else
		cd /sys/class/gpio/gpio332
	fi
	direc=`cat direction`
	if [ $direc == "in" ]; then
		echo "out" >direction
	fi
	data=`cat value`
	echo $data
	if [ "$data" == "0" ]; then
		echo 1 >value
	fi
	return 0
}
#cd /run/initramfs/recovery
cd /var
newsha256=`cat version-in-bmc`

#cd squashfs-root
#oldsha256=`sha256sum "fpga.bin" | awk '{print $1}'`
oldsha256=newsha256

switch_spidev_gpio
cd /sys/bus/platform/drivers/ast-spi
#add ast-spi driver
echo "1e631000.spi" >unbind
#switch_mtd_gpio
switch_mtd_gpio
cd /sys/bus/platform/drivers/aspeed-smc
#remove mtd driver
echo "1e631000.spi" >bind
#end

if [ -e "/dev/mtd6" ]; then
	mtd6=`cat /sys/class/mtd/mtd6/name`
	if [ $mtd6 == "fpga" ]; then
	#hexdump /dev/mtd6 >fpga.spi
		
		dd if=/dev/mtd6 of=/var/fpga.spi bs=2M count=1
		cd /var
		if [ -e "fpga.spi" ]; then
			oldsha256=`sha256sum "fpga.spi" | awk '{print $1}'`
			
		fi
	else
		if [ -e "/dev/mtd7" ]; then
			mtd7=`cat /sys/class/mtd/mtd7/name`
			if [ $mtd7 == "fpga" ]; then
				dd if=/dev/mtd7 of=/var/fpga.spi bs=2M count=1
				cd /var
				if [ -e "fpga.spi" ]; then
				oldsha256=`sha256sum "fpga.spi" | awk '{print $1}'`
				echo $oldsha256 >oldhash
				fi
			fi
		fi
		
	fi
	
fi


if [ -e "/var/fpga.spi" ]; then
	rm fpga.spi
fi


if [ $newsha256 != $oldsha256 ]; 
then
        echo "starting uncompress fpga image"
		#test on no-rework mb start
		
		#if [ ! -d /var/squashfs-root ]; then
		if [ -e /var/fpga_recovery ]; then
		cd /var
		#req=`unsquashfs fpga_recovery fpga.bin`
		req=`tar zxvf fpga_recovery`
		fi
        #cd /run/initramfs/recovery/squashfs-root
		#cd /var/squashfs-root
		#req=`unsquashfs fpga_recovery fpga.bin`
		 
        if [ -e "fpga.bin" ];
        then
			if [ -e "/dev/mtd6" ]; then
				mtd6=`cat /sys/class/mtd/mtd6/name`
				if [ $mtd6 == "fpga" ]; then
					echo "starting fpga update..."
					flashcp -v fpga.bin /dev/mtd6 
				fi
			fi
			if [ -e "/dev/mtd7" ]; then
				mtd7=`cat /sys/class/mtd/mtd7/name`
				if [ $mtd7 == "fpga" ]; then
					echo "starting fpga update..."
					flashcp -v fpga.bin /dev/mtd7 
				fi
			fi
        fi
		
        echo "starting delete uncompressed fpga image"
		cd /var
        rm -rf fpga.bin
	
else
		#cd /run/initramfs/recovery/squashfs-root
		#cd /var/squashfs-root
		#if [ -e "fpga.bin" ]; then
		#	rm -rf fpga.bin
        echo "do not need recovery fpga, so delete uncompressed fpga image"
fi

switch_mtd_gpio
cd /sys/bus/platform/drivers/aspeed-smc
#remove mtd driver
echo "1e631000.spi" >unbind
switch_spidev_gpio
cd /sys/bus/platform/drivers/ast-spi
#add ast-spi driver
echo "1e631000.spi" >bind
