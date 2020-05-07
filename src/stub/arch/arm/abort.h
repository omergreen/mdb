/*
 * abort.c and abort.S contains the logic that overrides the prefetch and data abort
 * handlers in order to handle them ourselves (prefetch for breakpoints, data aborts to
 * make sure we don't crash because the gdb user decided to read from 0xffffffff
 */

#pragma once

#include <stdbool.h>

extern void *prefetch_abort_interrupt_handler;
extern void *data_abort_interrupt_handler;

void install_abort_ivt_handlers();

extern bool g_abort_memory_test_active;
extern bool g_abort_memory_test_got_fault;

