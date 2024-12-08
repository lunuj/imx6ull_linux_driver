#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define CHRDEVBASE_MAJOR	200
#define CHRDEVBASE_NAME		"chrdevbase"
#define READBUF_SIZE		20
#define WRITEBUF_SIZE		20

static char readbuf[READBUF_SIZE];
static char writebuf[WRITEBUF_SIZE];
static char kernel_data[WRITEBUF_SIZE] = {"kernel data"};


static int chrdevbase_open(struct inode * inode, struct file * filp){
	printk("chrdevbase open!\r\n");
	return 0;
}	

static ssize_t chrdevbase_read(struct file * filp, char __user * buf, size_t cnt, loff_t * offt){
	int value = 0;
	printk("chardev read data!\r\n");
	memcpy(readbuf, kernel_data, sizeof(kernel_data));
	value = copy_to_user(buf, readbuf, cnt);
	if(value == 0){
		printk("kernel send data: %s\r\n", readbuf);
	}else{
		printk("kernel send data error!\r\n");
		return -1;
	}

	return 0;
}

static ssize_t chrdevbase_write(struct file * filp, const char __user * buf, size_t cnt, loff_t * offt){
	int value = 0;
	printk("chardev write data!\r\n");
	value = copy_from_user(writebuf, buf, cnt);
	memcpy(kernel_data, writebuf, sizeof(kernel_data));
	if(value == 0){
		printk("kernel reveive data: %s\r\n", writebuf);
	}else{
		printk("kernel reveive data error!");
		return -1;
	}
	
	return 0;
}

static int chrdevbase_release(struct inode * inode, struct file * filp){
	printk("chrdevbase release!\r\n");
	return 0;
}

static struct file_operations chrdevbase_fops = {
	.owner = THIS_MODULE,
	.open = chrdevbase_open,
	.write = chrdevbase_write,
	.read = chrdevbase_read,
	.release = chrdevbase_release,
};

static int __init chrdevbase_init(void){
	int value = 0;
	value = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
	if(value < 0){
		printk("chrdevbase driver register failed!\r\n");
	}
	printk("chrdevbase_init()\r\n");
	return 0;
}


static void __exit chrdevbase_exit(void){
	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
	printk("chrdevbase_exit()\r\n");
}

module_init(chrdevbase_init);

module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");

