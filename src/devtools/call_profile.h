#ifndef DEVTOOLS_CALL_PROFILE_H
#define DEVTOOLS_CALL_PROFILE_H

#include <stddef.h>
#include <stdint.h>

#include "devtools/draw_filter.h"
#include "devtools/draw_timing.h"
#include "p8/draw_backend.h"

/* full_game.json's 6,059 frames plus 1/8. */
#define CALL_PROFILE_FRAME_CAP 6817
#define CALL_PROFILE_CHILD_CAP 64
#define CALL_PROFILE_DEPTH_CAP 256
/* The per-child family table only keeps a short run's frames: it would not fit for a whole game. */
#define CALL_PROFILE_CHILD_FAMILY_FRAME_CAP DRAW_TIMING_CAP

typedef enum {
    CALL_PROFILE_UPDATE,
    CALL_PROFILE_DRAW,
    CALL_PROFILE_ROOT_COUNT
} call_profile_root_t;

/* Monotonic nanoseconds. */
typedef uint64_t (*call_profile_clock_t)(void);

/* A direct child of a root: one function entered from one call site. site is its rank, from 1,
 * among the sites of the same function under the same root, in first-entered order; sites is
 * how many such sites the run has seen so far. */
typedef struct {
    uintptr_t function;
    uintptr_t call_site;
    int site;
    int sites;
} call_profile_child_t;

typedef struct {
    uint32_t calls;
    uint64_t ns;
    /* Instrumented calls made under the child, at any depth. */
    uint32_t below;
} call_profile_child_figures_t;

/* units are the cost model's content primitives; a texture-window pair is one switch, never a unit. */
typedef struct {
    uint32_t calls;
    uint64_t ns;
    uint32_t units;
    uint32_t switches;
} call_profile_family_figures_t;

/* Empties every table and the call stack. Root addresses are non-zero and distinct. */
void call_profile_init(call_profile_clock_t clock, uintptr_t update_root, uintptr_t draw_root);

/* The compiler hooks' bookkeeping. Nesting past CALL_PROFILE_DEPTH_CAP, a root getting more
 * than CALL_PROFILE_CHILD_CAP children over the run, or an exit that is not the innermost
 * entry's, is a p8_fatal. */
void call_profile_enter(uintptr_t function, uintptr_t call_site);
void call_profile_exit(uintptr_t function, uintptr_t call_site);

/* Stores the figures since the last frame end as the next frame. Recording past
 * CALL_PROFILE_FRAME_CAP frames is a p8_fatal. */
void call_profile_frame_end(void);

int call_profile_frame_count(void);

uint64_t call_profile_root_ns(int frame, call_profile_root_t root);

/* Children of the whole run, in first-entered order. */
int call_profile_child_count(call_profile_root_t root);

call_profile_child_t call_profile_child_info(call_profile_root_t root, int child);

/* Writes symbol, or "symbol#<site>" when the function has several sites under its root.
 * Returns what snprintf returns. */
int call_profile_child_name(call_profile_child_t child, const char *symbol, char *buffer, size_t size);

/* A child first entered after frame has zeros there. */
call_profile_child_figures_t call_profile_child_figures(int frame, call_profile_root_t root, int child);

/* The root's time outside its children. */
uint64_t call_profile_self_ns(int frame, call_profile_root_t root);

/* ns minus below enter/exit pairs at pair_tenths_ns tenths of a nanosecond each, floored at 0. */
uint64_t call_profile_net_ns(call_profile_child_figures_t figures, uint64_t pair_tenths_ns);

/* Times pairs enter/exit pairs on a dummy address outside any root; returns tenths of a
 * nanosecond per pair. Must be called while no root is running; records nothing. */
uint64_t call_profile_measure_pair(int pairs);

/* Wraps the backend active at the call; call after call_profile_init. Every call counts under
 * its family; a rectfill or circfill with a non-zero fill pattern also counts under
 * DRAW_FILTER_PATTERN, its time and units staying inside its shape's. The cost model's texture
 * window restarts at every frame end. Returns the wrapped backend:
 * reinstalling it with p8_draw_backend_set removes the wrapper. */
const p8_draw_backend_t *call_profile_family_install(void);

call_profile_family_figures_t call_profile_family_figures(int frame, draw_filter_family_t family);

/* Every family's time but DRAW_FILTER_PATTERN's. */
uint64_t call_profile_in_call_ns(int frame);

/* The family figures of the calls made while child was the active direct child of root, at any
 * depth below it. Zeros at or past CALL_PROFILE_CHILD_FAMILY_FRAME_CAP: those frames are not kept. */
call_profile_family_figures_t call_profile_child_family_figures(int frame, call_profile_root_t root, int child,
                                                                draw_filter_family_t family);

/* The same for the calls made by root itself, outside any child. */
call_profile_family_figures_t call_profile_self_family_figures(int frame, call_profile_root_t root,
                                                               draw_filter_family_t family);

/* Every family's time under child but DRAW_FILTER_PATTERN's. */
uint64_t call_profile_child_in_call_ns(int frame, call_profile_root_t root, int child);

#endif
