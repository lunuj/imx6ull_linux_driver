/*
 * @Author: lunuj 2653050252@qq.com
 * @Date: 2025-01-18 01:22:34
 * @LastEditors: lunuj 2653050252@qq.com
 * @LastEditTime: 2025-01-19 15:03:23
 * @FilePath: /imx6ull_linux_driver/008_key_input/module/keyinput.h
 * @Description: input模块头文件
 * 
 * Copyright (c) 2025 by lunuj, All Rights Reserved. 
 */
#ifndef KEYINPUT_H
#define KEYINPUT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/input.h>

#define DEVICE_NAME                     "input_key"

struct irq_key{
    int gpio;                           //io编号
    int irqnum;                         //中断号
    char name[10];                      //中断名称
    irqreturn_t (*handle)(int,void*);   //中断处理函数
};

struct new_dev{
    struct device_node * dev_nd;
    
    struct irq_key irqkey;

    struct timer_list timer;
    atomic_t timer_per;

    struct input_dev * input_dev;
};

#endif // KEYINPUT_H