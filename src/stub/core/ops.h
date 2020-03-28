#pragma once

extern struct ops {
  void *(*malloc)(unsigned int size);
  void (*free)(void *addr);
  void (*cleanup)();
  void (*log)(unsigned char *data, unsigned int length);
} g_ops;

void target_init(void *args);
