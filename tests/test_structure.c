/* The laboratory `qh`, read on its struct members and the generated
 * defaults (pico8_game_data.h), and driven through the real
 * `ie:ldown`/`mv`/`dg`. Its descendants are read the same way. Games are
 * started directly: no events file. */

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
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "game/wormhole.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const vec_t *ra, const vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0x8CC38DAAu
#define LIFE_HASH 0x37B44597u
#define DESCENDANTS_HASH 0x129E3D90u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
static const char *const tags[] = {"structure", "planet", "node", "targeting"};

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

/* `e` as a planet of qh's lineage. */
static planet_t *as_qh(void *e) {
    const entity_t *h = e;

    assert(h != NULL && (h->class->lineage & LINEAGE(LINEAGE_STRUCTURE)));
    return e;
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

static uint32_t fold_qh(uint32_t h, const planet_t *qh) {
    return qh->archetype->is_structure ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const planet_t *jo) {
    return jo->is_dead ? 2u : 0u;
}

/* 1 no target, 3 a target: the bytes the frozen hashes fold. */
static uint32_t et_state(const planet_t *qh) {
    return qh->targeting->et == NULL ? 1u : 3u;
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static int is_bv_target(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_bv_et_struct_class();
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(live_entities(), ref));
    }
    /* The frozen hashes folded the INSTANCE tag the throwaway had. */
    assert(is_bv_target(ref) && "no test folds an entity outside hv that is not the throwaway target: to fold one, fold 0xEEEE then the frozen struct tag 0x9E5C57C7, as test_pico8_fr.c does");
    return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
}

/* `et` as qh's reader gave it: false or the target. */
static uint32_t fold_et(uint32_t h, const planet_t *qh) {
    switch (et_state(qh)) {
    case 1u:
        return fold_bool_member(h, 0);
    default:
        return fold_ref(fnv(h, 1u), qh->targeting->et);
    }
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static uint32_t fold_kw(uint32_t h, const planet_t *jo) {
    const production_t *k = jo->production;

    if (k == NULL) {
        return fnv(h, 0xA0u);
    }
    h = k->has_resource ? fold_fixed_member(h, k->resource) : fold_absent(h);
    h = fold_fixed_member(h, k->amount);
    return fnv(h, (uint32_t)(k == planet_shared_production()));
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
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

/* `c` holds `ref`'s very members but `kf`, `cn` and `mm`, `kj` and the
 * state `ly` included when `with_kj`/`with_ly`. */
static void assert_members_of(const entity_class_t *c, const entity_class_t *ref, int with_kj,
                              int with_ly) {
    assert(c->draw_overlay == ref->draw_overlay && c->hit_test == ref->hit_test && c->new_year == ref->new_year && c->refresh == ref->refresh && c->has_refresh == ref->has_refresh &&
           c->left_press == ref->left_press && c->right_press == ref->right_press && c->accept_drop == ref->accept_drop);
    assert(c->states[ENTITY_STATE_VANISHING] == ref->states[ENTITY_STATE_VANISHING] && c->drag == ref->drag && c->drop == ref->drop);
    assert(!with_kj || c->draw == ref->draw);
    assert(!with_ly || c->states[ENTITY_STATE_SETTLED] == ref->states[ENTITY_STATE_SETTLED]);
}

/* The research icon whose `ke` is `gx[0]`, the laboratory. */
static void *qh_icon(void) {
    const entity_list_t *hb = entities_by_message("refresh");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == pico8_qh_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the laboratory's icon, as the shop does. */
static void *build_lab(void) {
    void *icon = qh_icon();
    down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->left_press(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

/* A laboratory as `ie:ldown` and `jo:pq` left it: its scalars, which of
 * qh's and jo's members are set, its methods (the very functions of the
 * class chain), which default tables it shares with `qh` and `jo`, and
 * where `lt` registered it. */
static uint32_t fold_lab(uint32_t h, void *e) {
    const planet_t *qh = as_qh(e);
    const planet_t *jo = qh;
    size_t i;

    h = fold_string_member(h, jo->planet_id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_fixed_member(h, qh->archetype->project_index);
    h = fold_string_member(h, qh->targeting->def->confirmed_state);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->targeting->od);
    h = fold_fixed_member(h, qh->targeting->def->clock_ticks);
    h = fold_fixed_member(h, qh->targeting->def->attach_distance);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = fold_fixed_member(h, qh->sprite);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->archetype->trade_multiplier);
    h = fold_fixed_member(h, jo->archetype->layer);
    h = fold_fixed_member(h, jo->shake);
    h = fold_fixed_member(h, jo->archetype->back_color);
    h = fold_vec(h, &jo->archetype->row_step);
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->name != NULL));
    h = fnv(h, (uint32_t)(qh->targeting->def->confirmed_state != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->accepted_resources != NULL));
    h = fnv(h, qh->targeting->et != NULL ? 1u : 0u);
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
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
    assert(cls_of(e)->attach_slipway == pico8_qh_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_qh_struct_class()->offer_trade &&
           cls_of(e)->spawn_particle == NULL && planet_is_family(e));
    assert(jo->header.class->tags == (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && qh->archetype->name == &structure_name_row && qh->archetype->accepted_resources == &structure_accepted_resources_seq);
    assert(jo->archetype == &structure_archetype);
    assert(jo->archetype == &structure_archetype);
    assert(jo->archetype->spacer == &planet_spacer_row && jo->archetype->ring_colors == &planet_ring_colors_seq && jo->archetype->mood_rows == planet_mood_rows_rows &&
           jo->archetype->made_resources == &planet_made_resources_seq);
    assert(jo->production != NULL && jo->production != planet_shared_production());
    assert(pico8_gb_at(1) != NULL);
    assert(jo->project == pico8_gb_at(1));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_list_size(jo->slipways));
    h = fnv(h, (uint32_t)list_size(jo->needs));
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag(tags[i]), e));
    }
    return h;
}

