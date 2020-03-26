#pragma once

struct ops {
  void (*breakpoint_put)(struct breakpoint *);
  void (*breakpoint_remove)(struct breakpoint *);
  void (*cache_flush)();
  unsigned long (*reg_read)(enum registers reg);
  void (*reg_write)(enum registers reg, unsigned long value);
  unsigned int (*log)(char *data, unsigned int length);
  unsigned int (*recv)(char *output, unsigned int legnth);
  unsigned int (*send)(char *data, unsigned int legnth);
  void *(*malloc)(unsigned int size);
  void (*free)(void *addr);
  void (*cleanup)();
};

extern struct ops ops;

struct ops *target_init(void *args);
