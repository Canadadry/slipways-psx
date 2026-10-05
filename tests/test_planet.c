/* The planet, read on its struct members and its generated defaults
 * (game/game_data.h), and driven through the real galaxy_populate,
 * update_entities and draw_layers. Its subclasses are read the same way.
 * Games are built directly: no events file. */

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
#include "engine/entity_id.h"
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
#include "p8/vram.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/map_tables.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "game/ui_zone.h"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define DAUGHTERS_HASH 0x668D1EBEu
#define NEW_PLANET_HASH 0x48C32418u
#define UPDATE_VANISHING_CALLS 62

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static const entity_message_t messages[] = {ENTITY_MESSAGE_DRAW, ENTITY_MESSAGE_DRAW_OVERLAY, ENTITY_MESSAGE_HIT_TEST, ENTITY_MESSAGE_NEW_YEAR, ENTITY_MESSAGE_REFRESH};
static const tag_t tags[] = {TAG_PLANET, TAG_NODE, TAG_TARGETING};

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
    return jo->header.is_dead ? 2u : 0u;
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

static int id_list_count(const entity_id_list_t *l) {
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

/* A cart_init on the title screen, without a frame: the previous help
 * screen gives back its help particle list slot, the only one, before the
 * new help screen takes it. */
static void leave_help_screen(void) {
    const entity_list_t *hv = live_entities();
    for (int i = 0; i < hv->count; i++) {
        if (hv->items[i]->class == help_screen_class()) {
            entity_unregister(hv->items[i]);
            cart_flush_exit_queue();
            return;
        }
    }
}

static void start_title(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    leave_help_screen();
    cart_init();
    cart_flush_exit_queue();
}

/* A game started by cart_start_game with no difficulty picked: a NULL
 * difficulty reads as difficulty 0. */
static void start_game(void) {
    start_title();
    cart_start_game(NULL);
    cart_flush_exit_queue();
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static const entity_class_t *class_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* The planet class's own members: hit_test, draw, left_press, refresh,
 * accept_drop, new_year, attach_slipway, offer_trade and the vanishing
 * state; no other. */
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

/* A bare planet as planet_new and planet_construct left it: the scalars,
 * which of its 31 members are set, its methods, which default tables it
 * shares with the class, and where entity_register registered it. */
static uint32_t fold_planet(uint32_t hash, const void *entity) {
    const planet_t *planet = as_planet(entity);
    size_t index;

    hash = fold_string_member(hash, entity_state_name(planet->header.state));
    hash = fold_fixed_member(hash, planet->header.state_time);
    hash = fold_string_member(hash, planet->planet_id);
    hash = fold_is_planet(hash, planet);
    hash = fold_fixed_member(hash, planet->archetype->trade_multiplier);
    hash = fold_fixed_member(hash, planet->archetype->layer);
    hash = fold_fixed_member(hash, planet->header.shake);
    hash = fold_fixed_member(hash, planet->archetype->back_color);
    hash = fold_vec(hash, planet->map_pos);
    hash = fold_vec(hash, planet->pos);
    hash = fold_vec(hash, &planet->archetype->row_step);
    /* The 1s of is_planet, trade_multiplier, layer, shake and back_color:
     * constant by the struct's shape, kept so that GALAXY_HASH and
     * NEW_PLANET_HASH still prove the typed reads. */
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
    hash = fnv(hash, (uint32_t)(research_project_get(planet->project) != NULL));
    hash = fnv(hash, (uint32_t)(planet->description != NULL));
    hash = fnv(hash, (uint32_t)(planet->needs != NULL));
    hash = fnv(hash, (uint32_t)(planet->planet_type != NULL));
    hash = fnv(hash, (uint32_t)(planet->header.fold_anchor != 0));
    hash = fnv(hash, (uint32_t)(planet->layout_row != NULL));
    hash = fnv(hash, (uint32_t)(entity_get(planet->targeted_by) != NULL));
    hash = fnv(hash, (uint32_t)planet->has_production_bonus);
    hash = fnv(hash, (uint32_t)(planet->header.is_dead != 0));
    hash = fnv(hash, (uint32_t)planet->has_net_income);
    hash = fnv(hash, (uint32_t)(planet->incoming_routes != NULL));
    hash = fnv(hash, (uint32_t)(planet->outgoing_routes != NULL));
    hash = fnv(hash, (uint32_t)planet->has_development_tier);
    hash = fnv(hash, (uint32_t)(planet->header.is_hovered != 0));
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

/* After cart_start_game: the 90 planets galaxy_populate built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *jo;
    int i;

    start_game();
    jo = entities_by_tag(TAG_PLANET);
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i]);
    }
    check_int("galaxy_planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    const planet_t *jo = as_planet(e);

    h = fold_string_member(h, entity_state_name(jo->header.state));
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_fixed_member(h, jo->header.shake);
    h = (jo->header.fold_anchor != 0) ? fold_bool_member(h, jo->header.fold_anchor) : fold_absent(h);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = jo->has_net_income ? fold_fixed_member(h, jo->net_income) : fold_absent(h);
    h = fold_string_member(h, jo->description);
    h = fnv(h, is_dead_state(jo));
    h = fold_production(h, jo);
    h = fnv(h, (uint32_t)id_list_count(jo->slipways));
    h = fnv(h, (uint32_t)list_size(jo->needs));
    h = fnv(h, (uint32_t)(jo->outgoing_routes != NULL ? trade_route_list_count(jo->outgoing_routes) : -1));
    h = fnv(h, (uint32_t)(jo->incoming_routes != NULL ? trade_route_list_count(jo->incoming_routes) : -1));
    h = fnv(h, (uint32_t)(entity_get(jo->targeted_by) != NULL));
    h = fnv(h, (uint32_t)(jo->planet_type != NULL));
    h = fnv(h, (uint32_t)(jo->layout_row != NULL));
    h = fnv(h, (uint32_t)(jo->header.is_hovered != 0));
    return h;
}

enum { STRUCTURE, TRADING_HUB, PROCESSOR, ASCENSION, PROTOSTAR, SYNTHESIZER, SLIPGATE, INFRASPACE_GATE, BURST, DAUGHTERS };

static int daughter_of(const void *e) {
    if (e != NULL &&
        ((const entity_t *)e)->class == structure_class()) {
        return STRUCTURE;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == trading_hub_class()) {
        return TRADING_HUB;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == processor_class()) {
        return PROCESSOR;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == ascension_class()) {
        return ASCENSION;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == protostar_class()) {
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
        ((const entity_t *)e)->class == burst_class()) {
        return BURST;
    }
    return -1;
}

/* What a subclass inherits from the planet: back_color, row_step, the
 * spacer row itself, and planet_is_colonized and the planet's hit_test
 * called on it. */
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

/* The shop's structure_icon_left_press on the icon of `kind`, as the
 * player buys it, then dragged to the middle of the screen. */
static void *bought(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message(ENTITY_MESSAGE_REFRESH);
    const camera_t *mo = main_camera_typed();
    const vec_t *main_camera_pos;
    int i;

    assert(mo != NULL && mo->pos != NULL);
    main_camera_pos = mo->pos;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (class_of(icon) == structure_icon_class() && ((const structure_icon_t *)icon)->kind == kind) {
            down_t made = class_of(icon)->left_press(icon, NULL, NULL);
            vec_t middle = vec_make(p8_fixed_from_int(80), p8_fixed_from_int(60));
            assert(made.stop && entity_get(made.held) != NULL);
            class_of(entity_get(made.held))->drag(entity_get(made.held), vec_ref(vec_add(*main_camera_pos, middle)), vec_ref(middle));
            return entity_get(made.held);
        }
    }
    assert(0 && "no icon for that kind");
    return NULL;
}

/* The research project whose class_name is the given one. */
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

/* A generated planet turned by planet_apply_project into the subclass the
 * class name names: the newest live entity. */
static void *turned(int planet, const char *cw) {
    void *p = entities_by_tag(TAG_PLANET)->items[planet];

    planet_roll_type(p);
    planet_apply_project(p, class_name_project(cw));
    return live_entities()->items[live_entities()->count - 1];
}

/* Each subclass by the road a game takes to it: the seven of the shop
 * bought from their icons, protostar and burst turned from planets by the
 * collapse and harvest projects. */
static void test_daughters(void) {
    static const struct {
        int daughter;
        const entity_kind_t *(*kind)(void);
    } shop[] = {{STRUCTURE, structure_kind}, {PROCESSOR, processor_kind}, {SLIPGATE, slipgate_kind}, {INFRASPACE_GATE, infraspace_gate_kind},
                {TRADING_HUB, trading_hub_kind}, {SYNTHESIZER, synthesizer_kind}, {ASCENSION, ascension_kind}};
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
    assert(entity_list_listed(live_entities(), a) && entity_list_listed(entities_by_tag(TAG_PLANET), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new_planet", h, NEW_PLANET_HASH);
}

/* entities_by_tag answers NULL for a tag that no entity has carried since
 * entity_registry_init(), known or not, and keeps a list once emptied. */
static void test_entities_by_tag_null_until_seen(void) {
    void *a;

    start_title();
    entity_registry_init(NULL);
    assert(entities_by_tag(TAG_PLANET) == NULL && entities_by_tag(TAG_NODE) == NULL);
    a = new_planet("p902", 5, 6);
    assert(entities_by_tag(TAG_PLANET) != NULL && entities_by_tag(TAG_NODE) != NULL && entities_by_tag(TAG_TARGETING) != NULL);
    assert(entities_by_tag(TAG_STRUCTURE) == NULL);
    entity_unregister(a);
    assert(entities_by_tag(TAG_PLANET) != NULL && entities_by_tag(TAG_PLANET)->count == 0);
    entity_registry_init(NULL);
    assert(entities_by_tag(TAG_PLANET) == NULL);
}

/* update_entities dispatches by state: a vanishing planet runs
 * planet_update_vanishing until its state time passes 60, then leaves the
 * live entities. */
static void test_update_vanishing(void) {
    planet_t *a;
    int calls = 0;

    start_title();
    a = new_planet("p901", 3, 4);
    update_entities();
    assert(is_dead_state(a) == 0);
    entity_set_state(&a->header, ENTITY_STATE_VANISHING);
    while (entity_list_listed(live_entities(), a) && calls < 200) {
        update_entities();
        calls++;
    }
    assert(is_dead_state(a) == 2);
    assert(!entity_list_listed(live_entities(), a) && !entity_list_listed(entities_by_tag(TAG_PLANET), a));
    check_int("update_vanishing_calls", calls, UPDATE_VANISHING_CALLS);
}

/* The production of a planet that never ran planet_install_project is the
 * class's shared one: an in-place write to it is seen by every such planet
 * and by the class, until the next planet_init() puts it back to zero. */
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
    assert(research_project_get(a->project) == hp && list_size(a->needs) >= 0);

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

/* planet_roll_type takes the "vanishing" planet type itself on a vanish
 * roll under 0.45, and its state for the planet's; any other draw is a
 * weighted picker entry, and the known state. */
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
        assert(p->header.state == (took_vanishing ? ENTITY_STATE_VANISHING : ENTITY_STATE_KNOWN));
    }
    assert(took_vanishing);
}

static int is_a_planet_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == planet_class();
}

/* Built by galaxy_populate or planet_new, a planet is a struct whose class
 * holds the planet's methods, and no other. */
static void test_planet_is_a_struct(void) {
    const entity_list_t *jo;
    void *a;
    int j;

    start_game();
    jo = entities_by_tag(TAG_PLANET);
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

    (void)bought(structure_kind());
    jo = entities_by_tag(TAG_STRUCTURE);
    assert(jo != NULL && jo->count == 1);
    assert(jo->items[0] != NULL &&
           ((const entity_t *)jo->items[0])->class == structure_class());
}

static int row_slots(void) {
    return ui_zone_live(SLOT_KIND_ROW);
}

/* A galaxy planet given a production of its own with a resource, as
 * planet_install_project and update_tiers_visit would: planet_refresh then
 * gives it a row. */
static planet_t *planet_with_a_row(void) {
    planet_t *a = (planet_t *)entities_by_tag(TAG_PLANET)->items[0];

    a->production = &a->production_own;
    a->production_own.has_resource = 1;
    a->production_own.resource = p8_fixed_from_int(28);
    a->production_own.amount = p8_fixed_from_int(2);
    return a;
}

/* planet_refresh rebuilds the planet's row in a slot of its own and gives
 * the replaced one back on the spot; once the planet leaves the live
 * entities, emptying the exit queue gives its row's slot back. */
static void test_refresh_row_slot(void) {
    planet_t *a;
    int once;
    int i;

    start_game();
    cart_flush_exit_queue();
    a = planet_with_a_row();
    class_of(a)->refresh(&a->header);
    once = row_slots();
    assert(a->layout_row != NULL && once > 0);
    for (i = 0; i < 5; i++) {
        class_of(a)->refresh(&a->header);
        assert(row_slots() == once && "the replaced row gives its place back on the spot");
    }
    entity_unregister(&a->header);
    assert(row_slots() == once && "out of the live entities, the planet is still read this frame");
    cart_flush_exit_queue();
    assert(row_slots() == once - 1 && a->layout_row == NULL);
}

/* A planet whose production has no resource gets no row from
 * research_display_planet_resource_row, and no slot. */
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

/* reset_world() hands every planet to the exit queue: a title after a game
 * whose planets all built a row holds as many row slots as before. */
static void test_rows_given_back_after_restart(void) {
    const entity_list_t *jo;
    int title;
    int i;

    start_title();
    cart_flush_exit_queue();
    title = row_slots();
    start_game();
    jo = entities_by_tag(TAG_PLANET);
    (void)planet_with_a_row();
    for (i = 0; i < jo->count; i++) {
        class_of(jo->items[i])->refresh(jo->items[i]);
    }
    assert(row_slots() > title);
    start_title();
    cart_flush_exit_queue();
    assert(row_slots() == title);
}

/* The largest row planet_refresh can build: luxury produced, so two
 * outputs, the most wants a project has, and an amount of 5. */
static planet_t *planet_with_the_largest_row(void) {
    planet_t *a = (planet_t *)entities_by_tag(TAG_PLANET)->items[0];
    const pointer_list_t *is = NULL;
    int r_w;
    int k;

    for (k = 1; research_projects_at(k) != NULL; k++) {
        const research_project_t *p = research_projects_at(k);
        if (p->inputs != NULL && (is == NULL || p->inputs->count > is->count)) is = p->inputs;
    }
    assert(is != NULL && resource_code_lookup("w", &r_w));
    a->needs = (pointer_list_t *)is;
    a->production = &a->production_own;
    a->production_own.has_resource = 1;
    a->production_own.resource = p8_fixed_from_int(r_w);
    a->production_own.amount = p8_fixed_from_int(5);
    return a;
}

#define REFRESHES_IN_ONE_FRAME 200

/* One tick may rerun economy_recompute, so refresh every planet, several
 * times: a refresh leaves the frame zone as it found it, the row it builds
 * stays the same. */
static void test_refresh_many_times_in_one_frame(void) {
    layout_row_item_t first[16];
    int first_count;
    p8_fixed_t first_width;
    planet_t *a;
    int i;

    start_game();
    cart_flush_exit_queue();
    a = planet_with_the_largest_row();
    planet_refresh(a);
    first_count = a->layout_row->placed_items_count;
    assert(first_count <= 16);
    first_width = a->layout_row->width;
    memcpy(first, a->layout_row->placed_items, (size_t)first_count * sizeof *first);
    for (i = 0; i < REFRESHES_IN_ONE_FRAME; i++) planet_refresh(a);
    assert(a->layout_row->placed_items_count == first_count && a->layout_row->width == first_width);
    for (i = 0; i < first_count; i++) {
        const layout_row_item_t *now = &a->layout_row->placed_items[i];
        assert(now->kind == first[i].kind && now->sprite_id == first[i].sprite_id &&
               now->sprite_palette_block == first[i].sprite_palette_block &&
               now->offset.x == first[i].offset.x && now->offset.y == first[i].offset.y);
        assert(now->kind != LAYOUT_ROW_ITEM_TEXT || strcmp(now->text, first[i].text) == 0);
    }
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after cart_load(), as a fresh process would start it. */
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
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
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
    in_fresh_child(test_refresh_many_times_in_one_frame);

    printf("test_planet: all tests passed\n");
    return 0;
}
