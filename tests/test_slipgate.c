/* The slipgate `nr`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as the shop builds it (`ie:ldown` on `gx[2]`, `id`) and driven
 * through the real `mv`/`dg`/`dc`, in games started directly: no events file. */

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
#include "game/economy_score.h"
#include "game/economy_tiers.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "engine/vec.h"
#include "heap_allocator.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const vec_t *ra, const vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


#define BUILT_HASH 0xBAF7572Fu
#define LIFE_HASH 0xC22C89F7u

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
    p8_input_init(&input);
    dispatch_context_set_input(&input);
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

/* The members a planet of qh's tree holds: `kj`, `ev`, `hd`, `hk`, `hb`,
 * `ldown`, `gv`, `hq`, `ey`, `kf`, `cn` and the states `ll`, `ly`; `mm`
 * aside, no other. */
static void assert_qh_shape(const entity_class_t *c) {
    assert(c->draw != NULL && c->draw_overlay != NULL && c->hit_test != NULL && c->new_year != NULL && c->refresh != NULL && c->left_press != NULL &&
           c->accept_drop != NULL && c->states[ENTITY_STATE_VANISHING] != NULL && c->states[ENTITY_STATE_SETTLED] != NULL);
    assert(c->has_refresh == NULL && c->right_press == NULL && c->states[ENTITY_STATE_NORMAL] == NULL &&
           c->states[ENTITY_STATE_PLACING] == NULL && c->states[ENTITY_STATE_UNFOLDING] == NULL);
    assert(c->drag != NULL && c->drop != NULL && c->attach_slipway != NULL && c->offer_trade != NULL && c->draw_at == NULL);
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

static void *build_slipgate(void) {
    return build(slipgate_kind());
}

enum { NR, FR };

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * nr's class is a slipgate, of fr's class an infraspace gate. -1 for
 * anything else. */
static int gate_kind(const void *e) {
    const void *hz = NULL;

    if (e != NULL && (cls_of(e) == slipgate_class() || cls_of(e) == infraspace_gate_class())) {
        hz = ((const planet_t *)e)->archetype->link_sprites;
    }
    if (hz == NULL) {
        return -1;
    }
    if (hz == &slipgate_link_sprites_seq) {
        return NR;
    }
    if (hz == &infraspace_gate_link_sprites_seq) {
        return FR;
    }
    return -1;
}

/* A slipgate as `ie:ldown` and `jo:pq` left it: its scalars, which of its
 * own, qh's and jo's members are set, its methods (qh's very functions but
 * its own `gv`, `ldown`, `hb`, `cn`), which default tables it shares with
 * `nr`, `qh` and `jo`, and where `lt` registered it -- in
 * `cg.nr`/`np`/`prober`, never in `cg.jo` or `cg.qh`. */
static uint32_t fold_slipgate(uint32_t h, const void *e) {
    const entity_class_t *qh_cls = pico8_qh_struct_class();
    const planet_t *nr;
    const planet_t *qh;
    const planet_t *jo;
    size_t i;

    assert(gate_kind(e) == NR && cls_of(e) == slipgate_class());
    nr = e;
    qh = nr;
    jo = qh;
    h = fold_string_member(h, jo->planet_id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, nr->archetype->required_tech);
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
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
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
    assert(cls_of(e)->offer_trade == slipgate_class()->offer_trade && cls_of(e)->offer_trade != qh_cls->offer_trade &&
           cls_of(e)->offer_trade != planet_class()->offer_trade);
    assert(cls_of(e)->attach_slipway == qh_cls->attach_slipway && cls_of(e)->drag == qh_cls->drag && cls_of(e)->drop == qh_cls->drop &&
           cls_of(e)->spawn_particle == NULL && planet_is_family(e));
    assert_qh_shape(cls_of(e));
    assert(cls_of(e)->draw == qh_cls->draw && cls_of(e)->draw_overlay == qh_cls->draw_overlay && cls_of(e)->hit_test == qh_cls->hit_test &&
           cls_of(e)->new_year == qh_cls->new_year);
    assert(cls_of(e)->states[ENTITY_STATE_VANISHING] == qh_cls->states[ENTITY_STATE_VANISHING] &&
           cls_of(e)->states[ENTITY_STATE_SETTLED] == qh_cls->states[ENTITY_STATE_SETTLED]);
    assert(cls_of(e)->accept_drop != qh_cls->accept_drop && cls_of(e)->left_press != qh_cls->left_press && cls_of(e)->refresh != qh_cls->refresh);
    assert(cls_of(e)->accept_drop != planet_class()->accept_drop && cls_of(e)->left_press != planet_class()->left_press &&
           cls_of(e)->refresh != planet_class()->refresh);
    assert(jo->archetype == &slipgate_archetype);
    assert(jo->header.class->tags == (TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && nr->archetype->link_sprites == &slipgate_link_sprites_seq);
    assert(jo->header.class->tags != (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(jo->archetype->hit_box.left != planet_archetype.hit_box.left);
    assert(qh->archetype->name == &structure_name_row && qh->archetype->accepted_resources == &structure_accepted_resources_seq);
    assert(jo->archetype == &slipgate_archetype);
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

/* Two slipgates built by the shop's `ie:ldown` on `gx[2]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_slipgate();
    b = build_slipgate();
    assert(a != b && as_qh(a)->slipways != as_qh(b)->slipways && as_qh(a)->production != as_qh(b)->production);
    h = fold_slipgate(h, a);
    h = fold_slipgate(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    main_economy_typed()->nz = p8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const planet_t *qh) {
    const vec_t *ra = qh->pos;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the slipgate. */
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

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a slipgate. */
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

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * slipgate and a slipway drawn from itself; `ldown` with no link (a `jc`
 * whose `qu` is the slipgate) and with 3 (refused); `hb` with 0 to 3
 * links; an `fr`'s `hb`; `pico8_nr_new` with no key, then `p` and `ra`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *fr;
    void *bare;
    vec_t *mp;
    down_t jc;
    entity_list_t *oe;
    const vec_t *mo_ra;
    int i;

    start_game();
    idle = build_slipgate();
    h = fold_string_member(h, as_qh(idle)->header.state);
    h = fold_dg(h, idle);

    placed = build_slipgate();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->drag(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_qh(placed)->pos, mp));
    set_credits(200);
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
    assert(as_qh(placed)->sprite == p8_fixed_from_int(168));
    jc = cls_of(placed)->left_press(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_bool(h, jc.stop);
    as_qh(placed)->slipways = oe;
    cls_of(placed)->refresh(placed);
    h = fold_fixed_member(h, as_qh(placed)->sprite);
    h = fold_economy(h, placed);

    fr = build(infraspace_gate_kind());
    assert(gate_kind(fr) == FR);
    h = fold_fixed_member(h, as_qh(fr)->sprite);
    cls_of(fr)->refresh(fr);
    h = fold_fixed_member(h, as_qh(fr)->sprite);
    assert(as_qh(fr)->sprite == p8_fixed_from_int(78));

    bare = slipgate_new(NULL);
    assert(gate_kind(bare) == NR && as_qh(bare)->planet_id == NULL && as_qh(bare)->slipways != NULL &&
           as_qh(bare)->production != NULL);
    h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag("slipgate"), bare));
    {
        planet_keys_t keys;
        vec_t *p = vec_ref(vec_xy(3, 4));
        vec_t *ra = vec_ref(vec_xy(5, 6));
        void *with;
        keys.planet_id = NULL;
        keys.map_pos = p;
        keys.pos = ra;
        with = slipgate_new(&keys);
        assert(as_qh(with)->map_pos == p && as_qh(with)->pos == ra && as_qh(with)->planet_id == NULL);
        h = fnv(h, (uint32_t)ents_rank_in(live_entities(), with));
    }
    check("life", h, LIFE_HASH);
}

static int is_an_nr_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == slipgate_class();
}

/* Built by `ie:ldown` or pico8_nr_new, a slipgate is a struct whose
 * descriptor holds qh's very entries but its own `gv`, `ldown`, `hb`, `cn`,
 * which no class of its chain installs any more. */
static void test_nr_is_a_struct(void) {
    const entity_class_t *cls = slipgate_class();
    const entity_class_t *qh = pico8_qh_struct_class();
    void *a;
    void *fr;

    start_game();
    a = build_slipgate();
    assert(is_an_nr_struct(a) && cls != qh && ents_listed(entities_by_tag("slipgate"), a));
    assert(cls->offer_trade != qh->offer_trade && cls->attach_slipway == qh->attach_slipway && cls->drag == qh->drag && cls->drop == qh->drop && cls->spawn_particle == qh->spawn_particle);
    assert_qh_shape(cls);
    assert(cls->draw == qh->draw && cls->draw_overlay == qh->draw_overlay && cls->hit_test == qh->hit_test && cls->new_year == qh->new_year);
    assert(cls->states[ENTITY_STATE_VANISHING] == qh->states[ENTITY_STATE_VANISHING] &&
           cls->states[ENTITY_STATE_SETTLED] == qh->states[ENTITY_STATE_SETTLED]);
    assert(cls->accept_drop != qh->accept_drop && cls->left_press != qh->left_press && cls->refresh != qh->refresh);
    assert(cls->accept_drop != planet_class()->accept_drop && cls->left_press != planet_class()->left_press &&
           cls->refresh != planet_class()->refresh);
    assert(slipgate_ready() && infraspace_gate_ready());
    {
        planet_t *bare = slipgate_new(NULL);
        assert(is_an_nr_struct(bare) && bare->planet_id == NULL && bare->slipways != NULL && bare->production != NULL);
    }
    fr = build(infraspace_gate_kind());
    assert(fr != NULL &&
           ((const entity_t *)fr)->class == infraspace_gate_class());
    cls_of(fr)->refresh(fr);
    assert(as_qh(fr)->sprite == p8_fixed_from_int(78));
}

/* Counts the offers `cn` makes, taking each as `jo:ib`'s closure does. */
static int g_offers = 0;

static void take_offer(trade_offer_t *self, trade_route_t *a) {
    entity_list_add(self->accepted_routes, &a->header);
    self->amount -= p8_fixed_from_int(1);
    g_offers++;
}

/* `nr:cn` forwards a trade over each of its slipways but one the trade's
 * `rl` already runs through: over `w` to a planet wanting the trade's
 * resource, a trade whose `rl` holds `w` is not offered, one whose `rl`
 * does not is, once. */
static void test_cn_skips_a_slipway_already_in_rl(void) {
    planet_t *self;
    planet_t *other;
    slipway_keys_t jc_keys = {0};
    slipway_t *w;
    pointer_list_t *jy = malloc(sizeof *jy);
    entity_list_t *with_w = malloc(sizeof *with_w);
    entity_list_t *without_w = malloc(sizeof *without_w);
    entity_list_t ib;
    trade_keys_t keys = {0};
    trade_offer_t lw = {0};
    p8_fixed_t qe;
    int r_p;

    start_game();
    assert(resource_code_lookup("p", &r_p));
    qe = p8_fixed_from_int(r_p);
    self = slipgate_new(NULL);
    other = pico8_qh_new(NULL);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, pico8_jy_new(game_zone(), &qe));
    other->needs = jy;
    jc_keys.start = &self->header;
    w = slipway_new(&jc_keys);
    w->end = &other->header;
    assert(self->slipways != NULL && self->slipways->count == 0);
    entity_list_add(self->slipways, &w->header);

    entity_list_init(with_w, &heap_allocator);
    entity_list_add(with_w, &w->header);
    entity_list_init(&ib, &heap_allocator);
    keys.has_resource = 1;
    keys.resource = qe;
    keys.slipways = with_w;
    lw.route = trade_new(game_zone(), &keys);
    lw.amount = p8_fixed_from_int(2);
    lw.accept = take_offer;
    lw.accepted_routes = &ib;
    slipgate_offer_trade(self, &lw);
    assert(g_offers == 0 && ib.count == 0 && lw.amount == p8_fixed_from_int(2));

    entity_list_init(without_w, &heap_allocator);
    keys.slipways = without_w;
    lw.route = trade_new(game_zone(), &keys);
    slipgate_offer_trade(self, &lw);
    assert(g_offers == 1 && ib.count == 1 && lw.amount == p8_fixed_from_int(1));
    entity_list_free(&ib);
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
static pid_t spawn(void (*test)(void)) {
    pid_t child;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    return child;
}

static void reap(pid_t child) {
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void in_fresh_child(void (*test)(void)) {
    reap(spawn(test));
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
    in_fresh_child(test_nr_is_a_struct);
    in_fresh_child(test_cn_skips_a_slipway_already_in_rl);

    printf("test_pico8_nr: all tests passed\n");
    return 0;
}
