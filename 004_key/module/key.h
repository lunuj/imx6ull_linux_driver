#ifndef KEY_H
#define KEY_H

//内核
#include <linux/kernel.h>
//模块
#include <linux/module.h>
//初始化
#include <linux/init.h>
//文件系统
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
//设备
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

//内核软件定时器
#include <linux/timer.h>

//并发控制
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ide.h>

#define DEV_NAME                "key"
#define KEY_NUM                 1
#define KEY_MODE                0           //0：在irq中启用定时器
                                            //1：在tasklet中启用定时器
                                            //2：在work中启用定时器

#define APP_MODE                2           //0：不启用阻塞访问控制
                                            //1：使用等待队列头进行阻塞访问
                                            //2：使用等待队列进行阻塞访问
                                            //3：使用select进行非阻塞访问
                                            //4：使用信号进行异步访问

#define KEY0VALUE               0X00        // 按键值
#define AUTO_REGION             1           //是否自动申请设备号
#define AUTO_NODE               1

#define TIMER_PERIOD            10

#if defined(AUTO_REGION)
#define LED_MAJOR	200
#define DTSLED_CNT  1       //设备号个数
#endif

/**
 * @brief 按键中断结构体 
 * 
 */
struct irq_key {
    int gpio;                           //io编号
    int irqnum;                         //中断号
    atomic_t value;                          //键值
    char name[10];                      //按键名字
    irqreturn_t (*handler)(int,void*);  //中断处理函数
    struct tasklet_struct tasklet;      //下半部处理函数    
    struct work_struct work;
};

struct new_device{
    dev_t dev_id;                       //设备号

    int major;                          //主设备号
    int minor;                          //次设备号

    struct cdev cdev;                   //字符设备
    struct class *class;
    struct device *device;

    struct timer_list timer;            //内核软件定时器

    struct device_node *dev_nd;         //设备节点

    struct irq_key irqkey[KEY_NUM];

    wait_queue_head_t r_wait;
};


#endif // KEY_H