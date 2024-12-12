#ifndef LED_H
#define LED_H

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


#define DEV_NAME                "led"
#define LED_ON                  1
#define LED_OFF                 0

#define AUTO_REGION             1           //是否自动申请设备号
#define AUTO_NODE               1

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
};

/**
 * @brief 待使用的寄存器物理地址,使用设备树后从设备树中获取
 * 
 */
// #define CCM_CCGR1_BASE          (0X020C406C)
// #define SW_MUX_GPIO1_IO03_BASE  (0X020E0068)
// #define SW_PAD_GPIO1_IO03_BASE  (0X020E02F4)
// #define GPIO1_GDIR_BASE         (0X0209C004)
// #define GPIO1_DR_BASE           (0X0209C000)




#endif // LED_H

