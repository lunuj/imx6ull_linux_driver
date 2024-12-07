CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)
obj-m 			:= $(MODULES_NAME).o

build: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	rm -r .
clean:
	$(MAKE) clean -C $(KERNELDIR) M=$(CURRENT_PATH)