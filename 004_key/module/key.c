#include "key.h"

struct new_device key;

static int key_open(struct inode * inode, struct file * filp){
    printk("[INFO]: key_open!\r\n");
    filp->private_data = &key;
    return 0;
}

static ssize_t key_read(struct file * filp, __user char * buf, size_t count, loff_t * ppos){
    struct new_device * dev = filp->private_data;
    int ret = 0,value = 0;
    DECLARE_WAITQUEUE(wait, current);
    switch (APP_MODE)
    {
    case 1:
        wait_event_interruptible(dev->r_wait, atomic_read(&dev->irqkey[0].value));
        break;
    case 2:
        add_wait_queue(&dev->r_wait, &wait);
        __set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        __set_current_state(TASK_RUNNING);
        remove_wait_queue(&dev->r_wait, &wait);
        if(signal_pending(current)){
            return -ERESTARTSYS;
        }
    default:
        break;
    }
    value = atomic_read(&dev->irqkey[0].value);
    ret = copy_to_user(buf, &value, sizeof(value));
    return ret;
}

static unsigned int key_poll(struct file * filp, struct poll_table_struct * wait){
    int mask = 0;
    static int cont = 0;
    struct new_device * dev = filp->private_data;
    printk("cont = %d\r\n", cont++);
    poll_wait(filp, &dev->r_wait, wait);
    if(atomic_read(&dev->irqkey[0].value)){
        mask = POLLIN | POLLRDNORM;
    }
    return mask;
}

static int key_fsync(int fd, struct file * filp, int on){
    struct new_device * dev  = filp.private_data;
    return fasync_helper(fd, filp, on, &dev->fasync_queue);
}

static int key_close(struct inode * inode, struct file * filp){
    printk("[INFO]: key_close!\r\n");
    filp->private_data = &key;
    key_fsync(-1,filp,0);
    return 0;  
}

static const struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .poll = key_poll,
    .fsync = key_fsyn,
    .release = key_close
};

static irqreturn_t key0_handle_irq(int irq, void *dev_id)
{
    struct new_device *dev = dev_id;
    switch(KEY_MODE){
        case 1:
            tasklet_schedule(&dev->irqkey[0].tasklet);
            break;
        case 2:
            schedule_work(&dev->irqkey[0].work);
            break;
        default:
            key.timer.data = (uint32_t)&key;
            mod_timer(&key.timer, jiffies + msecs_to_jiffies(TIMER_PERIOD));
            break;
    }
    return IRQ_HANDLED;
}

static void key0_tasklet_handle(unsigned long arg){
    struct new_device *dev = (struct new_device *)arg;
    dev->timer.data = (uint32_t)arg;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(TIMER_PERIOD));
}

static void key0_work_handle(struct work_struct * work){
    key.timer.data = (uint32_t)&key;
    mod_timer(&key.timer, jiffies + msecs_to_jiffies(TIMER_PERIOD));
}

static int key_gpio_init(struct new_device *dev){
    int ret = 0;
    int i = 0;

    //获取设备树节点
    dev->dev_nd = of_find_node_by_path("/gpiokey");
    if(dev->dev_nd == NULL){
        printk("[ERROR]: of_find_node_by_path()");
        ret = -EINVAL;
        goto fail_nd;
    }
    printk("[INFO]: find device key!\r\n");

    for(i = 0; i < KEY_NUM; i++){
        //获取GPIO
        dev->irqkey[i].gpio = of_get_named_gpio(dev->dev_nd, "key-gpio", 0);
        if(dev->irqkey[i].gpio < 0){
            printk("[ERROR]: of_get_named_gpio()");
            ret = -EINVAL;
            goto fail_gpio;
        }

        //请求GPIO
        ret = gpio_request(dev->irqkey[i].gpio, dev->irqkey[i].name);
        if(ret < 0){
            printk("[ERROR]: gpio_request()");
            ret = -EINVAL;
            goto fail_request;
        }

        //配置GPIO
        ret = gpio_direction_input(dev->irqkey[i].gpio);
        if(ret < 0){
            printk("[ERROR]: gpio_direction_input()");
            ret = -EINVAL;
            goto fail_set;
        }

        dev->irqkey[i].irqnum = gpio_to_irq(dev->irqkey[i].gpio);
    }

    dev->irqkey[0].handler = key0_handle_irq;
    atomic_set(&dev->irqkey[0].value, KEY0VALUE);
    switch (APP_MODE)
    {
    case 3:
    case 2:
    case 1:
        init_waitqueue_head(&dev->r_wait);
        break;
    default:
        break;
    }
    
    switch (KEY_MODE)
    {
        case 1:
            tasklet_init(&dev->irqkey[0].tasklet, key0_tasklet_handle, (unsigned long)dev);
            break;
        case 2:
            INIT_WORK(&dev->irqkey[0].work, key0_work_handle);
            break;
        default:
            break;
    }

    for(i=0;i<KEY_NUM;i++){
        memset(dev->irqkey[i].name,0,sizeof(dev->irqkey[i].name));
        sprintf(dev->irqkey[i].name,"KEY%d",i);  //将格式化数据写入字符串中
        ret = request_irq(dev->irqkey[i].irqnum,                            //中断号
                            key0_handle_irq,                                //中断处理函数
                            IRQ_TYPE_EDGE_RISING|IRQ_TYPE_EDGE_FALLING,     //中断处理函数
                            dev->irqkey[i].name,                            //中断名称
                            dev                                             //设备结构体
                            );
        if(ret){
            printk("[ERROR]: irq %d request err\r\n",dev->irqkey[i].irqnum);
            goto fail_irq;
        }
    }
    printk("[INFO]: gpio init\r\n");
    return 0;
fail_irq:
fail_set:
    for(i = 0; i < KEY_NUM; i++){
        gpio_free(dev->irqkey[i].gpio);
    }
fail_request:
fail_gpio:
fail_nd:
    return ret;
}

void timer_func(unsigned long arg){
    struct new_device * dev = (struct new_device *)arg;
    static int trg = 0, cont = 0;
    int read_data = 0;
    read_data = ~gpio_get_value(dev->irqkey[0].gpio) + 2;
    trg = read_data & (read_data ^ cont);
    cont = read_data;
    if(trg == 1 && cont == 1){
        printk("[INFO]: key0 push!\r\n");
    }else if(trg ==0 && cont == 0){
        printk("[INFO]: key0 released\r\n");
    }
    atomic_set(&dev->irqkey[0].value, (cont&0x1) + ((trg&0x1) << 1));
    switch (APP_MODE)
    {
    case 2:
    case 1:
        wake_up(&dev->r_wait);
        break;
    case 4:
        kill_fasync(&dev->fasync_queue,SIGIO,POLL_IN);
    default:
        break;
    }
}

static int __init key_init(void){
    int ret = 0;
    
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

    //初始化定时器
    init_timer(&key.timer);
    key.timer.function = &timer_func;

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
    int i = 0;
    del_timer_sync(&key.timer);
    //释放中断
    for(i=0;i<KEY_NUM;i++){
        free_irq(key.irqkey[i].irqnum,&key);
    }

    for(i=0;i<KEY_NUM;i++){
        gpio_free(key.irqkey[i].gpio);
    }
    device_destroy(key.class, key.dev_id);
    class_destroy(key.class);
    cdev_del(&key.cdev);
    unregister_chrdev_region(key.dev_id, 1);
}

module_init(key_init);
module_exit(key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");