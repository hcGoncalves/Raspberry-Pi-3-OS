#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *(--d) = *(--s);
    }
    return dst;
}

void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = s;
    unsigned char uc = (unsigned char)c;

    for (size_t i = 0; i < n; i++) {
        if (p[i] == uc) {
            // return pointer to the first match
            return (void *)(p + i);
        }
    }
    return NULL; // not found
}

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    while (n--) {
        if (*x != *y) return *x - *y;
        x++; y++;
    }
    return 0;
}

void memset(void *dst, unsigned char c, size_t n) {
    unsigned char *d = dst;
    while (n--) *d++ = c;
}

void bzero(void * dest, size_t bytes) {
    memset(dest, 0, bytes);
}

size_t strlen(const char *s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}

char *strchr(const char *s, int c) {
    char ch = (char)c;

    while (*s) {
        if (*s == ch) {
            return (char *)s;  // found
        }
        s++;
    }

    // check for terminating null if c == '\0'
    if (ch == '\0') {
        return (char *)s;
    }

    return NULL; // not found
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // cast to unsigned char for proper comparison of negative chars
    return (unsigned char)*s1 - (unsigned char)*s2;
}