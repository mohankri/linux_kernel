#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>

 /*
  * msleep: sleep in msec
  * msleep_interruptible: time in msec.
  * ssleep: sleep in second.
  */

typedef struct tasklet_data {
	int cnt;
} tasklet_data_t;

typedef struct timer_data {
	int cnt;
	int delay;
	struct timer_list exp_timer;
} tdata_t;

typedef struct timer_rescan {
	struct task_struct *task;
	tdata_t	timer;
	tasklet_data_t	tasklet_data;
} timer_rescan_t;

timer_rescan_t timer_info;


/*
 * Kernel Timers: Timer functions do run while the process that
 * 	registered them is executing on the same cpu.
 * They run asynchronously. It runs as software interrupts.
 * Timer function always run the same CPU that register/re-register it
 * for better cache locality. Timer being asynchronous any data structures
 * need to be protected from concurrent access by atomic variable or spinlock.
 */

/*
 * Tasklet resembles kernel timer is someways and always run on CPU that schedules
 * them. Unlike timer you can't ask kernel to execute tasklet at specified time but
 * ask to execute at later time chosen by Kernel. This behaviour is especially useful
 * with interrupt handler. It's executed in context of a soft interrupt.
 */

int
schedule_timer(void *data)
{
	unsigned long after;
	unsigned long start, end;
	//printk("HZ(1 second will have this many ticks) = %d \n", HZ);
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(5 * HZ);
		//printk("Jiffies = %lu\n", jiffies);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(1+HZ);
		//printk("1 Second sleep  = %lu\n", jiffies);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(1+ HZ/2);
		//printk("1/2 Second sleep  = %lu\n", jiffies);

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(1+ HZ/2);
		//printk("1/2 Second sleep(uninterruptible  = %lu\n", jiffies);
		after = 10 * HZ;
		/* Timestamp Counter */
		rdtscl(start);
		if (time_after(jiffies, after)) {
			//printk("time_after returns true ..\n");
			;
		} else {
			printk("time_after returns false may be overflow..\n");
		}
		rdtscl(end);
		//printk("time lapse ...is %li\n", end-start);
	}
	return (0);
}

static void
my_timer_fn(unsigned long data)
{
	tdata_t	*ptr = (tdata_t *)data;
	if (--ptr->cnt) {
		ptr->exp_timer.expires += ptr->delay * HZ;
		add_timer(&ptr->exp_timer);
	} else {
		printk("wake_up_interruptible if required here.\n");
	}
}

static void
timer_tasklet(unsigned long data)
{
	printk("Tasklet Schedule...\n");
}

DECLARE_TASKLET(my_tasklet, timer_tasklet, (unsigned long) &timer_info.tasklet_data);
/*
 * kthread_create or kthread_run.
 * if kthread_run is used you don't have to call wake_up_process.
 */
int
timer_init_module(void)
{
	int err = 0;

	init_timer(&timer_info.timer.exp_timer);
	timer_info.timer.cnt = 10;
	timer_info.timer.delay = 2;
	timer_info.timer.exp_timer.data = (unsigned long)&timer_info.timer;
	timer_info.timer.exp_timer.function = my_timer_fn;
	timer_info.timer.exp_timer.expires = jiffies + timer_info.timer.delay*HZ;

	add_timer(&timer_info.timer.exp_timer);

	tasklet_schedule(&my_tasklet);
		
	timer_info.task = kthread_create(&schedule_timer, NULL, "timer");
	if (IS_ERR(timer_info.task)) {
		err = PTR_ERR(timer_info.task);
		timer_info.task = NULL;
		return err; 
	}
	wake_up_process(timer_info.task);
	return (err);
}

void
timer_exit_module(void)
{
	tasklet_kill(&my_tasklet);
	del_timer(&timer_info.timer.exp_timer);
	if (timer_info.task) {
		kthread_stop(timer_info.task);
	}
}

module_init(timer_init_module);
module_exit(timer_exit_module);
MODULE_AUTHOR("Krishna Mohan");
MODULE_LICENSE("GPL");
