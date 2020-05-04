/*
 * Inner functions required for ARM breakpoints
 * jump_breakpoint_stub* are implemented in assembly in breakpoint_stub.S
 */

#pragma once

#include <stdbool.h>
#include <core/breakpoint.h>

void *general_exception_handler_low;
unsigned int general_exception_stack;