/* Two laboratories built by the shop's `ie:ldown` on `gx[0]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_lab();
    b = build_lab();
    assert(a != b);
    h = fold_lab(h, a);
    h = fold_lab(h, b);
    check("built", h, BUILT_HASH);
}

static p8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = p8_fixed_from_int(n);
}

/* What `bv` left on a laboratory: `ra` (a copy of what was passed), `p`, and
 * what `et` is. */
static uint32_t fold_held(uint32_t h, void *e, const vec_t *mp) {
    const planet_t *qh = as_qh(e);

    assert(ra_copies(qh->pos, mp));
    h = fold_vec(h, qh->map_pos);
    h = fnv(h, et_state(qh));
    return fold_et(h, qh);
}

static uint32_t fold_ra_y(uint32_t h, void *e) {
    const vec_t *ra = as_qh(e)->pos;
    assert(ra != NULL);
    return fnv(h, (uint32_t)ra->y);
}

/* `hq` (`bv`) at an empty point, in the panel and out of it, then next to
 * a planet; `ey` (`d_`) with and without the credits; `dg` on a laboratory
 * in "ly" (`qh.ly` = `na`) and on one still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *out_lab;
    void *in_lab;
    void *poor_lab;
    void *planet;
    vec_t *mp;
    const vec_t *planet_p;
    const vec_t *ra_before;
    p8_fixed_t before;
    p8_fixed_t in_y;
    int i;

    start_game();

    out_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    cls_of(out_lab)->drag(out_lab, mp, vec_ref(vec_xy(0, 0)));
    h = fold_held(h, out_lab, mp);
    assert(et_state(as_qh(out_lab)) == 3 && !ents_listed(live_entities(), as_qh(out_lab)->targeting->et));

    planet = entities_by_tag("planet")->items[0];
    assert(planet_is_family(planet));
    planet_p = ((const planet_t *)planet)->map_pos;
    assert(planet_p != NULL);
    mp = vec_ref(vec_add(vec_mul(*planet_p, p8_fixed_from_int(128)), vec_xy(6, 0)));
    cls_of(out_lab)->drag(out_lab, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, out_lab, mp);
    assert(as_qh(out_lab)->targeting->et == planet &&
           ((const planet_t *)planet)->targeted_by == out_lab);

    poor_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    cls_of(poor_lab)->drag(poor_lab, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, poor_lab, mp);
    assert(et_state(as_qh(poor_lab)) == 1);
    set_credits(3);
    assert(cls_of(poor_lab)->drop == pico8_d__dispatch);
    h = pico8_d_(poor_lab) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(poor_lab)->header.state);
    h = fold_fixed_member(h, as_qh(poor_lab)->header.state_time);
    h = fnv(h, p__state(as_qh(poor_lab)));
    h = fnv(h, (uint32_t)credits());
    assert(p__state(as_qh(poor_lab)) == 2 && credits() == p8_fixed_from_int(3));

    in_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7552));
    cls_of(in_lab)->drag(in_lab, mp, vec_ref(vec_xy(64, 64)));
    set_credits(100);
    assert(cls_of(in_lab)->drop == pico8_d__dispatch);
    h = pico8_d_(in_lab) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(in_lab)->header.state);
    h = fold_fixed_member(h, as_qh(in_lab)->header.state_time);
    h = fnv(h, p__state(as_qh(in_lab)));
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(as_qh(in_lab)->header.state, "settled") == 0 && credits() == p8_fixed_from_int(100 - 15));
    in_y = as_qh(in_lab)->pos->y;

    ra_before = as_qh(out_lab)->pos;
    before = ra_before->y;
    for (i = 0; i < 4; i++) {
        h = fold_ra_y(h, in_lab);
        update_entities();
        h = fold_fixed_member(h, as_qh(in_lab)->header.state_time);
        assert(as_qh(out_lab)->pos == ra_before && ra_before->y == before);
    }
    h = fold_ra_y(h, in_lab);
    assert(as_qh(in_lab)->pos->y != in_y);
    h = fnv(h, (uint32_t)ents_listed(live_entities(), poor_lab));
    check("life", h, LIFE_HASH);
}

enum { NL, OS, HJ, PM, MX, NR, FR, DESCENDANTS };

static int descendant_of(const void *e) {
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_nl_struct_class()) {
        return NL;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_os_struct_class()) {
        return OS;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_hj_struct_class()) {
        return HJ;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_pm_struct_class()) {
        return PM;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == synthesizer_class()) {
        return MX;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == slipgate_class()) {
        return NR;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == infraspace_gate_class()) {
        return FR;
    }
    return -1;
}

/* What a descendant inherits from `qh`: `dq`, `fp`, `dw`, `hf`, whether
 * its tags/`px`/`ba` are qh's own, `hq`/`ev`/`ey` (qh's functions),
 * and its `ly` called once (`ra.y` before, after, then put back). */
