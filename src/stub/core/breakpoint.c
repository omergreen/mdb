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

bool breakpoint_enable(struct breakpoint *bp) {
    if (bp->enabled) {
        ERROR("breakpoint is already enabled");
        return false;
    }
    if (bp->type == BREAKPOINT_TYPE_JUMP_OR_SOFTWARE) {
        ERROR("invalid bp->type (JUMP_OR_SOFTWARE should be handled before this function)");
        return false;
    }
    if (!g_target_config.supports_real_breakpoints && (bp->type == BREAKPOINT_TYPE_SOFTWARE || bp->type == BREAKPOINT_TYPE_HARDWARE)) {
        return false;
    }

    bool success;
    switch (bp->type) {
        case BREAKPOINT_TYPE_JUMP:
            success = arch_jump_breakpoint_enable(bp); 
            break;
        case BREAKPOINT_TYPE_SOFTWARE:
            success = arch_software_breakpoint_enable(bp);
            break;
        case BREAKPOINT_TYPE_HARDWARE:
            success = arch_hardware_breakpoint_enable(bp);
            break;
        default:
            ERROR("unknown bp type (%d)", bp->type);
            success = false;
            break;
    }

    bp->enabled = success;
    return success;
}

void breakpoint_disable(struct breakpoint *bp) {
    if (!bp->enabled) {
        ERROR("breakpoint is already disabled");
        return;
    }
    if (bp->type == BREAKPOINT_TYPE_JUMP_OR_SOFTWARE) {
        ERROR("invalid bp->type (JUMP_OR_SOFTWARE should be handled before this function)");
        return;
    }

    switch (bp->type) {
        case BREAKPOINT_TYPE_JUMP:
            arch_jump_breakpoint_disable(bp); 
            break;
        case BREAKPOINT_TYPE_SOFTWARE:
            arch_software_breakpoint_disable(bp);
            break;
        case BREAKPOINT_TYPE_HARDWARE:
            arch_hardware_breakpoint_disable(bp);
            break;
        default:
            ERROR("unknown bp type (%d)", bp->type);
            return;
    }

    bp->enabled = false;
}

bool breakpoint_add(unsigned int address, bool temporary, enum breakpoint_type type) {
    if (find_breakpoint(address) != NULL) {
        ERROR("breakpoint already exists");
        return false;
    } 

    struct breakpoint bp = { .address = address, .temporary = temporary, .enabled = false };

    if (type != BREAKPOINT_TYPE_JUMP_OR_SOFTWARE) {
        bp.type = type;
        if (!breakpoint_enable(&bp)) {
            ERROR("unable to add breakpoint of type %d", type);
            return false;
        }
    }
    else {
        // first, try jump
        bp.type = BREAKPOINT_TYPE_JUMP;
        if (!breakpoint_enable(&bp)) {
            // try software
            bp.type = BREAKPOINT_TYPE_SOFTWARE;
            if (!breakpoint_enable(&bp)) {
                return false;
            }
        }
    }

    cvector_push_back(g_state.breakpoints, bp);
    
    return true;
}

void breakpoint_remove(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);

    if (bp == NULL) {
        ERROR("breakpoint doesn't exists");
        return;
    }

    if (bp->enabled) {
        breakpoint_disable(bp);
    }

    cvector_erase(g_state.breakpoints, bp-cvector_begin(g_state.breakpoints));
}

void breakpoint_disable_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if ((bp->type == BREAKPOINT_TYPE_JUMP || bp->type == BREAKPOINT_TYPE_SOFTWARE) && bp->enabled) {
            breakpoint_disable(bp);
        }
    }
}
void breakpoint_restore_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if ((bp->type == BREAKPOINT_TYPE_JUMP || bp->type == BREAKPOINT_TYPE_SOFTWARE) && !bp->enabled) {
            breakpoint_enable(bp);
        }
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

