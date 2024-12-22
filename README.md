# 项目说明
此项目基于正点原子imx6ull_emmc_512开发版，根据正点原子官方讲解，参考网络上文章写出。
- linux版本：4.1.15
- busy版本：1.29.0
- gcc版本：
    - ubuntu下使用linaro提供的arm-linux-gnueabihf-gcc-4.9.4
    - macOS(m1)下使用crosstool-ng编译生成的gcc 4.9.4 (crosstool-NG 1.26.0) 

# 目录结构
.demo                               //项目文件  
├── app                             //测试程序文件夹  
│   └── Makefile                    //测试程序子Makefile  
├── module                          //模块文件夹  
│   └── Makefile                    //模块子Makefile  
└── Makefile                        //工程总Makefile  

# 使用说明
1. 在总Makefile中指定kernel和rootfs路径
2. 指定编译模块名称MODULES_NAME
3. 使用make指定编译内容

## make使用
使用说明
- build:编译modules和app
- modules:只编译modules生成.ko
- app:只编译app生成.out
- cp:拷贝.ko模块到linux模块路径
- clean:清除编译生成文件

# 注意事项
1. 有些项目需修改设备树，linux源码相关修改未放出
2. 各个模块可能有特殊处理，Makefile略有不同