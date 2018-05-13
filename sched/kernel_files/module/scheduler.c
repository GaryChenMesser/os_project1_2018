#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define HELLO_IOCTL_PRINT    0x12345678

// File operations
static int hello_open(struct inode* inode, struct file* file)
{
	printk("%s\n", "open!\n");
	return 0;
}

static int hello_close(struct inode* inode, struct file* file)
{
	printk("%s\n", "close\n");
	return 0;
}

static long hello_ioctl(struct file* file, unsigned long ioctl_num, unsigned long ioctl_param)
{
	long ret = -EINVAL;
	char buf[32];
	
	switch(ioctl_num){
		case HELLO_IOCTL_PRINT:
			if(copy_from_user(buf, (char*)ioctl_param, 32))
				return -ENOMEM;
			printk(KERN_WARNING "%s\n", buf);
			ret = 0;
			break;
	}
  
	return ret;
}

// define file_operations
static struct file_operations hello_fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = hello_ioctl,
  .open = hello_open,
  .release = hello_close
};
// define miscdevice
static struct miscdevice hello_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "hello",
  .fops = &hello_fops
};

static int __init hello_init(void)
{
  int ret;
  
  if((ret = misc_register(&hello_dev)) < 0){
    printk(KERN_ERR "misc_register failed!\n");
    return ret;
  }
  
  printk(KERN_INFO "hello initialized!\n");
  
  return 0;
}

static void __exit hello_exit(void)
{
  misc_deregister(&hello_dev);
  printk(KERN_INFO "hello exited!\n");
}

module_init(hello_init);
module_exit(hello_exit);
