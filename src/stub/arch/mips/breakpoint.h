/*
 * Inner functions for mips breakpoints
 * general_exception_handler_low is implemented in breakpoint_stub.S
 */

#pragma once

#include <stdbool.h>
#include <core/breakpoint.h>

extern void *general_exception_handler_low;
extern unsigned int general_exception_stack;
extern unsigned int general_exception_original_ivt_handler;

extern bool g_memory_test_active;
extern bool g_memory_test_got_fault;

