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

#define NUM_ALLOC	40

static int *kmalloc_ptr[NUM_ALLOC] = {NULL};
static int *kmalloc_area = NULL; 
struct dentry  *file;
struct file *dumpfile = NULL;
 
struct mmap_info {
  char *data;
  int reference;      
};
 
void mdrv_pghdlr_open(struct vm_area_struct *vma)
{
  struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
  info->reference++;
  printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);
}
 
void mdrv_pghdlr_close(struct vm_area_struct *vma)
{
  struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
  info->reference--;
  printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);
}
 
static int mdrv_pghdlr_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
  struct page *page;
  struct mmap_info *info;    
  printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);

  info = (struct mmap_info *)vma->vm_private_data;

  if (!info->data) {
    printk("No data\n");
    return 0;
  }
  page = virt_to_page(info->data);
  get_page(page);
  vmf->page = page;
  return 0;
}
 
struct vm_operations_struct mdrv_pghdlr_ops = {
  .open =     mdrv_pghdlr_open,
  .close =    mdrv_pghdlr_close,
  .fault =    mdrv_pghdlr_fault,    
};

int mdrv_mmap_pages(struct file *filep, struct vm_area_struct *vma)
{
    unsigned int size = (vma->vm_end - vma->vm_start);
    unsigned addr;
    int i;
    int rc = 0;
    printk(KERN_INFO "mmap multi pages  ...%ld\n", size);
    for (i = 0; i < NUM_ALLOC; i++) {
        addr = vma->vm_start + (i*LEN);
        if (vm_insert_page(vma, addr, kmalloc_ptr[i]) < 0) {
            printk("Failed to insert pages...\n");
            rc = -1;
            break;
        }
    }
    return rc; 
}
 
int mdrv_mmap(struct file *filp, struct vm_area_struct *vma)
{
#ifdef PRIVATE_PAGE_HANDLER
  vma->vm_ops = &mdrv_pghdlr_ops,
  vma->vm_flags |= VM_RESERVED;
  vma->vm_private_data = filp->private_data;
  printk(KERN_INFO " MMAP CAlled ...\n");
  mdrv_pghdlr_open(vma);
  return 0;
#endif

  unsigned long offset = vma->vm_pgoff<<PAGE_SHIFT;
  unsigned long size = vma->vm_end - vma->vm_start;


  if (offset & ~PAGE_MASK) {
    printk("offset not aligned: %ld\n", offset);
    return -ENXIO;
  }

  if (size>LEN) {
    printk("size too big\n");
    return(-ENXIO);
  }

  /* we only support shared mappings. Copy on write mappings are
     rejected here. A shared mapping that is writeable must have the
     shared flag set.
   */

   if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
      printk("writeable mappings must be shared, rejecting\n");
      return(-EINVAL);
   }

   /* we do not want to have this area swapped out, lock it */
   vma->vm_flags |= VM_LOCKED;

    /* method 2: enter pages into mapping of application */
   if (remap_pfn_range(vma, vma->vm_start,
         virt_to_phys((void*)kmalloc_area) >> PAGE_SHIFT,
         size, vma->vm_page_prot) < 0) {
      printk("remap page range failed\n");
      return -ENXIO;
    }
    return 0;
}
 
int mdrv_close(struct inode *inode, struct file *filp)
{
   int i;
#ifdef PRIVATE_PAGE_HANDLER
   struct mmap_info *info = filp->private_data;
   free_page((unsigned long)info->data);
   kfree(info);
#else
   for (i = 0; i < NUM_ALLOC; i++) { 
       printk("%d) Free allocated %p\n", i, kmalloc_ptr[i]);
       kfree(kmalloc_ptr[i]);
   }
#endif
   filp->private_data = NULL;
   return 0;
}
 
int mdrv_open(struct inode *inode, struct file *filp)
{
  mm_segment_t    oldfs;
  loff_t  pos = 0;
  int ret, i;

#ifdef PRIVATE_PAGE_HANDLER
  struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL); 
  info->data = (char *)get_zeroed_page(GFP_KERNEL);
  memcpy(info->data, "hello from kernel this is file: ", 32);
  //memcpy(info->data + 32, filp->f_path.dentry->name, strlen(filp->f_dentry->d_name.name));
  filp->private_data = info;
#else
  char myString[] = "Hello Samiksha Message from vfs_write";

  for (i = 0; i < NUM_ALLOC; i++) { 
      kmalloc_ptr[i]=kmalloc(LEN, GFP_KERNEL);
      if (kmalloc_ptr[i] == NULL) {
          printk(KERN_INFO "Allocate failed %d\n", i);
      } else {
          printk("Allocated %p\n", kmalloc_ptr[i]);
      }
  }
  kmalloc_area = kmalloc_ptr[0];
  //kmalloc_area=(int *)(((unsigned long)kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);
  memcpy(kmalloc_area, myString, strlen(myString));

  printk(KERN_INFO "Allocate kmalloc %p\n", kmalloc_area);

#endif

#ifdef WRITE_FILE
  oldfs   = get_fs();
  set_fs(get_ds());

  spin_lock(&dumpfile->f_lock);
  dumpfile->f_pos = pos;
  spin_unlock(&dumpfile->f_lock);

  ret = vfs_write(dumpfile, (unsigned char *)kmalloc_ptr,
						LEN+2*PAGE_SIZE, &pos);
  if (ret < 0) {
  	printk(KERN_INFO "VFS write failed %p\n", kmalloc_ptr);
  }
  set_fs(oldfs); 
#endif

  return 0;
}


ssize_t
mdrv_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
  /* Transfering data to user space */
  int retval = copy_to_user(buf,kmalloc_ptr,count);
  printk("copy_to_user returned (%d)", retval);
  return count;
}
 
static const struct file_operations mdrv_fops = {
  .open = mdrv_open,
  .release = mdrv_close,
  .mmap = mdrv_mmap,
  .read = mdrv_read,
};
 
static int __init mdrv_init(void)
{
  mm_segment_t    oldfs;
  file = debugfs_create_file("mdriver", 0644, NULL, NULL, &mdrv_fops);
#ifdef WRITE_FILE
  oldfs   = get_fs();
  set_fs(get_ds());
  dumpfile = filp_open("/tmp/mmap.bin", O_CREAT|O_RDWR,
					S_IRWXU|S_IRWXG|S_IRWXO);
  set_fs(oldfs);
#endif
  //return (filp);
  return 0;
}
 
static void __exit mdrv_exit(void)
{
  debugfs_remove(file);
#ifdef WRITE_FILE 
  filp_close(dumpfile, NULL);
#endif
}
 
module_init(mdrv_init);
module_exit(mdrv_exit);
MODULE_LICENSE("GPL");
