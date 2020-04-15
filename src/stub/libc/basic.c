/*
 * Implements basic libc functions
 */

#include <stdint.h>
#include <string.h>
#include <machine/arch/libc.h>

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

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *c1 = s1, *c2 = s2;
	int d = 0;

	while (n--) {
		d = (int)*c1++ - (int)*c2++;
		if (d)
			break;
	}

	return d;
}

int strcmp(const char *s1, const char *s2)
{
	const unsigned char *c1 = (const unsigned char *)s1;
	const unsigned char *c2 = (const unsigned char *)s2;
	unsigned char ch;
	int d = 0;

	while (1) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *c1 = (const unsigned char *)s1;
	const unsigned char *c2 = (const unsigned char *)s2;
	unsigned char ch;
	int d = 0;

	while (n--) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

unsigned int swap32(unsigned int val) {
    return ((val>>24) & 0xff)     |
           ((val<<8)  & 0xff0000) |
           ((val>>8)  & 0xff00)   |
           ((val<<24) & 0xff000000);
}

unsigned short swap16(unsigned short val) {
    return (val>>8) | (val<<8);
}

unsigned int htonl(unsigned int hostlong) {
#if DATA_ENDIAN != BIG // network endian is big
    hostlong = swap32(hostlong);
#endif
    return hostlong;
}
unsigned short htons(unsigned short hostshort) {
#if DATA_ENDIAN != ENDIAN_BIG
    hostshort = swap16(hostshort);
#endif
    return hostshort;
}

