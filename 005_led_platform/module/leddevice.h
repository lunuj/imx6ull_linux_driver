#ifndef LEDDEVICE_H
#define LEDDEVICE_H

#include <linux/io.h>
#include <linux/device.h>

/**
 * @brief 寄存器物理地址
 *
 */
#define CCM_CCGR1_BASE          (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE  (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE  (0X020E02F4)
#define GPIO1_GDIR_BASE         (0X0209C004)
#define GPIO1_DR_BASE           (0X0209C000)

#define REGSITER_LENGTH         4

//5个内存段
static struct resource leddevice_resource[] = {
    [0] = {
        .start = CCM_CCGR1_BASE,
        .end   = CCM_CCGR1_BASE + REGSITER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = SW_MUX_GPIO1_IO03_BASE,
        .end   = SW_MUX_GPIO1_IO03_BASE + REGSITER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = SW_PAD_GPIO1_IO03_BASE,
        .end   = SW_PAD_GPIO1_IO03_BASE + REGSITER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [3] = {
        .start = GPIO1_GDIR_BASE,
        .end   = GPIO1_GDIR_BASE + REGSITER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [4] = {
        .start = GPIO1_DR_BASE,
        .end   = GPIO1_DR_BASE + REGSITER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
};
#endif // LEDDEVICE_H
