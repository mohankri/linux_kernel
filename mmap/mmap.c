#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  
 
#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#define LEN (64*1024)

static int *kmalloc_ptr = NULL;
/* pointer to page aligned area */
static int *kmalloc_area = NULL; 

struct dentry  *file;
 
struct mmap_info
{
    char *data;            
    int reference;      
};
 
void mmap_open(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference++;
	printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);
}
 
void mmap_close(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference--;
	printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    struct page *page;
    struct mmap_info *info;    
	printk(KERN_INFO "%d %s\n", __LINE__, __FUNCTION__);
     
    info = (struct mmap_info *)vma->vm_private_data;
    if (!info->data)
    {
        printk("No data\n");
        return 0;    
    }
     
    page = virt_to_page(info->data);    
     
    get_page(page);
    vmf->page = page;            
     
    return 0;
}
 
struct vm_operations_struct mmap_vm_ops =
{
    .open =     mmap_open,
    .close =    mmap_close,
    .fault =    mmap_fault,    
};
 
int op_mmap(struct file *filp, struct vm_area_struct *vma)
{
#ifdef PRIVATE_PAGE_HANDLER
    vma->vm_ops = &mmap_vm_ops;
    vma->vm_flags |= VM_RESERVED;    
    vma->vm_private_data = filp->private_data;
	printk(KERN_INFO " MMAP CAlled ...\n");
    mmap_open(vma);
    return 0;
#endif

    unsigned long offset = vma->vm_pgoff<<PAGE_SHIFT;
    unsigned long size = vma->vm_end - vma->vm_start;

    printk(KERN_INFO " New One MMAP CAlled ...\n");

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
 
int mmapfop_close(struct inode *inode, struct file *filp)
{
#ifdef PRIVATE_PAGE_HANDLER
    struct mmap_info *info = filp->private_data;
     
    free_page((unsigned long)info->data);
    kfree(info);
#else
    void *info =  filp->private_data;
    kfree(info);
#endif
    filp->private_data = NULL;
    return 0;
}
 
int mmapfop_open(struct inode *inode, struct file *filp)
{
#ifdef PRIVATE_PAGE_HANDLER
    struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
   
    info->data = (char *)get_zeroed_page(GFP_KERNEL);
    memcpy(info->data, "hello from kernel this is file: ", 32);
    //memcpy(info->data + 32, filp->f_path.dentry->name, strlen(filp->f_dentry->d_name.name));
    filp->private_data = info;
#else
    char myString[] = "Hello Samiksha Message from Kernel";
    kmalloc_ptr=kmalloc(LEN+2*PAGE_SIZE, GFP_KERNEL);
    kmalloc_area=(int *)(((unsigned long)kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);
    memcpy(kmalloc_area, myString, strlen(myString));

    /* assign this info struct to the file */
    filp->private_data = kmalloc_area;
    printk(KERN_INFO "Allocate kmalloc %p\n", kmalloc_area);
#endif

    return 0;
}
 
static const struct file_operations mmap_fops = {
    .open = mmapfop_open,
    .release = mmapfop_close,
    .mmap = op_mmap,
};
 
static int __init mmapexample_module_init(void)
{
    file = debugfs_create_file("mmap_example", 0644, NULL, NULL, &mmap_fops);
    return 0;
}
 
static void __exit mmapexample_module_exit(void)
{
    debugfs_remove(file);
}
 
module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");
