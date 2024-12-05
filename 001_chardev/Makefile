KERNELDIR := /lib/modules/5.15.0-124-generic/build
CURRENT_PATH := $(shell pwd)
obj-m := chardev.o

build: kernel_modules
	make rmnode
	make unloadmodule
	make kernel_modules
	make cp
	make loadmodule
	make node

cp:
	sudo cp ./chardev.ko /lib/modules/5.15.0-124-generic

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -c $(KERNELDIR) M=$(CURRENT_PATH) clean

testApp:
	gcc -o testApp.out testApp.c
test:
	./testApp.out /dev/chardev

printf:
	sudo dmesg >&0
loadmodule:
	sudo insmod chardev.ko
unloadmodule:
	sudo rmmod chardev
node:
	sudo mknod /dev/chardev c 200 0
	sudo chmod 777 /dev/chardev
rmnode:
	sudo rm /dev/chardev