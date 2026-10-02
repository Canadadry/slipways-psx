/* The help screen `fv`, built by its only road (`pico8__init()` on the
 * title screen, `pico8_fv_new()`), read only through the dual-form field
 * API, its defaults read on its archetype, and driven through the
 * real `mv`/`dc`/`dg`/`lj`. Its
 * methods are compared to the public functions. No events file. */

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


#define BUILT_HASH 0xFAA9865Eu
#define LIFE_HASH 0x64B6480Bu
#define HOVER_HASH 0xEE114A89u

static p8_memory_t mem;
static p8_graphics_t gfx;
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

static int ents_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const entity_list_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

/* 0 absent or false, 2 true. */
static uint32_t m__state(const probe_t *fv) {
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

/* A generated row: items (a nested row folded the same way), then `c`,
 * then `fn`, then the entry count. */
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
        h = fold_fixed(fnv_str(h, "c"), p8_fixed_from_int(row->color));
    }
    if (row->draw_function != NULL) {
        h = fold_string(fnv_str(h, "fn"), row->draw_function);
    }
    return fnv(h, (uint32_t)(row->count + (row->has_color != 0) + (row->draw_function != NULL)));
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(live_entities(), ref));
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

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static vec_t *vec_ref(int x, int y) {
    vec_t *v = malloc(sizeof *v);
    *v = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return v;
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

static void start_title(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    pico8__init();
}

/* The help, whatever its form: `g_`'s tags and `fa`=10 (a probe has 2). */
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
    assert(found != NULL && found->class == pico8_fv_struct_class());
    return (probe_t *)found;
}

static int in_layer(int layer, const void *e) {
    const pico8_ui_ic_t *ic = pico8_ok()->ic;

    if (ic == NULL) {
        return 0;
    }
    return ents_listed(&ic->layers[layer], e);
}

static const pico8_js_t *as_js(const void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == pico8_js_struct_class());
    return e;
}

