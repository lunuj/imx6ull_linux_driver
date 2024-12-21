#include "beep.h"
#include "myerror.h"

static int kernel_data;
static uint16_t ret_val;
struct new_device beep;
struct cdev beep_cdev;

static int beep_open(struct inode * inode, struct file * filp){
    if(!mutex_trylock(&beep.mut)){
        return -EBUSY;
    }
    filp->private_data = &beep;
    printk("[INFO]: beep open\r\n");
    return 0;
}
static int beep_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
    // struct new_device *dev = file->private_data;
    // kernel_data = gpio_get_value(dev->gpio_nm);
    if(copy_to_user(buf, &kernel_data, 4) != 0){
        printk("[ERROR]: copy_to_user()\r\n");
        SET_ERROR(ret_val, ERROR_LED_READ);
    }
    printk("[INFO]: beep read %d\r\n", kernel_data);
    return ret_val;
}
static int beep_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    struct new_device *dev = file->private_data;
    if(copy_from_user(&kernel_data, buf, 4)!=0){
        printk("[ERROR]: copy_from_user()\r\n");
        SET_ERROR(ret_val, ERROR_LED_READ);
    }
    if(IS_OK(ret_val)){
        gpio_set_value(dev->gpio_nm, kernel_data);
    }

    printk("[INFO]: beep write %d\r\n", kernel_data);
    return ret_val;
}
static int beep_close(struct inode * inode, struct file * filp){
    mutex_unlock(&beep.mut);
    printk("[INFO]: beep close\r\n");
    return 0;
}
static long beep_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
    int ret, value;
    struct new_device *dev = file->private_data;
    switch(cmd){
        case CMD_CLOSE:
            del_timer_sync(&dev->timer);
            gpio_set_value(dev->gpio_nm, 1);
            break;
        case CMD_OPEN:
            mod_timer(&dev->timer, atomic_read(&dev->atomic_data));
            break;
        case CMD_PERIOD:
            ret = copy_from_user(&value,(int *)arg,sizeof(int));  //arg是应用传递给驱动的周期值数据首地址，长度为4个字节
            if(ret<0){
                return -EFAULT;
            }
            atomic_set(&dev->atomic_data, msecs_to_jiffies(value));
            break;
        case CMD_READ:
            value = jiffies_to_msecs(atomic_read(&dev->atomic_data));
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

static const struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .read = beep_read,
    .write = beep_write,
    .release = beep_close,
    .unlocked_ioctl = beep_ioctl
};

static int devicetree_init(void){
    const char *str;

    beep.dev_nd = of_find_node_by_path("/gpiobeep");
    if(beep.dev_nd == NULL){
        SET_ERROR(ret_val, ERROR_LED_FINDNODE);
        printk("[ERROR]: of_find_node_by_path()\r\n");
        return ret_val;
    }

    beep.gpio_nm = of_get_named_gpio(beep.dev_nd, "beep-gpio", 0);
    if(beep.gpio_nm < 0){
        SET_ERROR(ret_val, ERROR_LED_OFREADSTRING); 
        printk("[ERROR]: of_get_named_gpio %d\r\n", beep.gpio_nm);
        return ret_val;
    }
    if(of_property_read_string(beep.dev_nd, "status", &str)!=0){
        SET_ERROR(ret_val, ERROR_LED_OFREADSTRING);
        printk("[ERROR]: of_property_read_string\r\n");
        return ret_val;
    }
    if(gpio_direction_output(beep.gpio_nm, 1)){
        SET_ERROR(ret_val, ERROR_LED_OFREADSTRING);
        printk("[ERROR]: of_property_read_string\r\n");
        return ret_val;
    }

    return ret_val;
}

static void beep_error_resolve(void){
    if(!IS_ERROR(ret_val, ERROR_LED_DEVICE)){
        if(!IS_ERROR(ret_val, ERROR_LED_CLASS)){
            if(!IS_ERROR(ret_val, ERROR_LED_CDEVADD)){
                if(!IS_ERROR(ret_val, ERROR_LED_ALLOCREG)){
                }else{
                    unregister_chrdev_region(beep.dev_id,1);
                }
            }else{
                cdev_del(&beep.cdev);
            }
        }else{
            class_destroy(beep.class);
        }
    }else{
        device_destroy(beep.class, beep.dev_id);
    }
}

