#include <math.h>

double pow(double base, int exp) {
    if(exp < 0) {
        if(base == 0)
            return -0; //undefined
        return 1 / (base * pow(base, (-exp) - 1));
    }
    if(exp == 0)
        return 1;
    if(exp == 1)
        return base;
    return base * pow(base, exp - 1);
}

int fact(int x) {
    return x <= 0 ? 1 : x * fact(x-1);
}

double sin(double x, int terms) {
    x = x - (2*PI) * (int)(x / (2*PI));

    double num = 0;

    for (int i=0;i<terms;i++) {
        num += (pow(-1,i)/fact(2*i+1)) * pow(x, 2*i+1);
    }

    return num;
}