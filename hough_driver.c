#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

MODULE_AUTHOR ("y23_g00");
MODULE_DESCRIPTION("Test Driver for Hough IP.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("custom: hough IP");

#define DEVICE_NAME "hough"
#define DRIVER_NAME "hough_driver"

//buffer size
#define BUFF_SIZE 1000

//addresses for registers
#define START_REG_OFFSET 0
#define RESET_REG_OFFSET 4
#define WIDTH_REG_OFFSET 8
#define HEIGHT_REG_OFFSET 12
#define RHO_REG_OFFSET 16
#define TRESHOLD_REG_OFFSET 20
#define READY_REG_OFFSET 24

#define ADDR_FACTOR 4

//*******************FUNCTION PROTOTYPES************************************
static int hough_probe(struct platform_device *pdev);
static int hough_open(struct inode *i, struct file *f);
static int hough_close(struct inode *i, struct file *f);
static ssize_t hough_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t hough_write(struct file *f, const char __user *buf, size_t count, loff_t *off);
static int __init hough_init(void);
static void __exit hough_exit(void);
static int hough_remove(struct platform_device *pdev);

//*********************GLOBAL VARIABLES*************************************
struct hough_info 
{
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

static struct cdev *my_cdev;
static dev_t my_dev_id;
static struct class *my_class;

static struct hough_info *img_bram = NULL;
static struct hough_info *acc0_bram = NULL;
static struct hough_info *acc1_bram = NULL;
static struct hough_info *hough_core = NULL;

static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = hough_open,
    .release = hough_close,
    .read = hough_read,
    .write = hough_write
};

static struct of_device_id device_of_match[] = 
{
	{ .compatible = "img_bram_ctrl", }, // acc0 bram
	{ .compatible = "acc0_bram_ctrl", }, // acc1 bram
	{ .compatible = "acc1_bram_ctrl", }, // img bram
	{ .compatible = "hough_", }, // hough)core
	{ /* end of list */ },
};

static struct platform_driver my_driver = {
    .driver = 
	{
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= device_of_match,
	},
	.probe = hough_probe,
	.remove	= hough_remove,
};

MODULE_DEVICE_TABLE(of, device_of_match);

//***************************************************************************
// PROBE AND REMOVE
int probe_counter = 0;

static int hough_probe(struct platform_device *pdev)
{
	struct resource *r_mem;
	int rc = 0;

	printk(KERN_INFO "Probing\n");

	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) 
	{
		printk(KERN_ALERT "hough_probe: invalid address\n");
		return -ENODEV;
	}

	switch (probe_counter)
	{
		case 0: // img_bram
			img_bram = (struct hough_info *) kmalloc(sizeof(struct hough_info), GFP_KERNEL);
			if (!img_bram)
			{
				printk(KERN_ALERT "hough_probe: Cound not allocate img_bram device\n");
				return -ENOMEM;
			}
			
			img_bram->mem_start = r_mem->start;
			img_bram->mem_end   = r_mem->end;
			if(!request_mem_region(img_bram->mem_start, img_bram->mem_end - img_bram->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "hough_probe: Couldn't lock memory region at %p\n",(void *)img_bram->mem_start);
				rc = -EBUSY;
				goto error1;
			}
			
			img_bram->base_addr = ioremap(img_bram->mem_start, img_bram->mem_end - img_bram->mem_start + 1);
			if (!img_bram->base_addr)
			{
				printk(KERN_ALERT "hough_probe: Could not allocate img_bram iomem\n");
				rc = -EIO;
				goto error2;
			}
			
			probe_counter++;
			printk(KERN_INFO "hough_probe: img_bram driver registered.\n");
			return 0;
			
			error2:
			release_mem_region(img_bram->mem_start, img_bram->mem_end - img_bram->mem_start + 1);
			
			error1:
			return rc;
		
			break;
			
		case 1: // acc0_bram
			acc0_bram = (struct hough_info *) kmalloc(sizeof(struct hough_info), GFP_KERNEL);
			if (!acc0_bram)
			{
				printk(KERN_ALERT "hough_probe: Cound not allocate acc0_bram device\n");
				return -ENOMEM;
			}
			
			acc0_bram->mem_start = r_mem->start;
			acc0_bram->mem_end   = r_mem->end;
			if(!request_mem_region(acc0_bram->mem_start, acc0_bram->mem_end - acc0_bram->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "hough_probe: Couldn't lock memory region at %p\n",(void *)acc0_bram->mem_start);
				rc = -EBUSY;
				goto error3;
			}
			
			acc0_bram->base_addr = ioremap(acc0_bram->mem_start, acc0_bram->mem_end - acc0_bram->mem_start + 1);
			if (!acc0_bram->base_addr)
			{
				printk(KERN_ALERT "hough_probe: Could not allocate acc0_bram iomem\n");
				rc = -EIO;
				goto error4;
			}
			
			probe_counter++;
			printk(KERN_INFO "hough_probe: acc0_bram driver registered.\n");
			return 0;
			
			error4:
			release_mem_region(acc0_bram->mem_start, acc0_bram->mem_end - acc0_bram->mem_start + 1);
			
			error3:
			return rc;
			
			break;
			
		case 2: // acc1_bram
			acc1_bram = (struct hough_info *) kmalloc(sizeof(struct hough_info), GFP_KERNEL);
			if (!acc1_bram)
			{
				printk(KERN_ALERT "hough_probe: Cound not allocate acc1_bram device\n");
				return -ENOMEM;
			}
			
			acc1_bram->mem_start = r_mem->start;
			acc1_bram->mem_end   = r_mem->end;
			if(!request_mem_region(acc1_bram->mem_start, acc1_bram->mem_end - acc1_bram->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "hough_probe: Couldn't lock memory region at %p\n",(void *)acc1_bram->mem_start);
				rc = -EBUSY;
				goto error5;
			}
			
			acc1_bram->base_addr = ioremap(acc1_bram->mem_start, acc1_bram->mem_end - acc1_bram->mem_start + 1);
			if (!acc1_bram->base_addr)
			{
				printk(KERN_ALERT "hough_probe: Could not allocate acc1_bram iomem\n");
				rc = -EIO;
				goto error6;
			}
			
			probe_counter++;
			printk(KERN_INFO "hough_probe: acc1_bram driver registered.\n");
			return 0;
			
			error6:
			release_mem_region(acc1_bram->mem_start, acc1_bram->mem_end - acc1_bram->mem_start + 1);
			
			error5:
			
			return rc;
			break;
			
		case 3: // hough_core
			hough_core = (struct hough_info *) kmalloc(sizeof(struct hough_info), GFP_KERNEL);
			if (!hough_core)
			{
				printk(KERN_ALERT "hough_probe: Cound not allocate hough_core\n");
				return -ENOMEM;
			}
			
			hough_core->mem_start = r_mem->start;
			hough_core->mem_end   = r_mem->end;
			if(!request_mem_region(hough_core->mem_start, hough_core->mem_end - hough_core->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "hough_probe: Couldn't lock memory region at %p\n",(void *)hough_core->mem_start);
				rc = -EBUSY;
				goto error7;
			}
			
			hough_core->base_addr = ioremap(hough_core->mem_start, hough_core->mem_end - hough_core->mem_start + 1);
			if (!hough_core->base_addr)
			{
				printk(KERN_ALERT "hough_probe: Could not allocate hough iomem\n");
				rc = -EIO;
				goto error8;
			}
			
			printk(KERN_INFO "hough_probe: hough_core driver registered.\n");
			return 0;
			
			error8:
			release_mem_region(hough_core->mem_start, hough_core->mem_end - hough_core->mem_start + 1);
			
			error7:
			return rc;
			
			break;
			
		default:
			printk(KERN_INFO "hough_probe: Counter has illegal value. \n");
			return -1;
	}
	return 0;
}	
		