static uint32_t fold_descendant(uint32_t h, void *d) {
    planet_t *qh = as_qh(d);
    planet_t *nl = cls_of(d) == pico8_nl_struct_class() ? (planet_t *)d : NULL;
    p8_fixed_t qw = nl != NULL ? nl->palette_block : 0;

    h = fold_string_member(h, qh->targeting->def->confirmed_state);
    h = fold_fixed_member(h, qh->targeting->def->clock_ticks);
    h = fold_fixed_member(h, qh->targeting->def->attach_distance);
    h = fold_hf(h, qh);
    h = qh->has_development_tier ? fold_fixed_member(h, qh->development_tier) : fold_absent(h);
    h = fnv(h, (uint32_t)(qh->header.class->tags == (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING))));
    h = fnv(h, (uint32_t)(qh->archetype->name == &structure_name_row));
    h = fnv(h, (uint32_t)(qh->archetype->accepted_resources == &structure_accepted_resources_seq));
    assert(cls_of(d)->drag == pico8_qh_struct_class()->drag && cls_of(d)->drop == pico8_qh_struct_class()->drop);
    assert(cls_of(d)->draw_overlay == pico8_qh_struct_class()->draw_overlay);
    if (qh->pos != NULL) {
        vec_t *v = qh->pos;
        p8_fixed_t y = v->y;
        h = fnv(fnv(h, 1u), (uint32_t)y);
        cls_of(d)->states[ENTITY_STATE_SETTLED](d, p8_fixed_from_int(50));
        h = fnv(h, (uint32_t)v->y);
        v->y = y;
    } else {
        cls_of(d)->states[ENTITY_STATE_SETTLED](d, p8_fixed_from_int(50));
        h = fnv(h, 0u);
    }
    h = nl != NULL ? fold_fixed_member(h, nl->palette_block) : fold_absent(h);
    if (nl != NULL) {
        nl->palette_block = qw;
    }
    return h;
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message("refresh");
    const pico8_qb_t *mo = pico8_mo_qb();
    const vec_t *mo_ra;
    int i;

    assert(mo != NULL && mo->ra != NULL);
    mo_ra = mo->ra;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (cls_of(icon) == pico8_ie_struct_class() && ((const pico8_ie_t *)icon)->ke == kind) {
            down_t made = cls_of(icon)->left_press(icon, NULL, NULL);
            assert(made.stop && made.held != NULL);
            cls_of(made.held)->drag(made.held, vec_ref(vec_add(*mo_ra, vec_xy(64, 64))),
                                  vec_ref(vec_xy(64, 64)));
            return made.held;
        }
    }
    assert(0 && "no icon for that kind");
    return NULL;
}