/* The title's help as `fv:pq`, `f_`, `lt` left it: its 22 defaults and the
 * 6 of g_, their tables' contents and identity, `me`, `t`, its empty `qy`;
 * no name only read or written elsewhere; its methods; the 4 buttons `pz`
 * built before it, each carrying the `my` stamped on its `bo` row; where
 * `lt` registered it; `ok.le` emptied. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *hv;
    probe_t *fv;
    const probe_t *g_;
    const emitter_t *em;
    const targeting_t *pr;
    int rank;
    size_t i;
    int k;

    start_title();
    fv = the_help();
    g_ = fv;
    em = g_->emitter;
    pr = g_->targeting;
    hv = live_entities();

    assert(g_->archetype->layer == p8_fixed_from_int(10));
    assert(em->archetype->lifetime == p8_fixed_from_int(17));
    assert(g_->archetype->back_color == p8_fixed_from_int(11));
    assert(g_->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)));
    assert(g_->pos != NULL && em->trail_colors != NULL);
    assert(g_->archetype->rows != NULL && pr->archetype->confirmed_state != NULL);
    h = fold_fixed_member(h, g_->archetype->layer);
    h = fold_ref_member(h, g_->pos);
    h = fold_ref_member(h, &g_->archetype->fixed_pos);
    h = fold_fixed_member(h, g_->archetype->back_color);
    h = fold_ref_member(h, &g_->archetype->row_step);
    h = fold_fixed_member(h, g_->archetype->open_sfx);
    h = fold_fixed_member(h, g_->archetype->clear_ephemerals_after);
    h = fold_fixed_member(h, em->archetype->spawn_rate);
    h = fold_ref_member(h, em->trail_colors);
    h = fold_fixed_member(h, em->matrix_xx);
    h = fold_fixed_member(h, em->matrix_xy);
    h = fold_fixed_member(h, em->matrix_yx);
    h = fold_fixed_member(h, em->matrix_yy);
    h = fold_fixed_member(h, em->angle_slices);
    h = fold_fixed_member(h, em->base_radius);
    h = fold_fixed_member(h, em->speed_factor);
    h = fold_fixed_member(h, em->archetype->quarter_turn);
    h = fold_fixed_member(h, em->archetype->lifetime);
    h = fold_ref_member(h, &g_->archetype->hit_box);
    h = fold_fixed_member(h, g_->archetype->cursor_sprite);
    h = fold_fixed_member(h, g_->archetype->gpio_signal);
    h = fold_ref_member(h, g_->archetype->rows);
    h = fold_tags(fnv(fnv(h, 1u), 0x7Au), g_->header.class->tags);
    h = fold_fixed_member(h, g_->archetype->fold_anchor);
    h = fold_string_member(h, pr->archetype->confirmed_state);
    h = fold_fixed_member(h, pr->cost);
    h = fold_bool_member(h, pr->archetype->clock_ticks != 0);
    h = fold_fixed_member(h, pr->archetype->attach_distance);
    h = fold_vec(h, g_->pos);
    h = fold_vec(h, &g_->archetype->fixed_pos);
    h = fold_vec(h, &g_->archetype->row_step);
    h = fold_box(h, &g_->archetype->hit_box);
    assert(g_->archetype == &help_screen_archetype && g_->pos == &help_screen_archetype.pos);
    assert(em->trail_colors == &help_screen_trail_colors_seq);
    assert(em->trail_colors != &probe_trail_colors_seq);
    assert(g_->archetype->rows == &help_screen_rows_row);
    h = fold_string_member(h, g_->header.state);
    h = fold_fixed_member(h, g_->header.state_time);
    h = fnv(h, (uint32_t)size_of(em->particles));
    assert(size_of(em->particles) == 0);
    assert(!fv->is_hovered && g_->map_pos == NULL && pr->target == NULL && !g_->is_dead);
    assert(cls_of(fv)->new_year == NULL && cls_of(fv)->refresh == NULL && cls_of(fv)->states[ENTITY_STATE_NORMAL] == NULL &&
           cls_of(fv)->left_press == NULL);

    assert(cls_of(fv)->draw == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->draw_overlay == targeting_tooltip_method);
    assert(cls_of(fv)->spawn_particle == probe_spawn_particle);
    assert(cls_of(fv)->drag == targeting_track_method);
    assert(cls_of(fv)->drop == targeting_confirm_method);
    assert(cls_of(fv)->hit_test != NULL);

    rank = ents_rank_in(hv, fv);
    h = fnv(h, (uint32_t)rank);
    h = fnv(h, (uint32_t)hv->count);
    assert(rank >= 4);
    for (k = 0; k < 4; k++) {
        const pico8_js_t *b = as_js(hv->items[rank - 4 + k]);
        vec_t qf;
        assert(b->fa == p8_fixed_from_int(11));
        assert(b->qf != NULL);
        qf = *b->qf;
        assert(qf.x == p8_fixed_from_int(34) && qf.y == p8_fixed_from_int(56 + 11 * k));
        h = fold_vec(h, b->qf);
        h = fold_vec(h, b->ra);
        h = fold_fixed_member(h, b->fa);
    }
    for (i = 0; i < 5; i++) {
        static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), fv));
    }
    assert(ents_listed(entities_by_message("draw"), fv) && ents_listed(entities_by_message("draw_overlay"), fv) && ents_listed(entities_by_message("hit_test"), fv));
    assert(!ents_listed(entities_by_message("new_year"), fv) && !ents_listed(entities_by_message("refresh"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag("probe"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag("targeting"), fv));
    assert(ents_listed(entities_by_tag("probe"), fv) && ents_listed(entities_by_tag("targeting"), fv));

    for (k = 0; k < 4; k++) {
        const pico8_js_t *row = (const pico8_js_t *)hv->items[rank - 4 + k];
        assert(row->has_my);
        assert(row->my == pico8_eu_field_value());
    }
    assert(pico8_ok()->le != NULL && pico8_ok()->le->count == 0);
    check("built", h, BUILT_HASH);
}

/* `dc("kj")` x3 with the screen and `qy`; `dc("ev")`; `dg` x3; `hd` in and
 * out of `kq`, `rp` retained since `fa` >= 10. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    probe_t *fv;
    int i;

    start_title();
    fv = the_help();
    for (i = 0; i < 3; i++) {
        draw_layers("draw");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv->emitter->particles));
    }
    draw_layers("draw_overlay");
    h = fnv(h, screen_hash());
    for (i = 0; i < 3; i++) {
        update_entities();
        h = fold_fixed_member(h, fv->header.state_time);
        h = fold_string_member(h, fv->header.state);
        assert(ents_listed(live_entities(), fv));
    }
    assert(cls_of(fv)->hit_test(&fv->header, vec_ref(64, 10), vec_ref(64, 112)) == 1);
    assert(cls_of(fv)->hit_test(&fv->header, vec_ref(64, 112), vec_ref(64, 10)) == 0);
    assert(cls_of(fv)->hit_test(&fv->header, vec_ref(24, 106), vec_ref(24, 106)) == 1);
    assert(cls_of(fv)->hit_test(&fv->header, vec_ref(23, 106), vec_ref(23, 106)) == 0);
    h = fnv(h, (uint32_t)size_of(fv->emitter->particles));
    check("life", h, LIFE_HASH);
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

/* The mouse over "need help?": `ok.o_`, `m_`, `fg` poked at 0x5f80, the
 * help in layer `fa` of `ic`, the cursor sprite `ql`; then out, `m_` false. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    probe_t *fv;
    uint32_t drawn;

    start_title();
    fv = the_help();
    mouse_at(64, 112);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ == fv);
    assert(m__state(fv) == 2);
    check_int("hover 0x5f80", p8_peek(&mem, 0x5f80), 5);
    assert(in_layer(10, fv));
    h = fnv(h, m__state(fv));
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));

    p8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    drawn = screen_hash();
    p8_cls(&mem);
    p8_spr_default(&gfx, &mem, 11, 64, 112);
    assert(drawn == screen_hash());
    h = fnv(h, drawn);

    mouse_at(64, 10);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ != fv);
    assert(m__state(fv) == 0);
    assert(!in_layer(10, fv));
    h = fnv(h, m__state(fv));
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* Gives back each particle of `qy` and leaves it empty. */
static void empty_qy(pointer_list_t *qy) {
    while (qy->count > 0) {
        void *particle = qy->items[0];
        pointer_list_del(qy, particle);
        allocator_release(particle_zone(PARTICLE_SLOT_PARTICLE), particle);
    }
}

