/*
 * @Author: lunuj 2653050252@qq.com
 * @Date: 2025-01-18 01:22:28
 * @LastEditors: lunuj 2653050252@qq.com
 * @LastEditTime: 2025-01-19 15:03:33
 * @FilePath: /imx6ull_linux_driver/008_key_input/module/keyinput.c
 * @Description: 输入子系统
 * 
 * Copyright (c) 2025 by lunuj, All Rights Reserved. 
 */

#include "keyinput.h"

struct new_dev input_key;

static irqreturn_t key0_handle_irq(int irq, void * dev_id)
{
    struct new_dev * dev = dev_id;
    dev->timer.data = (unsigned long)dev_id;
    mod_timer(&dev->timer, jiffies+msecs_to_jiffies(10));
    return IRQ_HANDLED;
}

void timer_func(unsigned long arg)
{
    int value = 0;
    struct new_dev * dev = (struct new_dev *)arg;
    value = gpio_get_value(dev->irqkey.gpio);
    input_event(dev->input_dev, EV_KEY, KEY_0, (value==0) ? 1 : 0);
    input_sync(dev->input_dev);
}

static int dev_gpio_init(struct new_dev * dev){
    int ret = 0;
    dev->dev_nd = of_find_node_by_path("/gpiokey");
    if(dev->dev_nd == NULL){
        printk("[ERROR]: of_find_node_by_path()\r\n");
        goto fail_nd;
    }

    dev->irqkey.gpio = of_get_named_gpio(dev->dev_nd, "key-gpio", 0);
    if(dev->irqkey.gpio < 0){
        printk("[ERROR]: of_get_named_gpio()\r\n");
        goto fail_gpio_get;
    }

    ret = gpio_request(dev->irqkey.gpio,dev->irqkey.name);
    if(ret < 0){
        ret = -EINVAL;
        goto fail_gpio_request;
    }

    gpio_direction_input(dev->irqkey.gpio);
    dev->irqkey.irqnum = gpio_to_irq(dev->irqkey.gpio);


    memset(dev->irqkey.name, 0, sizeof(dev->irqkey.name));
    sprintf(dev->irqkey.name, "KEY0");
    ret = request_irq(dev->irqkey.irqnum,
                        key0_handle_irq,
                        IRQ_TYPE_EDGE_RISING|IRQ_TYPE_EDGE_FALLING,
                        dev->irqkey.name,
                        dev
                        );
    if(ret){
        printk("[ERROR]: request_irq()\r\n");
        goto fail_gpio_irq;
    }

    init_timer(&dev->timer);
    dev->timer.function = timer_func;

    return 0;

fail_gpio_irq:
fail_gpio_request:
fail_gpio_get:
fail_nd:
    return ret;
}

static int __init key_init(void){
    int ret = 0;
    ret = dev_gpio_init(&input_key);
    if(ret < 0){
        goto fail_gpio_init;
    }

    input_key.input_dev = input_allocate_device();
    if(input_key.input_dev == NULL){
        ret = -ENAVAIL;
        goto fail_input_allocate;
    }

    input_key.input_dev->name = DEVICE_NAME;
    __set_bit(EV_KEY, input_key.input_dev->evbit);
    __set_bit(EV_REP, input_key.input_dev->evbit);
    __set_bit(KEY_0, input_key.input_dev->keybit);

    ret = input_register_device(input_key.input_dev);
    if(ret){
        goto fail_input_register;
    }
    return ret;
fail_input_register:
    input_free_device(input_key.input_dev);
fail_input_allocate:
fail_gpio_init:
    return ret;
}

static void __exit key_exit(void){
    free_irq(input_key.irqkey.irqnum, &input_key);

    gpio_free(input_key.irqkey.gpio);

    del_timer_sync(&input_key.timer);

    input_unregister_device(input_key.input_dev);
    input_free_device(input_key.input_dev);
}

module_init(key_init);
module_exit(key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");