static int hough_remove(struct platform_device *pdev)
{
	switch (probe_counter)
	{
		case 0: // img_bram
			printk(KERN_ALERT "hough_remove: img_bram platform driver removed\n");
			iowrite32(0, img_bram->base_addr);
			iounmap(img_bram->base_addr);
			release_mem_region(img_bram->mem_start, img_bram->mem_end - img_bram->mem_start + 1);
			kfree(img_bram);
			break;
			
		case 1: // acc0_bram
			printk(KERN_ALERT "hough_remove: acc0_bram platform driver removed\n");
			iowrite32(0, acc0_bram->base_addr);
			iounmap(acc0_bram->base_addr);
			release_mem_region(acc0_bram->mem_start, acc0_bram->mem_end - acc0_bram->mem_start + 1);
			kfree(acc0_bram);
			probe_counter--;
			break;
			
		case 2: // acc1_bram
			printk(KERN_ALERT "hough_remove: acc1_bram platform driver removed\n");
			iowrite32(0, acc1_bram->base_addr);
			iounmap(acc1_bram->base_addr);
			release_mem_region(acc1_bram->mem_start, acc1_bram->mem_end - acc1_bram->mem_start + 1);
			kfree(acc1_bram);
			probe_counter--;
			break;
			
		case 3: // hough_core
			printk(KERN_ALERT "hough_remove: hough_core platform driver removed\n");
			iowrite32(0, hough_core->base_addr);
			iounmap(hough_core->base_addr);
			release_mem_region(hough_core->mem_start, hough_core->mem_end - hough_core->mem_start + 1);
			kfree(hough_core);
			probe_counter--;
			break;
			
		default:
			printk(KERN_INFO "hough_remove: Counter has illegal value. \n");
			return -1;
	}
	return 0;
}		


