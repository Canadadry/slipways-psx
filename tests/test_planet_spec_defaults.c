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
static void assert_jo_tail(const pico8_jo_t *jo, const planet_archetype_t *def) {
    assert(jo->def == def);
    assert(jo->ma == def->shake);
    assert(jo->mi == def->development_tier && jo->has_mi == def->has_development_tier);
}

static void assert_box(const box_t *kq, int xl, int yt, int xr, int yb) {
    assert(kq->left == p8_fixed_from_int(xl) && kq->top == p8_fixed_from_int(yt) &&
           kq->right == p8_fixed_from_int(xr) && kq->bottom == p8_fixed_from_int(yb));
}

/* jo's box, which ga and nr replace. */
static void assert_jo_box(const box_t *kq) {
    assert_box(kq, -7, -7, 7, 7);
}

/* The members of qh every class below it inherits, its probe's archetype, and
 * the probe's init literals. */
static void assert_qh_tail(const pico8_jo_t *qh, const planet_archetype_t *def) {
    assert(qh->probe->def == &def->targeting);
    assert(qh->mi == def->development_tier && qh->has_mi == 1);
    assert(qh->probe->has_od == 1 && qh->probe->def->status_sprite == 0 && qh->probe->def->has_status_sprite == 0);
}

/* The emitter of an ascension, pm or mx, and its archetype. */
static void assert_emitter(const pico8_emitter_t *em, const planet_archetype_t *def) {
    assert(em->def == &def->emitter);
    assert(em->py == def->emitter.speed_factor && em->pn == def->emitter.angle_slices && em->k_ == def->emitter.trail_colors);
    assert(em->du == def->emitter.base_radius && em->qo == def->emitter.matrix_xx && em->ro == def->emitter.matrix_xy);
    assert(em->qg == def->emitter.matrix_yx && em->q_ == def->emitter.matrix_yy);
}

static void test_jo_defaults(void) {
    pico8_jo_t *jo = pico8_jo_new(NULL);

    assert(pico8_jo_ready());
    assert_same_string(jo->header.state, jo->def->state);
    assert_jo_box(&jo->def->hit_box);
    assert_jo_tail(jo, &planet_archetype);
    assert(jo->def->ring_colors == &planet_ring_colors_seq && jo->def->mood_rows == planet_mood_rows_rows && jo->def->made_resources == &planet_made_resources_seq);
    assert(jo->mi == 0 && !jo->has_mi);
    assert(jo->kw == pico8_jo_shared_kw());
    assert(jo->kw->amount == 0 && !jo->kw->has_resource);
    assert(jo->kw_own.amount == 0 && !jo->kw_own.has_resource && jo->kw_own.resource == 0);
}

static void test_qh_defaults(void) {
    pico8_jo_t *qh = pico8_qh_new(NULL);

    assert(pico8_qh_ready());
    assert_same_string(qh->header.state, structure_archetype.state);
    assert(qh->probe->od == structure_archetype.targeting.cost);
    assert(qh->s == structure_archetype.sprite);
    assert_qh_tail(qh, &structure_archetype);
    assert_jo_box(&qh->def->hit_box);
    assert_jo_tail(qh, &structure_archetype);
}

static void test_nl_defaults(void) {
    pico8_jo_t *nl = pico8_nl_new(NULL);

    assert(pico8_nl_ready());
    assert(nl->s == trading_hub_archetype.sprite);
    assert(nl->probe->od == trading_hub_archetype.targeting.cost);
    assert(nl->qw == trading_hub_archetype.palette_block);
    assert_same_string(nl->header.state, trading_hub_archetype.state);
    assert_qh_tail(nl, &trading_hub_archetype);
    assert_jo_box(&nl->def->hit_box);
    assert_jo_tail(nl, &trading_hub_archetype);
}

static void test_os_defaults(void) {
    pico8_jo_t *os = pico8_os_new(NULL);

    assert(pico8_os_ready());
    assert(os->s == processor_archetype.sprite);
    assert(os->probe->od == processor_archetype.targeting.cost);
    assert_same_string(os->header.state, processor_archetype.state);
    assert_qh_tail(os, &processor_archetype);
    assert_jo_box(&os->def->hit_box);
    assert_jo_tail(os, &processor_archetype);
}

static void test_hj_defaults(void) {
    pico8_jo_t *hj = pico8_hj_new(NULL);

    assert(pico8_hj_ready());
    assert(hj->probe->od == ascension_archetype.targeting.cost);
    assert(hj->s == ascension_archetype.sprite);
    assert_emitter(hj->emitter, &ascension_archetype);
    assert_same_string(hj->header.state, ascension_archetype.state);
    assert_qh_tail(hj, &ascension_archetype);
    assert_jo_box(&hj->def->hit_box);
    assert_jo_tail(hj, &ascension_archetype);
}

static void test_pm_defaults(void) {
    pico8_jo_t *pm = pico8_pm_new(NULL);

    assert(pico8_pm_ready());
    assert_same_string(pm->header.state, protostar_archetype.state);
    assert(pm->s == protostar_archetype.sprite);
    assert_emitter(pm->emitter, &protostar_archetype);
    assert(pm->probe->od == protostar_archetype.targeting.cost);
    assert_qh_tail(pm, &protostar_archetype);
    assert_jo_box(&pm->def->hit_box);
    assert_jo_tail(pm, &protostar_archetype);
}

