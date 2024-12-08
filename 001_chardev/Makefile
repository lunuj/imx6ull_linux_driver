# KERNELDIR := /lib/modules/5.15.0-124-generic/build
KERNELDIR := ~/project/linux
CURRENT_PATH := $(shell pwd)
obj-m := chardev.o

ROOTFS 	:=	~/project/share/rootfs

ARCH			:=arm
CROSS_COMPILE	:=arm-linux-gnueabihf-

export ARCH
export CROSS_COMPILE

build: kernel_modules
	make rmnode
	make unloadmodule
	make kernel_modules
	make cp
	make loadmodule
	make node

cp:
# sudo cp ./chardev.ko /lib/modules/5.15.0-124-generic
	sudo cp ./chardev.ko $(ROOTFS)/lib/modules/4.1.15


kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -c $(KERNELDIR) M=$(CURRENT_PATH) clean

testApp:
	$(CROSS_COMPILE)gcc -o testApp.out testApp.c
test:
	./testApp.out $(ROOTFS)/dev/chardev

printf:
	sudo dmesg >&0
loadmodule:
	sudo insmod chardev.ko
unloadmodule:
	sudo rmmod chardev
node:
	sudo mknod $(ROOTFS)/dev/chardev c 200 0
	sudo chmod 777 $(ROOTFS)/dev/chardev
rmnode:
	sudo rm $(ROOTFS)/dev/chardev