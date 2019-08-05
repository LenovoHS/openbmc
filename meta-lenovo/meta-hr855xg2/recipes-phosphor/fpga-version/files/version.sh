#!/bin/sh


SERVICE="org.openbmc.control.fpga"
OBJECT="/org/openbmc/control/fpga"
INTERFACE="org.openbmc.control.fpga"
READ_METHOD="read_fpga"


major=$(busctl call $SERVICE $OBJECT $INTERFACE $READ_METHOD yy 0 1 | awk '{print $2}')
minor=$(busctl call $SERVICE $OBJECT $INTERFACE $READ_METHOD yy 0 2 | awk '{print $2}')
sub1=$(busctl call $SERVICE $OBJECT $INTERFACE $READ_METHOD yy 0 3 | awk '{print $2}')
sub2=0

cd /var
if [ -e /var/cpld0.version ]; then
	mv cpld0.version /run
fi
cd /run

if [ $major != " " -a $minor != " " -a $sub1 != " " ]; then
version=$major.$minor.$sub1.$sub2
echo $version >cpld0.version
fi
