#define _POSIX_C_SOURCE 200809L

#include "../src/pico8_entity.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
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

/* The shape `js`/`nf`/`qb` all have: a real `ke` parent, whose pq is
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

/* A class with a struct constructor hands instantiation over to it whole:
 * no instance of its own, no pq of the class chain. */
typedef struct {
    pico8_struct_header_t header;
} fake_struct_t;

static const pico8_struct_class_t g_fake_struct_class = {{NULL}, NULL, NULL};

static int g_fake_struct_new_calls = 0;
static const pico8_ob_table_t *g_fake_struct_new_last_overrides = NULL;
static fake_struct_t *g_fake_struct_new_last_result = NULL;

static void *fake_struct_new(const pico8_ob_table_t *overrides) {
    fake_struct_t *s = malloc(sizeof *s);

    g_fake_struct_new_calls++;
    g_fake_struct_new_last_overrides = overrides;
    s->header.tag = PICO8_TYPE_TAG_STRUCT;
    s->header.cls = &g_fake_struct_class;
    g_fake_struct_new_last_result = s;
    return s;
}

static void test_struct_constructor_takes_over_instantiation(void) {
    pico8_class_t *cls;
    pico8_class_t *plain;
    pico8_ob_table_t *overrides;
    pico8_instance_t *built;
    pico8_instance_t *inst;

    pico8_entity_init();
    pico8_class_registry_clear();
    g_pq_a_calls = 0;

    cls = pico8_class_declare(NULL, "");
    pico8_class_set_pq(cls, fake_pq_a);
    pico8_class_set_struct_new(cls, fake_struct_new);

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_index(overrides, 1, pico8_value_bool(1));
    built = pico8_class_instantiate(cls, overrides);

    assert(g_fake_struct_new_calls == 1);
    assert(g_fake_struct_new_last_overrides == overrides);
    assert((void *)built == (void *)g_fake_struct_new_last_result);
    assert(pico8_type_tag_of(built) == PICO8_TYPE_TAG_STRUCT);
    assert(g_pq_a_calls == 0);

    /* Without a struct constructor, nothing changes. */
    plain = pico8_class_declare(NULL, "");
    pico8_class_set_pq(plain, fake_pq_a);
    inst = pico8_class_instantiate(plain, NULL);
    assert(pico8_type_tag_of(inst) == PICO8_TYPE_TAG_INSTANCE);
    assert(g_pq_a_calls == 1);
    assert(g_fake_struct_new_calls == 1);

    /* A class itself stays untagged: `ie.ke` asserts exactly that. */
    assert(pico8_type_tag_of(cls) == PICO8_TYPE_TAG_UNKNOWN);

    free(g_fake_struct_new_last_result);
    pico8_instance_free(inst);
    pico8_ob_table_free(overrides);
    pico8_class_free(plain);
    pico8_class_free(cls);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* A ke-descended entity struct built the way a converted class builds one:
 * ke's part first, then its own members, then its pq, then ke's. `ev` and
 * `hd` are plain members holding a function and a number, as a class
 * default would; `ph` is a real entry of its method table. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t ra;
    pico8_value_t ev;
    pico8_value_t hd;
    pico8_value_t nu;
    pico8_value_t p_;
} fake_ke_child_t;

static int g_fake_ph_calls = 0;
static void *g_fake_ph_last_self = NULL;
static pico8_value_t g_fake_ph_last_arg;

static pico8_value_t fake_child_ph(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    g_fake_ph_calls++;
    g_fake_ph_last_self = self;
    g_fake_ph_last_arg = arg0;
    return pico8_value_fixed(pico8_fixed_from_int(77));
}

