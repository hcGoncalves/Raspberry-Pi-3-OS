#include <delay.h>
#include <sd.h>
#include <terminal.h>
#include <io.h>
#include <stdint.h>

#include <mem.h>

#define EMMC_ARG2           ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300000))
#define EMMC_BLKSIZECNT     ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300004))
#define EMMC_ARG1           ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300008))
#define EMMC_CMDTM          ((volatile unsigned int*)(PERIPHERAL_BASE+0x0030000C))
#define EMMC_RESP0          ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300010))
#define EMMC_RESP1          ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300014))
#define EMMC_RESP2          ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300018))
#define EMMC_RESP3          ((volatile unsigned int*)(PERIPHERAL_BASE+0x0030001C))
#define EMMC_DATA           ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300020))
#define EMMC_STATUS         ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300024))
#define EMMC_CONTROL0       ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300028))
#define EMMC_CONTROL1       ((volatile unsigned int*)(PERIPHERAL_BASE+0x0030002C))
#define EMMC_INTERRUPT      ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300030))
#define EMMC_INT_MASK       ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300034))
#define EMMC_INT_EN         ((volatile unsigned int*)(PERIPHERAL_BASE+0x00300038))
#define EMMC_CONTROL2       ((volatile unsigned int*)(PERIPHERAL_BASE+0x0030003C))
#define EMMC_SLOTISR_VER    ((volatile unsigned int*)(PERIPHERAL_BASE+0x003000FC))


#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_READ_AVAILABLE   0x00000800
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

uint32_t sdScr[2], sdOcr, sdRca, sdErr, sdHv;

