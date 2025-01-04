#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

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
    int f = 0;
    int buf;
    f = open(filename, O_RDWR|O_NONBLOCK);

    if(f < 0){
        perror("open(): error");
        return -1;
    }

    int value = 0;
    while(1){
        FD_ZERO(&readfds);
        FD_SET(f,&readfds);
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
    }

    close(f);
    return 0;
}