static void test_ga_defaults(void) {
    pico8_jo_t *ga = pico8_ga_new(NULL);
    pico8_jo_t *other = pico8_ga_new(NULL);

    assert(pico8_ga_ready());
    assert(ga->by != NULL && ga->by == other->by);
    assert(ga->emitter->def == &burst_archetype.emitter);
    assert(ga->emitter->qo == burst_archetype.emitter.matrix_xx);
    assert(ga->emitter->ro == burst_archetype.emitter.matrix_xy);
    assert(ga->emitter->qg == burst_archetype.emitter.matrix_yx);
    assert(ga->emitter->q_ == burst_archetype.emitter.matrix_yy);
    assert(ga->emitter->k_ == burst_archetype.emitter.trail_colors);
    assert(ga->iu == burst_archetype.acceleration_factor);
    assert(ga->emitter->py == burst_archetype.emitter.speed_factor);
    assert(ga->ms == burst_archetype.speed);
    assert_box(&ga->def->hit_box, 0, 0, -1, -1);
    assert_same_string(ga->header.state, ga->def->state);
    assert_jo_tail(ga, &burst_archetype);
    assert(ga->def->trade_multiplier == p8_fixed_from_int(1));
    assert(ga->mi == 0 && !ga->has_mi);
    assert(ga->kw == pico8_jo_shared_kw());
    assert(ga->kw->amount == 0 && !ga->kw->has_resource);
    assert(ga->kw_own.amount == 0 && !ga->kw_own.has_resource && ga->kw_own.resource == 0);
}

/* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
static void assert_nr_inherited(const pico8_jo_t *nr, const planet_archetype_t *def) {
    assert(nr->probe->od == def->targeting.cost);
    assert(nr->s == def->sprite);
    assert_box(&nr->def->hit_box, -4, -4, 5, 5);
    assert_same_string(nr->header.state, def->state);
    assert_qh_tail(nr, def);
    assert_jo_tail(nr, def);
}

static void test_nr_defaults(void) {
    pico8_jo_t *nr = pico8_nr_new(NULL);

    assert(pico8_nr_ready());
    assert_nr_inherited(nr, &slipgate_archetype);
}

static void test_fr_defaults(void) {
    pico8_jo_t *fr = pico8_fr_new(NULL);

    assert(pico8_fr_ready());
    assert_nr_inherited(fr, &infraspace_gate_archetype);
}

static void test_mx_defaults(void) {
    pico8_jo_t *mx = pico8_mx_new(NULL);

    assert(pico8_mx_ready());
    assert(mx->s == synthesizer_archetype.sprite);
    assert(mx->probe->od == synthesizer_archetype.targeting.cost);
    assert(mx->ht == synthesizer_archetype.synthesizable_resources);
    assert_emitter(mx->emitter, &synthesizer_archetype);
    assert_same_string(mx->header.state, synthesizer_archetype.state);
    assert_qh_tail(mx, &synthesizer_archetype);
    assert_jo_box(&mx->def->hit_box);
    assert_jo_tail(mx, &synthesizer_archetype);
}

/* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
 * alone: a fresh struct of each module still holds `t` 0. */
static void test_second_jl_keeps_ke(void) {
    assert(entity_ready());
    reset_world();
    assert(entity_ready());
    assert(pico8_jo_new(NULL)->header.state_time == 0);
    assert(pico8_qh_new(NULL)->header.state_time == 0);
    assert(pico8_nl_new(NULL)->header.state_time == 0);
    assert(pico8_os_new(NULL)->header.state_time == 0);
    assert(pico8_hj_new(NULL)->header.state_time == 0);
    assert(pico8_pm_new(NULL)->header.state_time == 0);
    assert(pico8_ga_new(NULL)->header.state_time == 0);
    assert(pico8_nr_new(NULL)->header.state_time == 0);
    assert(pico8_fr_new(NULL)->header.state_time == 0);
    assert(pico8_mx_new(NULL)->header.state_time == 0);
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

    assert(!pico8_jo_ready());
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(!pico8_nr_ready());
    assert(!pico8_fr_ready());
    assert(!pico8_mx_ready());
    pico8_cart_load();
    reset_world();

    test_second_jl_keeps_ke();
    test_jo_defaults();
    test_qh_defaults();
    test_nl_defaults();
    test_os_defaults();
    test_hj_defaults();
    test_pm_defaults();
    test_ga_defaults();
    test_nr_defaults();
    test_fr_defaults();
    test_mx_defaults();

    pico8_planet_family_trade_shutdown();
    assert(!pico8_nr_ready());
    assert(!pico8_fr_ready());
    assert(!pico8_mx_ready());
    assert(pico8_qh_ready());
    pico8_planet_family_shutdown();
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(pico8_jo_ready());
    pico8_jo_shutdown();
    assert(!pico8_jo_ready());
    assert(entity_ready());
    entity_shutdown();
    assert(!entity_ready());

    printf("test_pico8_jo_spec_defaults: all tests passed\n");
    return 0;
}
