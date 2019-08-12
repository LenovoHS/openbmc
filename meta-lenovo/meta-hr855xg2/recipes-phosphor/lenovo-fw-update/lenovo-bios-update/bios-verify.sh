#"Copyright (c) 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

#!/bin/sh
IMAGE_FILE=/tmp/bios-image
SIG_FILE=/tmp/bmc.sig
OUT=/tmp/bios.verify
BURN_IMAGE=/tmp/image-bios
sha256_image="FFFF"
sha256_file="EEEE"

echo "running" > $OUT

if [ -e $IMAGE_FILE ] && [ -e $SIG_FILE ]; 
then
    sha256_image=`sha256sum "$IMAGE_FILE" | awk '{print $1}'`
    sha256_file=`awk '{print $1}' $SIG_FILE`
fi

if [ $sha256_image != $sha256_file ]; 
then
    echo "BIOS image verify fail."
    echo "failed" > $OUT
    sleep 5
    rm -f $OUT
    rm -f $IMAGE_FILE 
    echo "Remove Bios image"
else
    echo "BIOS image verify ok."
    mv $IMAGE_FILE $BURN_IMAGE
    echo "success" > $OUT   
    sleep 5
    rm -f $OUT    
fi