/* The infraspace gate `fr`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as
 * the shop builds it (`ie:ldown` on `gx[3]`, `id`) and driven through the
 * real `mv`/`dg`/`dc`. Its methods are compared to those of a slipgate built
 * in the same game, started directly: no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
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
#include "game/economy_score.h"
#include "game/economy_tiers.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "heap_allocator.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const vec_t *ra, const vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


#define BUILT_HASH 0x480897DDu
#define LIFE_HASH 0xC3A5DD13u

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
static const char *const tags[] = {"slipgate", "node", "targeting"};

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

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0u);
}

static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t v) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)v);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)(b != 0));
}

/* A NULL string folds as absent. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fold_absent(h) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

static uint32_t fold_hf(uint32_t h, const planet_t *jo) {
    return jo->archetype->is_planet ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(live_entities(), ref));
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);
}

/* The word the frozen hashes fold before a boolean, 0xB0, then the boolean
 * as 0 or 1; any other moves every *_HASH. */
static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

/* `e` as a planet of qh's lineage. */
static planet_t *as_qh(void *e) {
    const entity_t *h = e;

    assert(h != NULL && (h->class->lineage & LINEAGE(LINEAGE_STRUCTURE)));
    return e;
}

static uint32_t fold_qh(uint32_t h, const planet_t *qh) {
    return qh->archetype->is_structure ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
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
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8__init();
    pico8_eu(NULL);
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static vec_t vec_xy(int x, int y) {
    return vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message("refresh");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == kind) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the icon of `kind`, as the shop does. */
static void *build(const entity_kind_t *kind) {
    void *icon = icon_of(kind);
    down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->left_press(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

/* `hz` of a slipgate or a gate; NULL on any other struct, which has none. */
static const int_seq_t *hz_of(const void *e) {
    if (cls_of(e) == slipgate_class() || cls_of(e) == infraspace_gate_class()) {
        return ((const planet_t *)e)->archetype->link_sprites;
    }
    return NULL;
}

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * fr's class is an infraspace gate. */
static int is_a_gate(const void *e) {
    return hz_of(e) == &infraspace_gate_link_sprites_seq;
}

static void *build_gate(void) {
    void *e = build(infraspace_gate_kind());
    assert(is_a_gate(e));
    return e;
}

/* A gate as `ie:ldown` and `jo:pq` left it: its scalars, which of its own,
 * nr's, qh's and jo's members are set, its methods (a slipgate's very
 * ones), which default tables it shares with `fr`, `nr`, `qh` and `jo`, and
 * where `lt` registered it -- in `cg.nr`/`np`/`prober`, never in `cg.jo` or
 * `cg.qh`. */
static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
    const planet_t *fr;
    const planet_t *nr;
    const planet_t *qh;
    const planet_t *jo;
    size_t i;

    assert(is_a_gate(e) && !is_a_gate(slipgate) && cls_of(e) == infraspace_gate_class());
    fr = e;
    nr = fr;
    qh = nr;
    jo = qh;
    h = fold_string_member(h, jo->planet_id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, nr->archetype->required_tech);
    h = fold_fixed_member(h, fr->archetype->infraspace);
    h = fold_fixed_member(h, qh->archetype->project_index);
    h = fold_fixed_member(h, qh->sprite);
    h = fold_fixed_member(h, qh->targeting->cost);
    h = fold_fixed_member(h, qh->targeting->archetype->clock_ticks);
    h = fold_fixed_member(h, qh->targeting->archetype->attach_distance);
    h = fold_string_member(h, qh->targeting->archetype->confirmed_state);
    h = fold_qh(h, qh);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->archetype->trade_multiplier);
    h = fold_fixed_member(h, jo->archetype->layer);
    h = fold_fixed_member(h, jo->shake);
    h = fold_fixed_member(h, jo->archetype->back_color);
    h = fold_vec(h, &jo->archetype->row_step);
    h = fnv(h, (uint32_t)nr->archetype->link_sprites->count);
    /* The 1s of `mt`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`,
     * `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept so that
     * BUILT_HASH still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(nr->archetype->link_sprites != NULL));
    h = fnv(h, (uint32_t)(nr->archetype->required_tech != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->name != NULL));
    h = fnv(h, (uint32_t)(qh->targeting->archetype->confirmed_state != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->accepted_resources != NULL));
    h = fnv(h, qh->targeting->target != NULL ? 1u : 0u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->archetype->ring_colors != NULL));
    h = fnv(h, (uint32_t)(jo->archetype->mood_rows != NULL));
    h = fnv(h, (uint32_t)(jo->archetype->made_resources != NULL));
    h = fnv(h, (uint32_t)(jo->production != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->archetype->spacer != NULL));
    h = fnv(h, (uint32_t)(jo->planet_id != NULL));
    h = fnv(h, (uint32_t)(jo->map_pos != NULL));
    h = fnv(h, (uint32_t)(jo->pos != NULL));
    h = fnv(h, (uint32_t)(jo->slipways != NULL));
    h = fnv(h, (uint32_t)(jo->project != NULL));
    h = fnv(h, (uint32_t)(jo->description != NULL));
    h = fnv(h, (uint32_t)(jo->needs != NULL));
    h = fnv(h, (uint32_t)(jo->planet_type != NULL));
    h = fnv(h, (uint32_t)(jo->fold_anchor != 0));
    h = fnv(h, (uint32_t)(jo->layout_row != NULL));
    h = fnv(h, (uint32_t)(jo->targeted_by != NULL));
    h = fnv(h, (uint32_t)jo->has_production_bonus);
    h = fnv(h, (uint32_t)(jo->is_dead != 0));
    h = fnv(h, (uint32_t)jo->has_net_income);
    h = fnv(h, (uint32_t)(jo->incoming_routes != NULL));
    h = fnv(h, (uint32_t)(jo->outgoing_routes != NULL));
    h = fnv(h, (uint32_t)jo->has_development_tier);
    h = fnv(h, (uint32_t)(jo->is_hovered != 0));
    assert(cls_of(e)->attach_slipway == cls_of(slipgate)->attach_slipway && cls_of(e)->offer_trade == cls_of(slipgate)->offer_trade &&
           cls_of(e)->drag == cls_of(slipgate)->drag && cls_of(e)->drop == cls_of(slipgate)->drop);
    assert(cls_of(e)->attach_slipway != NULL && cls_of(e)->offer_trade != NULL && cls_of(e)->drag != NULL && cls_of(e)->drop != NULL &&
           planet_is_family(e));
    assert(cls_of(e)->hit_test == cls_of(slipgate)->hit_test && cls_of(e)->draw == cls_of(slipgate)->draw &&
           cls_of(e)->left_press == cls_of(slipgate)->left_press && cls_of(e)->refresh == cls_of(slipgate)->refresh &&
           cls_of(e)->accept_drop == cls_of(slipgate)->accept_drop && cls_of(e)->new_year == cls_of(slipgate)->new_year &&
           cls_of(e)->draw_overlay == cls_of(slipgate)->draw_overlay);
    assert(cls_of(e)->states[ENTITY_STATE_VANISHING] == cls_of(slipgate)->states[ENTITY_STATE_VANISHING] &&
           cls_of(e)->states[ENTITY_STATE_SETTLED] == cls_of(slipgate)->states[ENTITY_STATE_SETTLED]);
    assert(cls_of(e)->hit_test != NULL && cls_of(e)->draw != NULL && cls_of(e)->left_press != NULL && cls_of(e)->refresh != NULL &&
           cls_of(e)->accept_drop != NULL && cls_of(e)->new_year != NULL && cls_of(e)->draw_overlay != NULL &&
           cls_of(e)->states[ENTITY_STATE_VANISHING] != NULL && cls_of(e)->states[ENTITY_STATE_SETTLED] != NULL);
    assert(cls_of(e)->draw_at == NULL && cls_of(e)->spawn_particle == NULL);
    assert(cls_of(e)->states[ENTITY_STATE_NORMAL] == NULL && cls_of(e)->states[ENTITY_STATE_PLACING] == NULL &&
           cls_of(e)->states[ENTITY_STATE_UNFOLDING] == NULL && cls_of(e)->right_press == NULL);
    assert(nr->archetype->link_sprites != &slipgate_link_sprites_seq);
    assert(jo->archetype == &infraspace_gate_archetype);
    assert(jo->header.class->tags == (TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(qh->archetype->name == &structure_name_row && qh->archetype->accepted_resources == &structure_accepted_resources_seq);
    assert(jo->archetype == &infraspace_gate_archetype);
    assert(jo->archetype->spacer == &planet_spacer_row && jo->archetype->ring_colors == &planet_ring_colors_seq && jo->archetype->mood_rows == planet_mood_rows_rows &&
           jo->archetype->made_resources == &planet_made_resources_seq);
    assert(jo->production != NULL && jo->production != planet_shared_production());
    assert(pico8_gb_at(1) != NULL);
    assert(jo->project == pico8_gb_at(1));
    h = fnv(h, (uint32_t)ents_list_size(jo->slipways));
    h = fnv(h, (uint32_t)list_size(jo->needs));
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag(tags[i]), e));
    }
    assert(ents_listed(entities_by_tag("slipgate"), e) && !ents_listed(entities_by_tag("planet"), e) && !ents_listed(entities_by_tag("structure"), e));
    return h;
}

/* Two infraspace gates built by the shop's `ie:ldown` on `gx[3]`, around a
 * slipgate. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *slipgate;
    void *b;

    start_game();
    a = build_gate();
    slipgate = build(slipgate_kind());
    b = build_gate();
    assert(a != b && as_qh(a)->slipways != as_qh(b)->slipways && as_qh(a)->production != as_qh(b)->production);
    h = fold_gate(h, a, slipgate);
    h = fold_gate(h, b, slipgate);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    main_economy_typed()->nz = p8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const planet_t *qh) {
    const vec_t *ra = qh->pos;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the gate. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const planet_t *qh = as_qh(e);

    h = fold_ra_y(h, qh);
    update_entities();
    h = fold_fixed_member(h, qh->header.state_time);
    return fold_ra_y(h, qh);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const entity_list_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a gate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    entity_list_t *ib = planet_rebuild_trades(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), planet_is_colonized(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->has_development_tier ? fold_fixed_member(h, as_qh(e)->development_tier) : fold_absent(h);
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static entity_list_t *list_of(int n, entity_t *item) {
    entity_list_t *l = malloc(sizeof *l);
    int i;
    entity_list_init(l, &heap_allocator);
    for (i = 0; i < n; i++) {
        entity_list_add(l, item);
    }
    return l;
}

/* `hq` then `ey`, as the player drops a planet at `offset` from `mo`. */
static void place(void *e, int x, int y) {
    const vec_t *mo_ra;
    vec_t *mp;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(vec_add(*mo_ra, vec_xy(x, y)));
    cls_of(e)->drag(e, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_qh(e)->pos, mp));
}

/* A slipway drawn from `from` by its `ldown`, aimed by `jc:hq` at a point
 * with no planet: its `mt` (then `fa`) comes from `from` alone. */
static uint32_t fold_route(uint32_t h, void *from) {
    down_t jc = cls_of(from)->left_press(from, NULL, NULL);
    const vec_t *ra = as_qh(from)->pos;
    int from_has_mt = cls_of(from) == infraspace_gate_class();
    const slipway_t *made;

    assert(jc.stop && jc.held != NULL && cls_of(jc.held) == slipway_class());
    made = (const slipway_t *)jc.held;
    assert(made->start == from);
    assert(ra != NULL);
    cls_of(jc.held)->drag(jc.held, vec_ref(vec_add(*ra, vec_xy(20, 0))), NULL);
    assert(made->has_infraspace == from_has_mt);
    assert(made->layer == p8_fixed_from_int(from_has_mt ? 2 : 4));
    h = made->has_infraspace ? fold_fixed_member(h, made->infraspace) : fold_absent(h);
    return fold_fixed_member(h, made->layer);
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * gate and a slipway drawn from itself; `ldown` with no link (a `jc` whose
 * `qu` is the gate) and with 3 (refused); `hb` with 0 to 3 links; the `mt`
 * of a slipway drawn from a gate and from a slipgate. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *slipgate;
    down_t jc;
    entity_list_t *oe;
    int i;

    start_game();
    idle = build_gate();
    h = fold_string_member(h, as_qh(idle)->header.state);
    h = fold_dg(h, idle);

    placed = build_gate();
    place(placed, 12, 108);
    set_credits(999);
    assert(cls_of(placed)->drop == targeting_confirm_method);
    h = targeting_confirm(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(placed)->header.state);
    assert(strcmp(as_qh(placed)->header.state, "settled") == 0);
    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    for (i = 0; i < 2; i++) {
        draw_layers("draw");
        h = fnv(h, screen_hash());
    }

    h = fold_bool(h, cls_of(placed)->accept_drop(placed, NULL).stop);
    h = fold_bool(h, cls_of(placed)->accept_drop(placed, idle).stop);
    jc = cls_of(placed)->left_press(placed, NULL, NULL);
    assert(jc.stop && jc.held != NULL);
    assert(cls_of(jc.held) == slipway_class() && ((const slipway_t *)jc.held)->start == placed);
    h = fnv(h, FROZEN_STRUCT_TAG);
    h = fold_bool(h, cls_of(placed)->accept_drop(placed, jc.held).stop);
    h = fold_bool(h, cls_of(idle)->accept_drop(idle, jc.held).stop);

    oe = as_qh(placed)->slipways;
    for (i = 0; i <= 3; i++) {
        as_qh(placed)->slipways = list_of(i, idle);
        cls_of(placed)->refresh(placed);
        h = fold_fixed_member(h, as_qh(placed)->sprite);
        h = fold_bool(h, cls_of(placed)->accept_drop(placed, NULL).stop);
    }
    assert(as_qh(placed)->sprite == p8_fixed_from_int(76));
    jc = cls_of(placed)->left_press(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_bool(h, jc.stop);
    as_qh(placed)->slipways = oe;
    cls_of(placed)->refresh(placed);
    h = fold_fixed_member(h, as_qh(placed)->sprite);
    assert(as_qh(placed)->sprite == p8_fixed_from_int(78));
    h = fold_economy(h, placed);

    h = fold_route(h, placed);
    assert(is_a_gate(placed) && ((const planet_t *)placed)->archetype->infraspace == p8_fixed_from_int(1));
    slipgate = build(slipgate_kind());
    place(slipgate, 40, 108);
    h = fold_route(h, slipgate);
    assert(cls_of(slipgate) != infraspace_gate_class());
    check("life", h, LIFE_HASH);
}

static void must_abort(void (*write)(void *), void *e) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(e);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void build_with_unknown_key(void *e) {
    planet_keys_t keys;
    (void)e;
    keys.planet_id = NULL;
    keys.map_pos = vec_ref(vec_xy(1, 1));
    keys.pos = NULL;
    (void)infraspace_gate_new(&keys);
}

static int is_an_fr_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == infraspace_gate_class();
}

/* Built by `ie:ldown` or pico8_fr_new, a gate is a struct whose descriptor
 * holds nr's very entries; a construction key other than `id` stops the
 * run. Neither nr's nor qh's class installs a function any more. */
static void test_fr_is_a_struct(void) {
    const entity_class_t *cls = infraspace_gate_class();
    const entity_class_t *nr = slipgate_class();
    void *a;
    int j;

    start_game();
    a = build_gate();
    assert(is_an_fr_struct(a) && cls != nr && ents_listed(entities_by_tag("slipgate"), a));
    assert(cls->draw == nr->draw && cls->draw_overlay == nr->draw_overlay && cls->hit_test == nr->hit_test && cls->new_year == nr->new_year && cls->refresh == nr->refresh &&
           cls->has_refresh == nr->has_refresh && cls->left_press == nr->left_press && cls->right_press == nr->right_press && cls->accept_drop == nr->accept_drop &&
           cls->drag == nr->drag && cls->drop == nr->drop && cls->attach_slipway == nr->attach_slipway && cls->offer_trade == nr->offer_trade && cls->draw_at == nr->draw_at &&
           cls->spawn_particle == nr->spawn_particle);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == nr->states[j]);
    }
    assert(infraspace_gate_ready());

    must_abort(build_with_unknown_key, NULL);
    {
        planet_t *bare = infraspace_gate_new(NULL);
        assert(is_an_fr_struct(bare) && is_a_gate(bare) && bare->planet_id == NULL && bare->slipways != NULL &&
               bare->production != NULL);
        assert(ents_listed(entities_by_tag("slipgate"), bare));
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
    in_fresh_child(test_fr_is_a_struct);

    printf("test_pico8_fr: all tests passed\n");
    return 0;
}
