#include <fb.h>
#include <io.h>
#include <mb.h>
#include <fontmap.h>
#include <stdint.h>

unsigned int width, height, pitch, isrgb;
unsigned char *fb;

void framebufferInit()
{
    mailbox[0] = 35*4; // Length of message in bytes
    mailbox[1] = MBOX_REQUEST;

    mailbox[2] = MBOX_TAG_SETPHYWH; // Tag identifier
    mailbox[3] = 8; // Value size in bytes
    mailbox[4] = 0;
    mailbox[5] = BUFFER_WIDTH; // Value(width)
    mailbox[6] = BUFFER_HEIGHT; // Value(height)

    mailbox[7] = MBOX_TAG_SETVIRTWH;
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = 1920;
    mailbox[11] = 1080;

    mailbox[12] = MBOX_TAG_SETVIRTOFF;
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0; // Value(x)
    mailbox[16] = 0; // Value(y)

    mailbox[17] = MBOX_TAG_SETDEPTH;
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32; // Bits per pixel

    mailbox[21] = MBOX_TAG_SETPXLORDR;
    mailbox[22] = 4;
    mailbox[23] = 4;
    mailbox[24] = 1; // RGB

    mailbox[25] = MBOX_TAG_GETFB;
    mailbox[26] = 8;
    mailbox[27] = 8;
    mailbox[28] = 4096; // FrameBufferInfo.pointer
    mailbox[29] = 0;    // FrameBufferInfo.size

    mailbox[30] = MBOX_TAG_GETPITCH;
    mailbox[31] = 4;
    mailbox[32] = 4;
    mailbox[33] = 0; // Bytes per line

    mailbox[34] = MBOX_TAG_LAST;

    // Check call is successful and we have a pointer with depth 32
    if (mailboxCall(MBOX_CH_PROP) && mailbox[20] == 32 && mailbox[28] != 0) {
        mailbox[28] &= 0x3FFFFFFF; // Convert GPU address to ARM address
        width = mailbox[10];       // Actual physical width
        height = mailbox[11];      // Actual physical height
        pitch = mailbox[33];       // Number of bytes per line
        isrgb = mailbox[24];       // Pixel order
        fb = (unsigned char *)((uintptr_t) mailbox[28]);
    }
}

void drawPixel(int x, int y, unsigned int rgb) {
    //if (x < 0 || x >= width || y < 0 || y >= height) return;

    int offs = (y * pitch) + (x * 4);
    *((unsigned int*)(fb + offs)) = rgb;
}

void drawLine(int x0, int y0, int x1, int y1, unsigned int rgb) {
    int dx = x1-x0;
    if (dx < 0) {dx*=-1;}
    int sx = x0 < x1 ? 1 : -1;

    int dy = y1-y0;
    if (dy > 0) {dy*=-1;}
    int sy = y0 < y1 ? 1 : -1;

    int error = dx + dy;

    while (1) {
        drawPixel(x0, y0, rgb);
        int e2 = 2 * error;

        if (e2 >= dy) {
            if (x0 == x1) {break;}
            error = error + dy;
            x0 = x0 + sx;
        }

        if (e2 <= dx) {
            if (y0 == y1) {break;}
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void drawCircle(int x, int y, int r, unsigned int rgb, int fill) {
    int t1 = r / 16;
    int x0 = r;
    int y0 = 0;
    while (x0 >= y0) {
        if (fill) {
            drawLine(x+x0,y+y0, x+x0,y-y0, rgb);
            drawLine(x-x0,y+y0, x-x0,y-y0, rgb);

            drawLine(x+y0, y+x0, x+y0, y-x0, rgb);
            drawLine(x-y0, y+x0, x-y0, y-x0, rgb);
        } else {
            drawPixel(x+x0, y+y0, rgb);
            drawPixel(x-x0, y+y0, rgb);
            drawPixel(x+x0, y-y0, rgb);
            drawPixel(x-x0, y-y0, rgb);

            drawPixel(x+y0, y+x0, rgb);
            drawPixel(x-y0, y+x0, rgb);
            drawPixel(x+y0, y-x0, rgb);
            drawPixel(x-y0, y-x0, rgb);
        }

        y0++;
        t1 += y0;

        int t2 = t1-x0;
        if (t2 >= 0) {
            t1 = t2;
            x0--;
        }
    }
}

void drawRect(int x, int y, int width, int height, unsigned int rgb, int fill) {
    if (fill) {
        for (int x0=x; x0<=x+width;x0++) {
            drawLine(x0,y,x0,y+height, rgb);
        }
    } else {
        drawLine(x,y,x+width,y, rgb);
        drawLine(x+width,y,x+width,y+height, rgb);
        drawLine(x,y+height,x+width,y+height, rgb);
        drawLine(x,y,x,y+height, rgb);
    }
}

void drawChar(unsigned char ch, int x, int y, unsigned int rgb) {
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=0;i<FONT_HEIGHT;i++) {
	    for (int j=0;j<FONT_WIDTH;j++) {
            unsigned char mask = 1 << j;
            unsigned int col = (*glyph & mask) ? rgb : 0x000000; //~rgb;

            drawPixel(x+j, y+i, col);
            
	    }
	    glyph += FONT_BPL;
    }
}

void drawString(int x, int y, char *s, unsigned int rgb) {
    int x0 = 0;

    while (*s) {
       if (*s == '\r') {
          x0 = 0;
       } else if(*s == '\n') {
          x0 = 0; y += FONT_HEIGHT;
       } else {
	  drawChar(*s, x+x0, y, rgb);
          x0 += FONT_WIDTH;
       }
       s++;
    }
}

void fillFramebuffer(unsigned int rgb) {
    for (int x=0;x<BUFFER_WIDTH;x++) {
        drawLine(x,0,x,BUFFER_HEIGHT, rgb);
    }
}