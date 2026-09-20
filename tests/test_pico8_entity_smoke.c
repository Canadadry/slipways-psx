#include "../src/pico8_entity.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
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

static pico8_value_t fake_ev_a(void *self, pico8_value_t arg) {
    g_ev_a_calls++;
    g_ev_a_last_self = self;
    g_ev_a_last_arg = arg;
    if (g_call_order_count < 4) {
        g_call_order[g_call_order_count++] = 'A';
    }
    return pico8_value_fixed(pico8_fixed_from_int(901));
}

static pico8_value_t fake_ev_b(void *self, pico8_value_t arg) {
    g_ev_b_calls++;
    g_ev_b_last_self = self;
    g_ev_b_last_arg = arg;
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

    assert(pico8_mv(entity_a, "ev", pico8_value_fixed(pico8_fixed_from_int(55)), &out));
    assert(g_ev_a_calls == 2);
    assert(g_ev_a_last_self == entity_a);
    assert(g_ev_a_last_arg.as.fixed == pico8_fixed_from_int(55));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(901));

    /* "ra" is a plain fixed field, not a function -- mv() must return it
     * as-is rather than trying to call it. */
    assert(pico8_mv(entity_a, "ra", pico8_value_fixed(pico8_fixed_from_int(0)), &out));
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

int main(void) {
    test_entity_dispatch_smoke();

    printf("test_pico8_entity_smoke: all tests passed\n");
    return 0;
}
