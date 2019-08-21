# Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details.

RDEPENDS_${PN}-extras_remove_hr650x+ = " bmcweb \
                                         obmc-ikvm phosphor-nslcd-cert-config \
                                         phosphor-nslcd-authority-cert-config \
                                       "
RDEPENDS_${PN}-user-mgmt_remove_hr650x+ = " nss-pam-ldapd \
                                            phosphor-ldap \
                                          "
