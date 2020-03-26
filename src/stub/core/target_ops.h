#pragma once

struct ops {
  void *(*malloc)(unsigned int size);
  void (*free)(void *addr);
  void (*cleanup)();
};

struct ops *target_init(void *args);
