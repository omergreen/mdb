#pragma once


void target_cache_flush(void *start, unsigned int length);
unsigned int target_recv(char *output, unsigned int legnth);
unsigned int target_send(char *data, unsigned int legnth);
void *target_malloc(unsigned int size);
void *target_realloc(void *ptr, unsigned int size);
void target_free(void *addr);
void target_cleanup();
void target_init();
void target_log(const char *format, ...);


