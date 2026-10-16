#include "devtools/call_profile.h"

#include "p8/draw_cost.h"
#include "p8/fatal.h"
#include "p8/image.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint64_t root_ns[CALL_PROFILE_ROOT_COUNT];
    call_profile_child_figures_t children[CALL_PROFILE_ROOT_COUNT][CALL_PROFILE_CHILD_CAP];
    call_profile_family_figures_t families[DRAW_FILTER_FAMILY_COUNT];
} frame_record_t;

typedef enum {
    FRAME_PLAIN,
    FRAME_ROOT,
    FRAME_CHILD
} frame_kind_t;

typedef struct {
    uintptr_t function;
    frame_kind_t kind;
    int root;
    int child;
    uint64_t start;
} stack_entry_t;

static call_profile_clock_t clock_now;
static uintptr_t roots[CALL_PROFILE_ROOT_COUNT];
static call_profile_child_t children[CALL_PROFILE_ROOT_COUNT][CALL_PROFILE_CHILD_CAP];
static int child_counts[CALL_PROFILE_ROOT_COUNT];
static stack_entry_t stack[CALL_PROFILE_DEPTH_CAP];
static int depth;
static int active_root;
static int active_child;
static frame_record_t current;
static frame_record_t table[CALL_PROFILE_FRAME_CAP];
static int count;
static const p8_draw_backend_t *wrapped;
static p8_draw_cost_window_t window;
static int counting_units;
static p8_image_size_t image_sizes[P8_IMAGE_SLOT_COUNT];

void call_profile_init(call_profile_clock_t clock, uintptr_t update_root, uintptr_t draw_root) {
    assert(clock != NULL);
    assert(update_root != 0 && draw_root != 0 && update_root != draw_root);
    clock_now = clock;
    roots[CALL_PROFILE_UPDATE] = update_root;
    roots[CALL_PROFILE_DRAW] = draw_root;
    memset(children, 0, sizeof children);
    memset(child_counts, 0, sizeof child_counts);
    depth = 0;
    active_root = -1;
    active_child = -1;
    memset(&current, 0, sizeof current);
    memset(table, 0, sizeof table);
    count = 0;
}

static int root_of(uintptr_t function) {
    for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
        if (roots[root] == function) return root;
    }
    return -1;
}

static int child_index(int root, uintptr_t function, uintptr_t call_site) {
    int sites = 0;
    for (int child = 0; child < child_counts[root]; child++) {
        call_profile_child_t *known = &children[root][child];
        if (known->function != function) continue;
        if (known->call_site == call_site) return child;
        sites++;
    }
    if (child_counts[root] == CALL_PROFILE_CHILD_CAP) {
        char message[80];
        snprintf(message, sizeof message, "call profile: more than %d children under one root", CALL_PROFILE_CHILD_CAP);
        p8_fatal(message);
    }
    int child = child_counts[root]++;
    children[root][child].function = function;
    children[root][child].call_site = call_site;
    children[root][child].site = sites + 1;
    return child;
}

void call_profile_enter(uintptr_t function, uintptr_t call_site) {
    if (counting_units) return;
    if (depth == CALL_PROFILE_DEPTH_CAP) {
        char message[64];
        snprintf(message, sizeof message, "call profile: call stack deeper than %d", CALL_PROFILE_DEPTH_CAP);
        p8_fatal(message);
    }
    stack_entry_t *entry = &stack[depth++];
    entry->function = function;
    entry->kind = FRAME_PLAIN;
    int root = active_root < 0 ? root_of(function) : -1;
    if (root >= 0) {
        entry->kind = FRAME_ROOT;
        entry->root = root;
        active_root = root;
        entry->start = clock_now();
    } else if (depth >= 2 && stack[depth - 2].kind == FRAME_ROOT) {
        entry->kind = FRAME_CHILD;
        entry->root = active_root;
        entry->child = child_index(active_root, function, call_site);
        active_child = entry->child;
        entry->start = clock_now();
    } else if (active_child >= 0) {
        current.children[active_root][active_child].below++;
    }
}

void call_profile_exit(uintptr_t function, uintptr_t call_site) {
    (void)call_site;
    if (counting_units) return;
    if (depth == 0 || stack[depth - 1].function != function) p8_fatal("call profile: exit without its entry");
    stack_entry_t *entry = &stack[--depth];
    if (entry->kind == FRAME_ROOT) {
        current.root_ns[entry->root] += clock_now() - entry->start;
        active_root = -1;
    } else if (entry->kind == FRAME_CHILD) {
        call_profile_child_figures_t *figures = &current.children[entry->root][entry->child];
        figures->calls++;
        figures->ns += clock_now() - entry->start;
        active_child = -1;
    }
}

void call_profile_frame_end(void) {
    if (count == CALL_PROFILE_FRAME_CAP) {
        char message[80];
        snprintf(message, sizeof message, "call profile frame table full at frame %d, capacity %d", count,
                 CALL_PROFILE_FRAME_CAP);
        p8_fatal(message);
    }
    table[count++] = current;
    memset(&current, 0, sizeof current);
    p8_draw_cost_frame_begin(&window);
}

