KERNELDIR 		:= /home/lunuj/project/linux
CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)/module
APP_SRC			:= $(CURRENT_PATH)/app

MODULES_NAME	:= led
APP_NAME		:= $(MODULES_NAME)_test

ARCH			:=arm
CROSS_COMPILE	:=arm-unknown-linux-uclibcgnueabihf-
CC				:= $(CROSS_COMPILE)gcc

export KERNELDIR

export ARCH
export CROSS_COMPILE

export MODULES_NAME
export APP_NAME

build: modules app

modules:
	$(MAKE) -C $(MODULES_SRC)

app:
	$(MAKE) -C $(APP_SRC)


.PHONY: modules_clean app_clean clean modules app
clean: modules_clean app_clean

modules_clean:
	$(MAKE) -C $(MODULES_SRC) clean 

app_clean:
	$(MAKE) -C $(APP_SRC) clean 

	