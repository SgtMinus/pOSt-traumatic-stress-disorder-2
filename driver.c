#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/filter.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/types.h>


#define WR_VALUE _IOW('a','a',struct message*)
#define MAX_PCI_DEV_COUNT 64

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("AnimeLinuxEpicBattleScene");
MODULE_VERSION("1.0");

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

struct pci_dev_info {
    unsigned short  device[MAX_PCI_DEV_COUNT];
    unsigned short  vendor[MAX_PCI_DEV_COUNT];
    int pci_dev_count;
};

struct pg {
    bool status;
    unsigned long int virtualAddress;
    unsigned long int physicalAddress;
    int configPgtableLevels;
    long int pageSize;
    unsigned int flags;
    unsigned int refcount;

};

struct message {
    struct pci_dev_info* pdi;
    struct pg* page;
};

struct pci_dev* pci_dev;
struct pg* page;
struct message msg;

/*
** Function Prototypes
*/
static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

void fill_structs(void);

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}


/*
** This function will be called when we write IOCTL on the Device file
*/
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&msg ,(struct message*) arg, sizeof(msg)) )
                        {
                                pr_err("Data Write : Err!\n");
                        }
                        fill_structs();
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);

        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }

        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }

        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

void fill_structs() {
    int i = 0;
    while ((pci_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_dev))) {
        if (i >= MAX_PCI_DEV_COUNT) break;
        msg.pdi->device[i] = pci_dev->device;
        msg.pdi->vendor[i] = pci_dev->vendor;
        i++;
    }
    unsigned long vaddr = __get_free_page(GFP_KERNEL);
    if (vaddr == 0)
    {
        msg.page->status = false;
    }
    else {
        struct page* my_beloved_page;

        pgd_t *pgd;
        p4d_t *p4d;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;
        unsigned long paddr = 0;
        unsigned long page_addr = 0;
        unsigned long page_offset = 0;

        pgd = pgd_offset(current->mm, vaddr);
        pr_info("pgd_val = 0x%lx\n", pgd_val(*pgd));
        pr_info("pgd_index = %lu\n", pgd_index(vaddr));
        if (pgd_none(*pgd)) {
            pr_notice("Not mapped in pgd\n");
            return;
        }

        p4d = p4d_offset(pgd, vaddr);
        pr_info("p4d_val = 0x%lx\n", p4d_val(*p4d));
        pr_info("pgd_index = %lu\n", p4d_index(vaddr));
        if (p4d_none(*p4d) || p4d_bad(*p4d)) {
            pr_notice("Not mapped in p4d\n");
            return;
        }

        pud = pud_offset(p4d, vaddr);
        pr_info("pud_val = 0x%lx\n", pud_val(*pud));
        pr_info("pud_index = %lu\n", pud_index(vaddr));
        if (pud_none(*pud)) {
            pr_notice("Not mapped in pud\n");
            return;
        }

        pmd = pmd_offset(pud, vaddr);
        pr_info("pmd_val = 0x%lxn", pmd_val(*pmd));
        pr_info("pmd_index = %lun", pmd_index(vaddr));
        if (pmd_none(*pmd)) {
            pr_notice("Not mapped in pmd\n");
            return;
        }

        pte = pte_offset_kernel(pmd, vaddr);
        pr_info("pte_val = 0x%lx\n", pte_val(*pte));
        pr_info("pte_index = %lu\n", pte_index(vaddr));
        if (pte_none(*pte)) {
            pr_notice("Not mapped in pte\n");
            return;
        }

        page_addr = pte_val(*pte) & PAGE_MASK;
        page_offset = vaddr & ~PAGE_MASK;
        paddr = page_addr | page_offset;
        printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
        printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);
        my_beloved_page = pte_page(*pte);
        printk("Page flags = %lx\n", my_beloved_page->flags);

        msg.page->status = true;
        msg.page->virtualAddress = vaddr;
        msg.page->physicalAddress = paddr;
        msg.page->configPgtableLevels = CONFIG_PGTABLE_LEVELS;
        msg.page->pageSize = PAGE_SIZE;
        msg.page->flags = my_beloved_page->flags;
        msg.page->refcount = my_beloved_page->_refcount.counter;
        
    }
    free_page(vaddr);
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Questions?\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
