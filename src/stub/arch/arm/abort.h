#pragma once

#include <stdbool.h>

extern void *prefetch_abort_interrupt_handler;
extern void *data_abort_interrupt_handler;

void install_abort_ivt_handlers();

extern bool g_abort_memory_test_active;
extern bool g_abort_memory_test_got_fault;

