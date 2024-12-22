#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    unsigned int args;
    int flag = 0;
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
    f = open(filename, O_RDWR);

    if(f < 0){
        perror("open(): error");
        return -1;
    }

    int value = 0;
    while(1){
        read(f, &value, sizeof(value));
        if(value == 1){
            printf("key press\r\n");
        }
    }

    return 0;
}