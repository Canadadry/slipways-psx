/* The help screen, built by its only road (cart_init() on the title screen,
 * help_screen_new()), its defaults read on its archetype, and driven
 * through the real on_click, draw_layers(), update_entities() and
 * economy_recompute(). Its methods are compared to the public functions. No
 * events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/probe.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"


#define BUILT_HASH 0x05BC81D1u
#define LIFE_HASH 0x64B6480Bu
#define HOVER_HASH 0xB508C2D4u

static p8_memory_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static int entity_list_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int entity_list_listed(const entity_list_t *list, const void *e) {
    return entity_list_rank_in(list, e) >= 0;
}

/* 0 absent or false, 2 true. */
static uint32_t is_hovered_state(const probe_t *fv) {
    return fv->is_hovered ? 2 : 0;
}

/* The words the frozen hashes fold before a value: 0xF1 a number, 0xB0 a
 * boolean (then 0 or 1), 0x57 a string; any other moves every *_HASH. */
static uint32_t fold_fixed(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

/* A class's tags, in enum order: each position, then its name, then
 * the count. */
static uint32_t fold_tags(uint32_t h, unsigned tags) {
    static const char *const names[TAG_COUNT] = {
        [TAG_STRUCTURE] = "structure", [TAG_ASCENSION] = "ascension", [TAG_SLIPGATE] = "slipgate",
        [TAG_PROBE] = "probe", [TAG_MESSAGE_BOX] = "message_box", [TAG_SLIPWAY] = "slipway",
        [TAG_WORMHOLE] = "wormhole", [TAG_PLANET] = "planet", [TAG_NODE] = "node",
        [TAG_TARGETING] = "targeting",
    };
    uint32_t k = 0;
    int t;

    for (t = 0; t < TAG_COUNT; t++) {
        if (tags & TAG(t)) {
            k++;
            h = fnv(h, k);
            h = fold_string(h, names[t]);
        }
    }
    return fnv(h, k);
}

/* An integer sequence: each position, then its value, then the count. */
static uint32_t fold_seq(uint32_t h, const int_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_fixed(h, p8_fixed_from_int(s->items[i]));
    }
    return fnv(h, (uint32_t)s->count);
}

/* The names the frozen hashes folded, before draw functions were an enum. */
static const char *const draw_function_names[DRAW_FUNCTION_COUNT] = {
    [DRAW_FUNCTION_DRAW_BAR] = "draw_bar",
    [DRAW_FUNCTION_PRINT_OUTLINED] = "print_outlined",
    [DRAW_FUNCTION_DRAW_HELP_FRAME] = "draw_help_frame",
    [DRAW_FUNCTION_SPR] = "spr",
    [DRAW_FUNCTION_DRAW_ROW] = "draw_row",
};

/* A generated row: items (a nested row folded the same way), then its
 * color, then its draw function, then the entry count. */
static uint32_t fold_row(uint32_t h, const data_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        const data_item_t *item = &row->items[i];

        h = fnv(h, (uint32_t)(i + 1));
        if (item->row != NULL) {
            h = fold_row(fnv(h, 0x7Au), item->row);
        } else if (item->text != NULL) {
            h = fold_string(h, item->text);
        } else if (item->is_false) {
            h = fold_bool(h, 0);
        } else {
            h = fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->number));
        }
    }
    if (row->has_color) {
        h = fold_fixed(fnv_str(h, "color"), p8_fixed_from_int(row->color));
    }
    if (row->draw_function != DRAW_FUNCTION_NONE) {
        h = fold_string(fnv_str(h, "draw_function"), draw_function_names[row->draw_function]);
    }
    return fnv(h, (uint32_t)(row->count + (row->has_color != 0) + (row->draw_function != DRAW_FUNCTION_NONE)));
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (entity_list_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)entity_list_rank_in(live_entities(), ref));
    }
    if (ref == (const void *)&help_screen_trail_colors_seq) {
        return fold_seq(fnv(h, 0x7Au), ref);
    }
    if (ref == (const void *)&help_screen_rows_row) {
        return fold_row(fnv(h, 0x7Au), ref);
    }
    return fnv(fnv(h, 0xEEEEu), 0u);
}

/* A member the reader always answers: its presence bit and kind byte are
 * constant by the struct's shape, kept so that BUILT_HASH and LIFE_HASH
 * still prove the typed reads. */
static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)b);
}

static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fnv(h, 0u) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    return ref == NULL ? fnv(h, 0u) : fold_ref(fnv(h, 1u), ref);
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
}

