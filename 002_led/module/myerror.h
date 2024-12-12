#ifndef MYERROR_H
#define MYERROR_H

#define ERROR_LED_READ          (1<<0)       
#define ERROR_LED_WRITE         (1<<1)
#define ERROR_LED_FINDNODE      (1<<2)
#define ERROR_LED_OFREADSTRING  (1<<3)
#define ERROR_LED_OFREADU32A    (1<<4)
#define ERROR_LED_ALLOCREG      (1<<5)
#define ERROR_LED_CDEVADD       (1<<6)
#define ERROR_LED_CLASS         (1<<7)
#define ERROR_LED_DEVICE        (1<<8)

#define IS_OK(x)                (x==0)
#define IS_ERROR(x,y)           (x&y)
#define IS_ERROR_ALL(x,y)       (x|y==y)

#define SET_ERROR(x,y)          (x|=y)
#define SET_OK(x,y)             (x&=~y)

#endif // MYERROR_H