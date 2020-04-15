#include "breakpoint.h"
#include "state.h"
#include <libc/cvector.h>
#include "core.h"

static struct breakpoint *find_breakpoint(unsigned int address) {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if (bp->address == address) {
            return bp;
        }
    }

    return NULL;
}

void breakpoint_handler() {
    // do more breakpoint specific stuff?
    core_loop();
}

bool breakpoint_exists(unsigned int address) {
    return find_breakpoint(address) != NULL;
}

void breakpoint_enable(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        ERROR("Breakpoint doesn't exist");
        return;
    }

    if (bp->enabled) {
        return;
    }
    bp->enabled = true;

    arch_jump_breakpoint_enable(bp); // TODO: change this to more generic arch_x_breakpoint_enable in the future
}

void breakpoint_disable(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        ERROR("Breakpoint doesn't exist");
        return;
    }

    if (!bp->enabled) {
        return;
    }
    bp->enabled = false;

    arch_jump_breakpoint_disable(bp); // TODO: change this to more generic arch_x_breakpoint_disable in the future
}

void breakpoint_add(unsigned int address, bool temporary) {
    if (find_breakpoint(address) != NULL) {
        ERROR("breakpoint already exists");
        return;
    } 

    struct breakpoint bp = { .address = address, .temporary = temporary };
    cvector_push_back(g_state.breakpoints, bp);
    breakpoint_enable(bp.address);
}

void breakpoint_remove(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);

    if (bp == NULL) {
        ERROR("breakpoint doesn't exists");
        return;
    }

    breakpoint_disable(bp->address);

    cvector_erase(g_state.breakpoints, bp-cvector_begin(g_state.breakpoints));
}

void breakpoint_disable_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        breakpoint_disable(bp->address);
    }
}
void breakpoint_restore_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        breakpoint_enable(bp->address);
    }
}

void breakpoint_remove_temporay() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if (bp->temporary) {
            breakpoint_remove(bp->address);
            bp--; // because of cvector_erase
        }
    }
}

