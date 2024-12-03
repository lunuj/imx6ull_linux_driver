#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    if(argc < 3){
        printf("Usage: error\r\n");
        return -1;
    }

    char * filename;
    filename = argv[1];
    printf("filename: %s\r\n", filename);
    printf("modulname %s\r\n", argv[0]);
    int ret = 0;
    int f = 0;
    char readbuf[100];
    char writebuf[100];

    f = open(filename, O_RDWR);

    if(f < 0){
        perror("open(): error");
    }

    switch(argv[2][0]){
        case 'r':
            ret = read(f, readbuf, 20);
            if(ret < 0){
                perror("read(): error");
                return -1;
            }else{
                printf("chardev read: %s\r\n", readbuf);
                close(f);
                return 0;
            }
            break;

        case 'w':
            sprintf(writebuf, "%s", argv[3]);
            ret = write(f, writebuf, 20);
            if(ret < 0 || argc < 3){
                perror("write(): error");
                return -1;
            }else{
                printf("chardev write: %s\r\n", writebuf);
                close(f);
                return 0;
            }
            break;
            
        default:
            printf("Usage: error\r\n");
            break;
    }

    return 0;
}