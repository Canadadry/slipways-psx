/* The planet `jo`, read on its struct members and its generated defaults
 * (pico8_game_data.h), and driven through the real `bw`/`dg`/`dc`/`mv`. Its
 * daughters are read the same way. Games are built directly: no events
 * file. */

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
#include "game/economy.h"
#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/help.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "game/ui_zone.h"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define DAUGHTERS_HASH 0x668D1EBEu
#define NEW_PLANET_HASH 0x48C32418u
#define UPDATE_VANISHING_UPDATE_ENTITIES_CALLS 62

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const messages[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
static const char *const tags[] = {"planet", "node", "targeting"};

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

static const planet_t *as_planet(const void *e) {
    assert(planet_is_family(e));
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

static uint32_t fold_is_planet(uint32_t h, const planet_t *jo) {
    return jo->archetype->is_planet ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t is_dead_state(const planet_t *jo) {
    return jo->is_dead ? 2u : 0u;
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
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

static int entity_list_list_size(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static uint32_t fold_production(uint32_t h, const planet_t *jo) {
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

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
}

/* Un _init sur l'ecran titre, sans frame : l'aide d'avant rend sa place
 * qy_fv, unique, avant que la nouvelle la prenne. */
static void leave_help_screen(void) {
    const entity_list_t *hv = live_entities();
    for (int i = 0; i < hv->count; i++) {
        if (hv->items[i]->class == pico8_fv_struct_class()) {
            entity_unregister(hv->items[i]);
            pico8_cart_vider_file_sortie();
            return;
        }
    }
}

static void start_title(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    leave_help_screen();
    pico8__init();
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    start_title();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static const entity_class_t *class_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* jo's own members: `hd`, `kj`, `ldown`, `hb`, `gv`, `hk`, `kf`, `cn` and
 * the state `ll`; no other. */
static void assert_planet_members(const entity_class_t *c) {
    const entity_class_t *jo = planet_class();

    assert(c->hit_test == jo->hit_test && c->draw == jo->draw && c->left_press == jo->left_press && c->refresh == jo->refresh && c->accept_drop == jo->accept_drop &&
           c->new_year == jo->new_year && c->states[ENTITY_STATE_VANISHING] == jo->states[ENTITY_STATE_VANISHING]);
    assert(c->hit_test != NULL && c->draw != NULL && c->left_press != NULL && c->refresh != NULL && c->accept_drop != NULL && c->new_year != NULL &&
           c->states[ENTITY_STATE_VANISHING] != NULL);
    assert(c->attach_slipway == jo->attach_slipway && c->offer_trade == jo->offer_trade && c->attach_slipway != NULL && c->offer_trade != NULL);
    assert(c->drag == NULL && c->drop == NULL && c->draw_at == NULL && c->spawn_particle == NULL);
    assert(c->draw_overlay == NULL && c->has_refresh == NULL && c->right_press == NULL && c->states[ENTITY_STATE_NORMAL] == NULL &&
           c->states[ENTITY_STATE_SETTLED] == NULL && c->states[ENTITY_STATE_PLACING] == NULL &&
           c->states[ENTITY_STATE_UNFOLDING] == NULL);
}

/* A bare planet as `jo(...)` and `jo:pq` left it: the scalars, which of
 * its 31 members are set, its methods, which default tables it shares
 * with the class, and where `lt` registered it. */
static uint32_t fold_planet(uint32_t hash, const void *entity) {
    const planet_t *planet = as_planet(entity);
    size_t index;

    hash = fold_string_member(hash, planet->header.state);
    hash = fold_fixed_member(hash, planet->header.state_time);
    hash = fold_string_member(hash, planet->planet_id);
    hash = fold_is_planet(hash, planet);
    hash = fold_fixed_member(hash, planet->archetype->trade_multiplier);
    hash = fold_fixed_member(hash, planet->archetype->layer);
    hash = fold_fixed_member(hash, planet->shake);
    hash = fold_fixed_member(hash, planet->archetype->back_color);
    hash = fold_vec(hash, planet->map_pos);
    hash = fold_vec(hash, planet->pos);
    hash = fold_vec(hash, &planet->archetype->row_step);
    /* The 1s of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's
     * shape, kept so that GALAXY_HASH and NEW_PLANET_HASH still prove the
     * typed reads. */
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, (uint32_t)(planet->archetype->ring_colors != NULL));
    hash = fnv(hash, (uint32_t)(planet->archetype->mood_rows != NULL));
    hash = fnv(hash, (uint32_t)(planet->archetype->made_resources != NULL));
    hash = fnv(hash, (uint32_t)(planet->production != NULL));
    hash = fnv(hash, 1u);
    hash = fnv(hash, 1u);
    hash = fnv(hash, (uint32_t)(planet->archetype->spacer != NULL));
    hash = fnv(hash, (uint32_t)(planet->planet_id != NULL));
    hash = fnv(hash, (uint32_t)(planet->map_pos != NULL));
    hash = fnv(hash, (uint32_t)(planet->pos != NULL));
    hash = fnv(hash, (uint32_t)(planet->slipways != NULL));
    hash = fnv(hash, (uint32_t)(planet->project != NULL));
    hash = fnv(hash, (uint32_t)(planet->description != NULL));
    hash = fnv(hash, (uint32_t)(planet->needs != NULL));
    hash = fnv(hash, (uint32_t)(planet->planet_type != NULL));
    hash = fnv(hash, (uint32_t)(planet->fold_anchor != 0));
    hash = fnv(hash, (uint32_t)(planet->layout_row != NULL));
    hash = fnv(hash, (uint32_t)(planet->targeted_by != NULL));
    hash = fnv(hash, (uint32_t)planet->has_production_bonus);
    hash = fnv(hash, (uint32_t)(planet->is_dead != 0));
    hash = fnv(hash, (uint32_t)planet->has_net_income);
    hash = fnv(hash, (uint32_t)(planet->incoming_routes != NULL));
    hash = fnv(hash, (uint32_t)(planet->outgoing_routes != NULL));
    hash = fnv(hash, (uint32_t)planet->has_development_tier);
    hash = fnv(hash, (uint32_t)(planet->is_hovered != 0));
    assert_planet_members(class_of(entity));
    assert(planet->archetype == &planet_archetype);
    assert(planet->archetype == &planet_archetype);
    assert(planet->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && planet->production == planet_shared_production() && planet->archetype->spacer == &planet_spacer_row);
    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), entity));
    for (index = 0; index < sizeof messages / sizeof messages[0]; index++) {
        hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message(messages[index]), entity));
    }
    for (index = 0; index < sizeof tags / sizeof tags[0]; index++) {
        hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_tag(tags[index]), entity));
    }
    return hash;
}

