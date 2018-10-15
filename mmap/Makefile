obj-m += mdriver.o
KDIR = /lib/modules/`uname -r`/build
all: mdrvtest
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm mdrvtest

install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

mdrv: mdrvtest
	gcc -o mdrvtest mdrvtest.c
