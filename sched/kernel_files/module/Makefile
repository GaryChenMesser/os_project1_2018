MODULENAME = hello

obj-m += $(MODULENAME).o

all: module ap

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	 rm -f ap

ap: ap.c
	gcc -Wall -o ap ap.c
