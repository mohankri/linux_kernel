#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
//#include <linux

/*
 * kthread_create: will create a new task_struct with no process address space
 * 		queue it in workqueue for scheduler to invoke it.
 * kernel_thread: forks a new process with distinct new task_struct.
 */

 /*
  * msleep: sleep in msec
  * msleep_interruptible: time in msec.
  * ssleep: sleep in second.
  */
typedef struct pci_rescan {
	struct task_struct *task;
	struct	file *pcifd;
} pci_rescan_t;

#define PCI_SCAN_FILE	"/sys/bus/pci/rescan"

pci_rescan_t scan_info;	

struct file *
pci_open(const char *path, int flags, int mode) 
{
	struct file *filp;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, mode);
	set_fs(oldfs);
	return filp;
}

int
pci_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t	old_fs;
	int ret;
	loff_t	pos = offset;
	old_fs = get_fs();
	set_fs(get_ds());
	spin_lock(&file->f_lock);
	file->f_pos = pos;
	spin_unlock(&file->f_lock);
	ret = vfs_write(file, data, size, &pos);
	// or ret = file->f_op->write(file, data, size, &pos);
	set_fs(old_fs);
	printk("pci write %d %d ret %d\n", *data, size, ret);
	return (ret);
}

int
perform_pci_rescan(void *data)
{
	char val[] = "1";
	//scan_info.pcifd = pci_open(PCI_SCAN_FILE, O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);
	scan_info.pcifd = pci_open(PCI_SCAN_FILE, O_WRONLY, 0777);
	if (IS_ERR(scan_info.pcifd)) {
		printk("failed to open %s %ld\n", PCI_SCAN_FILE, PTR_ERR(scan_info.pcifd));
		scan_info.task = NULL;
		do_exit(0);
		return (-1);
	}
	while (!kthread_should_stop()) {
		ssleep(5);
		pci_write(scan_info.pcifd, 0, val, strlen(val));
	}
	return (0);
}

/*
 * kthread_create or kthread_run.
 * if kthread_run is used you don't have to call wake_up_process.
 */
int
pci_init_module(void)
{
	int err = 0;
	scan_info.task = kthread_create(&perform_pci_rescan, NULL, "pci-rescan");
	if (IS_ERR(scan_info.task)) {
		err = PTR_ERR(scan_info.task);
		scan_info.task = NULL;
		return err; 
	}
	wake_up_process(scan_info.task);
	return (err);
}

void
pci_exit_module(void)
{
	if (!IS_ERR(scan_info.pcifd)) {
		filp_close(scan_info.pcifd, NULL);
	}
	//if (!IS_ERR(scan_info.task)) {
	if (scan_info.task) {
		kthread_stop(scan_info.task);
	}
}

module_init(pci_init_module);
module_exit(pci_exit_module);
MODULE_AUTHOR("Krishna Mohan");
MODULE_LICENSE("GPL");