/* After `eu`: the 90 planets `bw` built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *jo;
    int i;

    start_game();
    jo = entities_by_tag("planet");
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i]);
    }
    check_int("galaxy planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    const planet_t *jo = as_planet(e);

    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_fixed_member(h, jo->shake);
    h = (jo->fold_anchor != 0) ? fold_bool_member(h, jo->fold_anchor) : fold_absent(h);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = jo->has_net_income ? fold_fixed_member(h, jo->net_income) : fold_absent(h);
    h = fold_string_member(h, jo->description);
    h = fnv(h, is_dead_state(jo));
    h = fold_production(h, jo);
    h = fnv(h, (uint32_t)entity_list_list_size(jo->slipways));
    h = fnv(h, (uint32_t)list_size(jo->needs));
    h = fnv(h, (uint32_t)entity_list_list_size(jo->outgoing_routes));
    h = fnv(h, (uint32_t)entity_list_list_size(jo->incoming_routes));
    h = fnv(h, (uint32_t)(jo->targeted_by != NULL));
    h = fnv(h, (uint32_t)(jo->planet_type != NULL));
    h = fnv(h, (uint32_t)(jo->layout_row != NULL));
    h = fnv(h, (uint32_t)(jo->is_hovered != 0));
    return h;
}

enum { STRUCTURE, TRADING_HUB, PROCESSOR, ASCENSION, PROTOSTAR, SYNTHESIZER, SLIPGATE, INFRASPACE_GATE, BURST, DAUGHTERS };

static int daughter_of(const void *e) {
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_qh_struct_class()) {
        return STRUCTURE;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_nl_struct_class()) {
        return TRADING_HUB;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_os_struct_class()) {
        return PROCESSOR;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_hj_struct_class()) {
        return ASCENSION;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_pm_struct_class()) {
        return PROTOSTAR;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == synthesizer_class()) {
        return SYNTHESIZER;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == slipgate_class()) {
        return SLIPGATE;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == infraspace_gate_class()) {
        return INFRASPACE_GATE;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_ga_struct_class()) {
        return BURST;
    }
    return -1;
}

/* What a daughter inherits from `jo`: `ca`, `gq`, the `em` table itself,
 * and `jo`'s `gg` and `hd` called on it. */
