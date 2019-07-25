#"Copyright (c) 2019-present Lenovo"

SUMMARY = "YAML configuration for hr650x+"
PR = "r1"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${LENOVOBASE}/COPYING.BSD;md5=efc72ac5d37ea632ccf0001f56126210"

inherit allarch

SRC_URI_append_hr650x+ = "file://hr650x+-ipmi-fru.yaml \
                           file://hr650x+-ipmi-fru-properties.yaml \
                           file://hr650x+-ipmi-sensors.yaml \
                           file://hr650x+-leds.yaml \
                           file://hr650x+-ipmi-channel.yaml \
                           file://hr650x+-defaults.yaml \
                          "

S = "${WORKDIR}"

do_install() {
    install -m 0644 -D hr650x+-ipmi-fru-properties.yaml \
        ${D}${datadir}/${BPN}/ipmi-extra-properties.yaml
    install -m 0644 -D hr650x+-ipmi-fru.yaml \
        ${D}${datadir}/${BPN}/ipmi-fru-read.yaml
    install -m 0644 -D hr650x+-ipmi-sensors.yaml \
        ${D}${datadir}/${BPN}/ipmi-sensors.yaml
    install -m 0644 -D hr650x+-leds.yaml \
        ${D}${datadir}/${BPN}/led.yaml
    install -m 0644 -D hr650x+-ipmi-channel.yaml \
        ${D}${datadir}/${BPN}/ipmi-channel.yaml
    install -m 0644 -D hr650x+-defaults.yaml \
	${D}${datadir}/${BPN}/defaults.yaml

}

FILES_${PN}-dev = " \
    ${datadir}/${BPN}/ipmi-extra-properties.yaml \
    ${datadir}/${BPN}/ipmi-fru-read.yaml \
    ${datadir}/${BPN}/ipmi-sensors.yaml \
    ${datadir}/${BPN}/led.yaml \
    ${datadir}/${BPN}/ipmi-channel.yaml \
    ${datadir}/${BPN}/defaults.yaml \
    "

ALLOW_EMPTY_${PN} = "1"
