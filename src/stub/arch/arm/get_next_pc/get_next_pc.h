#include <libc/cvector.h>
#include "gdb_get_next_pc.h"

typedef cvector_vector_type(CORE_ADDR) pc_list;

pc_list arch_get_next_pc(struct breakpoint *bp);

