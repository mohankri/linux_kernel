obj-m += mdriver.o
EXTRA_CFLAGS += -g -DDEBUG
KDIR = /lib/modules/`uname -r`/build
all: mdrvtest
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm mdrvtest

install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

mdrv: mdrvtest
	gcc -g -o mdrvtest mdrvtest.c
