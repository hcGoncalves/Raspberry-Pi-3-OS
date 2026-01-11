#include <terminal.h>
#include <utils.h>
#include <fb.h>
#include <window.h>
#include <stdint.h>
#include <math.h>

int plot_window_x, plot_window_y, plot_window_w, plot_window_h;
int window_min_x, window_max_x, window_min_y, window_max_y;

int current_plots;

void draw_axis() {
    //x axis
    if (window_min_y < 0 && window_max_y > 0) {
        int y_pos = plot_window_y + plot_window_h - ((double)(-window_min_y) / (double)(window_max_y - window_min_y)) * plot_window_h;
        drawLine(plot_window_x, y_pos, plot_window_x+plot_window_w, y_pos, 0xFFFFFF);

        int interval = plot_window_w/25;
        for (int i = 0; i < interval; i+=1) {
            drawLine(plot_window_x+i*25, y_pos-2, plot_window_x+i*25, y_pos+2, 0xFFFFFF);

            int label_int = window_min_x + i * (25 / ((window_min_x<0 ? -window_min_x : window_min_x) + (window_max_x<0 ? -window_max_x : window_max_x)));
            char label[8];
            intToString(label, label_int);
            drawString(plot_window_x+i*25, y_pos+8, label, 0xFFFFFF);
        }
        
    }

    //y axis
    if (window_min_x < 0 && window_max_x > 0) {
        int x_pos = plot_window_x + ((double)(-window_min_x) / (double)(window_max_x - window_min_x)) * plot_window_w;
        drawLine(x_pos, plot_window_y, x_pos, plot_window_y+plot_window_h, 0xFFFFFF);

        int interval = plot_window_h/25;
        for (int i = 0; i < interval; i+=1) {
            drawLine(x_pos-2, plot_window_y+plot_window_h - i*25, x_pos+2, plot_window_y+plot_window_h - i*25, 0xFFFFFF);

            int label_int = window_min_y + i * (25 / ((window_min_y<0 ? -window_min_y : window_min_y) + (window_max_y<0 ? -window_max_y : window_max_y)));
            char label[8];
            intToString(label, label_int);
            drawString(x_pos+8, plot_window_y+plot_window_h-i*25-4, label, 0xFFFFFF);
        }
    }
}

void init_plot(int initial_x, int initial_y, int initial_w, int initial_h) {
    plot_window_x = initial_x;
    plot_window_y = initial_y;
    plot_window_w = initial_w;
    plot_window_h = initial_h;


    window_min_x = -10;
    window_max_x = 10;
    window_min_y = -10;
    window_max_y = 10;

    current_plots = 0;

    draw_window(plot_window_x, plot_window_y, plot_window_w, plot_window_h, 0x000000, 0xFFFFFF, "Plot");
    draw_axis();
}


double process_op(double num1, double num2, char op) {
    switch(op) {
        case('+'):
            return num1 + num2;
        case('-'):
            return num1 - num2;
        case('*'):
            return num1 * num2;
        case('/'):
            if (num2 == 0) {return INT32_MAX;}
            return num1 / num2;
        case('^'):
            return pow(num1, num2);
        default:
            pushTerminal("PLOT: Unknown operation used: ");
            pushTerminalChar(op);
            pushTerminalChar('\n');


            return 0;
    }
}


double f(double x, char * expression) {
    double last_num = 0;
    double current_num = 0;
    int negative = 0;
    char current_op = '\0';

    while (*expression) {
        /*pushTerminal("PLOT: Parsing: ");
        pushTerminalChar(*expression);
        pushTerminal("\n    last_num: ");
        pushTerminalInt(last_num);
        pushTerminal("\n    current_num: ");
        pushTerminalInt(current_num);
        pushTerminalChar('\n');*/

        if (*expression == '+' || *expression == '*' || *expression == '/' || *expression == '^') {
            if (current_op) {
                last_num = process_op(last_num, current_num, current_op);
            } else {
                last_num = current_num;
            }


            current_num = 0;
            current_op = *expression;
            negative = 0;
        } else if (*expression == '-') {
            if (current_op) {
                last_num = process_op(last_num, current_num, current_op);
            } else {
                if (current_num == 0) {
                    negative = negative ^ 1;
                } else {
                    last_num = current_num;
                    current_num = 0;
                    current_op = '-';
                }
            }
        } else if (*expression == 'x') {
            if (current_num != 0) {
                logTerminal("PLOT: Invalid format used for x. Overriden current_num");
            }

            current_num = x;
        } else {
            int num = (*expression) - '0';


            if (num > 9 || num < 0) {
                pushTerminal("PLOT: Unknown symbol used: ");
                pushTerminalChar(*expression);
                pushTerminalChar('\n');
            } else {
                current_num = negative ? (current_num*10-num) : (current_num*10+num);
            }
        }


        expression++;
    }

    if (current_op) {
        last_num = process_op(last_num, current_num, current_op);
    } else {
        last_num = current_num;
    }

    return last_num;
}

void plot(int min, int max, double increment, char * expression, unsigned int color) {
    

    min = MAX(window_min_x, min);
    max = MIN(window_max_x, max);


    double x = min;
    double last_y = 0;

    int scale_x = plot_window_w / ((window_min_x<0 ? -window_min_x : window_min_x) + (window_max_x<0 ? -window_max_x : window_max_x));
    int scale_y = plot_window_h / ((window_min_y<0 ? -window_min_y : window_min_y) + (window_max_y<0 ? -window_max_y : window_max_y));


    while (x<=max) {
        double y = f(x, expression);
        
        if (window_min_y <= y && y <= window_max_y) {
            if (x != min && window_min_y <= last_y && last_y <= window_max_y) {
                drawLine(plot_window_x + (int)((x-increment - window_min_x)*scale_x), plot_window_y + (int)((window_max_y - last_y)*scale_y), 
                plot_window_x + (int)((x - window_min_x)*scale_x), plot_window_y + (int)((window_max_y - y)*scale_y), color);
            } else {
                drawPixel(plot_window_x + (int)((x - window_min_x)*scale_x), plot_window_y + (int)((window_max_y - y)*scale_y), color);
            }
            
            /*pushTerminal("PLOT: (");
            pushTerminalInt(x);
            pushTerminalChar(',');
            pushTerminalInt(y);
            pushTerminal(")\n");*/
        }

        last_y = y;
        x+=increment;
    }

    draw_axis();

    drawString(plot_window_x+8,plot_window_y+8 + current_plots*16, expression, color);
    current_plots++;
}

void clear_plot() {
    clear_window(plot_window_x,plot_window_y,plot_window_w,plot_window_h,0x000000);
    draw_axis();

    current_plots = 0;
}