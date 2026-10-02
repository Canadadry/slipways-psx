/* The particle burst `ga`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built by its three roads (`jo:hu`
 * on the harvest recipe, `pm:pq`, `pico8_ga_new(NULL)`) and driven through
 * the real `mv`/`dc`. jo's methods are named by `pico8_jo_struct_class()`.
 * No events file. */

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
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/game_zone.h"


#define BUILT_HASH 0xA539DD6Du
#define LIFE_HASH 0x7DAF9781u
#define BURST_FRAMES 86

/* The tag `lq.fs` carried as a tagged list, which the frozen hashes fold:
 * any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const tags[] = {"planet", "node", "targeting", "ascension", "structure"};
static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};

static int tag_count(unsigned tags) {
    int t, n = 0;

    for (t = 0; t < TAG_COUNT; t++) {
        n += (tags & TAG(t)) != 0;
    }
    return n;
}

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

static uint32_t fold_p_(uint32_t h, const planet_t *jo) {
    return (jo->is_dead != 0) ? fold_bool_member(h, jo->is_dead) : fold_absent(h);
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    assert(ref == NULL && "no test folds a set ref: to fold one, fold 1 then its rank in hv (0x4E, rank), or outside hv 0xEEEE then the frozen struct tag 0x9E5C57C7, as the frozen hashes did");
    return fold_absent(h);
}

/* The item count of a list member, -1 when NULL. */
static int ptrs_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_ptrs_size(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
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

static vec_t near_mo(int x, int y) {
    const vec_t *mo_ra;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    return vec_add(*mo_ra, vec_xy(x, y));
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* `fz` is "ga" on a burst and on nothing else. */
static int is_ga(const void *e) {
    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const planet_t *)e)->archetype->class_name != NULL &&
           strcmp(((const planet_t *)e)->archetype->class_name, "burst") == 0;
}

static planet_t *as_ga(void *e) {
    assert(e != NULL && cls_of(e) == pico8_ga_struct_class());
    return e;
}

/* The first burst of `hv` at rank `from` or later, NULL when none. */
static void *ga_from(int from) {
    const entity_list_t *hv = live_entities();
    int i;
    for (i = from; i < hv->count; i++) {
        if (is_ga(hv->items[i])) {
            return hv->items[i];
        }
    }
    return NULL;
}

/* The recipe of `gb` whose `cw` names "ga": the harvest. */
static void *harvest_recipe(void) {
    void *found = NULL;
    int i;

    for (i = 1; i <= research_projects_count(); i++) {
        research_project_t *hp = research_projects_at(i);
        if (hp->class_name != NULL && strcmp(hp->class_name, "burst") == 0) {
            assert(found == NULL || found == hp);
            found = hp;
        }
    }
    assert(found != NULL);
    return found;
}

/* Road (a): a bare planet on `id`/`p`/`ra`, harvested by `jo:hu`; the burst
 * is the one `hv` gained. */
static void *harvest(int x, int y, const char *id) {
    planet_keys_t keys;
    planet_t *planet;
    void *made;
    int before = live_entities()->count;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_xy(x / 2, y / 2));
    keys.pos = vec_ref(near_mo(x, y));
    planet = planet_new(&keys);
    assert(!is_ga(planet) && live_entities()->count == before + 1);
    planet_apply_project(planet, harvest_recipe());
    made = ga_from(before + 1);
    assert(made != NULL && live_entities()->count == before + 2 && ga_from(0) == made);
    assert(as_ga(made)->pos == planet->pos && as_ga(made)->map_pos == planet->map_pos);
    return made;
}

/* Road (b): `kv["pm"]` on `p`/`ra`/`id`; its burst is registered just
 * before the protostar. */
static void *spawn(int x, int y, const char *id, void **protostar) {
    planet_keys_t keys;
    void *pm;
    int at;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_xy(x / 2, y / 2));
    keys.pos = vec_ref(near_mo(x, y));
    pm = pico8_pm_new(&keys);
    at = ents_rank_in(live_entities(), pm);
    assert(at >= 1 && is_ga(live_entities()->items[at - 1]));
    {
        const vec_t *ra = as_ga(live_entities()->items[at - 1])->pos;
        vec_t want = vec_add(*keys.pos, ((planet_t *)pm)->archetype->spawn_offset);
        assert(ra != keys.pos && ra->x == want.x && ra->y == want.y);
    }
    if (protostar != NULL) {
        *protostar = pm;
    }
    return live_entities()->items[at - 1];
}