void timer_func(unsigned long arg){
    static int stat = 1;
    int value = 0;
    struct new_device *dev = (struct new_device*)arg;
    stat = !stat;
    gpio_set_value(dev->gpio_nm,stat);
    printk("time is %ld\r\n", jiffies);
    value = atomic_read(&dev->atomic_data);
    mod_timer(&dev->timer,jiffies + value);
}

static int __init beep_init(void)
{
    int ret = 0;
    kernel_data = 1;
    devicetree_init();
    mutex_init(&beep.mut);
    //申请设备号
    if(AUTO_REGION){
        //自动申请设备号
        if(alloc_chrdev_region(&beep.dev_id,0,1,DEV_NAME) != 0){
            SET_ERROR(ret_val, ERROR_LED_ALLOCREG);
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            beep_error_resolve();
            return ret_val;
        }else{
            beep.major = MAJOR(beep.dev_id);
            beep.minor = MINOR(beep.dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",beep.dev_id,beep.major,beep.minor);
        }
   }else{
        //手动申请设备号
        beep.dev_id = MKDEV(LED_MAJOR,0);                        //调用MKDEV函数构建设备号
        beep.major = MAJOR(beep.dev_id);
        beep.minor = MINOR(beep.dev_id);
        if(register_chrdev_region(beep.dev_id,1,DEV_NAME) == 0){
            SET_ERROR(ret_val, ERROR_LED_ALLOCREG);
            printk("[ERROR]: alloc_chrdev_region()\r\n");
            beep_error_resolve();
            return ret_val;
        }else{
            beep.major = MAJOR(beep.dev_id);
            beep.minor = MINOR(beep.dev_id);
            printk("dev_t = %d,major = %d,minor = %d\r\n",beep.dev_id,beep.major,beep.minor);
        }
    }

    //字符设备注册
    beep_cdev.owner = THIS_MODULE;
    cdev_init(&beep_cdev, &beep_fops);
    if(cdev_add(&beep_cdev, beep.dev_id, 1)!=0){
        SET_ERROR(ret_val, ERROR_LED_CDEVADD);
        printk("[ERROR]: cdev_add()\r\n");
        beep_error_resolve();
        return ret_val;
    }

    //创建设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        beep.class = class_create(THIS_MODULE, DEV_NAME);
        if(IS_ERR(beep.class)){
            SET_ERROR(ret_val, ERROR_LED_CLASS);
            printk("[ERROR]: class_create()\r\n");
            beep_error_resolve();
            return ret_val;
        }
        beep.device = device_create(beep.class, NULL, beep.dev_id, NULL, DEV_NAME);
        if(IS_ERR(beep.device)){
            SET_ERROR(ret_val, ERROR_LED_DEVICE);
            printk("[ERROR]: device_create()\r\n");
            beep_error_resolve();
            return ret_val;
        }
    }

    //软件定时器设置
    init_timer(&beep.timer);
    atomic_set(&beep.atomic_data, msecs_to_jiffies(TIMER_PERIOD_MS));
    beep.timer.function = &timer_func;
    beep.timer.data = (unsigned long)&beep;
    beep.timer.expires = jiffies + atomic_read(&beep.atomic_data);
    printk("timer add!\r\n");

    return ret;
}

static void __exit beep_exit(void)
{
    del_timer_sync(&beep.timer);
    //卸载设备节点
    if(AUTO_NODE){
        //自动创建设备节点
        device_destroy(beep.class, beep.dev_id);
        class_destroy(beep.class);
    }
    //卸载字符设备
    cdev_del(&beep_cdev);
    //注销设备号
    unregister_chrdev_region(beep.dev_id,1);
}

module_init(beep_init);
module_exit(beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");