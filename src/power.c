#include <io.h>
#include <mb.h>
#include <delay.h>

#define PM_RSTC (volatile unsigned int*)(PERIPHERAL_BASE+0x0010001c)
#define PM_RSTS (volatile unsigned int*)(PERIPHERAL_BASE+0x00100020)
#define PM_WDOG (volatile unsigned int*)(PERIPHERAL_BASE+0x00100024)
#define PM_WDOG_MAGIC   0x5a000000
#define PM_RSTC_FULLRST 0x00000020

void sysPowerOff() {
    unsigned long r;

    for (r=0; r<16; r++) {
        mailbox[0]=32;
        mailbox[1] = MBOX_REQUEST;
        mailbox[2] = MBOX_TAG_SETPOWER;
        mailbox[3] = 8;
        mailbox[4] = 8;
        mailbox[5] = (unsigned int) r;
        mailbox[6] = 0;
        mailbox[7] = MBOX_TAG_LAST;

        mailboxCall(MBOX_CH_PROP);
    }

    *GPFSEL0 = 0; *GPFSEL1 = 0; *GPFSEL2 = 0; *GPFSEL3 = 0; *GPFSEL4 = 0; *GPFSEL5 = 0;
    *GPPUD = 0;
    delay_cycles(150);
    *GPPUDCLK0 = 0xFFFFFFFF; *GPPUDCLK1 = 0xFFFFFFFF;
    delay_cycles(150);
    *GPPUDCLK0 = 0; *GPPUDCLK1 = 0;

    r = *PM_RSTS; r &= ~0xFFFFFAAA;
    r |= 0x555;
    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}

void sysReset() {
    unsigned int r;

    r = *PM_RSTS; r &= ~0xFFFFFAAA;
    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}