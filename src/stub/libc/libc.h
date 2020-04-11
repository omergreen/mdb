/*
 * Defines general functions, sort of subset of a real libc
 */

#pragma once

#define TINYPRINTF_OVERRIDE_LIBC 1

#include <stddef.h>
#include <machine/arch/libc.h>
#include <machine/target/libc.h>
#include "malloc.h"
#include "tinyprintf.h"
#include <target/interface.h>
#include <arch/interface.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strcat(char *dst, const char *src);

unsigned int htonl(unsigned int hostlong);
unsigned short htons(unsigned short hostshort);
#define ntohl htonl
#define ntohs htons

#ifdef OVERRIDE_ARCH_CACHE_FLUSH
#define cache_flush target_cache_flush
#else
#define cache_flush arch_cache_flush
#endif

