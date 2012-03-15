#
# Driver for mcbsp-serial
#
# Copyright (C) 2012 Galilsoft.
#
# Authors:
#	Kuzminov Dmitry	<dima@galilsoft.com>
#
#

obj-m += mcbsp_driver.o
mcbsp_driver-y = mcbsp_drv.o mcbsp_netif.o


all:
	$(MAKE) -C $(KERNEL) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL) M=$(PWD) clean
