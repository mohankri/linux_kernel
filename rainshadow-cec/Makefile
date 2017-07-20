#sudo yum --enablerepo=elrepo-kernel install kernel-ml-deve
obj-m += raidshadow-cec.o
KDIR = /lib/modules/`uname -r`/build
all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
