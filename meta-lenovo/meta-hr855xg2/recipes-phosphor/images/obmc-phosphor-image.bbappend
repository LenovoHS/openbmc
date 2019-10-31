# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

OBMC_IMAGE_EXTRA_INSTALL_append_hr855xg2 = " lenovo-pwr-sequence \
                                             fpga \
                                             id-button \
                                             pwr-button \
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
                                           "
                                           
#IMAGE_FEATURES_remove_hr855xg2 = " ssh-server-dropbear"




