#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>

int unsigned long data;

static irqreturn_t
irq_keyboard(int irq, void *data)
{
	unsigned char status;
	static unsigned char scancode;;
	status = inb(0x64);
	scancode = inb(0x60);
	printk("IRQ Handled...status %d scancode %d\n", status, scancode);
	return IRQ_HANDLED;
}

/*
 * irq 16 is assigned to USB keyboard.
 * cat /proc/interrupts will show the interrupt count increasing.
 * modifying the irq number accordingly based on setup.
 */
int
my_init_module(void)
{
	int res;
//	free_irq(1, NULL);
	res = request_irq(16, irq_keyboard, IRQF_SHARED, "test_keyboard_irq", &data);
	printk("request_irq ...%d\n", res);
	return 0;
}

void
my_cleanup_module(void)
{
	free_irq(16, &data);
	printk("Cleanup World...\n");
}

module_init(my_init_module);
module_exit(my_cleanup_module);
//MODULE_LICENSE("GPL");
