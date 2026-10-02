#include "game/mouse.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "game/game_data.h"
#include "game/mouse_button.h"
#include "game/draw_functions.h"
#include "engine/vec.h"
#include "engine/game_zone.h"

static int g_mouse_up = 0;
static mouse_button_t g_left_button;
static mouse_button_t g_right_button;
/* Not in the archetype: two TUs write `je.ra` in place. */
static vec_t g_mouse_pos;

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * `t` is the frame counter, which nf:ph does not read. */
static void mouse_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    mouse_update(dispatch_context_input(), (mouse_t *)self);
}

static const entity_class_t g_mouse_class = {
    .states = {[ENTITY_STATE_NORMAL] = mouse_update_method},
};

static void mouse_construct(p8_memory_t *mem) {
    if (mem == NULL) {
        return;
    }
    p8_poke(mem, 0x5f2d, 1);
}

static mouse_t *mouse_create(void) {
    mouse_t *self = (mouse_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &g_mouse_class, mouse_archetype.state, mouse_archetype.state_time);
    self->archetype = &mouse_archetype;
    /* Read-only: this module writes through its member. */
    self->header.pos = (const vec_t *const *)&self->pos;
    self->pos = &g_mouse_pos;
#ifdef PICO8_CLASS_COVERAGE
    class_coverage_record("mouse");
#endif
    mouse_construct(dispatch_context_mem());
    entity_construct(&self->header);
    return self;
}

void mouse_init(void) {
    mouse_shutdown();
    draw_functions_init();
    assert(entity_ready() && "nf descends from ke");
    g_mouse_up = 1;
    g_mouse_pos = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    memset(&g_left_button, 0, sizeof g_left_button);
    memset(&g_right_button, 0, sizeof g_right_button);
}

void mouse_shutdown(void) {
    g_mouse_up = 0;
}

const mouse_button_t *mouse_left_button(void) {
    return g_mouse_up ? &g_left_button : NULL;
}

const mouse_button_t *mouse_right_button(void) {
    return g_mouse_up ? &g_right_button : NULL;
}

int mouse_ready(void) {
    return g_mouse_up;
}

const entity_class_t *mouse_class(void) {
    return &g_mouse_class;
}

mouse_t *mouse_new(void) {
    return mouse_create();
}

void mouse_update(const p8_input_t *input, mouse_t *self) {
    if (self == NULL || input == NULL) {
        return;
    }

    self->pos_own = vec_make(p8_fixed_from_int(p8_stat(input, 32)), p8_fixed_from_int(p8_stat(input, 33)));
    self->pos = &self->pos_own;

    if (g_mouse_up) {
        mouse_button_update(input, &g_left_button, 1);
        mouse_button_update(input, &g_right_button, 6);
    }
}