/* A burst as its road and `ga:pq`, `jo:pq`, `ke:pq` left it: its scalars,
 * which of its own and jo's names it has, its 12 methods (jo's very
 * functions but `kj` and `mm`), which default tables it shares with `ga`
 * and `jo`, where `lt` registered it -- in `hv` alone -- and `mo.ma`. */
static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
    const planet_t *g = as_ga(e);
    const emitter_t *em = g->emitter;
    const planet_t *jo = g;
    int j;

    assert(is_ga(e));
    h = fold_string_member(h, g->archetype->class_name);
    h = fold_fixed_member(h, jo->archetype->layer);
    h = fold_fixed_member(h, em->archetype->spawn_rate);
    h = fold_fixed_member(h, em->matrix_xx);
    h = fold_fixed_member(h, em->matrix_xy);
    h = fold_fixed_member(h, em->matrix_yx);
    h = fold_fixed_member(h, em->matrix_yy);
    h = fold_fixed_member(h, g->acceleration_factor);
    h = fold_fixed_member(h, em->speed_factor);
    h = fold_fixed_member(h, g->speed);
    h = fold_hf(h, jo);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, jo->planet_id);
    h = fold_fixed_member(h, jo->shake);
    h = fold_p_(h, jo);
    h = fold_vec(h, jo->pos);
    h = fold_vec(h, jo->map_pos);
    h = fnv(h, (uint32_t)(em->trail_colors == NULL ? -1 : em->trail_colors->count));
    h = fnv(h, (uint32_t)tag_count(jo->header.class->tags));
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->outgoing_routes));
    h = fnv(h, (uint32_t)ptrs_size(em->particles));
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->slipways));
    assert(pico8_mo_qb() != NULL);
    h = fold_fixed_member(h, pico8_mo_qb()->ma);
    /* The 1s of `fc`, `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms` (ga's own)
     * and of `hf`, `hs`, `fa`, `ma`, `ca` (jo's) are constant by the
     * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
    h = fnv(h, (uint32_t)(g->archetype->class_name != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->trail_colors != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->particles != NULL));
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
    h = fnv(h, (uint32_t)(jo->has_production_bonus != 0));
    h = fnv(h, (uint32_t)(jo->is_dead != 0));
    h = fnv(h, (uint32_t)(jo->has_net_income != 0));
    h = fnv(h, (uint32_t)(jo->incoming_routes != NULL));
    h = fnv(h, (uint32_t)(jo->outgoing_routes != NULL));
    h = fnv(h, (uint32_t)(jo->has_development_tier != 0));
    h = fnv(h, (uint32_t)(jo->is_hovered != 0));
    assert(jo->needs == NULL && !jo->has_net_income);
    assert(cls_of(e)->attach_slipway == planet_class()->attach_slipway && cls_of(e)->offer_trade == planet_class()->offer_trade &&
           cls_of(e)->attach_slipway != NULL && cls_of(e)->offer_trade != NULL && planet_is_family(e));
    assert(cls_of(e)->spawn_particle != planet_class()->spawn_particle && cls_of(e)->spawn_particle != NULL);
    assert(cls_of(e)->draw != NULL && cls_of(e)->draw != planet_class()->draw);
    assert(cls_of(e)->hit_test == planet_class()->hit_test && cls_of(e)->left_press == planet_class()->left_press &&
           cls_of(e)->refresh == planet_class()->refresh && cls_of(e)->accept_drop == planet_class()->accept_drop &&
           cls_of(e)->new_year == planet_class()->new_year &&
           cls_of(e)->states[ENTITY_STATE_VANISHING] == planet_class()->states[ENTITY_STATE_VANISHING]);
    assert(cls_of(e)->hit_test != NULL && cls_of(e)->left_press != NULL && cls_of(e)->refresh != NULL && cls_of(e)->accept_drop != NULL &&
           cls_of(e)->new_year != NULL && cls_of(e)->states[ENTITY_STATE_VANISHING] != NULL);
    assert(cls_of(e)->states[ENTITY_STATE_NORMAL] == NULL && cls_of(e)->draw_overlay == NULL &&
           cls_of(e)->states[ENTITY_STATE_PLACING] == NULL && cls_of(e)->states[ENTITY_STATE_UNFOLDING] == NULL &&
           cls_of(e)->right_press == NULL && cls_of(e)->states[ENTITY_STATE_SETTLED] == NULL);
    assert(cls_of(e)->draw_at == NULL && cls_of(e)->drag == NULL && cls_of(e)->drop == NULL);
    assert(jo->header.class->tags == 0);
    assert(jo->archetype == &burst_archetype);
    assert(jo->outgoing_routes != NULL);
    if (from_pm) {
        assert(em->trail_colors != &burst_trail_colors_seq);
        assert(em->trail_colors != &protostar_trail_colors_seq);
    } else {
        assert(em->trail_colors == &burst_trail_colors_seq);
    }
    assert(jo->archetype == &burst_archetype);
    assert(jo->production == planet_shared_production());
    assert(jo->archetype->spacer == &planet_spacer_row);
    assert(jo->archetype->ring_colors == &planet_ring_colors_seq);
    assert(jo->archetype->mood_rows == planet_mood_rows_rows);
    assert(jo->archetype->made_resources == &planet_made_resources_seq);
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (j = 0; j < (int)(sizeof bm / sizeof bm[0]); j++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[j]), e));
    }
    for (j = 0; j < (int)(sizeof tags / sizeof tags[0]); j++) {
        assert(!ents_listed(entities_by_tag(tags[j]), e));
    }
    return h;
}

/* One burst by each road, the first two on screen. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;

    start_game();
    assert(ga_from(0) == NULL);
    a = harvest(40, 40, "s61");
    h = fold_burst(h, a, 0);
    b = spawn(80, 70, "s62", NULL);
    h = fold_burst(h, b, 1);
    c = pico8_ga_new(NULL);
    h = fold_burst(h, c, 0);
    assert(a != b && b != c && a != c);
    assert(as_ga(a)->emitter->particles != as_ga(b)->emitter->particles &&
           as_ga(b)->emitter->particles != as_ga(c)->emitter->particles);
    assert(as_ga(a)->slipways != as_ga(b)->slipways);
    assert(as_ga(a)->outgoing_routes == as_ga(b)->outgoing_routes && as_ga(b)->outgoing_routes == as_ga(c)->outgoing_routes);
    assert(as_ga(c)->pos == NULL && as_ga(c)->map_pos == NULL && as_ga(c)->planet_id == NULL);
    assert(as_ga(a)->archetype->layer == p8_fixed_from_int(3) && as_ga(a)->speed == p8_fixed_from_int(3));
    assert(as_ga(a)->archetype->is_planet == 0);
    assert(as_ga(b)->speed == P8_FIXED_LIT(0x00018000, 1.5));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, const particle_t *q) {
    assert(q != NULL);
    assert(q->has_acceleration);
    h = fnv(fnv(h, (uint32_t)q->pos.x), (uint32_t)q->pos.y);
    h = fnv(fnv(h, (uint32_t)q->velocity.x), (uint32_t)q->velocity.y);
    h = fnv(fnv(h, (uint32_t)q->acceleration.x), (uint32_t)q->acceleration.y);
    return fnv(h, (uint32_t)q->life);
}

/* A tuple of `lq:mn`: items, `qe`, `c`, then the entry count. */
static uint32_t fold_tuple(uint32_t h, const data_row_t *t) {
    int i;

    for (i = 0; i < t->count; i++) {
        const data_item_t *item = &t->items[i];

        h = fnv(h, (uint32_t)(i + 1));
        if (item->text != NULL) {
            h = fold_string(h, item->text);
        } else if (item->is_false) {
            h = fold_bool(h, 0);
        } else {
            assert(!item->is_ref && "no test folds a tuple item that points: to fold one, fold 0xDEAD if NULL, else 0xEEEE then 0, as the frozen hashes did");
            h = fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->number));
        }
    }
    if (t->has_resource) {
        h = fold_fixed(fnv_str(h, "resource"), t->resource);
    }
    if (t->has_color) {
        h = fold_fixed(fnv_str(h, "c"), p8_fixed_from_int(t->color));
    }
    return fnv(h, (uint32_t)(t->count + (t->has_resource != 0) + (t->has_color != 0)));
}

