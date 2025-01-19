#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/input.h>

int main(int argc, char * argv[]){
    char * file_name;
    int ret = 0;
    int f = 0;
    struct input_event inputevent;
    if(argc < 2){
        perror("Usage: input_test.out [filename] [R|W] [VAL]\r\n");
        return -1;
    }

    file_name = argv[1];
    f = open(file_name, O_RDWR);
    if(f < 0){
        perror("fopen: open file error");
        return -1;
    }
    int input = 0;
    static int trg = 0,cont = 0;
    switch (argc - 2)
    {
    case 0:
        printf("[INFO]: read info\r\n");
        do
        {
            ret = read(f, &inputevent, sizeof(inputevent));
            if(ret < 0){
                perror("read: read file error\r\n");
            }else{
                switch (inputevent.type)
                {
                case EV_SYN:
                    break;
                case EV_KEY:
                    printf("[INFO]: EV_KEY\r\n");
                    if(inputevent.value < 2){
                        if(inputevent.code < BTN_MISC){
                            printf("[INFO]: key %d %s\r\n", inputevent.code,inputevent.value?"pressed":"released");
                        }else{
                            printf("[INFO]: button %d %s\r\n", inputevent.code,inputevent.value?"pressed":"released");
                        }
                    }else{
                        if(inputevent.code < BTN_MISC){
                            printf("[INFO]: key %d %s\r\n", inputevent.code,inputevent.value == 2?"on":"off");
                        }else{
                            printf("[INFO]: button %d %s\r\n", inputevent.code,inputevent.value == 2?"on":"off");
                        }                 
                    }

                    break;
                case EV_REP:
                    break;
                default:
                    break;
                }
            }
        } while (input != 1);
        break;
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }

    close(f);

    return 0;
}