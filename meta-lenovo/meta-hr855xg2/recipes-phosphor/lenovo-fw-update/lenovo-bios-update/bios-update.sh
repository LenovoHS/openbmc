#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/sh

OUT=/tmp/bios.update
IMAGE_FILE=/tmp/image-bios

switch_bmc_bios()
{
	if [ ! -d /sys/class/gpio/gpio480 ]; then
		cd /sys/class/gpio
		echo 480 >export
		cd gpio480
	else
		cd /sys/class/gpio/gpio480
	fi
	direc=`cat direction`
	if [ $direc == "in" ]; then
		echo "out" >direction
	fi
	data=`cat value`
	echo $data
	if [ "$data" == "0" ]; then
		echo 1 > value
	fi
	return 0
}

switch_pch_bios()
{
	if [ ! -d /sys/class/gpio/gpio480 ]; then
		cd /sys/class/gpio
		echo 480 >export
		cd gpio480
	else
		cd /sys/class/gpio/gpio480
	fi
	direc=`cat direction`
	if [ $direc == "in" ]; then
		echo "out" >direction
	fi
	data=`cat value`
	echo $data
	if [ "$data" == "1" ]; then
		echo 0 > value
	fi
	return 0
}

switch_bmc_bios
cd /sys/bus/platform/drivers/aspeed-smc
#bind mtd driver
echo "1e630000.spi" >bind

if [ -e "$IMAGE_FILE" ];
then
    echo "running" > $OUT    

    if [ -e "/dev/mtd6" ]; then
        mtd6=`cat /sys/class/mtd/mtd6/name`
        if [ $mtd6 == "pnor" ]; then
            echo "starting bios update..."
            flashcp -v $IMAGE_FILE /dev/mtd6 
            if [ $? -eq 0 ]; then
                echo "success" > $OUT
                echo "bios update successfully..."
            else
                echo "failed" > $OUT
                echo "bios update failed..."
            fi
        fi
    fi
    
    if [ -e "/dev/mtd7" ]; then
        mtd7=`cat /sys/class/mtd/mtd7/name`
        if [ $mtd7 == "pnor" ]; then
            echo "starting bios update..."
            flashcp -v $IMAGE_FILE /dev/mtd7
            if [ $? -eq 0 ]; then
                echo "success" > $OUT
                echo "bios update successfully..."
            else
                echo "failed" > $OUT
                echo "bios update failed..."
            fi
        fi
    fi
fi

switch_pch_bios
cd /sys/bus/platform/drivers/aspeed-smc
#remove mtd driver
echo "1e630000.spi" >unbind
sleep 5 
rm -f $OUT
rm -f $IMAGE_FILE
