#"Copyright ? 2019-present Lenovo
#Licensed under BSD-3, see COPYING.BSD file for details."


FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

EXTRA_OECONF += " --disable-link-local-autoconfiguration"