/* A positional as the value it was sorted from folded; NOTHING as false
 * (no row folded here holds a nil at the top level). */
static uint32_t fold_lq_positional(uint32_t h, const layout_row_t *lq, int i) {
    layout_row_cell_t c = lq->items[i];

    h = fnv(h, (uint32_t)(i + 1));
    switch (c.kind) {
    case LAYOUT_ROW_CELL_ROW:
        return fold_tuple(h, c.as.row);
    case LAYOUT_ROW_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case LAYOUT_ROW_CELL_STRING:
        return fold_string(h, c.as.text);
    default:
        return fold_bool(h, c.kind == LAYOUT_ROW_CELL_TRUE);
    }
}

/* The row `jo:ex` built into `lq`: its first `#fs` positionals, `c`, `bg`,
 * `score`, `ra`, `fs`, `w`, the rest. */
static uint32_t fold_lq(uint32_t h, void *e) {
    const layout_row_t *lq = as_ga(e)->layout_row;
    int n = 0;
    int k;
    int i;

    assert(lq != NULL &&
           lq->header.class == layout_row_class());
    assert(!lq->has_background);
    assert(lq->placed_items != NULL);
    k = lq->placed_items_count;
    for (i = 0; i < k; i++) {
        h = fold_lq_positional(h, lq, i);
    }
    if (lq->has_color) {
        h = fold_fixed(fnv_str(h, "c"), lq->color);
        n++;
    }
    if (lq->has_score) {
        h = fold_fixed(fnv_str(h, "score"), lq->score);
        n++;
    }
    assert(lq->pos == NULL && "no test folds an lq whose ra is set: to fold one, fold the key \"ra\", then 0xEEEE then 0, count it in n, as the frozen hashes did");
    /* As fold_ref folded the tagged list `fs` was: never in `hv`, so the tag branch. */
    h = fnv(fnv(fnv_str(h, "placed_items"), 0xEEEEu), FROZEN_LIST_TAG);
    h = fold_fixed(fnv_str(h, "w"), lq->width);
    for (i = k; i < lq->items_count; i++) {
        h = fold_lq_positional(h, lq, i);
    }
    return fnv(h, (uint32_t)(lq->items_count + n + 2));
}