static uint32_t fold_box(uint32_t h, const box_t *b) {
    assert(b != NULL);
    h = fnv(fnv(h, (uint32_t)b->left), (uint32_t)b->top);
    return fnv(fnv(h, (uint32_t)b->right), (uint32_t)b->bottom);
}

/* The item count of a list member, -1 when NULL. */
static int size_of(const pointer_list_t *list) {
    return list == NULL ? -1 : list->count;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&memory, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static vec_t *vec_ref(int x, int y) {
    vec_t *v = malloc(sizeof *v);
    *v = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return v;
}

static const entity_class_t *class_of(const void *e) {
    return ((const entity_t *)e)->class;
}

static void start_title(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
}

/* The help, whatever its form: the probe's tags and layer 10 (a probe has
 * 2). */
static int is_help(entity_t *e) {
    const probe_t *g_ =
        e != NULL && (e->class->lineage & LINEAGE(LINEAGE_PROBE)) ? (const probe_t *)e : NULL;
    return g_ != NULL && g_->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)) && g_->archetype->layer == p8_fixed_from_int(10);
}

static probe_t *the_help(void) {
    const entity_list_t *hv = live_entities();
    entity_t *found = NULL;
    int i;

    for (i = 0; i < hv->count; i++) {
        if (is_help(hv->items[i])) {
            assert(found == NULL);
            found = hv->items[i];
        }
    }
    assert(found != NULL && found->class == help_screen_class());
    return (probe_t *)found;
}

static int in_layer(int layer, const void *e) {
    const ui_hovered_by_layer_t *ic = main_ui()->hovered_by_layer;

    if (ic == NULL) {
        return 0;
    }
    return entity_list_listed(&ic->layers[layer], e);
}

static const button_t *as_button(const void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == button_class());
    return e;
}

/* The title's help as its construction, emitter_init_in() and
 * entity_register() left it: its 22 defaults and the 6 of the probe, their
 * tables' contents and identity, its state and state time, its empty
 * particles; no name only read or written elsewhere; its methods; the 4
 * buttons message_box_open_box() built before it, each carrying the
 * on_click stamped from its difficulty_options_table row; where
 * entity_register() filed it; main_ui()'s ephemerals emptied. */
