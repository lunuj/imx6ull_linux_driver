CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)

build: app

app: $(APP_NAME).out 

reload: $(APP_NAME).out
	rmmod /dev/$(MODULES_NAME)
	modprobe $(MODULES_NAME)
	./$(APP_NAME).out /dev/$(MODULES_NAME)

$(APP_NAME).out: $(APP_NAME).c
	echo $(CC)
	$(CC) -o $@ $< 
clean:
	rm $(APP_NAME).out