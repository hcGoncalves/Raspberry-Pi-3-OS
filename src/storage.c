#include <window.h>
#include <mem.h>
#include <fb.h>

int storage_window_x, storage_window_y, storage_window_w, storage_window_h;
int storage_log_x, storage_log_y;

void init_storage_window(int initial_x, int initial_y, int initial_w, int initial_h) {
    storage_window_x = initial_x;
    storage_window_y = initial_y;
    storage_window_w = initial_w;
    storage_window_h = initial_h;

    storage_log_x = storage_window_x+5;
    storage_log_y = storage_window_y+10;

    draw_window(storage_window_x, storage_window_y, storage_window_w, storage_window_h, 0x000000, 0xFFFFFF, "Secondary Storage");
}

void clear_storage_window() {
    clear_window(storage_window_x,storage_window_y,storage_window_w,storage_window_h,0x000000);
    storage_log_x = storage_window_x+5;
    storage_log_y = storage_window_y+10;
}

void push_storage_window(char * name) {
    drawString(storage_log_x, storage_log_y, name, 0xFFFFFF);
    storage_log_y+=15;
}