/* Cuts `qy` to its first `n` particles, giving back the others. */
static void cut_qy(pointer_list_t *qy, int n) {
    while (qy->count > n) {
        allocator_release(particle_zone(PARTICLE_SLOT_PARTICLE), qy->items[qy->count - 1]);
        qy->count--;
    }
}

/* `kj` on `e` with its `qy` cut in place to `n` particles: `p_` after. */
static uint32_t fold_cut(uint32_t h, void *e, int n) {
    planet_t *g = as_ga(e);
    pointer_list_t *qy = g->emitter->particles;

    assert(qy != NULL && qy->count >= n);
    cut_qy(qy, n);
    cls_of(e)->draw(e, g->pos);
    h = fold_p_(h, g);
    return fnv(h, (uint32_t)qy->count);
}

/* `mm` on each burst; `dc("kj")` three times, then three more with the
 * protostar burst cut to 3 particles, so that its circles, drawn in the
 * class `k_` (TROU-21), are not all covered by `ei`'s dots in its own; `p_` once `qy` holds 3, 2, 0 particles; `hd` on and off the burst,
 * `gg`; `hb` on jo's `kw` (no row), then on one with a `qe`, before and
 * after two routes land in the shared `by`; `hk` with no `kr`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;
    void *pm;
    entity_list_t *by;
    static production_t kw;
    uint32_t row;
    int hv_count;
    int i;

    start_game();
    a = harvest(40, 40, "s71");
    b = spawn(84, 60, "s72", &pm);
    c = pico8_ga_new(NULL);
    h = fold_particle(h, cls_of(a)->spawn_particle(a));
    h = fold_particle(h, cls_of(b)->spawn_particle(b));
    h = fold_particle(h, cls_of(c)->spawn_particle(c));
    for (i = 0; i < 3; i++) {
        draw_layers("draw");
        h = fnv(h, screen_hash());
        h = fold_p_(h, as_ga(a));
        h = fold_p_(h, as_ga(b));
        h = fold_p_(h, as_ga(c));
        h = fnv(h, (uint32_t)ptrs_size(as_ga(b)->emitter->particles));
    }

    cut_qy(as_ga(b)->emitter->particles, 3);
    for (i = 0; i < 3; i++) {
        draw_layers("draw");
        h = fnv(h, screen_hash());
    }

    h = fold_cut(h, a, 3);
    assert(!as_ga(a)->is_dead);
    h = fold_cut(h, a, 2);
    assert(as_ga(a)->is_dead);
    h = fold_cut(h, a, 0);
    assert(as_ga(a)->is_dead);

    h = fold_bool(h, cls_of(b)->hit_test(b, as_ga(b)->pos, as_ga(b)->pos));
    h = fold_bool(h, cls_of(b)->hit_test(b, vec_ref(near_mo(0, 0)), vec_ref(near_mo(0, 0))));
    h = fnv(fnv(h, 0xB0u), planet_is_colonized(b) ? 1u : 0u);
    assert(planet_is_colonized(a) == 0);

    cls_of(a)->refresh(a);
    h = fold_ref_member(h, as_ga(a)->layout_row);
    assert(as_ga(a)->layout_row == NULL);
    kw.has_resource = 1;
    kw.resource = p8_fixed_from_int(12);
    kw.amount = p8_fixed_from_int(2);
    as_ga(a)->production = &kw;
    cls_of(a)->refresh(a);
    row = fold_lq(2166136261u, a);
    h = fnv(h, row);
    by = as_ga(c)->outgoing_routes;
    assert(by == as_ga(a)->outgoing_routes && by->count == 0);
    entity_list_add(by, &trade_new(game_zone(), NULL)->header);
    entity_list_add(by, &trade_new(game_zone(), NULL)->header);
    cls_of(a)->refresh(a);
    assert(fold_lq(2166136261u, a) != row);
    h = fold_lq(h, a);
    assert(ents_ptrs_size(as_ga(b)->outgoing_routes) == 2);
    /* `jo:ib` is `self.by or {}`: the `by` held, routes kept, never a fresh one. */
    assert(planet_rebuild_trades(as_ga(b)) == by && by->count == 2);

    hv_count = live_entities()->count;
    cls_of(b)->new_year(b, p8_fixed_from_int(0));
    assert(live_entities()->count == hv_count && !as_ga(b)->has_net_income);
    h = fnv(h, screen_hash());
    check("life", h, LIFE_HASH);
}