//***************************************************
// IMPLEMENTATION OF FILE OPERATION FUNCTIONS*************************************************

static int hough_open(struct inode *i, struct file *f)
{
    //printk("hough opened\n");
    return 0;
}
static int hough_close(struct inode *i, struct file *f)
{
    //printk("hough closed\n");
    return 0;
}

unsigned int img_i = 0;
unsigned int acc0_i = 0;
unsigned int acc1_i = 0;
int endRead = 0;

ssize_t hough_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	char buf[BUFF_SIZE];
	long int len = 0;
	unsigned int val;
	int minor = MINOR(pfile->f_inode->i_rdev);
	unsigned int width, height, rho;
	unsigned int theta = 135;
	
	width = ioread32(hough_core->base_addr + WIDTH_REG_OFFSET);
	height = ioread32(hough_core->base_addr + HEIGHT_REG_OFFSET);
	rho = ioread32(hough_core->base_addr + RHO_REG_OFFSET);
  
	if (endRead == 1)
	{
		endRead = 0;
		return 0;
	}

	switch (minor)
	{
		case 0: // img_bram
			if(img_i < theta * rho)
			{	
				val = ioread32(img_bram->base_addr + ADDR_FACTOR * img_i);
				len = scnprintf(buf, BUFF_SIZE, "%u ", val);
				img_i++;
				
				if (copy_to_user(buffer, buf, len))
				{
					printk(KERN_ERR "hough_read: Copy to user does not work.\n");
					return -EFAULT;
				}
			}
			else
			{
				endRead = 1;
				img_i = 0;
			}
			break;
		
		case 1: // acc0_bram
			if(acc0_i < theta * rho)
			{	
				val = ioread32(img_bram->base_addr + ADDR_FACTOR * acc0_i);
				len = scnprintf(buf, BUFF_SIZE, "%u ", val);
				acc0_i++;
				
				if (copy_to_user(buffer, buf, len))
				{
					printk(KERN_ERR "hough_read: Copy to user does not work.\n");
					return -EFAULT;
				}
			}
			else
			{
				endRead = 1;
				acc0_i = 0;
			}
			break;
			
		case 2: // acc1_bram
			if(acc1_i < width * height)
			{	
				val = ioread32(img_bram->base_addr + ADDR_FACTOR * acc1_i);
				len = scnprintf(buf, BUFF_SIZE, "%u ", val);
				acc1_i++;
				
				if (copy_to_user(buffer, buf, len))
				{
					printk(KERN_ERR "hough_read: Copy to user does not work.\n");
					return -EFAULT;
				}
			}
			else
			{
				endRead = 1;
				acc1_i = 0;
			}
			break;	
			
		case 3: // hough_core
			val = ioread32(hough_core->base_addr + READY_REG_OFFSET);

			len = scnprintf(buf, BUFF_SIZE, "%u\n", val);

			if (copy_to_user(buffer, buf, len))
			{
				printk(KERN_ERR "hough_read: Copy to user does not work.\n");
				return -EFAULT;
			}
			
			endRead = 1;
			break;

		default:
			printk(KERN_ERR "hough_read: Invalid minor. \n");
			endRead = 1;
	}
	return len;
}

