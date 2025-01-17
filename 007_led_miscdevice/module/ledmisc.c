#include "ledmisc.h"

struct new_dev led;

static int led_open(struct inode * inode, struct file * filp){
    filp->private_data = &led;
    return 0;
}
static ssize_t led_read(struct file * filp, __user char * buf, size_t count, loff_t * ppos){
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    uint8_t value = atomic_read(&dev->value);
    if(copy_to_user(buf, &value, sizeof(value)) != 0){
        printk("[ERROR]: copy_to_user()\r\n");
        return -1;
    }
    return 0;
}
static void led_switch(uint8_t sta){
    if(sta == 1){
        gpio_set_value(led.gpio_nm, 1);
    }else if(sta == 0){
        gpio_set_value(led.gpio_nm, 0);
    }
}

static int led_write(struct file * filp, const char __user * buf, size_t count, loff_t * ppos){
    uint8_t value = 0;
    struct new_dev * dev = (struct new_dev *)filp->private_data;
    if(copy_from_user(&value, buf, sizeof(value)) != 0){
        printk("[ERROR]: copy_from_user()\r\n");
        return -1;
    }
    atomic_set(&dev->value, value);
    led_switch(value);
    return 0;
}

static int led_release(struct inode * inode, struct file * filp){
    printk("led close!\r\n");
    filp->private_data = 0;
    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static struct miscdevice led_misc = {
    .minor = MISC_MINOR,
    .name = MISAC_NAME,
    .fops = &led_fops,
};

static int led_probe(struct platform_device * p_dev){
    int ret = 0;
    struct new_dev * dev = &led;
    atomic_set(&dev->value, 0);

    //获取设备树节点
    dev->dev_nd = p_dev->dev.of_node;
    //获取GPIO
    dev->gpio_nm = of_get_named_gpio(dev->dev_nd, "led-gpio", 0);
    if(dev->gpio_nm < 0){
        printk("[ERROR]: of_get_named_gpio()");
        ret = -EINVAL;
        goto fail_gpio;
    }
    //请求GPIO
    ret = gpio_request(dev->gpio_nm, "led_gpio");
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
    printk("[INFO]: gpio init\r\n");
    dev->misc_dev = &led_misc;
    ret = misc_register(dev->misc_dev);                             //注册misc设备驱动
    if(ret<0){
        goto fail_set;
    }

    return 0;
fail_set:
    gpio_free(dev->gpio_nm);
fail_request:
fail_gpio:
    return ret;
}

static int led_remove(struct platform_device * p_dev){
    struct new_dev * dev = &led;
    misc_deregister(dev->misc_dev);
    gpio_free(dev->gpio_nm);
    return 0;
}

struct of_device_id led_of_match[] = {
    {
        .compatible = "atkalpha-gpioled",
    },
    {/*Sentinel*/},
};

struct platform_driver platform_leddriver = {
    .driver = {
        .name = "imx6ull-led",
        .of_match_table = led_of_match,
    },
    .probe = led_probe,
    .remove = led_remove,
};

module_platform_driver(platform_leddriver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");