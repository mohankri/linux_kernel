obj-m += irq_keyboard.o
KDIR = /lib/modules/`uname -r`/build
all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
