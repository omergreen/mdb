#pragma once
#include <stdbool.h>

#include "breakpoint.h"
#include <machine/arch/registers.h>

extern struct ops {
  bool (*breakpoint_put)(struct breakpoint *);
  bool (*breakpoint_remove)(struct breakpoint *);
  void (*cache_flush)(void *start, unsigned int length);
  unsigned long (*reg_read)(enum registers_enum reg);
  void (*reg_write)(enum registers_enum reg, unsigned long value);
  unsigned int (*recv)(char *output, unsigned int legnth);
  unsigned int (*send)(char *data, unsigned int legnth);
  void *(*malloc)(unsigned int size);
  void (*free)(void *addr);
  void (*cleanup)();
  void (*log)(const char *format, ...);
} g_ops;

void target_init(void *args);

