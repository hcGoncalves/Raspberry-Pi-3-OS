void intToString(char *str, int N) {
    if (N == 0) {
        str[0] = '0'; str[1] = '\0';
        return;
    }

    int i = 0;
  
    int sign = N;

    if (N < 0) N = -N;

    while (N > 0) {
        str[i++] = N % 10 + '0';
      	N /= 10;
    } 

    if (sign < 0) str[i++] = '-';
    
    str[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

int stringLength(char str[]) {
    int length = 0;

    while (str[length] != '\0') {
        length++;
    }

    return length;
}

int equalStrings(char str1[], char str2[]) {
    int i = 0;
    while (str1[i] == str2[i]) {
        if (str1[i] == '\0' || str2[i] == '\0') return 1;
        i++;
    }

    return 0;
}

//Hello == Hel
int equalTruncatedStrings(char str1[], char str2[]) {
    int i = 0;
    while (str1[i] == str2[i] || (str1[i] == '\0' || str2[i] == '\0')) {
        if (str1[i] == '\0' || str2[i] == '\0') return 1;
        i++;
    }

    return 0;
}