#include "key.h"

struct new_device key;

static int key_open(struct inode * inode, struct file * filp){
    printk("[INFO]: key_open!\r\n");
    filp->private_data = &key;
    return 0;
}

static ssize_t key_read(struct file * filp, __user char * buf, size_t count, loff_t * ppos){
    struct new_device * dev = filp->private_data;
    static int trg = 0, cont = 0;
    int ret = 0, read_data = 0;
    read_data = ~(gpio_get_value(dev->gpio_nm));
    trg = read_data & (read_data ^ cont);
    cont = read_data;
    atomic_set(&dev->atomic_data, trg);
    ret = copy_to_user(buf, &trg, sizeof(trg));
    return ret;
}

static int key_close(struct inode * inode, struct file * filp){
    printk("[INFO]: key_close!\r\n");
    filp->private_data = &key;
    return 0;  
}

static const struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .release = key_close
};

static int key_gpio_init(struct new_device *dev){
    int ret = 0;

    //获取设备树节点
    dev->dev_nd = of_find_node_by_path("/gpiokey");
    if(dev->dev_nd == NULL){
        printk("[ERROR]: of_find_node_by_path()");
        ret = -EINVAL;
        goto fail_nd;
    }
    printk("[INFO]: find device key!\r\n");

    //获取GPIO
    dev->gpio_nm = of_get_named_gpio(dev->dev_nd, "key-gpio", 0);
    if(dev->gpio_nm < 0){
        printk("[ERROR]: of_get_named_gpio()");
        ret = -EINVAL;
        goto fail_gpio;
    }

    //请求GPIO
    ret = gpio_request(dev->gpio_nm, "key0");
    if(ret < 0){
        printk("[ERROR]: gpio_request()");
        ret = -EINVAL;
        goto fail_request;
    }

    //配置GPIO
    ret = gpio_direction_input(dev->gpio_nm);
    if(ret < 0){
        printk("[ERROR]: gpio_direction_input()");
        ret = -EINVAL;
        goto fail_set;
    }
    printk("[INFO]: gpio init\r\n");
    return 0;
fail_set:
    gpio_free(dev->gpio_nm);
fail_request:
fail_gpio:
fail_nd:
    return ret;
}

static int __init key_init(void){
    int ret = 0;
    atomic_set(&key.atomic_data, 0);
    
    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        ret = alloc_chrdev_region(&key.dev_id,0,1,DEV_NAME);
   }else{
        //手动申请设备号
        key.dev_id = MKDEV(LED_MAJOR,0);                        //调用MKDEV函数构建设备号
        key.major = MAJOR(key.dev_id);
        key.minor = MINOR(key.dev_id);
        ret = register_chrdev_region(key.dev_id,1,DEV_NAME);
    }
    if(ret < 0){
        printk("[ERROR]: %s()\r\n", AUTO_REGION == 1 ? "alloc_chrdev_region" : "register_chrdev_region");
        goto fail_devid;
    }
    key.major = MAJOR(key.dev_id);
    key.minor = MINOR(key.dev_id);
    printk("[INFO]: dev_t = %d,major = %d,minor = %d!\r\n",key.dev_id,key.major,key.minor);

    //字符设备
    key.cdev.owner = THIS_MODULE;
    cdev_init(&key.cdev, &key_fops);
    ret = cdev_add(&key.cdev, key.dev_id, 1);
    if(ret < 0){
        printk("[ERROR]: cdev_add()\r\n");
        goto fail_cdev;
    }
    printk("[INFO]: cdev init!\r\n");

    //自动创建设备节点
    if(AUTO_NODE){
        //创建类
        key.class = class_create(THIS_MODULE, DEV_NAME);
        if(IS_ERR(key.class)){
            printk("[ERROR]: class_create()\r\n");
            ret = PTR_ERR(key.class);
            goto fail_class;
        }

        //创建设备
        key.device = device_create(key.class, NULL, key.dev_id, NULL, DEV_NAME);
        if(IS_ERR(key.device)){
            printk("[ERROR]: device_create()\r\n");
            ret = PTR_ERR(key.device);
            goto fail_device;
        }
    }
    printk("[INFO]: device init\r\n");

    ret = key_gpio_init(&key);
    if(ret < 0){
        goto fail_device;
    }
    return 0;

fail_device:
    if(AUTO_NODE){
        device_destroy(key.class, key.dev_id);
    }
fail_class:
    if(AUTO_NODE){
        class_destroy(key.class);
    }
fail_cdev:
    if(AUTO_REGION){
        cdev_del(&key.cdev);
        unregister_chrdev_region(key.dev_id, 1);
    }
fail_devid:
    return ret;
}

static void __exit key_exit(void){
    device_destroy(key.class, key.dev_id);
    class_destroy(key.class);
    cdev_del(&key.cdev);
    unregister_chrdev_region(key.dev_id, 1);
    gpio_free(key.gpio_nm);
}

module_init(key_init);
module_exit(key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");