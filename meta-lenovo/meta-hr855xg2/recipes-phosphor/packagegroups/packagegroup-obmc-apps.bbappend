# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details."

RDEPENDS_${PN}-extras_remove_hr855xg2 = " bmcweb obmc-ikvm phosphor-nslcd-cert-config phosphor-nslcd-authority-cert-config"
RDEPENDS_${PN}-user-mgmt_remove_hr855xg2 = " nss-pam-ldapd phosphor-ldap"
RDEPENDS_${PN}-fan-control_remove_hr855xg2 = " ${VIRTUAL-RUNTIME_obmc-fan-control} phosphor-fan-monitor"
RDEPENDS_${PN}-inventory_remove_hr855xg2 = " ${VIRTUAL-RUNTIME_obmc-fan-presence}"
