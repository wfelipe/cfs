obj-$(CONFIG_CFS)    := cfs.o

cfs-y	:= inode.o compression.o super.o

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) CONFIG_CFS=m modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) CONFIG_CFS=m clean

test:
	sudo insmod cfs.ko
	sudo mount -t cfs /dev/debian/cfs /mnt
untest:
	sudo umount /mnt
	sudo rmmod cfs
