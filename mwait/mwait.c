#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/mwait.h>
/* 
 * The current code can be written with following code segment as well.
 * snippet from mwait-sysfs.c
 *
 * mwait_lock_rescan_remove()
 * 	while ((b = mwait_find_next_bus(b)) != NULL) {
 *		mwait_rescan_bus(b);
 *      }
 * mwait_unlock_rescan_remove();
 */
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
typedef struct mwait_monitor {
	struct task_struct *producer;
	struct task_struct *consumer;
	int global;
} mwait_monitor_t;

#define PCI_SCAN_FILE	"/sys/bus/mwait/rescan"

mwait_monitor_t scan_info;	

int
perform_monitor_scan(void *data)
{
	int global = *(int *)data;
	printk(KERN_INFO "Global is %d\n", global);
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(5 * HZ);	
	}
	return (0);
}

int
perform_mwait_scan(void *data)
{
	int global = *(int *)data;
	printk(KERN_INFO "Not working @ the moment %d\n", global);
	__monitor((void *)&scan_info.global, 0, 0);
	while (!kthread_should_stop()) {
		__mwait(0, 0);
		//printk(KERN_INFO "mwait is %d\n", global);
		
	}
	return (0);
}

/*
 * kthread_create or kthread_run.
 * if kthread_run is used you don't have to call wake_up_process.
 */
int
mwait_init_module(void)
{
	int err = 0;
	scan_info.global = 2;
	//__monitor((void *)&scan_info.global, 0, 0);
	scan_info.consumer = kthread_create(&perform_monitor_scan, (void *)&scan_info.global, "monitor");
	if (IS_ERR(scan_info.consumer)) {
		err = PTR_ERR(scan_info.consumer);
		scan_info.consumer= NULL;
		return err; 
	}
	wake_up_process(scan_info.consumer);

	scan_info.producer = kthread_create(&perform_mwait_scan, (void *)&scan_info.global, "mwait");
	if (IS_ERR(scan_info.producer)) {
		err = PTR_ERR(scan_info.producer);
		scan_info.producer = NULL;
		return err; 
	}
	wake_up_process(scan_info.producer);

	return (err);
}

void
mwait_exit_module(void)
{
	if (scan_info.consumer) {
		kthread_stop(scan_info.consumer);
	}
	if (scan_info.producer) {
		kthread_stop(scan_info.producer);
	}
}

module_init(mwait_init_module);
module_exit(mwait_exit_module);
MODULE_AUTHOR("Krishna Mohan");
MODULE_LICENSE("GPL");
