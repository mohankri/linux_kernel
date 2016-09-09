#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/mm.h>

#define	DEV_MAJOR	42
#define DEV_MINOR	0
#define DRIVER_NAME	"usermap"

static	struct	class *usermap_class;

static int usermap_open(struct inode *inode, struct file *file)
{
	printk("%s \n", __func__);
	return (0);
}

static int usermap_release(struct inode *inode, struct file *file)
{
	printk("%s \n", __func__);
	return (0);
}

static ssize_t usermap_write(struct file *file, const char __user *buf,
		size_t count, loff_t *off)
{
	struct page *page;
	char	*kaddr;
	int 	res;
	printk("%s \n", __func__);
	down_read(&current->mm->mmap_sem);
	res = get_user_pages(current, current->mm,
						(unsigned long)buf,
						1, 1, 1, &page, NULL); 
	if (res) {
		kaddr = kmap(page);
		printk("Data recevied %s\n", kaddr);
		strcpy(kaddr, "Data Acknowledged");	
		page_cache_release(page);
	}
	up_read(&current->mm->mmap_sem);
	return (0);
}

static struct file_operations usermap_ops = {
	.owner = THIS_MODULE,
	.open  = usermap_open,
	.release = usermap_release,
    .write = usermap_write
};

int
usermap_init_module(void)
{
	int ret;
	ret = register_chrdev(DEV_MAJOR, DRIVER_NAME, &usermap_ops);
	usermap_class = class_create(THIS_MODULE, DRIVER_NAME);
	device_create(usermap_class, NULL, MKDEV(DEV_MAJOR, DEV_MINOR),
													NULL, DRIVER_NAME);
	return ret;
}

void
usermap_exit_module(void)
{
	device_destroy(usermap_class, MKDEV(DEV_MAJOR, DEV_MINOR));
	class_destroy(usermap_class);
	unregister_chrdev(DEV_MAJOR, DRIVER_NAME);
}

module_init(usermap_init_module);
module_exit(usermap_exit_module);
MODULE_AUTHOR("Krishna Mohan");
MODULE_LICENSE("GPL");
