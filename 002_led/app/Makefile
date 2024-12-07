CURRENT_PATH 	:= $(shell pwd)
MODULES_SRC		:= $(CURRENT_PATH)

build: app

app: $(APP_NAME).out 

$(APP_NAME).out: $(APP_NAME).c
	$(CC) -o $@ $< 
clean:
	rm $(APP_NAME).out