int call_profile_frame_count(void) {
    return count;
}

uint64_t call_profile_root_ns(int frame, call_profile_root_t root) {
    assert(frame >= 0 && frame < count);
    return table[frame].root_ns[root];
}

int call_profile_child_count(call_profile_root_t root) {
    return child_counts[root];
}

call_profile_child_t call_profile_child_info(call_profile_root_t root, int child) {
    assert(child >= 0 && child < child_counts[root]);
    call_profile_child_t info = children[root][child];
    info.sites = 0;
    for (int other = 0; other < child_counts[root]; other++) {
        if (children[root][other].function == info.function) info.sites++;
    }
    return info;
}

int call_profile_child_name(call_profile_child_t child, const char *symbol, char *buffer, size_t size) {
    if (child.sites <= 1) return snprintf(buffer, size, "%s", symbol);
    return snprintf(buffer, size, "%s#%d", symbol, child.site);
}

call_profile_child_figures_t call_profile_child_figures(int frame, call_profile_root_t root, int child) {
    assert(frame >= 0 && frame < count);
    assert(child >= 0 && child < child_counts[root]);
    return table[frame].children[root][child];
}

uint64_t call_profile_self_ns(int frame, call_profile_root_t root) {
    uint64_t self = call_profile_root_ns(frame, root);
    for (int child = 0; child < child_counts[root]; child++) {
        self -= table[frame].children[root][child].ns;
    }
    return self;
}

uint64_t call_profile_net_ns(call_profile_child_figures_t figures, uint64_t pair_tenths_ns) {
    uint64_t hooks = (uint64_t)figures.below * pair_tenths_ns / 10;
    return hooks >= figures.ns ? 0 : figures.ns - hooks;
}

uint64_t call_profile_measure_pair(int pairs) {
    assert(pairs > 0);
    assert(active_root < 0 && "the dummy pairs would count as children");
    uint64_t start = clock_now();
    for (int pair = 0; pair < pairs; pair++) {
        call_profile_enter(0, 0);
        call_profile_exit(0, 0);
    }
    uint64_t span = clock_now() - start;
    return span * 10 / (uint64_t)pairs;
}

static uint64_t family_calls_end(draw_filter_family_t family, uint64_t start, int calls) {
    uint64_t ns = clock_now() - start;
    current.families[family].calls += (uint32_t)calls;
    current.families[family].ns += ns;
    return ns;
}

static uint64_t family_end(draw_filter_family_t family, uint64_t start) {
    return family_calls_end(family, start, 1);
}

static int patterned(const p8_graphics_t *gfx) {
    return (gfx->fillp_pattern & 0xFFFFu) != 0;
}

static void pattern_end(const p8_graphics_t *gfx, uint64_t ns) {
    if (!patterned(gfx)) return;
    current.families[DRAW_FILTER_PATTERN].calls++;
    current.families[DRAW_FILTER_PATTERN].ns += ns;
}

/* The cost model is instrumented code: between units_begin and units_end its hooks are ignored,
 * so that it never counts as calls below a child. Returns the window before the call. */
static p8_draw_cost_window_t units_begin(void) {
    counting_units = 1;
    return window;
}

static void units_add(draw_filter_family_t family, uint32_t units, uint32_t switches) {
    current.families[family].units += units;
    current.families[family].switches += switches;
}

static void units_end(draw_filter_family_t family, p8_draw_cost_window_t before, p8_draw_cost_t cost, int in_pattern) {
    counting_units = 0;
    uint32_t switches = before.patterned != window.patterned || before.pattern_key != window.pattern_key;
    assert(cost.primitives >= 2 * (int)switches && "a window switch is two primitives of its call");
    uint32_t units = (uint32_t)cost.primitives - 2 * switches;
    units_add(family, units, switches);
    if (in_pattern) units_add(DRAW_FILTER_PATTERN, units, switches);
}

static void family_cls(p8_vram_t *mem) {
    uint64_t start = clock_now();
    wrapped->cls(mem);
    family_end(DRAW_FILTER_CLS, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_CLS, before, p8_draw_cost_cls(), 0);
}

static void family_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    uint64_t start = clock_now();
    wrapped->pset(gfx, mem, x, y, col);
    family_end(DRAW_FILTER_PSET, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_PSET, before, p8_draw_cost_pset(gfx, x, y), 0);
}

static void family_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    uint64_t start = clock_now();
    wrapped->line(gfx, mem, x1, y1, x2, y2, col);
    family_end(DRAW_FILTER_LINE, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_LINE, before, p8_draw_cost_line(gfx, x1, y1, x2, y2), 0);
}

