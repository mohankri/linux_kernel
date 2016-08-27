#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>

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
} pci_rescan_t;

pci_rescan_t scan_info;	

int
perform_pci_rescan(void *data)
{
	while (!kthread_should_stop()) {
		ssleep(5);	
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
	kthread_stop(scan_info.task);
}

module_init(pci_init_module);
module_exit(pci_exit_module);