ssize_t hough_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	char buf[length + 1];
	int minor = MINOR(pfile->f_inode->i_rdev);
	unsigned int pos = 0;
	unsigned int val = 0;

	if (copy_from_user(buf, buffer, length))
		return -EFAULT;
	buf[length]='\0';
	
	printk(KERN_INFO "\n\n\n\n\nminor = %d\n", minor);
	sscanf(buf, "%u, %u", &val, &pos);
	printk(KERN_INFO "val = %u, pos = %u\n", val, pos);
	printk(KERN_INFO "img_bram address = %p\n", img_bram->base_addr);
	printk(KERN_INFO "acc0_bram address = %p\n", acc0_bram->base_addr);
	printk(KERN_INFO "acc1_bram address = %p\n", acc1_bram->base_addr);
	printk(KERN_INFO "hough_core address = %p\n\n\n", hough_core->base_addr);	
	
	switch (minor)
	{
		case 0: // img_bram
			iowrite32(val, img_bram->base_addr + ADDR_FACTOR * pos);
			break;
			
		case 1: // acc0_bram
			iowrite32(val, acc0_bram->base_addr + ADDR_FACTOR * pos);
			break;
			
		case 2: // acc1_bram
			iowrite32(val, acc1_bram->base_addr + ADDR_FACTOR * pos);
			break;
			
		case 3: // hough_core
			iowrite32(val, hough_core->base_addr + pos);
			break;
			
		default:
			printk(KERN_INFO "hough_write: Invalid minor. \n");
	}
	return length;
}

//***************************************************
// INIT AND EXIT FUNCTIONS OF THE DRIVER

static int __init hough_init(void)
{
	printk(KERN_INFO "\n");
	printk(KERN_INFO "hough driver starting insmod.\n");

	if (alloc_chrdev_region(&my_dev_id, 0, 4, "hough_region") < 0)
	{
		printk(KERN_ERR "failed to register char device\n");
		return -1;
	}
	printk(KERN_INFO "char device region allocated\n");

	my_class = class_create(THIS_MODULE, "hough_class");
	if (my_class == NULL)
	{
		printk(KERN_ERR "failed to create class\n");
		goto fail_0;
	}
	printk(KERN_INFO "class created\n");

	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 0), NULL, "img_bram_ctrl") == NULL) 
	{
		printk(KERN_ERR "failed to create device img_bram\n");
		goto fail_1;
	}
	printk(KERN_INFO "device created - img_bram\n");

	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 1), NULL, "acc0_bram_ctrl") == NULL) 
	{
		printk(KERN_ERR "failed to create device acc0_bram\n");
		goto fail_2;
	}
	printk(KERN_INFO "device created - acc0_bram\n");

	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 2), NULL, "acc1_bram_ctrl") == NULL) 
	{
		printk(KERN_ERR "failed to create device acc1_bram\n");
		goto fail_3;
	}
	printk(KERN_INFO "device created - acc1_bram\n");


	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 3), NULL, "hough_co") == NULL) 
	{
		printk(KERN_ERR "failed to create device hough_coe\n");
		goto fail_4;
	}
	printk(KERN_INFO "device created - hough_core\n");

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;

	if (cdev_add(my_cdev, my_dev_id, 4) == -1)
	{
		printk(KERN_ERR "failed to add cdev\n");
		goto fail_5;
	}
	printk(KERN_INFO "cdev added\n");
	printk(KERN_INFO "hough driver initialized.\n");

	return platform_driver_register(&my_driver);

	fail_5:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),3));
	fail_4:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),2));
	fail_3:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	fail_2:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}

static void __exit hough_exit(void)
{
	printk(KERN_INFO "hough driver starting rmmod.\n");
	platform_driver_unregister(&my_driver);
	cdev_del(my_cdev);

	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),3));	
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),2));
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1);
	printk(KERN_INFO "hough driver exited.\n");
}

module_init(hough_init);
module_exit(hough_exit);
