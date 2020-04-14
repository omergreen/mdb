#include "breakpoint.h"
#include "state.h"
#include <libc/cvector.h>
#include "core.h"

void breakpoint_enable(unsigned int address);
void breakpoint_disable(unsigned int address);

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
    core_loop();
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

    /* if (bp->enabled) { */
    breakpoint_disable(bp->address);
    /* } */

    cvector_erase(g_state.breakpoints, bp-cvector_begin(g_state.breakpoints));
}

bool breakpoint_exists(unsigned int address) {
    struct breakpoint *bp = find_breakpoint(address);
    if (bp == NULL) {
        return false;
    }

    return true;
    /* return bp->enabled; */
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
    /* if (bp->enabled) { */
    /*     ERROR("Breakpoint already enabled"); */
    /*     return; */
    /* } */

    arch_jump_breakpoint_enable(bp); // TODO: change this to more generic arch_x_breakpoint_enable in the future
    /* bp->enabled = true; */

    /* if (temporary) { */
    /*     bp->flip_on_next_stop = true; */
    /* } */
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
    /* if (!bp->enabled) { */
    /*     ERROR("Breakpoint already disabled"); */
    /*     return; */
    /* } */

    arch_jump_breakpoint_disable(bp); // TODO: change this to more generic arch_x_breakpoint_disable in the future
    /* bp->enabled = false; */

    /* if (temporary) { */
    /*     bp->flip_on_next_stop = true; */
    /* } */
}

struct breakpoint_state {
    unsigned int address;
    bool flip_on_next_stop;
};
cvector_vector_type(struct breakpoint_state) breakpoints_state = NULL;

void breakpoint_disable_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        breakpoint_disable(bp->address);
        /* if (bp->enabled) { */
        /*     struct breakpoint_state state = { .address = bp->address, .flip_on_next_stop = bp->flip_on_next_stop }; */
        /*     cvector_push_back(breakpoints_state, state); */
        /*     breakpoint_disable(bp->address, false); */
        /* } */
    }
}
void breakpoint_restore_all_temporarily() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        breakpoint_enable(bp->address);
    }
    /* for (struct breakpoint_state *bs = cvector_begin(breakpoints_state); bs != cvector_end(breakpoints_state); bs++) { */
    /*     breakpoint_enable(bs->address, bs->flip_on_next_stop); */
    /* } */

    /* cvector_free(breakpoints_state); */
    /* breakpoints_state = NULL; */
}

void breakpoint_remove_temporay() {
    for (struct breakpoint *bp = cvector_begin(g_state.breakpoints); bp != cvector_end(g_state.breakpoints); bp++) {
        if (bp->temporary) {
            breakpoint_remove(bp->address);
            bp--; // because of cvector_erase
        }
    }
}

