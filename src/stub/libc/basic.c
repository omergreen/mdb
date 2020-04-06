/*
 * Implements basic libc functions
 */

#include <stdint.h>
#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    const char *p = src;
    char *q = dst;
    while (n--) {
        *q++ = *p++;
    }
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    char *q = dst;
    while (n--) {
        *q++ = c;
    }
    return dst;
}

size_t strlen(const char *s)
{
    const char *ss = s;
    while (*ss)
        ss++;
    return ss - s;
}

char *strcpy(char *dst, const char *src)
{
    char *q = dst;
    const char *p = src;
    char ch;

    do {
        *q++ = ch = *p++;
    } while (ch);

    return dst;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c) {
        if (!*s)
            return NULL;
        s++;
    }

    return (char *)s;
}

char *strcat(char *dst, const char *src)
{
    strcpy(strchr(dst, '\0'), src);
    return dst;
}