/* The project of `gb` whose `cw` names `cw`. */
static pico8_hp_t *cw_project(const char *cw) {
    int k;

    for (k = 1;; k++) {
        pico8_hp_t *hp = pico8_gb_at(k);
        assert(hp != NULL);
        if (hp->cw != NULL && strcmp(hp->cw, cw) == 0) {
            return hp;
        }
    }
}

/* Each descendant by the road a game takes to it: the six of the shop
 * bought from their icons, `pm` turned from a generated planet by `jo:hu`
 * on the collapse project (the newest entity of `hv`). */
static void test_descendants(void) {
    static const struct {
        int d;
        const entity_kind_t *(*kind)(void);
    } shop[] = {{OS, pico8_os_kind}, {NR, slipgate_kind}, {FR, infraspace_gate_kind},
                {NL, pico8_nl_kind}, {MX, synthesizer_kind}, {HJ, pico8_hj_kind}};
    void *made[DESCENDANTS] = {NULL};
    uint32_t h = 2166136261u;
    void *p;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    p = entities_by_tag("planet")->items[0];
    planet_roll_type(p);
    planet_apply_project(p, cw_project("protostar"));
    made[PM] = live_entities()->items[live_entities()->count - 1];
    for (i = 0; i < DESCENDANTS; i++) {
        assert(made[i] != NULL && descendant_of(made[i]) == (int)i && ents_listed(live_entities(), made[i]));
        h = fold_descendant(fnv(h, (uint32_t)i), made[i]);
    }
    check("descendants", h, DESCENDANTS_HASH);
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
    (void)pico8_qh_new(&keys);
}

static int is_a_qh_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_qh_struct_class();
}

/* Built by `ie:ldown` or pico8_qh_new, a `qh` is a struct whose descriptor
 * holds jo's very dispatchers and qh's own `hq`, `ev`, `ey`, `ly`; a
 * construction key other than `id` stops the run. Every descendant is a
 * struct under its own descriptor. */
