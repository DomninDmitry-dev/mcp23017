#!/bin/bash

export MODNAME=mcp23017
export PREFIX=sun50i-h5-
export ARCH=arm64
export CROSS_COMPILE=${HOME}/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export KERNELDIR=${HOME}/MyProjects/MyServer/armbian/orange-pi-zero-plus-v5.4.85_x64/linux-mainline/orange-pi-5.4
export ADDR_BOARD=root@192.168.0.150
export MODULEDIR=/lib/modules/5.4.85-sunxi64/kernel/drivers/gpio
export DTSDIR=$PWD/dtsi-opi-zero
export DTC_FLAGS='--symbols'
export DTBDIR=/boot/dtb/allwinner/overlay
export PROGNAME=test-gpios

echo -e "\t MODNAME \t = ${MODNAME}"
echo -e "\t PREFIX \t = ${PREFIX}"
echo -e "\t ARCH \t\t = ${ARCH}"
echo -e "\t CROSS_COMPILE \t = ${CROSS_COMPILE}"
echo -e "\t BUILD_KERNEL \t = ${KERNELDIR}"
echo -e "\t IP_BOARD \t = ${ADDR_BOARD}"
echo -e "\t MODULE_DIR \t = ${MODULEDIR}"
echo -e "\t DTS_DIR \t = ${DTSDIR}"
echo -e "\t DTB_DIR \t = ${DTBDIR}"
echo -e "\t PROGNAME \t = ${PROGNAME}"