static uint32_t fold_daughter(uint32_t h, void *d) {
    const planet_t *jo = as_planet(d);
    vec_t ra;

    h = fold_fixed_member(h, jo->archetype->back_color);
    h = fold_vec(h, &jo->archetype->row_step);
    assert(jo->archetype->spacer == &planet_spacer_row);
    h = fnv(fnv(h, 0xB0u), planet_is_colonized(jo) ? 1u : 0u);
    assert(jo->pos != NULL);
    ra = *jo->pos;
    h = fold_bool(h, class_of(d)->hit_test(d, vec_ref(ra), vec_ref(ra)));
    ra = vec_add(ra, vec_make(p8_fixed_from_int(8), p8_fixed_from_int(8)));
    return fold_bool(h, class_of(d)->hit_test(d, vec_ref(ra), vec_ref(ra)));
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message("refresh");
    const pico8_qb_t *mo = pico8_mo_qb();
    const vec_t *main_camera_pos;
    int i;

    assert(mo != NULL && mo->ra != NULL);
    main_camera_pos = mo->ra;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (class_of(icon) == structure_icon_class() && ((const structure_icon_t *)icon)->kind == kind) {
            down_t made = class_of(icon)->left_press(icon, NULL, NULL);
            vec_t middle = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
            assert(made.stop && made.held != NULL);
            class_of(made.held)->drag(made.held, vec_ref(vec_add(*main_camera_pos, middle)), vec_ref(middle));
            return made.held;
        }
    }
    assert(0 && "no icon for that kind");
    return NULL;
}

/* The project of `gb` whose `cw` names `cw`. */
static research_project_t *class_name_project(const char *cw) {
    int k;

    for (k = 1;; k++) {
        research_project_t *hp = research_projects_at(k);
        assert(hp != NULL);
        if (hp->class_name != NULL && strcmp(hp->class_name, cw) == 0) {
            return hp;
        }
    }
}

/* A generated planet turned by `jo:hu` into the daughter `cw` names: the
 * newest entity of `hv`. */
static void *turned(int planet, const char *cw) {
    void *p = entities_by_tag("planet")->items[planet];

    planet_roll_type(p);
    planet_apply_project(p, class_name_project(cw));
    return live_entities()->items[live_entities()->count - 1];
}

/* Each daughter by the road a game takes to it: the seven of the shop
 * bought from their icons, `pm` and `ga` turned from planets by the
 * collapse and harvest projects. */
static void test_daughters(void) {
    static const struct {
        int daughter;
        const entity_kind_t *(*kind)(void);
    } shop[] = {{STRUCTURE, pico8_qh_kind}, {PROCESSOR, pico8_os_kind}, {SLIPGATE, slipgate_kind}, {INFRASPACE_GATE, infraspace_gate_kind},
                {TRADING_HUB, pico8_nl_kind}, {SYNTHESIZER, synthesizer_kind}, {ASCENSION, pico8_hj_kind}};
    void *made[DAUGHTERS] = {NULL};
    uint32_t h = 2166136261u;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].daughter] = bought(shop[i].kind());
    }
    made[PROTOSTAR] = turned(0, "protostar");
    made[BURST] = turned(1, "burst");
    for (i = 0; i < DAUGHTERS; i++) {
        assert(made[i] != NULL && daughter_of(made[i]) == (int)i && entity_list_listed(live_entities(), made[i]));
        h = fold_daughter(fnv(h, (uint32_t)i), made[i]);
    }
    check("daughters", h, DAUGHTERS_HASH);
}

static void *new_planet(const char *id, int x, int y) {
    planet_keys_t keys;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y)));
    keys.pos = vec_ref(vec_make(p8_fixed_from_int(x * 128), p8_fixed_from_int(y * 128)));
    return planet_new(&keys);
}

