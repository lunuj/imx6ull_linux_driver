#include "led.h"
#include "myerror.h"

static int kernel_data;
static uint16_t ret_val;
struct new_device led;
struct cdev led_cdev;

/**
 * @brief 映射后的虚拟地址
 * 
 */
static void __iomem *IMX6UL_CCM_CCGR1;              //映射后的虚拟地址
static void __iomem *IMX6UL_SW_MUX_GPIO1_IO03;      
static void __iomem *IMX6UL_SW_PAD_GPIO1_IO03;
static void __iomem *IMX6UL_GPIO1_DR;
static void __iomem *IMX6UL_GPIO1_GDIR;

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
    uint32_t val = 0;
    val = readl(IMX6UL_GPIO1_DR);
    val = (val >> 3) & 0x1;
    return LED_ON ? LED_OFF : val == 0;
}

static int led_open(struct inode * inode, struct file * filp){
    printk("[INFO]: led open\r\n");
    return 0;
}
static int led_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
    kernel_data = led_get();
    if(copy_to_user(buf, &kernel_data, 4) != 0){
        printk("[ERROR]: copy_to_user()\r\n");
        SET_ERROR(ret_val, ERROR_LED_READ);
    }
    printk("[INFO]: led read %d\r\n", kernel_data);
    return ret_val;
}
static int led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    if(copy_from_user(&kernel_data, buf, 4)!=0){
        printk("[ERROR]: copy_from_user()\r\n");
        SET_ERROR(ret_val, ERROR_LED_READ);
    }

    if(IS_OK(ret_val)){
        led_switch(kernel_data);
    }

    printk("[INFO]: led write %d\r\n", kernel_data);
    return ret_val;
}
static int led_close(struct inode * inode, struct file * filp){
    printk("[INFO]: led close\r\n");
    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_close,
};

static int devicetree_init(void){
    uint32_t regdata[10];
    const char *str;

    led.dev_nd = of_find_node_by_path("/alphaLED");
    if(led.dev_nd == NULL){
        SET_ERROR(ret_val, ERROR_LED_FINDNODE);
        printk("[ERROR]: of_find_node_by_path()\r\n");
    }

    if(of_property_read_string(led.dev_nd, "status", &str)!=0){
        SET_ERROR(ret_val, ERROR_LED_OFREADSTRING);
        printk("[ERROR]: of_property_read_string\r\n");
    }

    if(of_property_read_u32_array(led.dev_nd,"reg",regdata,10)!=0){
        SET_ERROR(ret_val, ERROR_LED_OFREADU32A);
        printk("[ERROR]: of_property_read_string\r\n");
    }

    if(IS_OK(ret_val)){
        //获取地址映射
        IMX6UL_CCM_CCGR1 = ioremap(regdata[0],regdata[1]);
        IMX6UL_SW_MUX_GPIO1_IO03 = ioremap(regdata[2],regdata[3]);
        IMX6UL_SW_PAD_GPIO1_IO03 = ioremap(regdata[4],regdata[5]);
        IMX6UL_GPIO1_DR = ioremap(regdata[6],regdata[7]);
        IMX6UL_GPIO1_GDIR = ioremap(regdata[8],regdata[9]);
    }

    return ret_val;
}

static void led_error_resolve(void){
    if(!IS_ERROR(ret_val, ERROR_LED_DEVICE)){
        if(!IS_ERROR(ret_val, ERROR_LED_CLASS)){
            if(!IS_ERROR(ret_val, ERROR_LED_CDEVADD)){
                if(!IS_ERROR(ret_val, ERROR_LED_ALLOCREG)){
                }else{
                    unregister_chrdev_region(led.dev_id,1);
                }
            }else{
                cdev_del(&led.cdev);
            }
        }else{
            class_destroy(led.class);
        }
    }else{
        device_destroy(led.class, led.dev_id);
    }
}

static int __init led_init(void)
{
    int ret = 0, val = 0;

    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        if(alloc_chrdev_region(&led.dev_id,0,1,DEV_NAME) != 0){
            SET_ERROR(ret_val, ERROR_LED_ALLOCREG);
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            led_error_resolve();
            return ret_val;
        }else{
            led.major = MAJOR(led.dev_id);
            led.minor = MINOR(led.dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",led.dev_id,led.major,led.minor);
        }
   }else{
        //手动申请设备号
        led.dev_id = MKDEV(LED_MAJOR,0);                        //调用MKDEV函数构建设备号
        led.major = MAJOR(led.dev_id);
        led.minor = MINOR(led.dev_id);
        if(register_chrdev_region(led.dev_id,1,DEV_NAME) == 0){
            SET_ERROR(ret_val, ERROR_LED_ALLOCREG);
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            led_error_resolve();
            return ret_val;
        }else{
            led.major = MAJOR(led.dev_id);
            led.minor = MINOR(led.dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",led.dev_id,led.major,led.minor);
        }
    }

    //字符设备注册
    led_cdev.owner = THIS_MODULE;
    cdev_init(&led_cdev, &led_fops);
    if(cdev_add(&led_cdev, led.dev_id, 1)!=0){
        SET_ERROR(ret_val, ERROR_LED_CDEVADD);
        printk("[ERROR]: cdev_add()\r\n");
        led_error_resolve();
        return ret_val;
    }

    //创建设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        led.class = class_create(THIS_MODULE, DEV_NAME);
        if(IS_ERR(led.class)){
            SET_ERROR(ret_val, ERROR_LED_CLASS);
            printk("[ERROR]: class_create()\r\n");
            led_error_resolve();
            return ret_val;
        }
        led.device = device_create(led.class, NULL, led.dev_id, NULL, DEV_NAME);
        if(IS_ERR(led.device)){
            SET_ERROR(ret_val, ERROR_LED_DEVICE);
            printk("[ERROR]: device_create()\r\n");
            led_error_resolve();
            return ret_val;
        }
    }

    if(IS_OK(devicetree_init())){
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
    }else{
        led_error_resolve();
        return ret_val;
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