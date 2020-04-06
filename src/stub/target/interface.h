/*
 * List of functions that implement the target interface.
 * target_cache_flush may override arch_cache_flush if needed (for example, in Linux)
 * target_malloc, target_realloc and target_free may use the internal malloc, realloc and free, or something that already exists
 * target_log - debug output, for example serial, stdout
 * target_recv & target_send - outside connection to gdb
 */

#pragma once

void target_cache_flush(void *start, unsigned int length);

unsigned int target_recv(char *output, unsigned int legnth);
unsigned int target_send(char *data, unsigned int legnth);
void target_log(const char *format, ...);

void *target_malloc(unsigned int size);
void *target_realloc(void *ptr, unsigned int size);
void target_free(void *addr);

void target_init();
void target_cleanup();