static void family_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    uint64_t start = clock_now();
    wrapped->rect(gfx, mem, x1, y1, x2, y2, col);
    family_end(DRAW_FILTER_RECT, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_RECT, before, p8_draw_cost_rect(gfx, x1, y1, x2, y2), 0);
}

static void family_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    uint64_t start = clock_now();
    wrapped->rectfill(gfx, mem, x1, y1, x2, y2, col);
    pattern_end(gfx, family_end(DRAW_FILTER_RECTFILL, start));
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_RECTFILL, before, p8_draw_cost_rectfill(&window, gfx, x1, y1, x2, y2, col), patterned(gfx));
}

static void family_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    uint64_t start = clock_now();
    wrapped->circ(gfx, mem, cx, cy, r, col);
    family_end(DRAW_FILTER_CIRC, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_CIRC, before, p8_draw_cost_circ(gfx, cx, cy, r), 0);
}

static void family_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    uint64_t start = clock_now();
    wrapped->circfill(gfx, mem, cx, cy, r, col);
    pattern_end(gfx, family_end(DRAW_FILTER_CIRCFILL, start));
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_CIRCFILL, before, p8_draw_cost_circfill(&window, gfx, cx, cy, r, col), patterned(gfx));
}

static void family_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    uint64_t start = clock_now();
    wrapped->spr(gfx, mem, n, x, y, w, h);
    family_end(DRAW_FILTER_SPR, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_SPR, before, p8_draw_cost_spr(&window, gfx, n, x, y, w, h), 0);
}

static void family_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    uint64_t start = clock_now();
    wrapped->print(gfx, mem, str, x, y, col);
    family_end(DRAW_FILTER_PRINT, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_PRINT, before, p8_draw_cost_print(&window, gfx, str, x, y), 0);
}

static void family_print_outlined(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    uint64_t start = clock_now();
    wrapped->print_outlined(gfx, mem, str, x, y, col);
    family_end(DRAW_FILTER_PRINT, start);
    p8_draw_cost_window_t before = units_begin();
    units_end(DRAW_FILTER_PRINT, before, p8_draw_cost_print_outlined(&window, gfx, str, x, y), 0);
}

static void family_image_begin(int slot, int w, int h) {
    image_sizes[slot].width = w;
    image_sizes[slot].height = h;
    wrapped->image_begin(slot, w, h);
}

static void family_image_window(int slot, int x, int y) {
    wrapped->image_window(slot, x, y);
}

static void family_image_end(int slot) {
    wrapped->image_end(slot);
}

/* Under spr, as draw_timing.c counts it: one sprite unit and one page switch when on screen. */
static void family_image_draw(const p8_graphics_t *gfx, p8_vram_t *mem, int slot, int x, int y) {
    uint64_t start = clock_now();
    wrapped->image_draw(gfx, mem, slot, x, y);
    family_end(DRAW_FILTER_SPR, start);
    (void)units_begin();
    p8_draw_cost_t cost = p8_draw_cost_image_draw(&window, gfx, x, y, image_sizes[slot].width, image_sizes[slot].height);
    counting_units = 0;
    uint32_t on_screen = cost.primitives > 0;
    units_add(DRAW_FILTER_SPR, on_screen, on_screen);
}

static void family_lines(const p8_graphics_t *gfx, p8_vram_t *mem, const p8_segment_t *segments, int count, int col) {
    uint64_t start = clock_now();
    wrapped->lines(gfx, mem, segments, count, col);
    family_calls_end(DRAW_FILTER_LINE, start, count);
    for (int i = 0; i < count; i++) {
        const p8_segment_t *s = &segments[i];
        p8_draw_cost_window_t before = units_begin();
        units_end(DRAW_FILTER_LINE, before, p8_draw_cost_line(gfx, s->x1, s->y1, s->x2, s->y2), 0);
    }
}

static const p8_draw_backend_t FAMILIES = {
    family_cls, family_pset, family_line, family_rect, family_rectfill,
    family_circ, family_circfill, family_spr, family_print, family_print_outlined,
    family_image_begin, family_image_window, family_image_end, family_image_draw,
    family_lines,
};

const p8_draw_backend_t *call_profile_family_install(void) {
    const p8_draw_backend_t *active = p8_draw_backend_get();
    assert(active != &FAMILIES && "the family wrapper would wrap itself");
    assert(clock_now != NULL && "call_profile_init first");
    wrapped = active;
    p8_draw_cost_frame_begin(&window);
    p8_draw_backend_set(&FAMILIES);
    return wrapped;
}

call_profile_family_figures_t call_profile_family_figures(int frame, draw_filter_family_t family) {
    assert(frame >= 0 && frame < count);
    assert(family >= 0 && family < DRAW_FILTER_FAMILY_COUNT);
    return table[frame].families[family];
}

uint64_t call_profile_in_call_ns(int frame) {
    uint64_t in_call = 0;
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        if (family != DRAW_FILTER_PATTERN) in_call += call_profile_family_figures(frame, (draw_filter_family_t)family).ns;
    }
    return in_call;
}