/* A harvest played frame by frame: one live burst at a time, gone after
 * BURST_FRAMES frames. */
static void test_burst(void) {
    int frames = 0;
    void *e;

    start_game();
    e = harvest(40, 40, "s64");
    assert(ga_from(0) == e);
    while (ga_from(0) != NULL) {
        e = ga_from(0);
        assert(ga_from(ents_rank_in(live_entities(), e) + 1) == NULL);
        pico8__update60();
        pico8__draw();
        frames++;
        assert(frames < 600);
    }
    check_int("burst frames", frames, BURST_FRAMES);
    pico8_cart_vider_file_sortie();
    check_int("ga places after the burst", particle_zone_live(PARTICLE_SLOT_BURST), 0);
}

/* `ei` moves a particle of `qy` in place: `p` by the old `v`, `v` turned
 * (its second row reads the x just written), then accelerated by `a`, and
 * `l` less 0.14. A protostar's burst turns (`qg` is not 0). */
static void test_ei_moves_a_particle_in_place(void) {
    particle_t *particle = particle_take();
    vec_t p0 = vec_xy(3, -2);
    vec_t v0 = vec_make(P8_FIXED_LIT(0x00018000, 1.5), P8_FIXED_LIT(0x00004000, 0.25));
    vec_t a0 = vec_make(P8_FIXED_LIT(0x00002000, 0.125), P8_FIXED_LIT(0xFFFF8000, -0.5));
    pointer_list_t *qy;
    p8_fixed_t vx;
    p8_fixed_t vy;
    void *e;
    const emitter_t *em;

    start_game();
    e = spawn(84, 60, "s73", NULL);
    em = as_ga(e)->emitter;
    assert(em->matrix_yx != 0);
    particle->pos = p0;
    particle->velocity = v0;
    particle->has_acceleration = 1;
    particle->acceleration = a0;
    particle->life = p8_fixed_from_int(5);
    qy = em->particles;
    cut_qy(qy, 0);
    pointer_list_add(qy, particle);

    emitter_update(&gfx, &mem, e);

    vx = p8_fixed_add(p8_fixed_mul(em->matrix_xx, v0.x), p8_fixed_mul(em->matrix_xy, v0.y));
    vy = p8_fixed_add(p8_fixed_mul(em->matrix_yx, vx), p8_fixed_mul(em->matrix_yy, v0.y));
    assert(qy->count >= 1 && qy->items[0] == particle);
    assert(particle->pos.x == p8_fixed_add(p0.x, v0.x) && particle->pos.y == p8_fixed_add(p0.y, v0.y));
    assert(particle->velocity.x == p8_fixed_add(vx, a0.x) && particle->velocity.y == p8_fixed_add(vy, a0.y));
    assert(particle->acceleration.x == a0.x && particle->acceleration.y == a0.y);
    assert(particle->life == p8_fixed_sub(p8_fixed_from_int(5), P8_FIXED_LIT(0x000023d7, 0.14)));
}

