#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define APP_MODE                4           //0：不启用阻塞访问控制
                                            //1：使用等待队列头进行阻塞访问
                                            //2：使用等待队列进行阻塞访问
                                            //3：使用select进行非阻塞访问
                                            //4：使用信号进行异步访问

int f = 0;

static void sigio_singal_func(int num)
{
    int value;
    read(f, &value, sizeof(value));
    if(value == 3){
        printf("key down\r\n");
    }else if(value == 1){
        printf("key on\r\n");
    }else if(value == 2){
        printf("key up\r\n");
    }else{
        printf("key off\r\n");
    }
}

int main(int argc, char * argv[])
{
    unsigned int args;
    int flags = 0;

    fd_set readfds;
    struct timeval timeout;

    if(argc < 2){
        printf("Usage: error\r\n");
        return -1;
    }else{
        flags = 1;
    }

    char * filename;
    filename = argv[1];
    printf("filename: %s\r\n", filename);
    printf("modulname %s\r\n", argv[0]);
    sleep(1);
    int ret = 0;
    unsigned char value = 0;
    int buf;
    f = open(filename, O_RDWR|O_NONBLOCK);

    if(f < 0){
        perror("open(): error");
        return -1;
    }

    switch (APP_MODE)
    {
    case 3:
        FD_ZERO(&readfds);
        FD_SET(f,&readfds);
    case 4:
        signal(SIGIO,sigio_singal_func);
        fcntl(f,F_SETOWN,getpid());     //设置当前进程接收信号
        flags =fcntl(f,F_GETFL);
        fcntl(f,F_SETFL,flags|FASYNC);  //开启异步通知
    default:
        break;
    }


    while(1){
        switch (APP_MODE)
        {
        case 3:
            timeout.tv_sec = 10;
            timeout.tv_usec = 500000;    //500ms
            ret = select(f+1,&readfds,NULL,NULL,&timeout);
            printf("[INFO]: ret value = %d!\r\n",ret);
            switch(ret){
                case 0:  //超时
                    printf("[INFO]: timeout!\r\n");
                    close(f);
                    return;
                case -1: //错误
                    printf("[INFO]: error!\r\n");
                    return;
                default:
                    printf("[INFO]: read start!\r\n");
                    //可以读数
                    if(FD_ISSET(f,&readfds)){
                        read(f, &value, sizeof(value));
                        if(value == 3){
                            printf("[INFO]: key down\r\n");
                        }
                        if(value == 2){
                            printf("[INFO]: key up\r\n");
                        }
                    }
                    break;
            }
            break;
        case 4:
            sleep(0);
            break;
        default:
            while(1){
                read(f, &value, sizeof(value));
                if(value == 3){
                    printf("key down\r\n");
                    while(1){
                        read(f, &value, sizeof(value));
                        if(value == 2){
                            printf("key up\r\n");
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    close(f);
    return 0;
}