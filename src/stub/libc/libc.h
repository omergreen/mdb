#pragma once

#include <stddef.h>
#include <machine/arch/libc.h>
#include <machine/target/libc.h>
#include "malloc.h"

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strcat(char *dst, const char *src);