static void test_built(void) {
    uint32_t hash = 2166136261u;
    const entity_list_t *live;
    probe_t *help_screen;
    const probe_t *probe;
    const emitter_t *emitter;
    const targeting_t *targeting;
    int rank;
    size_t index;
    int key_index;

    start_title();
    help_screen = the_help();
    probe = help_screen;
    emitter = probe->emitter;
    targeting = probe->targeting;
    live = live_entities();

    assert(probe->archetype->layer == p8_fixed_from_int(10));
    assert(emitter->archetype->lifetime == p8_fixed_from_int(17));
    assert(probe->archetype->back_color == p8_fixed_from_int(11));
    assert(probe->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)));
    assert(probe->pos != NULL && emitter->trail_colors != NULL);
    assert(probe->archetype->rows != NULL && targeting->archetype->confirmed_state != ENTITY_STATE_NONE);
    hash = fold_fixed_member(hash, probe->archetype->layer);
    hash = fold_ref_member(hash, probe->pos);
    hash = fold_ref_member(hash, &probe->archetype->fixed_pos);
    hash = fold_fixed_member(hash, probe->archetype->back_color);
    hash = fold_ref_member(hash, &probe->archetype->row_step);
    hash = fold_fixed_member(hash, probe->archetype->clear_ephemerals_after);
    hash = fold_fixed_member(hash, emitter->archetype->spawn_rate);
    hash = fold_ref_member(hash, emitter->trail_colors);
    hash = fold_fixed_member(hash, emitter->matrix_xx);
    hash = fold_fixed_member(hash, emitter->matrix_xy);
    hash = fold_fixed_member(hash, emitter->matrix_yx);
    hash = fold_fixed_member(hash, emitter->matrix_yy);
    hash = fold_fixed_member(hash, emitter->angle_slices);
    hash = fold_fixed_member(hash, emitter->base_radius);
    hash = fold_fixed_member(hash, emitter->speed_factor);
    hash = fold_fixed_member(hash, emitter->archetype->quarter_turn);
    hash = fold_fixed_member(hash, emitter->archetype->lifetime);
    hash = fold_ref_member(hash, &probe->archetype->hit_box);
    hash = fold_fixed_member(hash, probe->archetype->cursor_sprite);
    hash = fold_ref_member(hash, probe->archetype->rows);
    hash = fold_tags(fnv(fnv(hash, 1u), 0x7Au), probe->header.class->tags);
    hash = fold_fixed_member(hash, probe->archetype->fold_anchor);
    hash = fold_string_member(hash, entity_state_name(targeting->archetype->confirmed_state));
    hash = fold_fixed_member(hash, targeting->cost);
    hash = fold_bool_member(hash, targeting->archetype->clock_ticks != 0);
    hash = fold_fixed_member(hash, targeting->archetype->attach_distance);
    hash = fold_vec(hash, probe->pos);
    hash = fold_vec(hash, &probe->archetype->fixed_pos);
    hash = fold_vec(hash, &probe->archetype->row_step);
    hash = fold_box(hash, &probe->archetype->hit_box);
    assert(probe->archetype == &help_screen_archetype && probe->pos == &help_screen_archetype.pos);
    assert(emitter->trail_colors == &help_screen_trail_colors_seq);
    assert(emitter->trail_colors != &probe_trail_colors_seq);
    assert(probe->archetype->rows == &help_screen_rows_row);
    hash = fold_string_member(hash, entity_state_name(probe->header.state));
    hash = fold_fixed_member(hash, probe->header.state_time);
    hash = fnv(hash, (uint32_t)size_of(emitter->particles));
    assert(size_of(emitter->particles) == 0);
    assert(!help_screen->is_hovered && probe->map_pos == NULL && targeting->target == NULL && !probe->is_dead);
    assert(class_of(help_screen)->new_year == NULL && class_of(help_screen)->refresh == NULL && class_of(help_screen)->states[ENTITY_STATE_NORMAL] == NULL &&
           class_of(help_screen)->left_press == NULL);

    assert(class_of(help_screen)->draw == help_screen_draw_method);
    assert(class_of(help_screen)->draw_overlay == targeting_tooltip_method);
    assert(class_of(help_screen)->spawn_particle == probe_spawn_particle);
    assert(class_of(help_screen)->drag == targeting_track_method);
    assert(class_of(help_screen)->drop == targeting_confirm_method);
    assert(class_of(help_screen)->hit_test != NULL);

    rank = entity_list_rank_in(live, help_screen);
    hash = fnv(hash, (uint32_t)rank);
    hash = fnv(hash, (uint32_t)live->count);
    assert(rank >= 4);
    for (key_index = 0; key_index < 4; key_index++) {
        const button_t *b = as_button(live->items[rank - 4 + key_index]);
        vec_t qf;
        assert(b->layer == p8_fixed_from_int(11));
        assert(b->slide_target != NULL);
        qf = *b->slide_target;
        assert(qf.x == p8_fixed_from_int(34) && qf.y == p8_fixed_from_int(56 + 11 * key_index));
        hash = fold_vec(hash, b->slide_target);
        hash = fold_vec(hash, b->pos);
        hash = fold_fixed_member(hash, b->layer);
    }
    for (index = 0; index < 5; index++) {
        static const entity_message_t messages[] = {ENTITY_MESSAGE_DRAW, ENTITY_MESSAGE_DRAW_OVERLAY, ENTITY_MESSAGE_HIT_TEST, ENTITY_MESSAGE_NEW_YEAR, ENTITY_MESSAGE_REFRESH};
        hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message(messages[index]), help_screen));
    }
    assert(entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), help_screen) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), help_screen) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_HIT_TEST), help_screen));
    assert(!entity_list_listed(entities_by_message(ENTITY_MESSAGE_NEW_YEAR), help_screen) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_REFRESH), help_screen));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_tag(TAG_PROBE), help_screen));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_tag(TAG_TARGETING), help_screen));
    assert(entity_list_listed(entities_by_tag(TAG_PROBE), help_screen) && entity_list_listed(entities_by_tag(TAG_TARGETING), help_screen));

    for (key_index = 0; key_index < 4; key_index++) {
        const button_t *row = (const button_t *)live->items[rank - 4 + key_index];
        assert(row->has_on_click);
        assert(row->on_click == help_start_game_field_value());
    }
    assert(main_ui()->ephemerals != NULL && main_ui()->ephemerals->count == 0);
    check("built", hash, BUILT_HASH);
}

/* draw_layers(ENTITY_MESSAGE_DRAW) x3 with the screen and the particles;
 * draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY); update_entities() x3; hit_test in and out of
 * the hit box, the screen cursor kept since the layer is 10 or more. */
