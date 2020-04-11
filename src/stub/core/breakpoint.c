#include "breakpoint.h"
#include "state.h"
#include <libc/cvector.h>

static struct breakpoint *find_breakpoint(unsigned int address) {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if (bp->address == address) {
            return bp;
        }
    }

    return NULL;
}


void breakpoint_handler(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        ERROR("We triggered on a breakpoint that doesn't exist");
        return;
    }

    // do more breakpoint specific stuff?
    core_loop(bp);
}

void breakpoint_add(unsigned int address, enum breakpoint_type type) {
    if (find_breakpoint(address) != NULL) {
        ERROR("breakpoint already exists");
        return;
    }    

    struct breakpoint bp = { .address = address, .type = type, .enabled = false };
    cvector_push_back(g_state.breakpoints, bp);
}

void breakpoint_enable(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        ERROR("Breakpoint doesn't exist");
        return;
    }
    if (bp->enabled) {
        ERROR("Breakpoint already enabled");
        return;
    }

    arch_jump_breakpoint_enable(bp); // TODO: change this to more generic arch_x_breakpoint_enable in the future
    bp->enabled = true;
}

void breakpoint_disable(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        ERROR("Breakpoint doesn't exist");
        return;
    }
    if (!bp->enabled) {
        ERROR("Breakpoint already disabled");
        return;
    }

    arch_jump_breakpoint_disable(bp); // TODO: change this to more generic arch_x_breakpoint_disable in the future
    bp->enabled = false;
}



