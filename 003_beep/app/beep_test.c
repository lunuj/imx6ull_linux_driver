#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>

#define CMD_CLOSE               _IO(0xEF, 1)
#define CMD_OPEN                _IO(0xEF, 2)
#define CMD_PERIOD              _IOW(0xEF, 3, int)
#define CMD_READ                _IOR(0xEF, 4, int)

int main(int argc, char * argv[])
{
    unsigned int args;
    int flag = 0;
    if(argc < 3){
        if(argc < 2){
            printf("Usage: error\r\n");
            return -1;
        }else{
            flag = 1;
        }
    }

    char * filename;
    filename = argv[1];
    printf("filename: %s\r\n", filename);
    printf("modulname %s\r\n", argv[0]);
    sleep(1);
    int ret = 0;
    int f = 0;
    int buf;
    f = open(filename, O_RDWR);

    if(f < 0){
        perror("open(): error");
    }

    while(flag){
        int a = 0;
        unsigned int args = 0;
        printf("请输入指令:\r\n1:打开定时器\r\n2:关闭定时器\r\n3:修改定时器周期值\r\n4:读取定时器周期值\r\n5:退出\r\n");
        scanf("%d", &a);
        switch (a)
        {
        case 1:
            ioctl(f, CMD_OPEN, &args);
            break;
        case 2:
            ioctl(f, CMD_CLOSE, &args);
            break;
        case 3:
            printf("请输入要修改的周期值(ms):");
            scanf("%d", &args);
            ioctl(f, CMD_PERIOD, &args);
            break;
        case 4:
            ioctl(f, CMD_READ, &args);
            printf("timer period is %d\r\n", args);
            break;
        case 5:
            return 0;
        default:
            break;
        }
    }


    switch(argv[2][0]){
        case 'r':
            ret = read(f, &buf, 4);
            if(ret < 0){
                perror("read(): error");
                return -1;
            }else{
                printf("asd beep read: %d\r\n", buf);
                sleep(1);
                close(f);
                return 0;
            }
            break;

        case 'w':
            if(argc < 4){
                perror("write(): error");
                return -1;
            }else{
                buf = atoi(argv[3]);
                ret = write(f, &buf, 4);
                printf("beep write: %d\r\n", buf);
                sleep(1);
                close(f);
                return 0;
            }
            break;
        
        case 'b':
            ioctl(f, CMD_OPEN, &args);
            break;
        case 'e':
            ioctl(f, CMD_CLOSE, &args);
            break;
        case 'c':
            if(argc < 4){
                perror("ioctl(): CMD_PERIOD error");
                return -1;
            }
            args = atoi(argv[3]);
            ioctl(f, CMD_PERIOD, &args);
            break;
        default:
            printf("Usage: error\r\n");
            break;
    }

    return 0;
}