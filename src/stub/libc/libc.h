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
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

// swap the endian of `val` (32 bit)
unsigned int swap32(unsigned int val);
// swap the endian of `val` (16 bit)
unsigned short swap16(unsigned short val);

unsigned int htonl(unsigned int hostlong);
unsigned short htons(unsigned short hostshort);
#define ntohl htonl
#define ntohs htons

// memcpy(dst, src, n), but checks first if src is readable and if
// dst is writeable
bool safe_memcpy(void *dst, const void *src, size_t n);

#ifdef OVERRIDE_ARCH_CACHE_FLUSH
#define cache_flush target_cache_flush
#else
#define cache_flush arch_cache_flush
#endif

#ifdef OVERRIDE_ARCH_TEST_ADDRESS
#define test_address target_test_address
#else
#define test_address arch_test_address
#endif

