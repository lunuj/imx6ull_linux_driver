CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)
obj-m 			:= $(MODULES_NAME).o

export KERNELDIR

export ARCH
export CROSS_COMPILE

export MODULES_NAME
export APP_NAME

build: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) clean -C $(KERNELDIR) M=$(CURRENT_PATH)