#include <fb.h>

void draw_window(int x, int y, int w, int h, unsigned int background_color, unsigned int border_color, char * window_name) {
    drawRect(x,y,w,h, background_color, 1);
    drawRect(x-2,y-2,w+4,h+4, border_color, 0);
    drawString(x+4, y-4, window_name, border_color);
}


void clear_window(int x, int y, int w, int h, unsigned int background_color) {
    drawRect(x,y+8,w,h-8, background_color, 1);
}