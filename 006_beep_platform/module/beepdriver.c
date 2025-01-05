#include "beepdriver.h"

struct of_device_id beep_of_match[] = {
    {
        .compatible = "atkalpha-gpiobeep",
    },
    {/*Sentinel*/},
};

struct new_dev beep;


static int beep_open(struct inode * inode, struct file * filp){
    printk("beep open!\r\n");
    filp->private_data = &beep;
    return 0;
}

static ssize_t beep_read(struct file * filp, __user char * buf, size_t count, loff_t * ppos){
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    uint8_t value = atomic_read(&dev->beep_value);
    if(copy_to_user(buf, &value, sizeof(value)) != 0){
        printk("[ERROR]: copy_to_user()\r\n");
        return -1;
    }
    return 0;
}

static void beep_switch(uint8_t sta){
    if(sta == 1){
        gpio_set_value(beep.gpio_nm, 1);
    }else if(sta == 0){
        gpio_set_value(beep.gpio_nm, 0);
    }
}

static int beep_write(struct file * filp, const char __user * buf, size_t count, loff_t * ppos){
    uint8_t value = 0;
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    if(copy_from_user(&value, buf, sizeof(value)) != 0){
        printk("[ERROR]: copy_from_user()\r\n");
        return -1;
    }
    atomic_set(&dev->beep_value, value);
    beep_switch(value);
    return 0;
}

static long beep_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
    int ret, value;
    struct new_dev * dev = file->private_data;
    switch(cmd){
        case CMD_CLOSE:
            del_timer_sync(&dev->timer);
            gpio_set_value(dev->gpio_nm, 1);
            break;
        case CMD_OPEN:
            mod_timer(&dev->timer, atomic_read(&dev->timer_period));
            break;
        case CMD_PERIOD:
            ret = copy_from_user(&value,(int *)arg,sizeof(int));  //arg是应用传递给驱动的周期值数据首地址，长度为4个字节
            if(ret<0){
                return -EFAULT;
            }
            atomic_set(&dev->timer_period, msecs_to_jiffies(value));
            break;
        case CMD_READ:
            value = jiffies_to_msecs(atomic_read(&dev->timer_period));
            ret = copy_to_user((int *)arg, &value,sizeof(int));  //arg是应用传递给驱动的周期值数据首地址，长度为4个字节
            if(ret<0){
                return -EFAULT;
            }
            break;
        default:
            printk("[ERROR]: beep_ioctl() cmd\r\n");
    }
    return 0;
}

static int beep_close(struct inode * inode, struct file * filp){
    printk("beep close!\r\n");
    filp->private_data = 0;
    return 0;
}

static const struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .read = beep_read,
    .write = beep_write,
    .unlocked_ioctl = beep_ioctl,
    .release = beep_close,
};

void timer_func(unsigned long arg){
    static int stat = 1;
    int value = 0;
    struct new_dev *dev = (struct new_dev *)arg;
    stat = !stat;
    gpio_set_value(dev->gpio_nm,stat);
    printk("[INFO]: time is %ld\r\n", jiffies);
    value = atomic_read(&dev->timer_period);
    mod_timer(&dev->timer,jiffies + value);
}


