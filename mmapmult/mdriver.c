#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
 
#ifndef VM_RESERVED
#define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#define LEN (4*1024*1024)
//#define	ORDER	10
#define	ORDER	0
#define NUM_ALLOC	1

struct dentry *file;
struct file *dumpfile = NULL;

static struct page *page_ptr[NUM_ALLOC] = {NULL};
 
 
int
mdrv_mmap_pages(struct file *filep, struct vm_area_struct *vma)
{
    unsigned int size = (vma->vm_end - vma->vm_start);
    unsigned long addr;
    unsigned long pgbuf;
    int i;
    int rc = 0;
    struct page *pg;
    printk(KERN_INFO "mmap multi pages  ...%d\n", size);
    printk(KERN_INFO "vm_end %lu vm_start %lu\n", vma->vm_end, vma->vm_start);
    vma->vm_flags |= VM_LOCKED;
    vma->vm_flags |= VM_RESERVED;
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    for (i = 0; i < NUM_ALLOC; i++) {
        /*pg = alloc_page(GFP_KERNEL);  */
        pg = page_ptr[i];
        //pgbuf = (unsigned long)page_to_virt(pg);
        pgbuf = (unsigned long)__va(page_to_phys(pg));
       	pgbuf = 0xDEADBEEF;
        printk("Value Wrote...%lx \n", pgbuf);
        addr = vma->vm_start + (i*(PAGE_SIZE*ORDER));
        rc = vm_insert_page(vma, addr, pg);
        if (rc < 0) {
            printk("Failed to insert pages...%d \n", rc);
            break;
        } else {
	    printk("Insert Pages @ %d %p\n", i, pg);
            //__free_page(pg);
        }
    }
    return rc; 
}
 
int
mdrv_close(struct inode *inode, struct file *filp)
{
   int i;
   for (i = 0; i < NUM_ALLOC; i++) { 
       __free_pages(page_ptr[i], ORDER);
       //__free_page(page_ptr[i]);
   }
   filp->private_data = NULL;
   return 0;
}
 
int
mdrv_open(struct inode *inode, struct file *filp)
{
  mm_segment_t    oldfs;
  loff_t  pos = 0;
  int i;

  char myString[] = "Hello Samiksha Message from vfs_write";

  for (i = 0; i < NUM_ALLOC; i++) { 
      page_ptr[i]=alloc_pages(GFP_KERNEL, ORDER);
      //page_ptr[i]=alloc_page(GFP_KERNEL);
      if (page_ptr[i] == NULL) {
          printk(KERN_INFO "Allocate failed %d\n", i);
      } else {
          printk("Allocated page %p\n", page_ptr[i]);
      }
  }

  return 0;
}


ssize_t
mdrv_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
  return 0;
}
 
static const struct file_operations mdrv_fops = {
  .open = mdrv_open,
  .release = mdrv_close,
  .mmap = mdrv_mmap_pages,
  .read = mdrv_read,
};
 
static int __init mdrv_init(void)
{
  mm_segment_t    oldfs;
  file = debugfs_create_file("mdriver", 0644, NULL, NULL, &mdrv_fops);
  return 0;
}
 
static void __exit mdrv_exit(void)
{
  debugfs_remove(file);
}
 
module_init(mdrv_init);
module_exit(mdrv_exit);
MODULE_LICENSE("GPL");
