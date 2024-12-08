#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/device.h>

#include "led.h"

struct new_device led;
static int kernel_data;
struct cdev led_cdev;

static void led_switch(u8 sta){
    uint32_t val = 0;
    if(sta == LED_ON){
        val = readl(IMX6UL_GPIO1_DR);
        val &= ~(1<<3);
        writel(val, IMX6UL_GPIO1_DR);
    }else if(sta == LED_OFF){
        val = readl(IMX6UL_GPIO1_DR);
        val |= (1<<3);
        writel(val, IMX6UL_GPIO1_DR);
    }
}

static int led_get(void){
    u32 val = 0;
    val = readl(IMX6UL_GPIO1_DR);
    val = (val >> 3) & 0x1;
    return LED_ON ? LED_OFF : val == 0;
}


static int led_open(struct inode * inode, struct file * filp){
    printk("led open\r\n");
    return 0;
}
static int led_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
    uint32_t value = 0;
    printk("led read data!\r\n");
    kernel_data = led_get();
	value = copy_to_user(buf, &kernel_data, 4);
    printk("led read: %d\r\n", kernel_data);
    return 0;
}
static int led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    uint32_t value = 0;
    value = copy_from_user(&kernel_data, buf, 4);
    led_switch(kernel_data);
    printk("led write: %d\r\n", kernel_data);
    return 0;
}
static int led_close(struct inode * inode, struct file * filp){
    printk("led close\r\n");
    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_close,
};

static int __init led_init(void)
{
    int ret = 0, val = 0;

    // ioremap
    IMX6UL_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,4);
    IMX6UL_SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    IMX6UL_SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    IMX6UL_GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
    IMX6UL_GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);
    printk("led ioremap finisinithed\r\n");

    //CCM初始化
    val = readl(IMX6UL_CCM_CCGR1);  //读取CCM_CCGR1的值
    val &= ~(3<<26);                //清除bit26、27
    val |= (3<<26);                //bit26、27置1
    writel(val, IMX6UL_CCM_CCGR1);
    printk("CCM init finisinithed\r\n");

    //GPIO初始化
    writel(0x5, IMX6UL_SW_MUX_GPIO1_IO03);
    writel(0x10B0, IMX6UL_SW_PAD_GPIO1_IO03);
    printk("GPIO SW init finished\r\n");

    val = readl(IMX6UL_GPIO1_GDIR);
    val |= 1<<3;                        //bit3=1,设置为输出
    writel(val, IMX6UL_GPIO1_GDIR);
    printk("GPIO GDIR init finished\r\n");

    val = readl(IMX6UL_GPIO1_DR);
    val &= ~(1<<3);
    writel(val,IMX6UL_GPIO1_DR);
    printk("GPIO DR init finished\r\n");

    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        ret = alloc_chrdev_region(&led.dev_id,0,1,DEV_NAME);
        led.major = MAJOR(led.dev_id);
        led.minor = MINOR(led.dev_id);
        printk("dev_t = %d,major = %d,minor = %d\r\n",led.dev_id,led.major,led.minor);
    }else{
        //手动申请设备号
        led.dev_id = MKDEV(LED_MAJOR,0);                        //调用MKDEV函数构建设备号
        led.major = MAJOR(led.dev_id);
        led.minor = MINOR(led.dev_id);
        ret = register_chrdev_region(led.dev_id,1,DEV_NAME);    //注册设备
        printk("dev_t = %d,major = %d,minor = %d\r\n",led.dev_id,led.major,led.minor);
    }

    //字符设备注册
    led_cdev.owner = THIS_MODULE;
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, led.dev_id, 1);

    //创建设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        led.class = class_create(THIS_MODULE, DEV_NAME);
        led.device = device_create(led.class, NULL, led.dev_id, NULL, DEV_NAME);
    }

    return ret;
}

static void __exit led_exit(void)
{
    //卸载设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        device_destroy(led.class, led.dev_id);
        class_destroy(led.class);
    }
    //卸载字符设备
    cdev_del(&led_cdev);
    //注销设备号
    unregister_chrdev_region(led.dev_id,1);
    //取消地址映射
    iounmap(IMX6UL_CCM_CCGR1);
    iounmap(IMX6UL_SW_MUX_GPIO1_IO03);
    iounmap(IMX6UL_SW_PAD_GPIO1_IO03);
    iounmap(IMX6UL_GPIO1_DR);
    iounmap(IMX6UL_GPIO1_GDIR);

    printk("led iounmap finisinithed\r\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");