CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)
obj-m 			:= $(MODULES_NAME).o

export KERNELDIR

export ARCH
export CROSS_COMPILE

export MODULES_NAME
export DEVICE_NAME
export DRIVER_NAME
export APP_NAME

build: modules

modules:
	$(eval obj-m:=$(DEVICE_NAME).o)
	echo $(obj-m)
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	$(eval obj-m:=$(DRIVER_NAME).o)
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) clean -C $(KERNELDIR) M=$(CURRENT_PATH)