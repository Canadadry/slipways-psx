/* The defaults of the ten classes of the `jo` tree, on a fresh struct of
 * each, against its kind's archetype; and each module's `_ready` over init
 * and shutdown. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

/* The very string of the generated data, not a copy. */
static void assert_same_string(const char *a, const char *b) {
    assert(a != NULL && a == b && strcmp(a, b) == 0);
}

/* The members of jo every class of the tree inherits, and its kind's
 * archetype; kw aside, which `lz` points at the planet's own when it has an
 * `hp`. */
static void assert_planet_tail(const planet_t *jo, const planet_archetype_t *archetype) {
    assert(jo->archetype == archetype);
    assert(jo->shake == archetype->shake);
    assert(jo->development_tier == archetype->development_tier && jo->has_development_tier == archetype->has_development_tier);
}

static void assert_box(const box_t *kq, int xl, int yt, int xr, int yb) {
    assert(kq->left == p8_fixed_from_int(xl) && kq->top == p8_fixed_from_int(yt) &&
           kq->right == p8_fixed_from_int(xr) && kq->bottom == p8_fixed_from_int(yb));
}

/* jo's box, which ga and nr replace. */
static void assert_planet_box(const box_t *kq) {
    assert_box(kq, -7, -7, 7, 7);
}

/* The members of qh every class below it inherits, its probe's archetype, and
 * the probe's init literals. */
static void assert_structure_tail(const planet_t *qh, const planet_archetype_t *archetype) {
    assert(qh->targeting->def == &archetype->targeting);
    assert(qh->development_tier == archetype->development_tier && qh->has_development_tier == 1);
    assert(qh->targeting->has_od == 1 && qh->targeting->def->status_sprite == 0 && qh->targeting->def->has_status_sprite == 0);
}

/* The emitter of an ascension, pm or mx, and its archetype. */
static void assert_emitter(const pico8_emitter_t *em, const planet_archetype_t *archetype) {
    assert(em->def == &archetype->emitter);
    assert(em->py == archetype->emitter.speed_factor && em->pn == archetype->emitter.angle_slices && em->k_ == archetype->emitter.trail_colors);
    assert(em->du == archetype->emitter.base_radius && em->qo == archetype->emitter.matrix_xx && em->ro == archetype->emitter.matrix_xy);
    assert(em->qg == archetype->emitter.matrix_yx && em->q_ == archetype->emitter.matrix_yy);
}

static void test_planet_defaults(void) {
    planet_t *jo = planet_new(NULL);

    assert(planet_ready());
    assert_same_string(jo->header.state, jo->archetype->state);
    assert_planet_box(&jo->archetype->hit_box);
    assert_planet_tail(jo, &planet_archetype);
    assert(jo->archetype->ring_colors == &planet_ring_colors_seq && jo->archetype->mood_rows == planet_mood_rows_rows && jo->archetype->made_resources == &planet_made_resources_seq);
    assert(jo->development_tier == 0 && !jo->has_development_tier);
    assert(jo->production == planet_shared_production());
    assert(jo->production->amount == 0 && !jo->production->has_resource);
    assert(jo->production_own.amount == 0 && !jo->production_own.has_resource && jo->production_own.resource == 0);
}

static void test_structure_defaults(void) {
    planet_t *qh = pico8_qh_new(NULL);

    assert(pico8_qh_ready());
    assert_same_string(qh->header.state, structure_archetype.state);
    assert(qh->targeting->od == structure_archetype.targeting.cost);
    assert(qh->sprite == structure_archetype.sprite);
    assert_structure_tail(qh, &structure_archetype);
    assert_planet_box(&qh->archetype->hit_box);
    assert_planet_tail(qh, &structure_archetype);
}

static void test_trading_hub_defaults(void) {
    planet_t *nl = pico8_nl_new(NULL);

    assert(pico8_nl_ready());
    assert(nl->sprite == trading_hub_archetype.sprite);
    assert(nl->targeting->od == trading_hub_archetype.targeting.cost);
    assert(nl->palette_block == trading_hub_archetype.palette_block);
    assert_same_string(nl->header.state, trading_hub_archetype.state);
    assert_structure_tail(nl, &trading_hub_archetype);
    assert_planet_box(&nl->archetype->hit_box);
    assert_planet_tail(nl, &trading_hub_archetype);
}

static void test_processor_defaults(void) {
    planet_t *os = pico8_os_new(NULL);

    assert(pico8_os_ready());
    assert(os->sprite == processor_archetype.sprite);
    assert(os->targeting->od == processor_archetype.targeting.cost);
    assert_same_string(os->header.state, processor_archetype.state);
    assert_structure_tail(os, &processor_archetype);
    assert_planet_box(&os->archetype->hit_box);
    assert_planet_tail(os, &processor_archetype);
}

static void test_ascension_defaults(void) {
    planet_t *hj = pico8_hj_new(NULL);

    assert(pico8_hj_ready());
    assert(hj->targeting->od == ascension_archetype.targeting.cost);
    assert(hj->sprite == ascension_archetype.sprite);
    assert_emitter(hj->emitter, &ascension_archetype);
    assert_same_string(hj->header.state, ascension_archetype.state);
    assert_structure_tail(hj, &ascension_archetype);
    assert_planet_box(&hj->archetype->hit_box);
    assert_planet_tail(hj, &ascension_archetype);
}