static int beep_gpio_init(struct platform_device * p_dev){
    int ret = 0;
    struct new_dev * dev = &beep;
    atomic_set(&dev->beep_value, 0);

    //获取设备树节点
    dev->dev_nd = of_find_node_by_path("/gpiobeep");
    dev->dev_nd = p_dev->dev.of_node;
    if(dev->dev_nd == NULL){
        printk("[ERROR]: of_find_node_by_path()");
        ret = -EINVAL;
        goto fail_nd;
    }
    printk("[INFO]: find device key!\r\n");

    //获取GPIO
    dev->gpio_nm = of_get_named_gpio(dev->dev_nd, "beep-gpio", 0);
    if(dev->gpio_nm < 0){
        printk("[ERROR]: of_get_named_gpio()");
        ret = -EINVAL;
        goto fail_gpio;
    }

    //请求GPIO
    ret = gpio_request(dev->gpio_nm, DEV_NAME);
    if(ret < 0){
        printk("[ERROR]: gpio_request()");
        ret = -EINVAL;
        goto fail_request;
    }

    //配置GPIO
    ret = gpio_direction_output(dev->gpio_nm, 1);
    if(ret < 0){
        printk("[ERROR]: gpio_direction_input()");
        ret = -EINVAL;
        goto fail_set;
    }

    //软件定时器设置
    init_timer(&dev->timer);
    atomic_set(&dev->timer_period, msecs_to_jiffies(TIMER_PERIOD_MS));
    dev->timer.function = &timer_func;
    dev->timer.data = (unsigned long)dev;
    dev->timer.expires = jiffies + atomic_read(&beep.timer_period);
    printk("timer add!\r\n");
    printk("[INFO]: gpio init\r\n");

    return 0;
fail_set:
    gpio_free(dev->gpio_nm);
fail_request:
fail_gpio:
fail_nd:
    return ret;
}

static int beep_probe(struct platform_device *p_dev){
    int ret = 0;
    struct new_dev * dev = &beep;
    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        if(alloc_chrdev_region(&dev->dev_id,0,1,DEV_NAME) != 0){
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            goto fail_devid;
        }else{
            dev->major = MAJOR(dev->dev_id);
            dev->minor = MINOR(dev->dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",dev->dev_id,dev->major,dev->minor);
        }
   }else{
        //手动申请设备号
        dev->dev_id = MKDEV(DEV_MAJOR,0);                        //调用MKDEV函数构建设备号
        dev->major = MAJOR(dev->dev_id);
        dev->minor = MINOR(dev->dev_id);
        if(register_chrdev_region(dev->dev_id,1,DEV_NAME) == 0){
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            goto fail_devid;
        }else{
            dev->major = MAJOR(dev->dev_id);
            dev->minor = MINOR(dev->dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",dev->dev_id,dev->major,dev->minor);
        }
    }

    //字符设备注册
    dev->cdev.owner = THIS_MODULE;
    cdev_init(&dev->cdev, &beep_fops);
    if(cdev_add(&dev->cdev, dev->dev_id, 1)!=0){
        printk("[ERROR]: cdev_add()\r\n");
        goto fail_cdev;
    }

    //创建设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        dev->class = class_create(THIS_MODULE, DEV_NAME);
        if(IS_ERR(dev->class)){
            printk("[ERROR]: class_create()\r\n");
            goto fail_class;
        }
        dev->device = device_create(dev->class, NULL, dev->dev_id, NULL, DEV_NAME);
        if(IS_ERR(dev->device)){
            printk("[ERROR]: device_create()\r\n");
            goto fail_device;
        }
    }
    if(beep_gpio_init(p_dev) != 0){
        printk("[ERROR]: beep_gpio_init()\r\n");
        goto fail_gpio;
    }
    return ret;

fail_gpio:
    gpio_free(dev->gpio_nm);
fail_device:
    if(AUTO_NODE){
        device_destroy(dev->class, dev->dev_id);
    }
fail_class:
    if(AUTO_NODE){
        class_destroy(dev->class);
    }
fail_cdev:
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->dev_id, 1);
fail_devid:
    return ret;
}

static int beep_remove(struct platform_device * p_dev){
    struct new_dev * dev = &beep;
    gpio_free(dev->gpio_nm);
    del_timer_sync(&dev->timer);
    //卸载设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        device_destroy(dev->class, dev->dev_id);
        class_destroy(dev->class);
    }
    //卸载字符设备
    cdev_del(&dev->cdev);
    //注销设备号
    unregister_chrdev_region(dev->dev_id,1);
    return 0;
}

struct platform_driver beepdriver = {
    .driver = {
        .name = "imx6ull-beep",
        .of_match_table = beep_of_match,
    },
    .probe = beep_probe,
    .remove = beep_remove,
};

static int __init beep_init(void){
    return platform_driver_register(&beepdriver);
}

static void __exit beep_exit(void){
    platform_driver_unregister(&beepdriver);
}

module_init(beep_init);
module_exit(beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");