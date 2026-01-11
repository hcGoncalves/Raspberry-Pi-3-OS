#include <fb.h>
#include <io.h>
#include <utils.h>
#include <power.h>
#include <window.h>
#include <string.h>

#define LOG_LENGTH 64
#define MSG_LENGTH 128

int x,y, w,h;
char log[LOG_LENGTH][MSG_LENGTH];
int logPointer;
int msgPointer;
//log[logPointer][msgPointer]

char in[MSG_LENGTH];
int inI;
int inOffset;

void initTerminal() {
    x=5;
    y=15;
    w=1000;
    h=1005;
    logPointer = 0;
    msgPointer = 0;
    
    inI = 0;
    inOffset = 0;

    bzero(log, sizeof(log));
}

void drawIn() {
    if (inI < MSG_LENGTH-1) {in[inI] = ' ';}
    drawString(x+4, y+inOffset, in, 0xFFFFFF);
    if (inI < MSG_LENGTH-1) {in[inI] = '\0';}
}

void drawTerminal(int full_draw) {
    if (full_draw) {
        draw_window(x, y, w, h, 0x000000, 0xFFFFFF, "<Terminal>");
        //drawRect(x,y,1000,y+inOffset+20, 0x000000, 1);
        //drawRect(x+2,y+2, 996, y+inOffset+18, 0xFFFFFF, 0);
        //drawString(x+4,y, "<Terminal>", 0xFFFFFF);
    }
    
    int yOffset = 25;

    for (int i=0; i<LOG_LENGTH; i++) {
        drawRect(x+4, y+yOffset, 990, 15, 0x000000, 1);
        drawString(x+4, y+yOffset, log[(logPointer+i) % LOG_LENGTH], 0xFFFFFF);
        if (log[(logPointer+i) % LOG_LENGTH][0] != '\0') {
            yOffset += 15;
        }
    }

    inOffset = yOffset+15;

    drawIn();
}

void newLog() {
    log[logPointer][MSG_LENGTH-1] = '\0';
    logPointer = (logPointer+1) % LOG_LENGTH;
    msgPointer = 0;

    for (int i = 0; i < MSG_LENGTH; i++) {
        log[logPointer][i] = '\0';
    }

    drawTerminal(0);
}

void logTerminal(char *msg) {
    if (msgPointer != 0) {newLog();}

    for (int i=0; i<MSG_LENGTH-1; i++) {
        if (msg[i] == '\n') {newLog(); continue;}
        log[logPointer][msgPointer] = msg[i];
        msgPointer++;
        if (msg[i] == '\0') {break;}
    }

    newLog();
}

void logTerminalInt(int num) {
    if (msgPointer != 0) {newLog();}

    intToString(log[logPointer], num);

    newLog();
}

void logTerminalChar(char chr) {
    if (msgPointer != 0) {newLog();}

    log[logPointer][0] = chr;
    log[logPointer][1] = '\0';

    newLog();
}

void logTerminalHex(unsigned int num) {
    if (msgPointer != 0) {newLog();}

    unsigned int a;
    int b;

    int i = 0;
    for(b=28; b >= 0; b -= 4) {
        a = (num >> b) & 0xF;
        
        a += a > 9 ? 0x37 : 0x30;
        
        log[logPointer][i] = (char) a;
        i++;
    }

    log[logPointer][i] = '\0';
    newLog();
}

void pushTerminal(char *msg) {
    for (int i=0; i<MSG_LENGTH-1; i++) {
        if (msg[i] == '\0') {break;}
        if (msg[i] == '\n') {newLog(); continue;}
        if (msgPointer > MSG_LENGTH-2) {newLog();}

        log[logPointer][msgPointer] = msg[i];
        msgPointer++;
    }
}

void pushTerminalHex(unsigned int hex) {
    unsigned int a;
    int b;

    for(b=28; b >= 0; b -= 4) {
        a = (hex >> b) & 0xF;
        
        a += a > 9 ? 0x37 : 0x30;
        
        if (msgPointer > MSG_LENGTH-2) {newLog();}
        log[logPointer][msgPointer] = (char) a;
        msgPointer++;
    }
}

void pushTerminalChar(char chr) {
    if (chr == '\n') {newLog(); return;}

    log[logPointer][msgPointer] = chr;
    msgPointer++;
}

void pushTerminalInt(int num) {
    char str[MSG_LENGTH-1];
    intToString(str, num);

    for (int i=0; i<MSG_LENGTH-1; i++) {
        if (str[i] == '\0') {break;}
        if (msgPointer > MSG_LENGTH-2) {newLog();}

        log[logPointer][msgPointer] = str[i];
        msgPointer++;
    }
}

void dumpToTerminal(void *ptr) {
    unsigned long a,b,d;
    unsigned char c;

    for(a=(unsigned long)ptr;a<(unsigned long)ptr+512;a+=16) {
        pushTerminalHex(a);
        pushTerminal(": ");

        for(b=0;b<16;b++) {
            c=*((unsigned char*)(a+b));
            d=(unsigned int)c;d>>=4;d&=0xF;d+=d>9?0x37:0x30; pushTerminalChar(d);
            d=(unsigned int)c;d&=0xF;d+=d>9?0x37:0x30; pushTerminalChar(d);

            pushTerminalChar(' ');
            if(b%4==3) pushTerminalChar(' ');
        }
        for(b=0;b<16;b++) {
            c=*((unsigned char*)(a+b));
            pushTerminalChar((char) (c<32||c>=127?'.':c));
        }

        pushTerminalChar('\n');
    }
}

void clearIn() {
    for (int i=0; i<MSG_LENGTH-1; i++) {
        in[i] = '\0';
    }

    inI = 0;
}

//echos inputs back and processes it.
void processIn() {
    logTerminal(in);

    if (equalStrings(in, "sys power off")) sysPowerOff();
    else if (equalStrings(in, "sys power reset")) sysReset();

    clearIn();
    drawIn();
}

//echos input back to user, no action taken
void echoIn() {
    logTerminal(in);
    clearIn();
    drawIn();
}

void writeIn(char chr) {
    if (chr == '\0') {return;}

    if (chr == '\r') {
        processIn();
        return;
    }

    if (chr == 127 || chr == '\b') {if (inI > 0) {
        inI--;
        in[inI] = '\0';

        drawIn();
    } return;}


    if (inI > MSG_LENGTH-1) {logTerminal("Maximum Input Reached (128)"); drawTerminal(0); inI--; return;}

    in[inI] = chr;
    inI++;

    drawIn();
}