static void test_new_planet(void) {
    uint32_t h = 2166136261u;
    void *a;

    start_title();
    a = new_planet("p900", 1, 2);
    assert(entity_list_listed(live_entities(), a) && entity_list_listed(entities_by_tag("planet"), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new planet", h, NEW_PLANET_HASH);
}

/* cg answers NULL for a tag that no entity has carried since
 * pico8_entity_init(), known or not, and keeps a list once emptied. */
static void test_entities_by_tag_null_until_seen(void) {
    void *a;

    start_title();
    entity_registry_init();
    assert(entities_by_tag("planet") == NULL && entities_by_tag("node") == NULL && entities_by_tag("background") == NULL);
    a = new_planet("p902", 5, 6);
    assert(entities_by_tag("planet") != NULL && entities_by_tag("node") != NULL && entities_by_tag("targeting") != NULL);
    assert(entities_by_tag("structure") == NULL && entities_by_tag("background") == NULL);
    entity_unregister(a);
    assert(entities_by_tag("planet") != NULL && entities_by_tag("planet")->count == 0);
    entity_registry_init();
    assert(entities_by_tag("planet") == NULL);
}

/* `dg` calls `qc[qc.me]`: a planet whose `me` is "ll" runs `jo:ll` until
 * its `t` passes 60, then leaves `hv`. */
static void test_update_vanishing(void) {
    planet_t *a;
    int calls = 0;

    start_title();
    a = new_planet("p901", 3, 4);
    update_entities();
    assert(is_dead_state(a) == 0);
    entity_set_state(&a->header, "vanishing");
    while (entity_list_listed(live_entities(), a) && calls < 200) {
        update_entities();
        calls++;
    }
    assert(is_dead_state(a) == 2);
    assert(!entity_list_listed(live_entities(), a) && !entity_list_listed(entities_by_tag("planet"), a));
    check_int("ll dg calls", calls, UPDATE_VANISHING_UPDATE_ENTITIES_CALLS);
}

/* `kw` of a planet that never ran `lz` is the class's shared one: an
 * in-place write to it is seen by every such planet and by the class, until
 * the next pico8_jo_init() puts it back to `{mu=0}`. */
static void test_production_shared(void) {
    planet_t *a;
    planet_t *b;
    production_t *kw;
    research_project_t *hp;
    resource_map_t production_before;
    resource_map_t production_after;
    p8_fixed_t before = p8_fixed_from_int(0);
    p8_fixed_t after = p8_fixed_from_int(0);

    start_title();
    a = new_planet("p902", 5, 6);
    b = new_planet("p903", 7, 8);
    kw = a->production;
    assert(kw == planet_shared_production() && b->production == kw);
    kw->amount = p8_fixed_from_int(7);
    assert(b->production->amount == p8_fixed_from_int(7));
    assert(planet_shared_production()->amount == p8_fixed_from_int(7));

    assert(research_projects_count() > 0);
    hp = research_projects_find("");
    assert(hp != NULL);
    planet_install_project(a, hp);
    assert(a->production != NULL && a->production != planet_shared_production());
    assert(a->production->amount == p8_fixed_from_int(0));
    assert(b->production == planet_shared_production());
    assert(a->project == hp && list_size(a->needs) >= 0);

    economy_total_production(&production_before);
    kw->has_resource = 1;
    kw->resource = p8_fixed_from_int(28);
    economy_total_production(&production_after);
    resource_map_get(&production_before, kw->resource, &before);
    resource_map_get(&production_after, kw->resource, &after);
    assert(after == before);
    planet_init();
    assert(b->production == planet_shared_production());
    assert(!kw->has_resource && kw->amount == p8_fixed_from_int(0));
}

/* `jo:hh` takes `df.ll` itself on a roll of `jo.ir()` under 0.45, and its
 * `me` for the planet's state; any other draw is a `jo.oc()` entry, and
 * `known`. */
static void test_roll_type_takes_vanishing(void) {
    char id[8];
    int took_vanishing = 0;
    int i;

    start_game();
    for (i = 0; i < 20 && !took_vanishing; i++) {
        planet_t *p;

        snprintf(id, sizeof id, "p%d", 950 + i);
        p = new_planet(id, i, i);
        planet_roll_type(p);
        assert(p->planet_type != NULL && p->description == p->planet_type->description);
        took_vanishing = p->planet_type == planet_types_lookup("vanishing");
        assert(strcmp(p->header.state, took_vanishing ? "vanishing" : "known") == 0);
    }
    assert(took_vanishing);
}

static int is_a_planet_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == planet_class();
}

/* Built by `bw` or pico8_jo_new, a `jo` is a struct whose descriptor holds
 * jo's 11 dispatchers, and its class installs none of them, nor a `pq`. */
static void test_planet_is_a_struct(void) {
    const entity_list_t *jo;
    void *a;
    int j;

    start_game();
    jo = entities_by_tag("planet");
    for (j = 0; j < jo->count; j++) {
        assert(is_a_planet_struct(jo->items[j]));
    }
    assert_planet_members(planet_class());
    assert(planet_ready());

    assert(planet_mood_rows() == planet_mood_rows_rows && planet_made_resources() == &planet_made_resources_seq &&
           planet_ring_colors() == &planet_ring_colors_seq && planet_spacer() == &planet_spacer_row);

    a = new_planet("p904", 9, 10);
    assert(is_a_planet_struct(a));
    {
        planet_t *bare = planet_new(NULL);
        assert(is_a_planet_struct(bare) && bare->planet_id == NULL && bare->pos == NULL && bare->slipways != NULL);
    }

    (void)bought(pico8_qh_kind());
    jo = entities_by_tag("structure");
    assert(jo != NULL && jo->count == 1);
    assert(jo->items[0] != NULL &&
           ((const entity_t *)jo->items[0])->class == pico8_qh_struct_class());
}

static int row_slots(void) {
    return pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
}

/* A galaxy planet given a `kw` of its own with a `qe`, as `lz` and `bb`
 * would: `jo:hb` then gives it a row. */
static planet_t *planet_with_a_row(void) {
    planet_t *a = (planet_t *)entities_by_tag("planet")->items[0];

    a->production = &a->production_own;
    a->production_own.has_resource = 1;
    a->production_own.resource = p8_fixed_from_int(28);
    a->production_own.amount = p8_fixed_from_int(2);
    return a;
}

/* `jo:hb` rebuilds the planet's row in a place of its own and gives the
 * replaced one back on the spot; once the planet leaves `hv`, emptying the
 * exit queue gives its row's place back. */
static void test_refresh_row_slot(void) {
    planet_t *a;
    int once;
    int i;

    start_game();
    pico8_cart_vider_file_sortie();
    a = planet_with_a_row();
    class_of(a)->refresh(&a->header);
    once = row_slots();
    assert(a->layout_row != NULL && once > 0);
    for (i = 0; i < 5; i++) {
        class_of(a)->refresh(&a->header);
        assert(row_slots() == once && "the replaced row gives its place back on the spot");
    }
    entity_unregister(&a->header);
    assert(row_slots() == once && "out of hv, the planet is still read this frame");
    pico8_cart_vider_file_sortie();
    assert(row_slots() == once - 1 && a->layout_row == NULL);
}

/* A planet whose `kw` has no `qe` gets no row from `jo.ex`, and no place. */
static void test_refresh_without_resource_takes_no_slot(void) {
    planet_t *a;
    int before;

    start_title();
    a = new_planet("p903", 7, 8);
    assert(a->production != NULL && !a->production->has_resource);
    before = row_slots();
    class_of(a)->refresh(&a->header);
    assert(a->layout_row == NULL && row_slots() == before);
}

/* `jl()` hands every planet to the exit queue: a title after a game whose
 * planets all built a row holds as many row places as before. */
static void test_rows_given_back_after_restart(void) {
    const entity_list_t *jo;
    int title;
    int i;

    start_title();
    pico8_cart_vider_file_sortie();
    title = row_slots();
    start_game();
    jo = entities_by_tag("planet");
    (void)planet_with_a_row();
    for (i = 0; i < jo->count; i++) {
        class_of(jo->items[i])->refresh(jo->items[i]);
    }
    assert(row_slots() > title);
    start_title();
    pico8_cart_vider_file_sortie();
    assert(row_slots() == title);
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
    in_fresh_child(test_galaxy);
    in_fresh_child(test_daughters);
    in_fresh_child(test_new_planet);
    in_fresh_child(test_entities_by_tag_null_until_seen);
    in_fresh_child(test_update_vanishing);
    in_fresh_child(test_production_shared);
    in_fresh_child(test_roll_type_takes_vanishing);
    in_fresh_child(test_planet_is_a_struct);
    in_fresh_child(test_refresh_row_slot);
    in_fresh_child(test_refresh_without_resource_takes_no_slot);
    in_fresh_child(test_rows_given_back_after_restart);

    printf("test_planet: all tests passed\n");
    return 0;
}
