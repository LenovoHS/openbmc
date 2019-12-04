# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

OBMC_IMAGE_EXTRA_INSTALL_append_hr855xg2 = " lenovo-pwr-sequence \
                                             fpga \
                                             id-button \
                                             pwr-button \
                                             cpu-caterr \
                                             int-fpga-bmc \
                                             cpu-vr-bmc-alert-n \
                                             ddr-vr-bmc-alert-n \
                                             fm-pvccin-cpu1-pwr-in-alert-n \
                                             fm-pvccin-cpu2-pwr-in-alert-n \
                                             fm-pvccin-cpu3-pwr-in-alert-n \
                                             fm-pvccin-cpu4-pwr-in-alert-n \
                                             p12v-aux1-alert1-n \
                                             p12v-aux2-alert1-n \
                                             p12v-aux3-alert1-n \
                                             pdb-alt-n \
                                             riser1-vr-al-r \
                                             riser2-vr-al-r \
                                             riser3-vr-al-r \
                                             lenovo-ipmi-oem \
                                             lenovo-frus \
                                             fpga-version \
                                             hr855xg2-fpga-update \
                                             hr855xg2-bmc-update \
                                             hr855xg2-bios-update \
                                             lenovo-fan-service \
                                             lenovo-uart-switch \
                                             uart-routing \
                                             lenovo-resetpsu \
                                             lenovo-led-manager \
                                             lenovo-sensors \
                                             lenovo-acpi-state \
                                             usb-lan \
                                             hr855xg2-fpga-util \
                                           "
                                           
#IMAGE_FEATURES_remove_hr855xg2 = " ssh-server-dropbear"




