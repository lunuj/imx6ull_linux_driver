#ifndef LEDMISC_H
#define LEDMISC_H

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
#include <linux/miscdevice.h>

#define MISC_MINOR              240
#define MISAC_NAME              "led_miscdevice"
struct new_dev {
    struct device_node *dev_nd;
    struct miscdevice * misc_dev;
    int gpio_nm;
    atomic_t value;
};

#endif // LEDMISC_H