static void test_protostar_defaults(void) {
    planet_t *pm = pico8_pm_new(NULL);

    assert(pico8_pm_ready());
    assert_same_string(pm->header.state, protostar_archetype.state);
    assert(pm->sprite == protostar_archetype.sprite);
    assert_emitter(pm->emitter, &protostar_archetype);
    assert(pm->targeting->od == protostar_archetype.targeting.cost);
    assert_structure_tail(pm, &protostar_archetype);
    assert_planet_box(&pm->archetype->hit_box);
    assert_planet_tail(pm, &protostar_archetype);
}

static void test_burst_defaults(void) {
    planet_t *ga = pico8_ga_new(NULL);
    planet_t *other = pico8_ga_new(NULL);

    assert(pico8_ga_ready());
    assert(ga->outgoing_routes != NULL && ga->outgoing_routes == other->outgoing_routes);
    assert(ga->emitter->def == &burst_archetype.emitter);
    assert(ga->emitter->qo == burst_archetype.emitter.matrix_xx);
    assert(ga->emitter->ro == burst_archetype.emitter.matrix_xy);
    assert(ga->emitter->qg == burst_archetype.emitter.matrix_yx);
    assert(ga->emitter->q_ == burst_archetype.emitter.matrix_yy);
    assert(ga->emitter->k_ == burst_archetype.emitter.trail_colors);
    assert(ga->acceleration_factor == burst_archetype.acceleration_factor);
    assert(ga->emitter->py == burst_archetype.emitter.speed_factor);
    assert(ga->speed == burst_archetype.speed);
    assert_box(&ga->archetype->hit_box, 0, 0, -1, -1);
    assert_same_string(ga->header.state, ga->archetype->state);
    assert_planet_tail(ga, &burst_archetype);
    assert(ga->archetype->trade_multiplier == p8_fixed_from_int(1));
    assert(ga->development_tier == 0 && !ga->has_development_tier);
    assert(ga->production == planet_shared_production());
    assert(ga->production->amount == 0 && !ga->production->has_resource);
    assert(ga->production_own.amount == 0 && !ga->production_own.has_resource && ga->production_own.resource == 0);
}

/* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
static void assert_slipgate_inherited(const planet_t *nr, const planet_archetype_t *archetype) {
    assert(nr->targeting->od == archetype->targeting.cost);
    assert(nr->sprite == archetype->sprite);
    assert_box(&nr->archetype->hit_box, -4, -4, 5, 5);
    assert_same_string(nr->header.state, archetype->state);
    assert_structure_tail(nr, archetype);
    assert_planet_tail(nr, archetype);
}

static void test_slipgate_defaults(void) {
    planet_t *nr = slipgate_new(NULL);

    assert(slipgate_ready());
    assert_slipgate_inherited(nr, &slipgate_archetype);
}

static void test_infraspace_gate_defaults(void) {
    planet_t *fr = infraspace_gate_new(NULL);

    assert(infraspace_gate_ready());
    assert_slipgate_inherited(fr, &infraspace_gate_archetype);
}

static void test_synthesizer_defaults(void) {
    planet_t *mx = synthesizer_new(NULL);

    assert(synthesizer_ready());
    assert(mx->sprite == synthesizer_archetype.sprite);
    assert(mx->targeting->od == synthesizer_archetype.targeting.cost);
    assert(mx->synthesizable_resources == synthesizer_archetype.synthesizable_resources);
    assert_emitter(mx->emitter, &synthesizer_archetype);
    assert_same_string(mx->header.state, synthesizer_archetype.state);
    assert_structure_tail(mx, &synthesizer_archetype);
    assert_planet_box(&mx->archetype->hit_box);
    assert_planet_tail(mx, &synthesizer_archetype);
}

/* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
 * alone: a fresh struct of each module still holds `t` 0. */
static void test_second_reset_world_keeps_entity(void) {
    assert(entity_ready());
    reset_world();
    assert(entity_ready());
    assert(planet_new(NULL)->header.state_time == 0);
    assert(pico8_qh_new(NULL)->header.state_time == 0);
    assert(pico8_nl_new(NULL)->header.state_time == 0);
    assert(pico8_os_new(NULL)->header.state_time == 0);
    assert(pico8_hj_new(NULL)->header.state_time == 0);
    assert(pico8_pm_new(NULL)->header.state_time == 0);
    assert(pico8_ga_new(NULL)->header.state_time == 0);
    assert(slipgate_new(NULL)->header.state_time == 0);
    assert(infraspace_gate_new(NULL)->header.state_time == 0);
    assert(synthesizer_new(NULL)->header.state_time == 0);
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

    assert(!planet_ready());
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(!slipgate_ready());
    assert(!infraspace_gate_ready());
    assert(!synthesizer_ready());
    pico8_cart_load();
    reset_world();

    test_second_reset_world_keeps_entity();
    test_planet_defaults();
    test_structure_defaults();
    test_trading_hub_defaults();
    test_processor_defaults();
    test_ascension_defaults();
    test_protostar_defaults();
    test_burst_defaults();
    test_slipgate_defaults();
    test_infraspace_gate_defaults();
    test_synthesizer_defaults();

    planet_family_trade_shutdown();
    assert(!slipgate_ready());
    assert(!infraspace_gate_ready());
    assert(!synthesizer_ready());
    assert(pico8_qh_ready());
    pico8_planet_family_shutdown();
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(planet_ready());
    planet_shutdown();
    assert(!planet_ready());
    assert(entity_ready());
    entity_shutdown();
    assert(!entity_ready());

    printf("test_planet_spec_defaults: all tests passed\n");
    return 0;
}