/* `eu`, as a difficulty button starts it, takes the help out of `hv`,
 * `cg.g_` and `cv.kj`/`cv.hd`. */
/* A particle of the help is drawn in the help's own `k_`: at `l`=6.5, less
 * the frame's 0.14, index 6 is colour 9, where `g_`'s five have none. */
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
    empty_qy(qy);
    pointer_list_add(qy, particle);
    p8_camera_reset(&gfx);
    at = P8_ADDR_SCREEN + p8_fixed_flr_int(ra.y) * P8_SCREEN_WIDTH + p8_fixed_flr_int(ra.x);
    p8_poke(&mem, at, 3);
    emitter_update(&gfx, &mem, &fv->header);
    assert(p8_peek(&mem, at) == help_screen_trail_colors_seq.items[5] && help_screen_trail_colors_seq.items[5] == 9);
}

static void test_eu_retires_the_help(void) {
    void *fv;

    start_title();
    fv = the_help();
    pico8_eu(NULL);
    assert(!ents_listed(live_entities(), fv));
    assert(!ents_listed(entities_by_tag("probe"), fv) && !ents_listed(entities_by_message("draw"), fv) && !ents_listed(entities_by_message("hit_test"), fv));
}

/* The title's help is a struct whose descriptor shares `g_`'s 4 inherited
 * entries and holds its own `kj`, `hd`. Ready from its init to its
 * shutdown. */
static void test_fv_is_a_struct(void) {
    const entity_class_t *cls = pico8_fv_struct_class();
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
    assert(cls->draw == pico8_fv_kj_dispatch && cls->hit_test != NULL);
    assert(cls->new_year == NULL && cls->refresh == NULL && cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL &&
           cls->accept_drop == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_fv_ready());

    pico8_help_shutdown();
    assert(!pico8_fv_ready());
    pico8_help_init();
    assert(pico8_fv_ready());
}

static void call_f_(void *e) {
    emitter_init(e);
}

static void call_ez(void *e) {
    emitter_spawn(e, p8_fixed_from_int(1));
}

static void call_ei(void *e) {
    emitter_update(&gfx, &mem, e);
}

static void call_g__mm(void *e) {
    (void)probe_spawn_particle(e);
}

static void call_bv(void *e) {
    targeting_track(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_dl(void *e) {
    targeting_tooltip(&gfx, &mem, e);
}

static void call_d_(void *e) {
    (void)targeting_confirm(e);
}

static void call_gk(void *e) {
    (void)entity_geom_contains_cursor(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_na(void *e) {
    planet_bob(e, p8_fixed_from_int(1));
}

/* On NULL `self`: `d_`, `gk` return 0; `g__mm` draws three `rnd`, divides by
 * `pn` = 0 and returns a new particle; the six others do nothing. */
static void test_g__closed(void) {
    static void (*const fns[])(void *) = {call_f_, call_ez, call_ei, call_g__mm, call_bv,
                                          call_dl, call_d_, call_gk, call_na};
    size_t i;

    start_title();
    assert(probe_ready());
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        fns[i](NULL);
    }
}

/* Each scenario runs in a child forked right after pico8_cart_load(), as a
 * fresh process would start it: the rnd state is the same for all. */
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
    int particules;
    int qy;
    int qy_fv;
    int rangees;
} held_t;

static held_t held_now(void) {
    held_t h;
    h.particules = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    h.qy = particle_zone_live(PARTICLE_SLOT_LIST);
    h.qy_fv = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    h.rangees = pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
    return h;
}

/* The help that `eu` takes out gives back, at the exit queue's flush, its
 * particles and its `qy_fv`, the one place of its sort. */
static void test_eu_gives_back_the_help_particles(void) {
    probe_t *fv;
    held_t title;
    held_t after;

    start_title();
    fv = the_help();
    emitter_spawn(&fv->header, p8_fixed_from_int(5));
    title = held_now();
    assert(title.qy_fv == 1 && fv->emitter->particles->count >= 5);
    assert(title.particules == fv->emitter->particles->count);
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.qy_fv == 0 && after.particules == 0);
    assert(fv->emitter->particles == NULL);
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_hover);
    in_fresh_child(test_particle_takes_the_help_palette);
    in_fresh_child(test_eu_retires_the_help);
    in_fresh_child(test_fv_is_a_struct);
    in_fresh_child(test_eu_gives_back_the_help_particles);
    in_fresh_child(test_g__closed);

    printf("test_pico8_fv: all tests passed\n");
    return 0;
}
