#include <core/ops.h>
#include "libc.h"
#include "breakpoint.h"

void init_arch() {
    g_ops.cache_flush = cache_flush;
    g_ops.breakpoint_put = jump_breakpoint_put;
}