int sdStatus(unsigned int mask) {
    int count = 500000; 
    while((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && count--) delay_u_secs(1);
    return (count <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

int sdInterrupt(unsigned int mask) {
    unsigned int r, m = mask | INT_ERROR_MASK;

    int count = 1000000;
    while(!(*EMMC_INTERRUPT & m) && count--) delay_u_secs(1);
    r = *EMMC_INTERRUPT;

    if (count <= 0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT)) {
        *EMMC_INTERRUPT = r;
        return SD_TIMEOUT;
    } else {
        if (r & INT_ERROR_MASK) {
            *EMMC_INTERRUPT = r;
            return SD_ERROR;
        }
    }

    *EMMC_INTERRUPT = mask;
    return 0;
}

//send command
int sdCommand(unsigned int code, unsigned int argument) {
    int r = 0;
    sdErr = SD_OK;

    if (code & CMD_NEED_APP) {
        r=sdCommand(CMD_APP_CMD | (sdRca ? CMD_RSPNS_48 : 0), sdRca);
        if (sdRca && !r) {
            logTerminal("ERROR: Failed to send SD APP Command");
            sdErr = SD_ERROR;
            return 0;
        }
        code &= ~CMD_NEED_APP;
    }

    if (sdStatus(SR_CMD_INHIBIT)) {
        logTerminal("ERROR: EMMC busy");
        sdErr = SD_TIMEOUT;
        return 0;
    }

    pushTerminal("EMMC: Sending command "); pushTerminalHex(code); pushTerminal(" arg "); pushTerminalHex(argument); pushTerminalChar('\n');

    *EMMC_INTERRUPT = *EMMC_INTERRUPT;
    *EMMC_ARG1 = argument;
    *EMMC_CMDTM = code;

    if (code == CMD_SEND_OP_COND) {delay_u_secs(1000);}
    else if (code == CMD_SEND_IF_COND || code == CMD_APP_CMD) {delay_u_secs(100);}
    
    if ((r=sdInterrupt(INT_CMD_DONE))) {
        logTerminal("ERROR: Failed to send EMMC command");
        sdErr = r;
        return 0;
    }

    r = *EMMC_RESP0;

    if (code == CMD_GO_IDLE || code == CMD_APP_CMD) {return 0;}
    else if (code == (CMD_APP_CMD | CMD_RSPNS_48)) {return r & SR_APP_CMD;}
    else if (code == CMD_SEND_OP_COND) {return r;}
    else if (code == CMD_SEND_IF_COND) {return r == argument ? SD_OK : SD_ERROR;}
    else if (code == CMD_ALL_SEND_CID) {
        r |= *EMMC_RESP3; 
        r |= *EMMC_RESP2; 
        r |= *EMMC_RESP1;
        return r;
    }
    else if (code == CMD_SEND_REL_ADDR) {
        sdErr = (((r & 0x1FFF)) | ((r & 0x2000) << 6) | ((r & 0x4000) << 8) | ((r & 0x8000) << 8))&CMD_ERRORS_MASK;
        return r&CMD_RCA_MASK;
    }

    return r&CMD_ERRORS_MASK;

    return 0;
}

int sdReadblock(unsigned int lba, unsigned char *buffer, unsigned int num) {
    //if (!buffer) return 0;

    int r, c=0, d;

    if (num < 1) num = 1;

    pushTerminal("sdReadblock lba "); pushTerminalHex(lba); pushTerminal(" num "); pushTerminalHex(num); pushTerminalChar('\n');

    if (sdStatus(SR_DAT_INHIBIT)) {
        sdErr = SD_TIMEOUT;
        return 0;
    }

    unsigned int *buf = (unsigned int *) buffer;
    
    if (sdScr[0] & SCR_SUPP_CCS) {
        if (num > 1 && (sdScr[0] & SCR_SUPP_SET_BLKCNT)) {
            sdCommand(CMD_SET_BLOCKCNT, num);
            if (sdErr) return 0;
        }

        *EMMC_BLKSIZECNT = (num << 16) | 512;
        sdCommand(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, lba);
        if (sdErr) return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }

    while (c < num) {
        if (!(sdScr[0] & SCR_SUPP_CCS)) {
            sdCommand(CMD_READ_SINGLE, (lba+c)*512);
            if (sdErr) return 0;
        }

        if ((r=sdInterrupt(INT_READ_RDY))) {
            logTerminal("ERROR: Timeout waiting for ready to read");
            sdErr = r;
            return 0;
        }

        for (d=0; d<128; d++) buf[d] = *EMMC_DATA;

        c++;
        buf += 128;
    }

    if (num > 1 && !(sdScr[0] & SCR_SUPP_SET_BLKCNT) && (sdScr[0] & SCR_SUPP_CCS)) sdCommand(CMD_STOP_TRANS, 0);
    return sdErr != SD_OK || c != num ? 0 : num * 512;
}

int sdSetClockFreq(unsigned int freq) {
    unsigned int d, c = 41666666 / freq, x, s=32, h=0;
    int count = 100000;

    while ((*EMMC_STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && count--) delay_u_secs(1);

    if (count <= 0) {
        logTerminal("ERROR: Timeout whilst waiting for inhibit flag");
        return SD_ERROR;
    }

    *EMMC_CONTROL1 &= ~C1_CLK_EN;
    delay_u_secs(10);

    x = c-1;
    if (!x) s = 0;
    else {
        if(!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if(!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
        if(!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
        if(!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
        if(!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
        if(s>0) s--;
        if(s>7) s=7;
    }

    if (sdHv > HOST_SPEC_V2) d = c;
    else d = (1 << s);

    if (d <= 2) {d=2; s=0;}

    pushTerminal("sdSetClockFreq divisor"); pushTerminalHex(d); pushTerminal(", shift "); pushTerminalHex(s); pushTerminalChar('\n');

    if (sdHv > HOST_SPEC_V2) h = (d & 0x300) >> 2;
    d = (((d & 0x0FF) << 8 ) | h);

    *EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xFFFF003F) | d;
    delay_u_secs(10);
    *EMMC_CONTROL1 |= C1_CLK_EN; 
    delay_u_secs(10);
    
    count = 10000;
    while (!(*EMMC_CONTROL1 & C1_CLK_STABLE) && count--) delay_u_secs(10);
    if (count <= 0) {
        logTerminal("ERROR: Failed to get a stable clock");
        return SD_ERROR;
    }

    return SD_OK;
}

//init EMMC
int sdInit() {
    long r, count, ccs = 0;

    r = *GPFSEL4; r &= ~(7 << (7*3)); *GPFSEL4 = r;
    *GPPUD=2; delay_cycles(150); *GPPUDCLK1 = (1 << 15); delay_cycles(150); *GPPUD = 0; *GPPUDCLK1 = 0;
    r = *GPHEN1; r |= 1 << 15; *GPHEN1 = r;

    r = *GPFSEL4; r |= (7 << (8*3)) | (7 << (9*3)); *GPFSEL4 = r;
    *GPPUD = 2; delay_cycles(150); *GPPUDCLK1 = (1 << 16) | (1 << 17); delay_cycles(150); *GPPUD = 0; *GPPUDCLK1 = 0;

    r = *GPFSEL5; r |= (7 << (0*3)) | (7 << (1*3)) | (7 << (2*3)) | (7 << (3*3)); *GPFSEL5 = r;
    *GPPUD=2; delay_cycles(150);
    *GPPUDCLK1 = (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    delay_cycles(150); *GPPUD = 0; *GPPUDCLK1 = 0;

    sdHv = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    logTerminal("EMMC: GPIO set up");

    //reset
    *EMMC_CONTROL0 = 0; *EMMC_CONTROL1 |= C1_SRST_HC;
    count = 10000; 
    
    do {delay_u_secs(10);} while ((*EMMC_CONTROL1 & C1_SRST_HC) && count--);
    if (count <= 0) {
        logTerminal("ERROR: Failed to reset EMMC");
        return SD_ERROR;
    }

    logTerminal("EMMC: Reset Ok");
    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    delay_u_secs(10);

    //set clock freq
    if ((r = sdSetClockFreq(400000))) return r;
    *EMMC_INT_EN = 0xFFFFFFFF;
    *EMMC_INT_MASK = 0xFFFFFFFF;
    sdScr[0] = sdScr[1] = sdRca = sdErr = 0;
    sdCommand(CMD_GO_IDLE, 0);
    if (sdErr) return sdErr;

    sdCommand(CMD_SEND_IF_COND, 0x000001AA);
    if (sdErr) return sdErr;

    count = 6; r = 0;
    while(!(r & ACMD41_CMD_COMPLETE) && count--) {
        delay_cycles(400);
        r = sdCommand(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        pushTerminal("EMMC: CMD_SEND_OP_COND returned: ");

        if (r & ACMD41_CMD_COMPLETE) pushTerminal("Complete ");
        if (r & ACMD41_VOLTAGE) pushTerminal("Voltage ");
        if (r & ACMD41_CMD_CCS) pushTerminal("Ccs ");
        pushTerminalHex(r>>32);
        pushTerminalHex(r);
        pushTerminalChar('\n');

        if (sdErr != SD_TIMEOUT && sdErr != SD_OK) {
            logTerminal("ERROR: EMMC ACM41 returned error");
            return sdErr;
        }
    }

    if (!(r & ACMD41_CMD_COMPLETE || !count)) return SD_TIMEOUT;
    if (!(r & ACMD41_VOLTAGE)) return SD_ERROR;
    if (r & ACMD41_CMD_CCS) ccs = SCR_SUPP_CCS;

    sdCommand(CMD_ALL_SEND_CID, 0);

    sdRca = sdCommand(CMD_SEND_REL_ADDR, 0);
    pushTerminal("EMMC: CMD_SEND_REL_ADDR returned: ");
    pushTerminalHex(sdRca);
    pushTerminalChar('\n');
    if (sdErr) return sdErr;

    if ((r = sdSetClockFreq(25000000))) return r;

    sdCommand(CMD_CARD_SELECT, sdRca);
    if (sdErr) return sdErr;

    if (sdStatus(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    *EMMC_BLKSIZECNT = (1 << 16) | 8;
    sdCommand(CMD_SEND_SCR, 0);
    if (sdErr) return sdErr;
    if (sdInterrupt(INT_READ_RDY)) return SD_TIMEOUT;

    r = 0; count = 100000;
    while (r<2 && count) {
        if (*EMMC_STATUS & SR_READ_AVAILABLE) sdScr[r++] = *EMMC_DATA;
        else delay_u_secs(1);
    }

    if (r != 2) return SD_TIMEOUT;
    if (sdScr[0] & SCR_SD_BUS_WIDTH_4) {
        sdCommand(CMD_SET_BUS_WIDTH, sdRca | 2);
        if (sdErr) return sdErr;
        *EMMC_CONTROL0 |= C0_HCTL_DWITDH;
    }

    pushTerminal("EMMC: Supports: ");
    if (sdScr[0] & SCR_SUPP_SET_BLKCNT) pushTerminal("SET_BLKCNT ");
    if (ccs) pushTerminal("CSS ");
    pushTerminalChar('\n');

    sdScr[0] &= ~SCR_SUPP_CCS;
    sdScr[0] |= ccs;

    return SD_OK;
}