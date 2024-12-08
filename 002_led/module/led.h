#ifndef LED_H
#define LED_H

#include <linux/io.h>

#define LED_MAJOR	200

#define DEV_NAME                "led"
#define LED_ON                  1
#define LED_OFF                 0


struct LED_REG{
    uint32_t reg_address;
    uint32_t reg_data;
};

struct new_device{
    struct cdev cdev;   //字符设备
    dev_t dev_id;       //设备号
    struct class *class;
    struct device *device;
    int major;          //主设备号
    int minor;          //次设备号
};

/**
 * @brief 待使用的寄存器物理地址
 * 
 */
#define CCM_CCGR1_BASE          (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE  (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE  (0X020E02F4)
#define GPIO1_GDIR_BASE         (0X0209C004)
#define GPIO1_DR_BASE           (0X0209C000)


/**
 * @brief 映射后的虚拟地址
 * 
 */
static void __iomem *IMX6UL_CCM_CCGR1;              //映射后的虚拟地址
static void __iomem *IMX6UL_SW_MUX_GPIO1_IO03;      
static void __iomem *IMX6UL_SW_PAD_GPIO1_IO03;
static void __iomem *IMX6UL_GPIO1_DR;
static void __iomem *IMX6UL_GPIO1_GDIR;

#endif // LED_H

