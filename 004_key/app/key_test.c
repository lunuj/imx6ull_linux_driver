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
        sleep(0);
    }
    close(f);
    return 0;
}