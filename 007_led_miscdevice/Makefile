KERNELDIR 		:= ~/project/linux
ROOTFS 			:=	~/project/share/rootfs
CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)/module
APP_SRC			:= $(CURRENT_PATH)/app

MODULES_NAME	:= ledmisc
APP_NAME		:= $(MODULES_NAME)_test

ARCH			:=arm
CROSS_COMPILE	:=arm-linux-gnueabihf-
CC				:= $(CROSS_COMPILE)gcc

export KERNELDIR

export ARCH
export CROSS_COMPILE

export MODULES_NAME
export APP_NAME

export CC

build: modules app

cp:
	sudo cp $(MODULES_SRC)/$(MODULES_NAME).ko $(ROOTFS)/lib/modules/4.1.15/

modules:
	$(MAKE) -C $(MODULES_SRC) modules

app:
	$(MAKE) -C $(APP_SRC)


.PHONY: modules_clean app_clean clean modules app printf loadmodule unloadmodule node rmnode
clean: modules_clean app_clean

modules_clean:
	$(MAKE) -C $(MODULES_SRC) clean 

app_clean:
	$(MAKE) -C $(APP_SRC) clean 

printf:
	sudo dmesg >&0
loadmodule:
	sudo insmod $(MODULES_NAME).ko
unloadmodule:
	sudo rmmod $(MODULES_NAME)
node:
	sudo mknod $(ROOTFS)/dev/$(MODULES_NAME) c 200 0
	sudo chmod 777 $(ROOTFS)/dev/$(MODULES_NAME)
rmnode:
	sudo rm $(ROOTFS)/dev/$(MODULES_NAME)