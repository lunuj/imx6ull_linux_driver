#include "leddriver.h"

struct new_dev led;


static int led_open(struct inode * inode, struct file * filp){
    printk("led open!\r\n");
    filp->private_data = &led;
    return 0;
}

static ssize_t led_read(struct file * filp, __user char * buf, size_t count, loff_t * ppos){
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    uint8_t value = atomic_read(&dev->gpio_value);
    if(copy_to_user(buf, &value, sizeof(value)) != 0){
        printk("[ERROR]: copy_to_user()\r\n");
        return -1;
    }
    return 0;
}

static void led_switch(uint8_t sta){
    uint32_t val = 0;
    if(sta == 1){
        val = readl(IMX6UL_GPIO1_DR);
        val &= ~(1<<3);
        writel(val, IMX6UL_GPIO1_DR);
    }else if(sta == 0){
        val = readl(IMX6UL_GPIO1_DR);
        val |= (1<<3);
        writel(val, IMX6UL_GPIO1_DR);
    }
}

static int led_write(struct file * filp, const char __user * buf, size_t count, loff_t * ppos){
    uint8_t value = 0;
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    if(copy_from_user(&value, buf, sizeof(value)) != 0){
        printk("[ERROR]: copy_from_user()\r\n");
        return -1;
    }
    atomic_set(&dev->gpio_value, value);
    led_switch(value);
    return 0;
}

static int led_close(struct inode * inode, struct file * filp){
    printk("led close!\r\n");
    filp->private_data = 0;
    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_close,
};


static int dev_init(void){
    int ret = 0;
    atomic_set(&led.gpio_value, 0);
    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        ret = alloc_chrdev_region(&led.dev_id,0,1,DEV_NAME);
        led.major = MAJOR(led.dev_id);
        led.minor = MINOR(led.dev_id);
    }else{
        //手动申请设备号
        led.dev_id = MKDEV(240,0);                        //调用MKDEV函数构建设备号
        led.major = MAJOR(led.dev_id);
        led.minor = MINOR(led.dev_id);
        ret = register_chrdev_region(led.dev_id,1,DEV_NAME);    //注册设备
    }
    if(ret < 0){
        printk("new device region err!\r\n");
        return -1;
    }
    printk("dev_t = %d,major = %d,minor = %d\r\n", led.dev_id,led.major,led.minor);

    //字符设备注册
    led.cdev.owner = THIS_MODULE;
    cdev_init(&led.cdev, &led_fops);
    cdev_add(&led.cdev, led.dev_id, 1);

    //创建设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        led.class = class_create(THIS_MODULE, DEV_NAME);
        if(IS_ERR(led.class)){
            return PTR_ERR(led.class);
        }
        led.device = device_create(led.class, NULL, led.dev_id, NULL, DEV_NAME);
        if(IS_ERR(led.device)){
            return PTR_ERR(led.device);
        }
    }

    return 0;
}

static int leddriver_probe(struct platform_device *dev){
    int i = 0, val = 0;
    struct resource * ledsource[5];
    printk("led driver match\r\n");

    //获取资源
    for(;i<5;i++){
        ledsource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);
        if(ledsource[i] == NULL){
            return -EINVAL;
        }
    }
    
    //IO映射
    IMX6UL_CCM_CCGR1 = ioremap(ledsource[0]->start,resource_size(ledsource[0]));
    IMX6UL_SW_MUX_GPIO1_IO03 = ioremap(ledsource[1]->start,resource_size(ledsource[1]));
    IMX6UL_SW_PAD_GPIO1_IO03 = ioremap(ledsource[2]->start,resource_size(ledsource[2]));
    IMX6UL_GPIO1_GDIR = ioremap(ledsource[3]->start,resource_size(ledsource[3]));
    IMX6UL_GPIO1_DR = ioremap(ledsource[4]->start,resource_size(ledsource[3]));


    //CCM初始化
    val = readl(IMX6UL_CCM_CCGR1);  //读取CCM_CCGR1的值
    val &= ~(3<<26);                //清除bit26、27
    val |= (3<<26);                 //bit26、27置1
    writel(val, IMX6UL_CCM_CCGR1);
    printk("CCM init finisinithed\r\n");

    //GPIO初始化
    writel(0x15, IMX6UL_SW_MUX_GPIO1_IO03);
    writel(0x10B0, IMX6UL_SW_PAD_GPIO1_IO03);
    printk("GPIO SW init finished\r\n");

    val = readl(IMX6UL_GPIO1_GDIR);
    val |= 1<<3;                        //bit3=1,设置为输出
    writel(val, IMX6UL_GPIO1_GDIR);
    printk("GPIO GDIR init finished\r\n");

    val = readl(IMX6UL_GPIO1_DR);
    val &= (1<<3);
    writel(val,IMX6UL_GPIO1_DR);
    printk("GPIO DR init finished\r\n");

    if(dev_init() != 0){
        return -EINVAL;
    }
    return 0;
}

static int leddriver_remove(struct platform_device *dev){
    printk("led driver remove\r\n");
    //卸载设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        device_destroy(led.class, led.dev_id);
        class_destroy(led.class);
    }
    //卸载字符设备
    cdev_del(&led.cdev);
    //注销设备号
    unregister_chrdev_region(led.dev_id,1);
    //取消地址映射
    iounmap(IMX6UL_CCM_CCGR1);
    iounmap(IMX6UL_SW_MUX_GPIO1_IO03);
    iounmap(IMX6UL_SW_PAD_GPIO1_IO03);
    iounmap(IMX6UL_GPIO1_DR);
    iounmap(IMX6UL_GPIO1_GDIR);

    printk("led iounmap finisinithed\r\n");
    return 0;
}

static struct platform_driver leddriver = {
    .driver = {
        .name = "imx6ull-led",
    },
    .probe = leddriver_probe,
    .remove = leddriver_remove,
};

static int __init leddriver_init(void){
    return platform_driver_register(&leddriver);
}

static void __exit leddriver_exit(void){
    platform_driver_unregister(&leddriver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");