static void test_life(void) {
    uint32_t hash = 2166136261u;
    probe_t *fv;
    int index;

    start_title();
    fv = the_help();
    for (index = 0; index < 3; index++) {
        draw_layers(ENTITY_MESSAGE_DRAW);
        hash = fnv(hash, screen_hash());
        hash = fnv(hash, (uint32_t)size_of(fv->emitter->particles));
    }
    draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY);
    hash = fnv(hash, screen_hash());
    for (index = 0; index < 3; index++) {
        update_entities();
        hash = fold_fixed_member(hash, fv->header.state_time);
        hash = fold_string_member(hash, entity_state_name(fv->header.state));
        assert(entity_list_listed(live_entities(), fv));
    }
    assert(class_of(fv)->hit_test(&fv->header, vec_ref(64, 10), vec_ref(64, 112)) == 1);
    assert(class_of(fv)->hit_test(&fv->header, vec_ref(64, 112), vec_ref(64, 10)) == 0);
    assert(class_of(fv)->hit_test(&fv->header, vec_ref(24, 106), vec_ref(24, 106)) == 1);
    assert(class_of(fv)->hit_test(&fv->header, vec_ref(23, 106), vec_ref(23, 106)) == 0);
    hash = fnv(hash, (uint32_t)size_of(fv->emitter->particles));
    check("life", hash, LIFE_HASH);
}

static p8_input_frame_t g_mouse;

static void mouse_at(int x, int y) {
    memset(&g_mouse, 0, sizeof g_mouse);
    g_mouse.mouse_x = x;
    g_mouse.mouse_y = y;
    input.frames = &g_mouse;
    input.frame_count = 1;
    input.current_frame = 0;
    dispatch_context_set_input(&input);
    update_entities();
}

/* The mouse over "need help?": main_ui()'s hovered, is_hovered, the help
 * in layer 10 of hovered_by_layer, the cursor sprite; then out, is_hovered
 * false. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    probe_t *fv;
    uint32_t drawn;

    start_title();
    fv = the_help();
    mouse_at(64, 112);
    ui_update(&memory, main_ui());
    assert((void *)main_ui()->hovered == fv);
    assert(is_hovered_state(fv) == 2);
    assert(in_layer(10, fv));
    h = fnv(h, is_hovered_state(fv));

    p8_cls(&memory);
    ui_draw_overlay(&graphics, &memory, main_ui());
    drawn = screen_hash();
    p8_cls(&memory);
    p8_spr_default(&graphics, &memory, 11, 64, 112);
    assert(drawn == screen_hash());
    h = fnv(h, drawn);

    mouse_at(64, 10);
    ui_update(&memory, main_ui());
    assert((void *)main_ui()->hovered != fv);
    assert(is_hovered_state(fv) == 0);
    assert(!in_layer(10, fv));
    h = fnv(h, is_hovered_state(fv));
    check("hover", h, HOVER_HASH);
}

/* Gives back each particle of the list and leaves it empty. */
static void empty_particles(pointer_list_t *qy) {
    while (qy->count > 0) {
        void *particle = qy->items[0];
        pointer_list_del(qy, particle);
        allocator_release(particle_zone(PARTICLE_SLOT_PARTICLE), particle);
    }
}

/* A particle of the help is drawn in the help's own trail colors: at life
 * 6.5, less the frame's 0.14, index 6 is colour 9, where the probe's five
 * have none. */
static void test_particle_takes_the_help_palette(void) {
    particle_t *particle = particle_take();
    pointer_list_t *qy;
    vec_t ra;
    probe_t *fv;
    int at;

    start_title();
    fv = the_help();
    assert(fv->pos != NULL);
    ra = *fv->pos;
    particle->pos = vec_make(0, 0);
    particle->velocity = vec_make(0, 0);
    particle->has_acceleration = 0;
    particle->life = P8_FIXED_LIT(0x00068000, 6.5);
    qy = fv->emitter->particles;
    empty_particles(qy);
    pointer_list_add(qy, particle);
    p8_camera_reset(&graphics);
    at = P8_ADDR_SCREEN + p8_fixed_flr_int(ra.y) * P8_SCREEN_WIDTH + p8_fixed_flr_int(ra.x);
    p8_poke(&memory, at, 3);
    emitter_update(&graphics, &memory, &fv->header);
    assert(p8_peek(&memory, at) == help_screen_trail_colors_seq.items[5] && help_screen_trail_colors_seq.items[5] == 9);
}

/* cart_start_game(), as a difficulty button starts it, takes the help out of
 * the live entities, the probe tag and the draw and hit_test message lists. */
