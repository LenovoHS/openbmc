#"Copyright ? 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."

OBMC_IMAGE_EXTRA_INSTALL_append_hr855xg2 = " phosphor-ipmi-blobs \
                                             phosphor-ipmi-flash \
                                             lenovo-pwr-sequence \
                                             fpga \
                                             id-button \
                                             lenovo-ipmi-oem \
                                             frus \
                                             obmc-phosphor-buttons-signals \
                                             obmc-phosphor-buttons-handler \
                                             obmc-phosphor-buttons \
                                             fpga-version \
                                             lenovo-bmc-update \
                                             lenovo-bios-update \
                                             lenovo-fan-service \
                                             lenovo-uart-switch \
                                             phosphor-gpio-monitor \
                                           "
                                           
#IMAGE_FEATURES_remove_hr855xg2 = " ssh-server-dropbear"




