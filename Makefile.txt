obj-m += driver.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	sudo gcc app.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	sudo rm a.out
