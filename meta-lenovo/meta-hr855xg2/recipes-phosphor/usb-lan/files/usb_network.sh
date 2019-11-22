#!/bin/bash

# Adapted from upstream script:
# https://github.com/openbmc/meta-quanta/blob/41ff54605a8df6e456e2035b410114545bc94284/meta-gsj/recipes-phosphor/usb-network/files/usb_network.sh

function start() {
  if [[ ! -d g1 ]]; then
    mkdir g1
  fi

  cd g1

  # TODO: Update VID and PID
  echo 0x1209 > idVendor
  echo 0x1234 > idProduct

  if [[ ! -d strings/0x409 ]]; then
    mkdir -p strings/0x409
  fi
  echo "My Name" > strings/0x409/manufacturer
  echo "My Product" > strings/0x409/product

  if [[ ! -d configs/c.1 ]]; then
    mkdir -p configs/c.1
  fi
  echo 100 > configs/c.1/MaxPower
  if [[ ! -d configs/c.1/strings/0x409 ]]; then
    mkdir -p configs/c.1/strings/0x409
  fi
  echo "ECM" > configs/c.1/strings/0x409/configuration

  if [[ ! -d functions/ecm.usb0 ]]; then
    mkdir -p functions/ecm.usb0
  fi

  if [[ ! -L configs/c.1/ecm.usb0 ]]; then
    # TODO:
    # MAC addresses generated at random using
    # https://www.browserling.com/tools/random-mac
    echo a2:e9:fa:86:25:ac > functions/ecm.usb0/dev_addr
    echo a8:4a:04:e8:09:96 > functions/ecm.usb0/host_addr

    ln -s functions/ecm.usb0 configs/c.1
  fi

  if [[ -z "$(cat UDC)" ]]; then
    echo "1e6a0000.usb-vhub:p1" > UDC
  fi
}

function rmdir_if_exists {
  if [[ -d "$1" ]]; then
    rmdir "$1"
  fi
}

function stop() {
  if [[ -d g1 ]]; then
    cd g1
    rm -f configs/c.1/ecm.usb0
    rmdir_if_exists configs/c.1/strings/0x409
    rmdir_if_exists configs/c.1
    rmdir_if_exists strings/0x409
    rmdir_if_exists functions/ecm.usb0
    cd ..
    rmdir g1
  fi
}

cd /sys/kernel/config/usb_gadget
if [[ "$1" == "stop" ]]; then
  stop
else
  start
fi
