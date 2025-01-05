#ifndef BEEPDRIVER_H
#define BEEPDRIVER_H

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
#include <linux/fcntl.h>
#include <linux/ide.h>
#include <linux/platform_device.h>

#define DEV_NAME                "beep"
#define KEY_NUM                 1

#define TIMER_PERIOD_MS         1000

#define KEY0VALUE               0X00        // 按键值
#define AUTO_REGION             1           //是否自动申请设备号
#define AUTO_NODE               1

#if defined(AUTO_REGION)
#define DEV_MAJOR	            200
#define DEV_CNT                 1       //设备号个数
#endif

#define CMD_CLOSE               _IO(0xEF, 1)
#define CMD_OPEN                _IO(0xEF, 2)
#define CMD_PERIOD              _IOW(0xEF, 3, int)
#define CMD_READ                _IOR(0xEF, 4, int)

struct new_dev {
    dev_t dev_id;
    int major;
    int minor;

    struct cdev cdev;
    struct class * class;
    struct device * device;
    struct device_node *dev_nd;

    int gpio_nm;

    atomic_t beep_value;

    struct timer_list timer;
    atomic_t timer_period;               //添加原子变量，存放周期值
};


#endif // BEEPDRIVER_H