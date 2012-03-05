#
# Copyright (C) 2012 Galilsoft.
#
# Authors:
#	Kuzminov Dmitry	<dima@galilsoft.com>
#
export PROJECT=$(PWD)
export LINUX=$(PROJECT)/linux-omap-2.6
export KERNEL=$(ROJECT)/$(LINUX)
export FILESYSTEM=$(PROJECT)/fs
export DEFAULT_FILESYSTEM=$(PROJECT)/default_fs
export INSTALL_MOD_PATH=$(FILESYSTEM)
export CROSS_COMPILE=/opt/codesourcery/arm-2009q1/bin/arm-none-linux-gnueabi-
export ARCH=arm
export MCBSP=$(PROJECT)/mcbsp
export MCBSP_FLAT=$(PROJECT)/../mcbsp_flat
export PATH:=$(PROJECT)/u-boot-main/tools:$(PATH)

BUSYBOX=busybox-1.19.3
KERNEL_VERSION=`cd $(LINUX); make -s kernelversion`

all: kerneloldconfig kernel modules external_modules busybox
	 
remove_all:
	rm -fr $(FILESYSTEM) $(DEFAULT_FILESYSTEM) $(BUSYBOX) $(LINUX)

clean: 
	cd $(LINUX); make distclean
	cd $(LINUX); make clean
	cd $(MCBSP); make clean
	cd $(BUSYBOX); make clean

kerneloldconfig: $(LINUX).tar.bz2
	tar xf $<
	cp kernel.config $(LINUX)/.config
	cd $(LINUX); make oldconfig

config:
	cd $(LINUX); make menuconfig

kernel:
	cd $(LINUX); make uImage -j 4

kernel_info:
	cd $(LINUX); make kernelversion
	cd $(LINUX); make kernelrelease

modules:
	cd $(LINUX); make modules

external_modules:
	cd $(MCBSP); make

modules_install:
	mkdir -p $(FILESYSTEM)/fs 
	cd $(LINUX); make modules_install

busybox: $(BUSYBOX).tar.bz2
	tar xvf $<
	cp -a busybox.config $(BUSYBOX)/.config
	cd $(BUSYBOX); make oldconfig
	cd $(BUSYBOX); make

busybox_install:
	mkdir -p $(FILESYSTEM)/fs 
	cd $(BUSYBOX); make CONFIG_PREFIX=$(FILESYSTEM) install

create-fs: $(DEFAULT_FILESYSTEM).tar.bz2
	rm -fr $(FILESYSTEM)
	tar xf $<
	mv $(DEFAULT_FILESYSTEM) $(FILESYSTEM)
	cd $(BUSYBOX); make CONFIG_PREFIX=$(FILESYSTEM) install
	cd $(LINUX); make modules_install
	cp -a $(MCBSP)/mcbsp_driver.ko $(FILESYSTEM)/lib/modules/$(KERNEL_VERSION)/kernel/drivers/
	depmod -a -b $(FILESYSTEM) -F $(LINUX)/System.map $(KERNEL_VERSION)
	sed -i "s;kernel;/lib/modules/$(KERNEL_VERSION)/kernel;g" $(FILESYSTEM)/lib/modules/$(KERNEL_VERSION)/modules.dep
