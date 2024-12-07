#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cdev.h>

#include "led.h"

struct LED_REG led_reg[LEDREG_NUM];
struct DEVICE led;

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
    led_reg[0].reg_address = IMX6UL_CCM_CCGR1;
    led_reg[1].reg_address = IMX6UL_SW_MUX_GPIO1_IO03;
    led_reg[2].reg_address = IMX6UL_SW_PAD_GPIO1_IO03;
    led_reg[3].reg_address = IMX6UL_GPIO1_DR;
    led_reg[4].reg_address = IMX6UL_GPIO1_GDIR;
    for(int i = 0; i < LEDREG_NUM; i++){
        led_reg[i].reg_data = readl(led_reg[i].reg_address);
    }

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

    //设备注册
    cdev_init(&led.cdev, &led_fops);
    ret = cdev_add(&led.cdev,led.dev_id, 1);

    return 0;
}


static int led_open(struct inode * inode, struct file * filp)
static int led_read(struct inode * inode, struct file * filp)
static int led_write(struct inode * inode, struct file * filp)
static int led_close(struct inode * inode, struct file * filp)

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_close,
};

static int __exit led_exit(void)
{
    for(int i = 0; i < LEDREG_NUM; i++){
        writel(led_reg[i].reg_data, led_reg[i].reg_address);
    }
    //取消地址映射
    iounmap(IMX6UL_CCM_CCGR1);
    iounmap(IMX6UL_SW_MUX_GPIO1_IO03);
    iounmap(IMX6UL_SW_PAD_GPIO1_IO03);
    iounmap(IMX6UL_GPIO1_DR);
    iounmap(IMX6UL_GPIO1_GDIR);
    printk("led iounmap finisinithed\r\n");

    cdev_del(&led.cdev);
    //注销设备号
    unregister_chrdev_region(led.dev_id,1);

    return 0;
}

module_init(&led_init);
module_exit(&led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");