static void test_qh_is_a_struct(void) {
    const entity_kind_t *const descendants[] = {pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(), pico8_pm_kind(),
                                                      synthesizer_kind(), slipgate_kind(), infraspace_gate_kind()};
    const entity_class_t *cls = pico8_qh_struct_class();
    void *a;
    size_t i;

    start_game();
    a = build_lab();
    assert(is_a_qh_struct(a) && ents_listed(entities_by_tag("structure"), a));
    assert(pico8_da(a) == p8_fixed_from_int(entities_by_tag("structure")->count));
    assert(cls->attach_slipway == planet_class()->attach_slipway && cls->offer_trade == planet_class()->offer_trade);
    assert_qh_shape(cls);
    assert(cls->hit_test == planet_class()->hit_test && cls->draw == planet_class()->draw &&
           cls->left_press == planet_class()->left_press && cls->refresh == planet_class()->refresh &&
           cls->accept_drop == planet_class()->accept_drop && cls->new_year == planet_class()->new_year &&
           cls->states[ENTITY_STATE_VANISHING] == planet_class()->states[ENTITY_STATE_VANISHING]);
    assert(cls->draw_overlay != planet_class()->draw_overlay && cls->states[ENTITY_STATE_SETTLED] != NULL);
    assert(cls->drag != planet_class()->drag && cls->drop != planet_class()->drop && cls->spawn_particle == NULL);

    must_abort(build_with_unknown_key, NULL);
    {
        planet_t *bare = pico8_qh_new(NULL);
        assert(is_a_qh_struct(bare) && bare->planet_id == NULL && bare->slipways != NULL && ents_listed(entities_by_tag("structure"), bare));
    }
    for (i = 0; i < sizeof descendants / sizeof descendants[0]; i++) {
        planet_keys_t keys;
        void *d;
        keys.planet_id = "s99";
        keys.map_pos = NULL;
        keys.pos = NULL;
        d = descendants[i]->new_entity(&keys);
        if (descendants[i] == pico8_nl_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_nl_struct_class());
            continue;
        }
        if (descendants[i] == pico8_os_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_os_struct_class());
            continue;
        }
        if (descendants[i] == pico8_hj_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_hj_struct_class());
            continue;
        }
        if (descendants[i] == pico8_pm_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_pm_struct_class());
            continue;
        }
        if (descendants[i] == synthesizer_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == synthesizer_class());
            continue;
        }
        if (descendants[i] == slipgate_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == slipgate_class());
            continue;
        }
        assert(d != NULL &&
               ((const entity_t *)d)->class == infraspace_gate_class());
    }
}

static unsigned lineage_of(const entity_t *e) {
    return e->class->lineage;
}

/* The eight kinds of qh's lineage are of jo's and qh's, nl alone of nl's
 * too; ga and jo of jo's alone, g_ and fv of g_'s alone, the other classes
 * of none. */
static void test_lineage_of_each_class(void) {
    const unsigned jo = LINEAGE(LINEAGE_PLANET);
    const unsigned qh = jo | LINEAGE(LINEAGE_STRUCTURE);
    const entity_kind_t *const lineage[] = {pico8_qh_kind(), pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(),
                                                  pico8_pm_kind(), synthesizer_kind(), slipgate_kind(), infraspace_gate_kind()};
    planet_keys_t keys = {"s99", NULL, NULL};
    const entity_t *e;
    size_t i;

    start_game();
    for (i = 0; i < sizeof lineage / sizeof lineage[0]; i++) {
        e = lineage[i]->new_entity(&keys);
        assert(lineage_of(e) == (lineage[i] == pico8_nl_kind() ? (qh | LINEAGE(LINEAGE_TRADING_HUB)) : qh));
    }
    assert(lineage_of(pico8_ga_kind()->new_entity(&keys)) == jo);
    assert(lineage_of(&planet_new(&keys)->header) == jo);
    assert(lineage_of(&pico8_g__new(NULL)->header) == LINEAGE(LINEAGE_PROBE));
    assert(lineage_of(&pico8_fv_new()->header) == LINEAGE(LINEAGE_PROBE));
    assert(lineage_of(&pico8_qb_new()->header) == 0 && lineage_of(&pico8_bg_new()->header) == 0);
    assert(lineage_of(&pico8_mz_new()->header) == 0 && lineage_of(&slipway_new(NULL)->header) == 0);
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
    in_fresh_child(test_descendants);
    in_fresh_child(test_qh_is_a_struct);
    in_fresh_child(test_lineage_of_each_class);

    printf("test_pico8_qh: all tests passed\n");
    return 0;
}
