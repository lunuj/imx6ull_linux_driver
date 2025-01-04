#ifndef LEDDRIVER_H
#define LEDDRIVER_H

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

#define DEV_NAME                "platform_led"
#define DEV_CNt                 1

#define AUTO_REGION             1
#ifndef AUTO_REGION
#define DEV_MAJOR               240
#endif
#define AUTO_NODE               1

static void __iomem *IMX6UL_CCM_CCGR1;
static void __iomem *IMX6UL_SW_MUX_GPIO1_IO03;
static void __iomem *IMX6UL_SW_PAD_GPIO1_IO03;
static void __iomem *IMX6UL_GPIO1_GDIR;
static void __iomem *IMX6UL_GPIO1_DR;

struct new_dev {
    dev_t dev_id;
    int major;
    int minjor;

    struct device_node * device_node;
    struct cdev cdev;

    struct class * class;
    struct device * device;

    int dev_gpio;
    atomic_t gpio_value;
};


#endif // LEDDRIVER_H