static int fake_child_field(const void *self, const char *name, pico8_value_t *out_value) {
    const fake_ke_child_t *c = self;
    const pico8_value_t *member = NULL;

    if (strcmp(name, "fa") == 0) {
        member = &c->fa;
    } else if (strcmp(name, "ra") == 0) {
        member = &c->ra;
    } else if (strcmp(name, "ev") == 0) {
        member = &c->ev;
    } else if (strcmp(name, "hd") == 0) {
        member = &c->hd;
    } else if (strcmp(name, "nu") == 0) {
        member = &c->nu;
    } else if (strcmp(name, "p_") == 0) {
        member = &c->p_;
    }
    if (member == NULL) {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static int fake_child_set_field(void *self, const char *name, pico8_value_t value) {
    fake_ke_child_t *c = self;

    if (strcmp(name, "p_") != 0) {
        return 0;
    }
    c->p_ = value;
    return 1;
}

static const pico8_struct_class_t g_fake_child_class = {
    {[PICO8_METHOD_PH] = fake_child_ph},
    fake_child_field,
    fake_child_set_field,
};

static fake_ke_child_t *fake_child_new(int fa, int ra, pico8_value_function_t ev, pico8_ob_table_t *nu) {
    fake_ke_child_t *c = malloc(sizeof *c);

    pico8_ke_struct_init(&c->ke, &g_fake_child_class);
    c->fa = pico8_value_fixed(pico8_fixed_from_int(fa));
    c->ra = pico8_value_fixed(pico8_fixed_from_int(ra));
    c->ev = ev != NULL ? pico8_value_function(ev) : pico8_value_ref(NULL);
    c->hd = pico8_value_fixed(pico8_fixed_from_int(1));
    c->nu = pico8_value_ref(nu);
    c->p_ = pico8_value_ref(NULL);
    pico8_ke_struct_pq(&c->ke);
    return c;
}

/* ke's struct pq is lt: the struct lands in hv, in cv exactly under the bm
 * names it carries (a method entry or a non-nil member), and in cg under
 * its `nu`; he takes it back out of all of them. */
static void test_ke_struct_registers_like_an_instance(void) {
    pico8_ob_table_t *nu = pico8_ob_table_new();
    fake_ke_child_t *c;
    pico8_value_t v;

    pico8_entity_init();
    pico8_class_registry_clear();
    pico8_ob_table_set_index(nu, 1, pico8_value_string("gamma"));

    c = fake_child_new(4, 5, fake_ev_a, nu);
    assert(pico8_type_tag_of(c) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_entity_get_field(c, "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ph") == 0);
    assert(pico8_entity_get_field(c, "t", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
    assert(!pico8_entity_get_field(c, "p_", &v));
    assert(!pico8_entity_get_field(c, "kj", &v));

    assert(list_contains_instance(pico8_hv(), (pico8_instance_t *)c));
    assert(list_contains_instance(pico8_cv("ev"), (pico8_instance_t *)c));
    assert(list_contains_instance(pico8_cv("hd"), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cv("kj"), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cv("hk"), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cv("hb"), (pico8_instance_t *)c));
    assert(list_contains_instance(pico8_cg("gamma"), (pico8_instance_t *)c));

    pico8_he((pico8_instance_t *)c);
    assert(!list_contains_instance(pico8_hv(), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cv("ev"), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cv("hd"), (pico8_instance_t *)c));
    assert(!list_contains_instance(pico8_cg("gamma"), (pico8_instance_t *)c));

    free(c);
    pico8_ob_table_free(nu);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* Instances and structs registered alternately keep their order in hv and
 * cv; dc("ev") then walks them by ascending `fa`, passing each its `ra`. */
static void test_instances_and_structs_mix_in_order(void) {
    pico8_class_t *cls;
    pico8_ob_table_t *overrides;
    pico8_instance_t *a;
    pico8_instance_t *b;
    fake_ke_child_t *s;
    const pico8_list_t *hv;
    const pico8_list_t *ev;

    pico8_entity_init();
    pico8_class_registry_clear();
    cls = pico8_class_declare(NULL, "");

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "fa", pico8_value_fixed(pico8_fixed_from_int(7)));
    pico8_ob_table_set_name(overrides, "ra", pico8_value_fixed(pico8_fixed_from_int(111)));
    pico8_ob_table_set_name(overrides, "ev", pico8_value_function(fake_ev_a));
    a = pico8_lt(pico8_class_instantiate(cls, overrides));
    s = fake_child_new(3, 333, fake_ev_b, NULL);
    b = pico8_lt(pico8_class_instantiate(cls, overrides));

    hv = pico8_hv();
    assert(hv->count == 3 && hv->items[0].as.ref == a && hv->items[1].as.ref == s && hv->items[2].as.ref == b);
    ev = pico8_cv("ev");
    assert(ev->count == 3 && ev->items[0].as.ref == a && ev->items[1].as.ref == s && ev->items[2].as.ref == b);

    pico8_he(a);
    g_ev_a_calls = 0;
    g_ev_b_calls = 0;
    g_call_order_count = 0;
    pico8_dc("ev");
    assert(g_call_order_count == 2 && g_call_order[0] == 'B' && g_call_order[1] == 'A');
    assert(g_ev_b_last_self == s);
    assert(g_ev_b_last_arg.kind == PICO8_VALUE_FIXED && g_ev_b_last_arg.as.fixed == pico8_fixed_from_int(333));
    assert(g_ev_a_last_self == b);

    free(s);
    pico8_instance_free(a);
    pico8_instance_free(b);
    pico8_ob_table_free(overrides);
    pico8_class_free(cls);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* mv on a struct: a method of its table is called with the struct, a
 * non-function member comes back as is, a name that is neither is 0 and
 * leaves the output alone. */
static void test_mv_on_a_struct(void) {
    fake_ke_child_t *c;
    pico8_value_t out;

    pico8_entity_init();
    pico8_class_registry_clear();
    c = fake_child_new(4, 5, NULL, NULL);

    g_fake_ph_calls = 0;
    assert(pico8_mv((pico8_instance_t *)c, "ph", 1, pico8_value_fixed(pico8_fixed_from_int(9)),
                    pico8_value_ref(NULL), &out));
    assert(g_fake_ph_calls == 1 && g_fake_ph_last_self == c);
    assert(g_fake_ph_last_arg.kind == PICO8_VALUE_FIXED && g_fake_ph_last_arg.as.fixed == pico8_fixed_from_int(9));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(77));

    assert(pico8_mv((pico8_instance_t *)c, "hd", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(1));

    out = pico8_value_fixed(pico8_fixed_from_int(-5));
    assert(!pico8_mv((pico8_instance_t *)c, "zz", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv((pico8_instance_t *)c, "kj", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv((pico8_instance_t *)c, "ev", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(-5));

    free(c);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

/* Runs `write` in a child process and reports whether it died of an
 * assert. */
static int dies_of_an_assert(void (*write)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    fflush(stderr);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (freopen("/dev/null", "w", stderr) == NULL) {
            _exit(2);
        }
        write();
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}

static void write_an_unknown_name(void) {
    fake_ke_child_t *c = fake_child_new(4, 5, NULL, NULL);
    pico8_entity_set_field(c, "zz", pico8_value_bool(1));
}

static void write_through_no_writer(void) {
    fake_struct_t s;
    s.header.tag = PICO8_TYPE_TAG_STRUCT;
    s.header.cls = &g_fake_struct_class;
    pico8_entity_set_field(&s, "p_", pico8_value_bool(1));
}

/* `e[name]=value` on either form: into an instance's own fields, through a
 * struct's writer into its member, which its reader then reads back. A
 * struct never grows a field. */
static void test_set_field_on_both_forms(void) {
    pico8_class_t *cls;
    pico8_instance_t *inst;
    fake_ke_child_t *c;
    pico8_value_t v;

    pico8_entity_init();
    pico8_class_registry_clear();
    cls = pico8_class_declare(NULL, "");
    inst = pico8_class_instantiate(cls, NULL);
    pico8_entity_set_field(inst, "m_", pico8_value_bool(1));
    assert(pico8_ob_table_get_name(inst->fields, "m_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean);
    pico8_entity_set_field(inst, "m_", pico8_value_bool(0));
    assert(pico8_entity_get_field(inst, "m_", &v) && v.kind == PICO8_VALUE_BOOL && !v.as.boolean);

    c = fake_child_new(4, 5, NULL, NULL);
    assert(!pico8_entity_get_field(c, "p_", &v));
    pico8_entity_set_field(c, "p_", pico8_value_fixed(pico8_fixed_from_int(3)));
    assert(c->p_.kind == PICO8_VALUE_FIXED && c->p_.as.fixed == pico8_fixed_from_int(3));
    assert(pico8_entity_get_field(c, "p_", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(3));
    pico8_entity_set_field(c, "p_", pico8_value_ref(NULL));
    assert(!pico8_entity_get_field(c, "p_", &v));

    assert(dies_of_an_assert(write_an_unknown_name));
    assert(dies_of_an_assert(write_through_no_writer));

    free(c);
    pico8_instance_free(inst);
    pico8_class_free(cls);
    pico8_entity_shutdown();
    pico8_class_registry_clear();
}

int main(void) {
    test_entity_dispatch_smoke();
    test_ke_pq_registers_via_lt();
    test_real_ke_subclass_with_own_distinct_pq();
    test_mv_two_and_zero_argument_dispatch();
    test_closure_context_survives_instantiate_field_copy();
    test_struct_constructor_takes_over_instantiation();
    test_ke_struct_registers_like_an_instance();
    test_instances_and_structs_mix_in_order();
    test_mv_on_a_struct();
    test_set_field_on_both_forms();

    printf("test_pico8_entity_smoke: all tests passed\n");
    return 0;
}
