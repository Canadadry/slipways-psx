#include "../src/pico8_entity.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_value.h"

/* Chunk 11's own smoke test -- this phase's exit gate (PRD's
 * "Implementation Chunks" item 11 / "Testing Decisions"'s "The smoke
 * test" paragraph). ob()/kv:jr/hv/cv/cg/dc/mv/lt/he get no per-function
 * unit test of their own (deferred to source/port comparison per the
 * PRD); this is the one integration-shaped check exercising all of them
 * together through 2 fake entity classes. Never calls dc("kj"), so
 * pico8_mo()'s placeholder never needs a real "md" method. */

static int g_ev_a_calls = 0;
static void *g_ev_a_last_self = NULL;
static pico8_value_t g_ev_a_last_arg;

static int g_ev_b_calls = 0;
static void *g_ev_b_last_self = NULL;
static pico8_value_t g_ev_b_last_arg;

/* Records call order across a single pico8_dc("ev") so ascending-.fa-
 * layer dispatch order can be asserted precisely, not just "both ran". */
static char g_call_order[4];
static int g_call_order_count = 0;

/* Phase 3, Chunk 5 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md) extended pico8_value_function_t/pico8_mv() to carry an argc
 * plus two extra pico8_value_t slots instead of exactly one -- see
 * pico8_value.h's own comment on pico8_value_function_t. Every real
 * dispatch call site this existing smoke test exercises (dc()'s own
 * per-entity "ev" dispatch, and a direct pico8_mv() call) always passes
 * argc=1, so these fakes assert that backward-compatible shape explicitly
 * rather than silently ignoring argc. */
static pico8_value_t fake_ev_a(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)arg1;
    assert(argc == 1);
    g_ev_a_calls++;
    g_ev_a_last_self = self;
    g_ev_a_last_arg = arg0;
    if (g_call_order_count < 4) {
        g_call_order[g_call_order_count++] = 'A';
    }
    return pico8_value_fixed(pico8_fixed_from_int(901));
}

static pico8_value_t fake_ev_b(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)arg1;
    assert(argc == 1);
    g_ev_b_calls++;
    g_ev_b_last_self = self;
    g_ev_b_last_arg = arg0;
    if (g_call_order_count < 4) {
        g_call_order[g_call_order_count++] = 'B';
    }
    return pico8_value_fixed(pico8_fixed_from_int(902));
}

/* classA's own pq: records both that it ran and how many times, so the
 * Chunk 9 dedup rule (a class walk never re-fires the SAME inherited pq
 * twice) is exercised through real lt/he/dc/mv machinery, not re-derived. */
static int g_pq_a_calls = 0;

static void fake_pq_a(pico8_instance_t *instance) {
    g_pq_a_calls++;
    pico8_ob_table_set_name(instance->fields, "pq_a_ran", pico8_value_bool(1));
}

static int list_contains_instance(const pico8_list_t *list, const pico8_instance_t *inst) {
    int i;
    if (list == NULL) {
        return 0;
    }
    for (i = 0; i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == inst) {
            return 1;
        }
    }
    return 0;
}

/* A class's "nu" default is a nested ob()-parsed table of tag strings;
 * pico8_ob_table_free never owns/frees a PICO8_VALUE_REF/STRING value it
 * holds (pico8_ob.h), so it's extracted and freed here for hygiene --
 * not something this test asserts on. */
