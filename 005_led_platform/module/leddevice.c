#include "leddevice.h"

void leddevice_release(struct device *dev)
{
    printk("device release\r\n");
}

struct platform_device leddevice = {
    .name = "imx6ull-led",
    .id = -1,
    .dev = {
        .release = leddevice_release,
    },
    .num_resources = ARRAY_SIZE(leddevice_resource),     //资源大小
    .resource = leddevice_resource
};

static int __init leddevice_init(void){
    //注册platform设备
    return platform_device_register(&leddevice);
}

static void __exit leddevice_exit(void){
    platform_device_unregister(&leddevice);
}

module_init(leddevice_init);
module_exit(leddevice_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lunuj");