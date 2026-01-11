#include <peripheral.h>

#define GPFSEL0             (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200000)
#define GPFSEL1             (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200004)
#define GPFSEL2             (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200008)
#define GPFSEL3             (volatile unsigned int*)(PERIPHERAL_BASE + 0x0020000C)
#define GPFSEL4             (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200010)
#define GPFSEL5             (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200014)
#define GPSET0              (volatile unsigned int*)(PERIPHERAL_BASE + 0x0020001C)
#define GPCLR0              (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200028)

#define GPLEV0              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200034)
#define GPLEV1              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200038)
#define GPEDS0              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200040)
#define GPEDS1              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200044)
#define GPHEN0              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200064)
#define GPHEN1              (volatile unsigned int*)(PERIPHERAL_BASE+0x00200068)

#define GPPUD               (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200094)
#define GPPUDCLK0           (volatile unsigned int*)(PERIPHERAL_BASE + 0x00200098)
#define GPPUDCLK1           (volatile unsigned int*)(PERIPHERAL_BASE + 0x0020009C)
#define GPPUPPDN0           (volatile unsigned int*)(PERIPHERAL_BASE + 0x002000E4)

enum {
    GPSET0_ADDRESS =        PERIPHERAL_BASE + 0x0020001C,
    GPCLR0_ADDRESS =        PERIPHERAL_BASE + 0x00200028,
    GPPUPPDN0_ADDRESS =     PERIPHERAL_BASE + 0x002000E4,
    GPFSEL0_ADDRESS =       PERIPHERAL_BASE + 0x00200000
};


enum {
    GPIO_MAX_PIN       = 53,
    GPIO_FUNCTION_OUT  = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7
};

enum {
    Pull_None = 0,
    Pull_Up = 1,
    Pull_Down = 2
};

void uartInit();
void uartWriteText(char *buffer);
void uartLoadOutputFifo();
unsigned char uartReadByte();
unsigned int uartIsReadByteReady();
void uartWriteByteBlocking(unsigned char ch);
void uartUpdateEcho();
char uartUpdateReturn();

void mmioWrite(long reg, unsigned int val);
unsigned int mmioRead(long reg);