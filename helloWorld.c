#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/ktime.h>
#include <linux/hrtimer.h>

#define DEVICE_NAME "HelloWorld"
#define CLASS_NAME "chardrv"


static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

static enum hrtimer_restart timerCallback( struct hrtimer * param);

static int helloworld_open(struct inode *i, struct file *f)
{
  printk("(hello world) open()\n");
  return 0;
}

static int helloworld_close(struct inode *i, struct file *f)
{
  printk("(hello world) close()\n");
  return 0;
}

static ssize_t helloworld_read(	struct file *f, 
								char __user *buf, 
								size_t	len, 
								loff_t *off)
{
  printk("(hello world) read()\n");
  return 0;
}


static ssize_t helloworld_write(	struct file *f,
								 	const char __user *buf,
								  	size_t len, 
								  	loff_t *off)
{
  printk("(hello world) write()\n");
  //return 0; //NOTE: this puts helloworld_write() in infinite loop, retrying to write again and again
  return len;
}

static struct file_operations helloworld_fops =
{
  .owner = THIS_MODULE,
  .open = helloworld_open,
  .release = helloworld_close,
  .write = helloworld_write,
  .read = helloworld_read,
  
};

unsigned long timer_interval_ns = 1e6;
static struct hrtimer hr_timer;
 
static enum hrtimer_restart timerCallback( struct hrtimer * timer_for_restart)
{
  	ktime_t currtime , interval;
  	currtime  = ktime_get();
  	interval = ktime_set(1,timer_interval_ns); 
  	hrtimer_forward(timer_for_restart, currtime , interval);
  	printk("timer callback");
	// set_pin_value(PIO_G,9,(cnt++ & 1)); //Toggle LED 
	return HRTIMER_RESTART;
} 

static int __init hello_init(void)
{

	ktime_t k_timer;

	printk("(hello world) init()\n");
	
	//Register a range of char device numbers baseminor = 0, count = 1 name HelloWorld
	if (alloc_chrdev_region(&first, 0, 1, DEVICE_NAME) < 0)
	{
	    return -1;
	}
	    
	//Create a struct class structure    
	if ((cl = class_create(THIS_MODULE, CLASS_NAME)) == NULL)
	{
	    unregister_chrdev_region(first, 1);
	    return -1;
	}
	    
	if (device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL)
	{
	    class_destroy(cl);
	    unregister_chrdev_region(first, 1);
	    return -1;
	}
	    
	cdev_init(&c_dev, &helloworld_fops);
	    
	if (cdev_add(&c_dev, first, 1) == -1)
	{
	    device_destroy(cl, first);
	    class_destroy(cl);
	    unregister_chrdev_region(first, 1);
	    return -1;
	}


	k_timer = ktime_set(1, 0);

	// void hrtimer_init(struct hrtimer *timer, clockid_t which_clock);
	hrtimer_init( &hr_timer , CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &timerCallback;

	// int hrtimer_start(struct hrtimer *timer, ktime_t time, enum hrtimer_mode mode);
	hrtimer_start(&hr_timer, k_timer, HRTIMER_MODE_REL);
	
    return 0;
}

static void __exit hello_exit(void)
{

    int ret;
  	ret = hrtimer_cancel( &hr_timer );
  	if (ret) printk("The timer was still in use...\n");
  	printk("HR Timer module uninstalling\n");
	
    printk("(hello world) exit()\n");
	cdev_del(&c_dev);
	device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Breno");
MODULE_DESCRIPTION("Hello");
MODULE_VERSION("1.0.0");
