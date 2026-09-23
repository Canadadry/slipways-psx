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

/* The one integration-shaped check over `ob`/`kv:jr`/`hv`/`cv`/`cg`/`dc`/
 * `mv`/`lt`/`he` together, through fake classes rather than real ones so the
 * test binary's dependency footprint stays small. No test here calls
 * `dc("kj")`: that branch dispatches "md" on `pico8_mo()`, which none of
 * these tests ever sets up. */

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

/* Every dispatch these two fakes are reached through -- `dc("ev")` and a
 * direct `mv()` call -- passes argc=1, hence the assert. */
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

/* Counts its calls so the dedup rule -- a class walk never re-fires the same
 * inherited pq twice -- can be asserted. */
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

/* `pico8_ob_table_free` never owns the REF/STRING values a table holds, so
 * a class's nested "nu" tag table has to be freed by hand. Hygiene only. */
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

    /* `ev=0` is a placeholder: `ob()` cannot parse a function, so the real
     * one is an instance override below. classB has no pq of its own, and
     * takes a different `bm` field and tag so `lt()`'s `cv`/`cg` placement
     * tells the two apart. */
    class_a = pico8_class_declare(NULL, "ev=0,nu=o(\"alpha\"),");
    pico8_class_set_pq(class_a, fake_pq_a);
    class_b = pico8_class_declare(class_a, "hd=0,nu=o(\"beta\"),");

    /* Distinct `.fa` layers, 3 and 7, so `dc()`'s ascending-layer order is
     * observable below. */
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

    /* 2, not 3: entity_b's walk (classB -> classA) resolves the same
     * inherited pq twice but must fire it once. */
    assert(g_pq_a_calls == 2);
    assert(pico8_ob_table_get_name(entity_a->fields, "pq_a_ran", &v) && v.as.boolean == 1);
    assert(pico8_ob_table_get_name(entity_b->fields, "pq_a_ran", &v) && v.as.boolean == 1);

    /* B before A on purpose, the opposite of ascending `.fa`: a dispatch
     * order that matches `.fa` can then only come from real layer
     * bucketing, not from registration order. */
    pico8_lt(entity_b);
    pico8_lt(entity_a);

    assert(list_contains_instance(pico8_hv(), entity_a));
    assert(list_contains_instance(pico8_hv(), entity_b));

    assert(list_contains_instance(pico8_cv("ev"), entity_a));
    assert(!list_contains_instance(pico8_cv("hd"), entity_a));
    assert(!list_contains_instance(pico8_cv("kj"), entity_a));
    assert(!list_contains_instance(pico8_cv("hk"), entity_a));
    assert(!list_contains_instance(pico8_cv("hb"), entity_a));
    assert(list_contains_instance(pico8_cg("alpha"), entity_a));
    assert(!list_contains_instance(pico8_cg("beta"), entity_a));

    /* entity_b is tagged "beta" and not "alpha": classB's own `nu` shadows
     * classA's, the chain walk stopping at the first match. */
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
    assert(list_contains_instance(pico8_hv(), entity_b));
    assert(list_contains_instance(pico8_cv("ev"), entity_b));
    assert(list_contains_instance(pico8_cv("hd"), entity_b));
    assert(list_contains_instance(pico8_cg("beta"), entity_b));

    pico8_he(entity_b);
    assert(!list_contains_instance(pico8_hv(), entity_b));
    assert(!list_contains_instance(pico8_cv("ev"), entity_b));
    assert(!list_contains_instance(pico8_cv("hd"), entity_b));
    assert(!list_contains_instance(pico8_cg("beta"), entity_b));

    /* Re-registered, again B before A, for the same reason as above: the
     * "AB" call order below can only come from `.fa` ordering. */
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

    /* "ra" is a plain fixed field: `mv()` returns it instead of calling it. */
    assert(pico8_mv(entity_a, "ra", 1, pico8_value_fixed(pico8_fixed_from_int(0)), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED);
    assert(out.as.fixed == pico8_fixed_from_int(111));

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

/* `ke.pq` is `lt` itself, so a real `ke` registers itself on
 * instantiation -- unlike the fake pq above, which only sets a marker and
 * leaves `lt()` to the test. */
static void test_ke_pq_registers_via_lt(void) {
    pico8_instance_t *e;

    pico8_entity_init();
    pico8_class_registry_clear();
    pico8_ke_init();

    /* No `lt()` call anywhere here: instantiation alone must register `e`
     * into `hv`, and -- `ke`'s spec carrying no `bm` field and no `nu` tag
     * -- into no `cv`/`cg` bucket at all. */
    e = pico8_class_instantiate(pico8_ke_class(), NULL);
    assert(list_contains_instance(pico8_hv(), e));
    assert(!list_contains_instance(pico8_cv("ev"), e));
    assert(!list_contains_instance(pico8_cv("hd"), e));
    assert(!list_contains_instance(pico8_cv("hk"), e));
    assert(!list_contains_instance(pico8_cv("hb"), e));
    assert(!list_contains_instance(pico8_cv("kj"), e));

    /* `ke`'s own defaults must be reachable through the ordinary chain
     * fallback, like any other class's. */
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

/* The shape `js`/`nf`/`fq`/`qb` all have: a real `ke` parent, whose pq is
 * the self-registering `lt`, under a child with a distinct pq of its own.
 * Both must fire. */
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

/* The two arities the rest of this file never reaches: `ui:gw()`'s
 * 2-argument `mv(e,"hd",self.qa,je.ra)` and `js:pq()`'s 0-argument
 * `mv(self,"lq")`. */
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

    /* The two arguments must arrive distinct: 10 and 20 differ, so a
     * duplicated or dropped slot cannot pass unnoticed. */
    assert(pico8_mv(entity_c, "hd", 2, pico8_value_fixed(pico8_fixed_from_int(10)),
                     pico8_value_fixed(pico8_fixed_from_int(20)), &out));
    assert(g_hd_calls == 1);
    assert(g_hd_last_self == entity_c);
    assert(g_hd_last_argc == 2);
    assert(g_hd_last_arg0.kind == PICO8_VALUE_FIXED && g_hd_last_arg0.as.fixed == pico8_fixed_from_int(10));
    assert(g_hd_last_arg1.kind == PICO8_VALUE_FIXED && g_hd_last_arg1.as.fixed == pico8_fixed_from_int(20));
    assert(g_hd_last_arg0.as.fixed != g_hd_last_arg1.as.fixed);
    assert(out.kind == PICO8_VALUE_BOOL && out.as.boolean == 1);

    /* argc==0 must reach the function as 0, not be coerced to 1. */
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

/* How this port carries a Lua closure that is called much later, as
 * `jo:fe()`'s "my" is: the captured values are stashed as extra fields on
 * the very entity the function field lives on, and the function reads them
 * back off its own `self`. */
static pico8_value_t fake_captured_closure(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *inst = (const pico8_instance_t *)self;
    pico8_value_t tag_val;
    (void)argc;
    (void)arg0;
    (void)arg1;
    assert(pico8_instance_get_field(inst, "_hidden_tag", &tag_val));
    assert(tag_val.kind == PICO8_VALUE_FIXED);
    return tag_val;
}

static void test_closure_context_survives_instantiate_field_copy(void) {
    pico8_class_t *row_class;
    pico8_ob_table_t *row; /* stands in for jo:fe's own per-item "row" table */
    pico8_instance_t *built;
    pico8_value_t out;

    pico8_entity_init();
    pico8_class_registry_clear();

    row_class = pico8_class_declare(NULL, "");

    /* A `jo:fe` row's shape: a real field, the "my" function, and an extra
     * field with no Lua equivalent holding the captured value. `row_class`
     * knows none of these names, as a real row's class does not either. */
    row = pico8_ob_table_new();
    pico8_ob_table_set_name(row, "some_real_field", pico8_value_bool(1));
    pico8_ob_table_set_name(row, "_hidden_tag", pico8_value_fixed(pico8_fixed_from_int(4242)));
    pico8_ob_table_set_name(row, "my", pico8_value_function(fake_captured_closure));

    /* `pz()`'s `js(b)` in miniature: the hidden field must ride along into
     * the fresh instance untouched. */
    built = pico8_class_instantiate(row_class, row);

    {
        pico8_value_t real_field;
        assert(pico8_instance_get_field(built, "some_real_field", &real_field));
        assert(real_field.kind == PICO8_VALUE_BOOL && real_field.as.boolean == 1);
    }

    /* The function receives `built`, not the original `row` table, as
     * `self`, and must still recover the captured value through it. */
    assert(pico8_mv(built, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(4242));

    pico8_instance_free(built);
    pico8_ob_table_free(row);
    pico8_class_free(row_class);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

int main(void) {
    test_entity_dispatch_smoke();
    test_ke_pq_registers_via_lt();
    test_real_ke_subclass_with_own_distinct_pq();
    test_mv_two_and_zero_argument_dispatch();
    test_closure_context_survives_instantiate_field_copy();

    printf("test_pico8_entity_smoke: all tests passed\n");
    return 0;
}