static int is_a_ga_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_ga_struct_class();
}

/* Built by any road, a burst is a struct whose descriptor holds jo's very
 * entries but its own `kj`, plus its own `mm`. Neither ga's class nor jo's
 * installs a function or a `pq`. */
static void test_ga_is_a_struct(void) {
    const entity_class_t *cls = pico8_ga_struct_class();
    const entity_class_t *jo = planet_class();
    void *roads[3];
    size_t i;
    int j;

    start_game();
    roads[0] = harvest(40, 40, "s81");
    roads[1] = spawn(80, 70, "s82", NULL);
    roads[2] = pico8_ga_new(NULL);
    for (i = 0; i < 3; i++) {
        assert(is_a_ga_struct(roads[i]));
    }
    assert(cls->draw != NULL && cls->draw != jo->draw);
    assert(cls->draw_overlay == jo->draw_overlay && cls->hit_test == jo->hit_test && cls->new_year == jo->new_year && cls->refresh == jo->refresh &&
           cls->has_refresh == jo->has_refresh && cls->left_press == jo->left_press && cls->right_press == jo->right_press && cls->accept_drop == jo->accept_drop);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == jo->states[j]);
    }
    assert(cls->spawn_particle == pico8_ga_mm && jo->spawn_particle == NULL);
    assert(cls->drag == jo->drag && cls->drop == jo->drop && cls->attach_slipway == jo->attach_slipway && cls->offer_trade == jo->offer_trade && cls->draw_at == jo->draw_at);
    assert(pico8_ga_ready());
    assert(planet_ready());
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
    int ga;
    int rangees;
} held_t;

static held_t held_now(void) {
    held_t h;
    h.particules = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    h.qy = particle_zone_live(PARTICLE_SLOT_LIST);
    h.qy_fv = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    h.ga = particle_zone_live(PARTICLE_SLOT_BURST);
    h.rangees = pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
    return h;
}

/* A burst that leaves (`he`, then the exit queue's flush) gives back the
 * particles it still holds, its `qy`, then its own place. */
static void test_leaving_gives_back_its_particles(void) {
    held_t built;
    held_t after;
    planet_t *e;
    int count;

    start_game();
    pico8_cart_vider_file_sortie();
    e = as_ga(harvest(40, 40, "s74"));
    count = e->emitter->particles->count;
    assert(count == 140);
    built = held_now();
    entity_unregister(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == built.particules - count && after.qy == built.qy - 1);
    assert(after.ga == built.ga - 1);
}

/* The largest bursts: a protostar's, which copies its `ra` into its place,
 * and a harvest's, which borrows the planet's; each with its `oe`. Neither
 * stops on « zone place ga pleine ». */
static void test_the_largest_bursts_fit_their_place(void) {
    held_t before;

    start_game();
    pico8_cart_vider_file_sortie();
    before = held_now();
    (void)harvest(40, 40, "s84");
    (void)spawn(80, 70, "s83", NULL);
    assert(held_now().ga == before.ga + 2);
    printf("ga place: %lu bytes at most\n", (unsigned long)particle_zone_max_bytes(PARTICLE_SLOT_BURST));
}

/* A burst can be hovered, so a dragged slipway can end on it; the flush
 * that gives the burst back lets go of that end too. */
static void test_leaving_lets_go_of_a_dragged_slipway(void) {
    planet_keys_t keys;
    slipway_keys_t jc_keys;
    slipway_t *jc;
    vec_t mp = vec_xy(40, 40);
    void *e;

    start_game();
    pico8_cart_vider_file_sortie();
    keys.planet_id = "s85";
    keys.map_pos = vec_ref(vec_xy(30, 30));
    keys.pos = vec_ref(near_mo(60, 60));
    jc_keys.start = &planet_new(&keys)->header;
    jc = slipway_new(&jc_keys);
    e = harvest(40, 40, "s86");
    pico8_ok()->lheld = &jc->header;
    pico8_ok()->o_ = e;
    slipway_drag(jc, &mp);
    assert(jc->end == e);
    entity_unregister(e);
    pico8_cart_vider_file_sortie();
    assert(jc->end == NULL);
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
    in_fresh_child(test_burst);
    in_fresh_child(test_ei_moves_a_particle_in_place);
    in_fresh_child(test_ga_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);
    in_fresh_child(test_the_largest_bursts_fit_their_place);
    in_fresh_child(test_leaving_lets_go_of_a_dragged_slipway);

    printf("test_pico8_ga: all tests passed\n");
    return 0;
}
