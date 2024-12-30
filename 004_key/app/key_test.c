#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
int f = 0;

static void sigio_singal_func(int num)
{
    int value;
    read(f, &value, sizeof(value));
    if(value == 3){
        printf("key trg down\r\n");
        printf("key on\r\n");
    }else{
        printf("error\r\n");
    }
}

int main(int argc, char * argv[])
{
    unsigned int args;
    int flag = 0;

    fd_set readfds;
    struct timeval timeout;

    if(argc < 2){
        printf("Usage: error\r\n");
        return -1;
    }else{
        flag = 1;
    }

    char * filename;
    filename = argv[1];
    printf("filename: %s\r\n", filename);
    printf("modulname %s\r\n", argv[0]);
    sleep(1);
    int ret = 0;
    int buf;
    f = open(filename, O_RDWR|O_NONBLOCK);

    if(f < 0){
        perror("open(): error");
        return -1;
    }

    signal(SIGIO,sigio_singal_func);
    fcntl(f,F_SETOWN,getpid());     //设置当前进程接收信号
    flags =fcntl(f,F_GETFL);
    fcntl(f,F_SETFL,flags|FASYNC);  //开启异步通知
    while(1){
        sleep(2);
    }
    int value = 0;
    while(1){
        FD_ZERO(&readfds);
        FD_SET(f,&readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;    //500ms
        ret = select(f+1,&readfds,NULL,NULL,&timeout);
        switch(ret){
            case 0:  //超时
                break;
            case -1: //错误
                break;
            default:
                //可以读数
                if(FD_ISSET(f,&readfds)){
                    read(f, &value, sizeof(value));
                    if(value == 3){
                        printf("key trg down\r\n");
                        printf("key on\r\n");
                    }    
                }
                break;
        }
    }
    close(f);
    return 0;
}