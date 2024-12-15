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
    int buf;
    f = open(filename, O_RDWR);

    if(f < 0){
        perror("open(): error");
    }

    switch(argv[2][0]){
        case 'r':
            ret = read(f, &buf, 4);
            if(ret < 0){
                perror("read(): error");
                return -1;
            }else{
                printf("led read: %d\r\n", buf);
                close(f);
                return 0;
            }
            break;

        case 'w':
            if(argc < 3){
                perror("write(): error");
                return -1;
            }else{
                buf = atoi(argv[3]);
                ret = write(f, &buf, 4);
                printf("led write: %d\r\n", buf);
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