#!/usr/bin/env bash

set -ex

make clean
make
sudo rmmod system76_coreboot || true
sudo insmod system76-coreboot.ko
dmesg -w
