#pragma once
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void bzero(void * dest, size_t bytes);
void memset(void *dst, unsigned char c, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);