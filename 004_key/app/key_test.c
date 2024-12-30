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

    return 0;
}