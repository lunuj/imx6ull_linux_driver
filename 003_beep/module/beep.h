#ifndef BEEP_H
#define BEEP_H

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
#include <linux/timer.h>

//并发控制
#include <linux/atomic.h>
#include <linux/mutex.h>

#define DEV_NAME                "beep"

#define AUTO_REGION             1           //是否自动申请设备号
#define AUTO_NODE               1

#define CMD_CLOSE               _IO(0xEF, 1)
#define CMD_OPEN                _IO(0xEF, 2)
#define CMD_PERIOD              _IOW(0xEF, 3, int)
#define CMD_READ                _IOR(0xEF, 4, int)

#define TIMER_PERIOD_MS         1000

#if defined(AUTO_REGION)
#define LED_MAJOR	200
#define DTSLED_CNT  1       //设备号个数
#endif

struct new_device{
    struct cdev cdev;   //字符设备
    dev_t dev_id;       //设备号
    int major;          //主设备号
    int minor;          //次设备号
    struct class *class;
    struct device *device;
    struct device_node *dev_nd;
    int gpio_nm;
    struct timer_list timer;
    atomic_t atomic_data;       //添加原子变量，存放周期值
    struct mutex mut;
};

#endif // BEEP_H