static void test_start_game_retires_the_help(void) {
    void *fv;

    start_title();
    fv = the_help();
    cart_start_game(NULL);
    assert(!entity_list_listed(live_entities(), fv));
    assert(!entity_list_listed(entities_by_tag(TAG_PROBE), fv) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), fv) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_HIT_TEST), fv));
}

/* The title's help is a struct whose descriptor shares the probe's 4
 * inherited entries and holds its own draw and hit_test. Ready from its
 * init to its shutdown. */
static void test_help_screen_is_a_struct(void) {
    const entity_class_t *cls = help_screen_class();
    const entity_class_t *g_ = probe_class();
    void *fv;
    int j;

    start_title();
    fv = the_help();
    assert(fv != NULL && ((const entity_t *)fv)->class == cls);
    assert(cls->drag != NULL && cls->drag == g_->drag && cls->drop != NULL && cls->drop == g_->drop && cls->spawn_particle != NULL &&
           cls->spawn_particle == g_->spawn_particle);
    assert(cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL);
    assert(cls->draw_overlay != NULL && cls->draw_overlay == g_->draw_overlay);
    assert(cls->draw == help_screen_draw_method && cls->hit_test != NULL);
    assert(cls->new_year == NULL && cls->refresh == NULL && cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL &&
           cls->accept_drop == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(help_screen_ready());

    help_shutdown();
    assert(!help_screen_ready());
    help_init();
    assert(help_screen_ready());
}

static void call_emitter_init(void *e) {
    emitter_init(e);
}

static void call_emitter_spawn(void *e) {
    emitter_spawn(e, p8_fixed_from_int(1));
}

static void call_emitter_update(void *e) {
    emitter_update(&graphics, &memory, e);
}

static void call_probe_spawn_particle(void *e) {
    (void)probe_spawn_particle(e);
}

static void call_targeting_track(void *e) {
    targeting_track(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_targeting_tooltip(void *e) {
    targeting_tooltip(&graphics, &memory, e);
}

static void call_targeting_confirm(void *e) {
    (void)targeting_confirm(e);
}

static void call_contains_cursor(void *e) {
    (void)entity_geom_contains_cursor(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_bob(void *e) {
    planet_bob(e, p8_fixed_from_int(1));
}

/* On a NULL instance: targeting_confirm() and entity_geom_contains_cursor()
 * return 0; probe_spawn_particle() draws three random numbers, divides by
 * zero angle slices and returns a new particle; the six others do nothing. */
static void test_probe_closed(void) {
    static void (*const fns[])(void *) = {call_emitter_init, call_emitter_spawn, call_emitter_update, call_probe_spawn_particle, call_targeting_track,
                                          call_targeting_tooltip, call_targeting_confirm, call_contains_cursor, call_bob};
    size_t i;

    start_title();
    assert(probe_ready());
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        fns[i](NULL);
    }
}

/* Each scenario runs in a child forked right after cart_load(), as a fresh
 * process would start it: the random state is the same for all. */
static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/* What an emitter holds in the particle zone, by sort. */
typedef struct {
    int particle;
    int particles;
    int particles_help_screen;
    int rows;
} held_t;

static held_t held_now(void) {
    held_t h;
    h.particle = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    h.particles = particle_zone_live(PARTICLE_SLOT_LIST);
    h.particles_help_screen = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    h.rows = ui_zone_live(SLOT_KIND_ROW);
    return h;
}

/* The help that cart_start_game() takes out gives back, at the exit queue's
 * flush, its particles and its particle list, the one slot of its sort. */
static void test_start_game_gives_back_the_help_particles(void) {
    probe_t *fv;
    held_t title;
    held_t after;

    start_title();
    fv = the_help();
    emitter_spawn(&fv->header, p8_fixed_from_int(5));
    title = held_now();
    assert(title.particles_help_screen == 1 && fv->emitter->particles->count >= 5);
    assert(title.particle == fv->emitter->particles->count);
    cart_start_game(NULL);
    cart_flush_exit_queue();
    after = held_now();
    assert(after.particles_help_screen == 0 && after.particle == 0);
    assert(fv->emitter->particles == NULL);
}

int main(void) {
    p8_mem_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_hover);
    in_fresh_child(test_particle_takes_the_help_palette);
    in_fresh_child(test_start_game_retires_the_help);
    in_fresh_child(test_help_screen_is_a_struct);
    in_fresh_child(test_start_game_gives_back_the_help_particles);
    in_fresh_child(test_probe_closed);

    printf("test_help_screen: all tests passed\n");
    return 0;
}
