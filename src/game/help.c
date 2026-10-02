#include "game/help.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "engine/entity.h"
#include "game/message_box.h"
#include "game/particle.h"
#include "game/draw_functions.h"
#include "game/probe.h"
#include "engine/entity_class.h"
#include "engine/game_zone.h"

static int g_help_screen_up = 0;

static void screen_start_game_placeholder(button_t *self) {
    (void)self;
}

static button_fn_t g_start_game_method = screen_start_game_placeholder;

button_fn_t help_start_game_field_value(void) {
    return g_start_game_method;
}

void help_set_start_game(button_fn_t fn) {
    g_start_game_method = (fn != NULL) ? fn : screen_start_game_placeholder;
}

static int help_screen_hit_test_method(entity_t *self, const vec_t *qa, const vec_t *ra) {
    return entity_geom_contains_cursor(self, qa, ra);
}

/* The most items a row of `bo` holds. */
#define DIFFICULTY_OPTIONS_ROW_CAP 4

/* `pz(self, bo)`, each row's `my` the `eu` of this game. */
void help_screen_construct(probe_t *self) {
    button_keys_t bs[DIFFICULTY_OPTIONS_COUNT];
    layout_row_keys_t lq[DIFFICULTY_OPTIONS_COUNT];
    layout_row_cell_t items[DIFFICULTY_OPTIONS_COUNT][DIFFICULTY_OPTIONS_ROW_CAP];
    open_box_frame_t frame = {0};
    int i;

    if (self == NULL) {
        return;
    }
    for (i = 0; i < DIFFICULTY_OPTIONS_COUNT; i++) {
        const difficulty_options_entry_t *e = &difficulty_options_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = layout_row_keys_of_row(e->layout_row, items[i], DIFFICULTY_OPTIONS_ROW_CAP);
        bs[i].layout_row_keys = &lq[i];
        bs[i].name = e->name;
        bs[i].has_trade = 1;
        bs[i].trade = p8_fixed_from_int(e->trade);
        bs[i].has_type_bias = 1;
        bs[i].type_bias = e->type_bias;
        bs[i].has_on_click = 1;
        bs[i].on_click = help_start_game_field_value();
    }
    frame.has_open_sfx = 1;
    frame.open_sfx = self->archetype->open_sfx;
    frame.fixed_pos = &self->archetype->fixed_pos;
    frame.clear_ephemerals_after = 1;
    frame.has_back_color = 1;
    frame.back_color = self->archetype->back_color;
    frame.row_step = &self->archetype->row_step;
    frame.pos = self->pos;
    message_box_open_box(&frame, bs, DIFFICULTY_OPTIONS_COUNT);
}

/* `fv.rb` holds four articles in the cart. */
enum { HELP_SCREEN_ROWS_CAP = 4 };

void help_screen_draw(p8_graphics_t *gfx, p8_memory_t *mem, probe_t *self, vec_t p) {
    (void)p;

    emitter_update(gfx, mem, (entity_t *)self);

    if (gfx != NULL && mem != NULL) {
        const data_row_t *items[HELP_SCREEN_ROWS_CAP];
        int n = 0;
        int i;

        for (i = 0; self != NULL && self->archetype->rows != NULL && i < self->archetype->rows->count; i++) {
            assert(n < HELP_SCREEN_ROWS_CAP && "fv.rb has four articles in the cart");
            items[n++] = self->archetype->rows->items[i].row;
        }
        draw_functions_draw_rows_default(gfx, mem, items, n);
    }
}

void help_screen_draw_method(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    help_screen_draw(dispatch_context_gfx(), dispatch_context_mem(), (probe_t *)self, p);
}

static const entity_class_t g_help_screen_class = {
    .draw = help_screen_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = help_screen_hit_test_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .spawn_particle = probe_spawn_particle,
    .release = probe_release_method,
    .tags = TAG(TAG_PROBE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PROBE),
};

static probe_t *screen_create(void) {
    probe_t *self;

    assert(g_help_screen_up && "pico8_help_init() must run first");
    self = (probe_t *)allocator_alloc(game_zone(), sizeof *self);
    probe_init_instance(self, &g_help_screen_class, &help_screen_archetype);
    self->header.hit_box = &self->archetype->hit_box;
    self->header.cursor_sprite = &self->archetype->cursor_sprite;
    self->header.gpio_signal = &self->archetype->gpio_signal;
    self->header.is_hovered = &self->is_hovered;
    self->is_hovered = 0;
#ifdef PICO8_CLASS_COVERAGE
    class_coverage_record("help_screen");
#endif
    help_screen_construct(self);
    emitter_init_in((entity_t *)self, PARTICLE_SLOT_HELP_LIST);
    entity_construct(&self->header);
    return self;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * is built by `g_`'s init: both preconditions are asserted, not repaired. */
void help_init(void) {
    assert(draw_functions_dispatch_lookup("s") != NULL && "pico8_help_init() runs after pico8_pi_init()");
    assert(probe_ready() && "pico8_help_init() runs after pico8_probe_init()");

    help_shutdown();

    g_help_screen_up = 1;

    help_set_start_game(NULL);
}

void help_shutdown(void) {
    g_help_screen_up = 0;
    help_set_start_game(NULL);
}

int help_screen_ready(void) {
    return g_help_screen_up;
}

const entity_class_t *help_screen_class(void) {
    return &g_help_screen_class;
}

probe_t *help_screen_new(void) {
    return screen_create();
}