static void free_nested_nu_tag_table(const pico8_ob_table_t *class_defaults) {
    pico8_value_t nu_val;
    if (pico8_ob_table_get_name(class_defaults, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF) {
        pico8_ob_table_t *nu = nu_val.as.ref;
        int i;
        pico8_value_t v;
        for (i = 1; pico8_ob_table_get_index(nu, i, &v); i++) {
            if (v.kind == PICO8_VALUE_STRING) {
                free((void *)v.as.string);
            }
        }
        pico8_ob_table_free(nu);
    }
}

static void test_entity_dispatch_smoke(void) {
    pico8_class_t *class_a;
    pico8_class_t *class_b;
    pico8_ob_table_t *overrides_a;
    pico8_ob_table_t *overrides_b;
    pico8_instance_t *entity_a;
    pico8_instance_t *entity_b;
    pico8_value_t v;
    pico8_value_t out;

    pico8_entity_init();
    pico8_class_registry_clear();
    g_pq_a_calls = 0;
    g_ev_a_calls = 0;
    g_ev_b_calls = 0;
    g_call_order_count = 0;

    /* classA (root, own pq) sets bm field "ev" (placeholder; overridden
     * per-instance below with a real function -- ob() can't parse a
     * PICO8_VALUE_FUNCTION) and tag "alpha". classB (child, NO own pq --
     * purely inherits classA's) sets a DIFFERENT bm field ("hd") and tag
     * ("beta"), so lt()'s cv/cg placement is exercised distinguishably. */
    class_a = pico8_class_declare(NULL, "ev=0,nu=o(\"alpha\"),");
    pico8_class_set_pq(class_a, fake_pq_a);
    class_b = pico8_class_declare(class_a, "hd=0,nu=o(\"beta\"),");

    /* entity_a (layer 3, classA), entity_b (layer 7, classB) -- distinct
     * .fa layers so dc()'s ascending-order dispatch is exercised below. */
    overrides_a = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides_a, "fa", pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_ob_table_set_name(overrides_a, "ra", pico8_value_fixed(pico8_fixed_from_int(111)));
    pico8_ob_table_set_name(overrides_a, "ev", pico8_value_function(fake_ev_a));
    entity_a = pico8_class_instantiate(class_a, overrides_a);

    overrides_b = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides_b, "fa", pico8_value_fixed(pico8_fixed_from_int(7)));
    pico8_ob_table_set_name(overrides_b, "ra", pico8_value_fixed(pico8_fixed_from_int(222)));
    pico8_ob_table_set_name(overrides_b, "ev", pico8_value_function(fake_ev_b));
    entity_b = pico8_class_instantiate(class_b, overrides_b);

    /* entity_a's instantiate fires classA's own pq once; entity_b's walk
     * (classB -> classA) resolves the SAME inherited pq at both steps but
     * must fire it only once too (dedup) -- 2 total, not 3. */
    assert(g_pq_a_calls == 2);
    assert(pico8_ob_table_get_name(entity_a->fields, "pq_a_ran", &v) && v.as.boolean == 1);
    assert(pico8_ob_table_get_name(entity_b->fields, "pq_a_ran", &v) && v.as.boolean == 1);

    /* Registered B-before-A on purpose (opposite of ascending .fa order:
     * entity_a=3, entity_b=7) so a later dc("ev") dispatch order can only
     * match ascending .fa layers by actually bucketing on .fa, not by
     * coincidentally matching registration order. */
    pico8_lt(entity_b);
    pico8_lt(entity_a);

    assert(list_contains_instance(pico8_hv(), entity_a));
    assert(list_contains_instance(pico8_hv(), entity_b));

    /* entity_a: "ev" only (its own override); classA has no parent, so
     * "hd"/"kj"/"hk"/"hb" are absent everywhere. Tagged "alpha" only. */
    assert(list_contains_instance(pico8_cv("ev"), entity_a));
    assert(!list_contains_instance(pico8_cv("hd"), entity_a));
    assert(!list_contains_instance(pico8_cv("kj"), entity_a));
    assert(!list_contains_instance(pico8_cv("hk"), entity_a));
    assert(!list_contains_instance(pico8_cv("hb"), entity_a));
    assert(list_contains_instance(pico8_cg("alpha"), entity_a));
    assert(!list_contains_instance(pico8_cg("beta"), entity_a));

    /* entity_b: "ev" (own) AND "hd" (classB's own default); classB's own
     * "nu" ("beta") shadows classA's inherited "nu" ("alpha") -- the
     * chain walk stops at the first match, like a real __index chain. */
    assert(list_contains_instance(pico8_cv("ev"), entity_b));
    assert(list_contains_instance(pico8_cv("hd"), entity_b));
    assert(!list_contains_instance(pico8_cv("kj"), entity_b));
    assert(!list_contains_instance(pico8_cv("hk"), entity_b));
    assert(!list_contains_instance(pico8_cv("hb"), entity_b));
    assert(list_contains_instance(pico8_cg("beta"), entity_b));
    assert(!list_contains_instance(pico8_cg("alpha"), entity_b));

    pico8_he(entity_a);
    assert(!list_contains_instance(pico8_hv(), entity_a));
    assert(!list_contains_instance(pico8_cv("ev"), entity_a));
    assert(!list_contains_instance(pico8_cg("alpha"), entity_a));
    /* entity_b (still lt()'d) is untouched by entity_a's he(). */
    assert(list_contains_instance(pico8_hv(), entity_b));
    assert(list_contains_instance(pico8_cv("ev"), entity_b));
    assert(list_contains_instance(pico8_cv("hd"), entity_b));
    assert(list_contains_instance(pico8_cg("beta"), entity_b));

    pico8_he(entity_b);
    assert(!list_contains_instance(pico8_hv(), entity_b));
    assert(!list_contains_instance(pico8_cv("ev"), entity_b));
    assert(!list_contains_instance(pico8_cv("hd"), entity_b));
    assert(!list_contains_instance(pico8_cg("beta"), entity_b));

    /* Re-lt() both, again B-before-A (see the initial lt() pair above):
     * he() already proved removal; dc() dispatches only to entities
     * currently registered under cv[nt]. Registering B (fa=7) before A
     * (fa=3) here means the g_call_order == "AB" assertion below can
     * only pass if pico8_dc() truly dispatches in ascending .fa order,
     * not plain registration order. */
    pico8_lt(entity_b);
    pico8_lt(entity_a);
    g_ev_a_calls = 0;
    g_ev_b_calls = 0;
    g_call_order_count = 0;

    pico8_dc("ev");

    assert(g_ev_a_calls == 1);
    assert(g_ev_b_calls == 1);
    assert(g_ev_a_last_self == entity_a);
    assert(g_ev_a_last_arg.kind == PICO8_VALUE_FIXED);
    assert(g_ev_a_last_arg.as.fixed == pico8_fixed_from_int(111));
    assert(g_ev_b_last_self == entity_b);
    assert(g_ev_b_last_arg.kind == PICO8_VALUE_FIXED);
    assert(g_ev_b_last_arg.as.fixed == pico8_fixed_from_int(222));
    assert(g_call_order_count == 2);
    assert(g_call_order[0] == 'A');
    assert(g_call_order[1] == 'B');

    assert(pico8_mv(entity_a, "ev", 1, pico8_value_fixed(pico8_fixed_from_int(55)), pico8_value_ref(NULL), &out));
    assert(g_ev_a_calls == 2);
    assert(g_ev_a_last_self == entity_a);
    assert(g_ev_a_last_arg.as.fixed == pico8_fixed_from_int(55));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(901));

    /* "ra" is a plain fixed field, not a function -- mv() must return it
     * as-is rather than trying to call it. */
    assert(pico8_mv(entity_a, "ra", 1, pico8_value_fixed(pico8_fixed_from_int(0)), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED);
    assert(out.as.fixed == pico8_fixed_from_int(111));

    /* Deliberately never calls pico8_dc("kj") anywhere -- that branch
     * (mo:md()) stays compiled-but-unexercised this phase. */
    pico8_he(entity_a);
    pico8_he(entity_b);
    pico8_instance_free(entity_a);
    pico8_instance_free(entity_b);
    pico8_ob_table_free(overrides_a);
    pico8_ob_table_free(overrides_b);
    free_nested_nu_tag_table(class_b->defaults);
    free_nested_nu_tag_table(class_a->defaults);
    pico8_class_free(class_b);
    pico8_class_free(class_a);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* Phase 3, Chunk 3's own targeted extension to this smoke test (per this
 * PRD's own per-lot "entity/dispatch mechanism coverage" check): the
 * FIRST real (non-fake) class whose pq genuinely self-registers a fresh
 * instance into hv/cv/cg on instantiation, rather than a test-only fake pq
 * that merely sets a marker field and leaves pico8_lt() to be called
 * separately by the test itself (as test_entity_dispatch_smoke above
 * does). `ke.pq=lt` (pico8_ke.h/.c) is the real cart's own version of this
 * exact mechanism -- see pico8_ke.h's own top comment for the full trace.
 * This is a genuinely new structural exercise Phase 2's own smoke test
 * (built before any real class existed) had no way to cover. */
static void test_ke_pq_registers_via_lt(void) {
    pico8_instance_t *e;

    pico8_entity_init();
    pico8_class_registry_clear();
    pico8_ke_init();

    /* No explicit pico8_lt(e) call anywhere in this test -- ke.pq=lt means
     * instantiation ALONE must already register e into hv (and, since ke's
     * own spec sets no bm-listed field or "nu" tag, into no cv[]/cg[]
     * bucket at all). */
    e = pico8_class_instantiate(pico8_ke_class(), NULL);
    assert(list_contains_instance(pico8_hv(), e));
    assert(!list_contains_instance(pico8_cv("ev"), e));
    assert(!list_contains_instance(pico8_cv("hd"), e));
    assert(!list_contains_instance(pico8_cv("hk"), e));
    assert(!list_contains_instance(pico8_cv("hb"), e));
    assert(!list_contains_instance(pico8_cv("kj"), e));

    /* ke's own defaults ("me"="ph","t"=0) must be reachable via the
     * ordinary prototype-chain fallback, same as any other class. */
    {
        pico8_value_t v;
        assert(pico8_instance_get_field(e, "me", &v) && v.kind == PICO8_VALUE_STRING);
        assert(pico8_instance_get_field(e, "t", &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int(0));
    }

    pico8_he(e);
    assert(!list_contains_instance(pico8_hv(), e));
    pico8_instance_free(e);

    pico8_ke_shutdown();
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* Phase 3, Chunk 4's own targeted extension to this smoke test (per this
 * PRD's own per-lot "entity/dispatch mechanism coverage" check and its own
 * explicit flag: "the first real multi-level inheritance chain deeper than
 * Phase 2's smoke test used -- js/nf/fq/qb all extend ke, which itself
 * extends kv"). Reasoned through carefully rather than added reflexively:
 * Chunk 3's own test_ke_pq_registers_via_lt already exercises a REAL `ke`
 * with NO subclass (depth 1: kv -> ke). test_entity_dispatch_smoke (Phase
 * 2) already exercises a two-level chain (class_a -> class_b) with the
 * SAME structural shape nf/js embody (leaf sets its own distinct pq,
 * separate from an inherited ancestor's) -- but with an entirely FAKE
 * root (class_a's own fake_pq_a, requiring an explicit pico8_lt() call by
 * the test itself, never the real ke.pq=lt auto-registration mechanism).
 * Neither prior test combines a REAL `ke` parent (whose own pq IS lt,
 * genuinely self-registering) with a REAL child's own distinct pq -- the
 * one new ingredient nf/js's own real shape (`X=ke:jr(...)` plus their own
 * `function X:pq()...end`) introduces. This test closes exactly that gap,
 * with a small fake child class (not nf/js.c themselves, to avoid growing
 * this test binary's own Makefile dependency footprint for logic this
 * chunk's own source/port comment comparison already verifies) standing
 * in for nf/js's own structural shape. */
static int g_child_pq_calls = 0;

static void fake_child_pq(pico8_instance_t *instance) {
    g_child_pq_calls++;
    pico8_ob_table_set_name(instance->fields, "child_pq_ran", pico8_value_bool(1));
}

static void test_real_ke_subclass_with_own_distinct_pq(void) {
    pico8_class_t *child;
    pico8_instance_t *e;

    pico8_entity_init();
    pico8_class_registry_clear();
    pico8_ke_init();
    g_child_pq_calls = 0;

    child = pico8_class_declare(pico8_ke_class(), "");
    pico8_class_set_pq(child, fake_child_pq);

    e = pico8_class_instantiate(child, NULL);

    assert(g_child_pq_calls == 1);
    {
        pico8_value_t v;
        assert(pico8_ob_table_get_name(e->fields, "child_pq_ran", &v) && v.kind == PICO8_VALUE_BOOL &&
               v.as.boolean == 1);
    }
    assert(list_contains_instance(pico8_hv(), e));

    {
        pico8_value_t v;
        assert(pico8_instance_get_field(e, "me", &v) && v.kind == PICO8_VALUE_STRING);
        assert(pico8_instance_get_field(e, "t", &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int(0));
    }

    pico8_he(e);
    assert(!list_contains_instance(pico8_hv(), e));
    pico8_instance_free(e);
    pico8_class_free(child);

    pico8_ke_shutdown();
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* Phase 3, Chunk 5's own targeted extension to this smoke test (per this
 * PRD's own per-lot "entity/dispatch mechanism coverage" check, and its
 * own explicit flag: "this chunk is explicitly the FIRST real 0- and
 * 2-argument mv() call sites"). Neither prior test above ever calls
 * pico8_mv()/dispatches through pico8_value_function_t with argc other
 * than 1 -- this is the first structurally new exercise of the mechanism
 * Chunk 5's own arity extension (pico8_value.h) makes possible: a real
 * 2-argument dispatch (`ui:gw()`'s own `mv(e,"hd",self.qa,je.ra)` shape)
 * with both arguments threaded through DISTINCTLY (not one duplicated
 * into the other, not one silently dropped), and a real 0-argument
 * dispatch (`js:pq()`'s own `mv(self,"lq")` shape). A small fake class
 * (not `js`/`ui` themselves, to avoid growing this test binary's own
 * Makefile dependency footprint for logic this chunk's own source/port
 * comment comparison already verifies) stands in for both shapes. */
static int g_hd_calls = 0;
static void *g_hd_last_self = NULL;
static int g_hd_last_argc = -1;
static pico8_value_t g_hd_last_arg0;
static pico8_value_t g_hd_last_arg1;

static pico8_value_t fake_hd(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    g_hd_calls++;
    g_hd_last_self = self;
    g_hd_last_argc = argc;
    g_hd_last_arg0 = arg0;
    g_hd_last_arg1 = arg1;
    return pico8_value_bool(1);
}

static void test_mv_two_and_zero_argument_dispatch(void) {
    pico8_class_t *class_c;
    pico8_ob_table_t *overrides_c;
    pico8_instance_t *entity_c;
    pico8_value_t out;

    pico8_entity_init();
    pico8_class_registry_clear();
    g_hd_calls = 0;

    class_c = pico8_class_declare(NULL, "");
    overrides_c = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides_c, "hd", pico8_value_function(fake_hd));
    entity_c = pico8_class_instantiate(class_c, overrides_c);

    /* 2-argument call (ui:gw()'s own real shape): arg0 and arg1 must both
     * reach the dispatched function DISTINCTLY -- not one duplicated into
     * the other (the old, knowingly-incomplete js.hd=gk adapter's own
     * bug before this chunk's fix) and not one dropped. */
    assert(pico8_mv(entity_c, "hd", 2, pico8_value_fixed(pico8_fixed_from_int(10)),
                     pico8_value_fixed(pico8_fixed_from_int(20)), &out));
    assert(g_hd_calls == 1);
    assert(g_hd_last_self == entity_c);
    assert(g_hd_last_argc == 2);
    assert(g_hd_last_arg0.kind == PICO8_VALUE_FIXED && g_hd_last_arg0.as.fixed == pico8_fixed_from_int(10));
    assert(g_hd_last_arg1.kind == PICO8_VALUE_FIXED && g_hd_last_arg1.as.fixed == pico8_fixed_from_int(20));
    assert(g_hd_last_arg0.as.fixed != g_hd_last_arg1.as.fixed);
    assert(out.kind == PICO8_VALUE_BOOL && out.as.boolean == 1);

    /* 0-argument call (js:pq()'s own `mv(self,"lq")` shape): argc==0 must
     * reach the dispatched function too, not silently coerced to 1. */
    g_hd_calls = 0;
    g_hd_last_argc = -1;
    assert(pico8_mv(entity_c, "hd", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(g_hd_calls == 1);
    assert(g_hd_last_argc == 0);

    pico8_instance_free(entity_c);
    pico8_ob_table_free(overrides_c);
    pico8_class_free(class_c);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

int main(void) {
    test_entity_dispatch_smoke();
    test_ke_pq_registers_via_lt();
    test_real_ke_subclass_with_own_distinct_pq();
    test_mv_two_and_zero_argument_dispatch();

    printf("test_pico8_entity_smoke: all tests passed\n");
    return 0;
}
