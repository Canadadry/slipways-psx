/* The spec defaults of `ke` and of the ten modules of the `jo` tree, read
 * only through the shared readers spec_field()/is_ready() and a fresh
 * struct of each. test_spec_defaults(), test_shared_with_parent() and
 * test_struct_defaults() hold values frozen on the code where these were
 * still classes and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_value.h"

#define READY_HASH 0x5D360CB4u
#define SPEC_HASH 0xB291EE16u
#define SHARE_HASH 0x963AC40Fu
#define STRUCT_HASH 0x4FFBBB8Eu

enum { M_KE, M_JO, M_QH, M_NL, M_OS, M_HJ, M_PM, M_GA, M_NR, M_FR, M_MX, MODULE_COUNT };

/* The module each one resolves a name on next, -1 for ke. */
static const int PARENT[MODULE_COUNT] = {-1, M_KE, M_JO, M_QH, M_QH, M_QH, M_HJ, M_JO, M_QH, M_NR, M_HJ};

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int spec_field(int i, const char *name, pico8_value_t *out) {
    switch (i) {
    case M_KE:
        return pico8_ke_start_field(name, out);
    case M_JO:
        return pico8_jo_default(name, out);
    case M_QH:
        return pico8_qh_default(name, out);
    case M_NL:
        return pico8_nl_default(name, out);
    case M_OS:
        return pico8_os_default(name, out);
    case M_HJ:
        return pico8_hj_default(name, out);
    case M_PM:
        return pico8_pm_default(name, out);
    case M_GA:
        return pico8_ga_default(name, out);
    case M_NR:
        return pico8_nr_default(name, out);
    case M_FR:
        return pico8_fr_default(name, out);
    default:
        return pico8_mx_default(name, out);
    }
}

static int is_ready(int i) {
    switch (i) {
    case M_KE:
        return pico8_ke_ready();
    case M_JO:
        return pico8_jo_ready();
    case M_QH:
        return pico8_qh_ready();
    case M_NL:
        return pico8_nl_ready();
    case M_OS:
        return pico8_os_ready();
    case M_HJ:
        return pico8_hj_ready();
    case M_PM:
        return pico8_pm_ready();
    case M_GA:
        return pico8_ga_ready();
    case M_NR:
        return pico8_nr_ready();
    case M_FR:
        return pico8_fr_ready();
    default:
        return pico8_mx_ready();
    }
}

static void *build(int i) {
    switch (i) {
    case M_KE:
        return NULL;
    case M_JO:
        return pico8_jo_new(NULL);
    case M_QH:
        return pico8_qh_new(NULL);
    case M_NL:
        return pico8_nl_new(NULL);
    case M_OS:
        return pico8_os_new(NULL);
    case M_HJ:
        return pico8_hj_new(NULL);
    case M_PM:
        return pico8_pm_new(NULL);
    case M_GA:
        return pico8_ga_new(NULL);
    case M_NR:
        return pico8_nr_new(NULL);
    case M_FR:
        return pico8_fr_new(NULL);
    default:
        return pico8_mx_new(NULL);
    }
}

static void shutdown_of(int i) {
    switch (i) {
    case M_KE:
        pico8_ke_shutdown();
        break;
    case M_JO:
        pico8_jo_shutdown();
        break;
    case M_NR:
    case M_FR:
    case M_MX:
        pico8_planet_family_trade_shutdown();
        break;
    default:
        pico8_planet_family_shutdown();
        break;
    }
}

/* Every key of the 11 specs, then names absent from all. */
static const char *const NAMES[] = {
    "nu", "me", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq", "ca", "em", "hp", "px", "dq",
    "qh", "od", "fp", "dw", "mi", "s",  "ba", "rr", "nl", "qw", "py", "ki", "pn", "pk", "fc", "k_", "du",
    "qo", "ro", "qg", "q_", "fz", "ng", "by", "iu", "ms", "hz", "mt", "ht", "t",  "zz", "kf", "kj", "ly",
    "mm", "hq", "id", "p",  "ra",
};

#define NAME_COUNT ((int)(sizeof NAMES / sizeof NAMES[0]))

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFFFFFFFu);
}

/* A REF folds as its tag, and its entry count when it is a table or a
 * list: never its address. */
static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fold_string(h, v.as.string);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0u);
        }
        h = fnv(h, (uint32_t)pico8_type_tag_of(v.as.ref) + 1u);
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            h = fnv(h, (uint32_t)((const pico8_ob_table_t *)v.as.ref)->count);
        } else if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
            h = fnv(h, (uint32_t)((const pico8_list_t *)v.as.ref)->count);
        }
        return h;
    default:
        return h;
    }
}

static int same_value(pico8_value_t a, pico8_value_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    switch (a.kind) {
    case PICO8_VALUE_FIXED:
        return a.as.fixed == b.as.fixed;
    case PICO8_VALUE_STRING:
        return a.as.string == b.as.string;
    case PICO8_VALUE_BOOL:
        return a.as.boolean == b.as.boolean;
    case PICO8_VALUE_REF:
        return a.as.ref == b.as.ref;
    default:
        return 0;
    }
}

/* The generated data module `m` sets for the table member `name`
 * (pico8_game_data.h); NULL when `m` sets none. */
static const void *generated(int m, const char *name) {
    static const struct {
        int m;
        const char *name;
        const void *data;
    } table[] = {
        {M_JO, "nu", &pico8_jo_nu_tags}, {M_QH, "nu", &pico8_qh_nu_tags}, {M_NL, "nu", &pico8_nl_nu_tags},
        {M_OS, "nu", &pico8_os_nu_tags}, {M_HJ, "nu", &pico8_hj_nu_tags}, {M_PM, "nu", &pico8_pm_nu_tags},
        {M_GA, "nu", &pico8_ga_nu_tags}, {M_NR, "nu", &pico8_nr_nu_tags}, {M_MX, "nu", &pico8_mx_nu_tags},
        {M_JO, "ka", &pico8_jo_ka_seq},  {M_JO, "dz", pico8_jo_dz_rows},   {M_JO, "cq", &pico8_jo_cq_seq},
        {M_JO, "em", &pico8_jo_em_row},  {M_QH, "px", &pico8_qh_px_row},  {M_NL, "px", &pico8_nl_px_row},
        {M_OS, "px", &pico8_os_px_row},  {M_HJ, "px", &pico8_hj_px_row},  {M_PM, "px", &pico8_pm_px_row},
        {M_MX, "px", &pico8_mx_px_row},  {M_QH, "ba", &pico8_qh_ba_seq},  {M_NL, "ba", &pico8_nl_ba_seq},
        {M_HJ, "k_", &pico8_hj_k__seq},  {M_PM, "k_", &pico8_pm_k__seq},  {M_MX, "k_", &pico8_mx_k__seq},
        {M_GA, "k_", &pico8_ga_k__seq},  {M_NR, "hz", &pico8_nr_hz_seq},  {M_FR, "hz", &pico8_fr_hz_seq},
        {M_MX, "ht", &pico8_mx_ht_seq},
    };
    size_t k;

    if (m == M_JO && strcmp(name, "kw") == 0) {
        return pico8_jo_shared_kw();
    }
    for (k = 0; k < sizeof table / sizeof table[0]; k++) {
        if (table[k].m == m && strcmp(table[k].name, name) == 0) {
            return table[k].data;
        }
    }
    return NULL;
}

/* The module whose own spec holds the value module `i` serves as `name`. */
static int answering(int i, const char *name, pico8_value_t v) {
    pico8_value_t up;

    while (PARENT[i] >= 0 && spec_field(PARENT[i], name, &up) && same_value(up, v)) {
        i = PARENT[i];
    }
    return i;
}

static int positional_ints_equal(const pico8_ob_table_t *t, const int *items, int count) {
    int j;

    if (t->count != count) {
        return 0;
    }
    for (j = 0; j < count; j++) {
        if (t->entries[j].key.kind != PICO8_OB_KEY_INDEX || t->entries[j].value.kind != PICO8_VALUE_FIXED ||
            t->entries[j].value.as.fixed != pico8_fixed_from_int(items[j])) {
            return 0;
        }
    }
    return 1;
}

/* Scalars only: every row of the jo tree holds strings, numbers or false. */
static int tables_equal(const pico8_ob_table_t *a, const pico8_ob_table_t *b) {
    int j;

    if (a->count != b->count) {
        return 0;
    }
    for (j = 0; j < a->count; j++) {
        const pico8_ob_entry_t *x = &a->entries[j];
        const pico8_ob_entry_t *y = &b->entries[j];

        if (x->key.kind != y->key.kind || (x->key.kind == PICO8_OB_KEY_INDEX && x->key.index != y->key.index) ||
            (x->key.kind == PICO8_OB_KEY_NAME && strcmp(x->key.name, y->key.name) != 0) ||
            x->value.kind != y->value.kind) {
            return 0;
        }
        if ((x->value.kind == PICO8_VALUE_FIXED && x->value.as.fixed != y->value.as.fixed) ||
            (x->value.kind == PICO8_VALUE_BOOL && x->value.as.boolean != y->value.as.boolean) ||
            (x->value.kind == PICO8_VALUE_STRING && strcmp(x->value.as.string, y->value.as.string) != 0) ||
            x->value.kind == PICO8_VALUE_REF) {
            return 0;
        }
    }
    return 1;
}

static int row_equal(const pico8_ob_table_t *t, const pico8_jb_row_t *row) {
    pico8_ob_table_t *view = pico8_jb_row_view(row);
    int equal = tables_equal(t, view);

    pico8_ob_table_free(view);
    return equal;
}

/* Asserts that the generated `data` holds what the spec table `t` holds. */
static void assert_same_content(const char *name, const pico8_ob_table_t *t, const void *data) {
    int j;

    if (strcmp(name, "nu") == 0) {
        const pico8_str_seq_t *tags = data;
        assert(t->count == tags->count);
        for (j = 0; j < tags->count; j++) {
            assert(t->entries[j].value.kind == PICO8_VALUE_STRING &&
                   strcmp(t->entries[j].value.as.string, tags->items[j]) == 0);
        }
    } else if (strcmp(name, "px") == 0 || strcmp(name, "em") == 0) {
        assert(row_equal(t, data));
    } else if (strcmp(name, "dz") == 0) {
        assert(t->count == PICO8_JO_DZ_COUNT);
        for (j = 0; j < PICO8_JO_DZ_COUNT; j++) {
            assert(t->entries[j].value.kind == PICO8_VALUE_REF &&
                   row_equal(t->entries[j].value.as.ref, &((const pico8_jb_row_t *)data)[j]));
        }
    } else if (strcmp(name, "kw") == 0) {
        const pico8_kw_t *kw = data;
        pico8_value_t mu;
        assert(t->count == 1 && pico8_ob_table_get_name(t, "mu", &mu) && !kw->has_qe && mu.as.fixed == kw->mu);
    } else {
        const pico8_int_seq_t *seq = data;
        assert(positional_ints_equal(t, seq->items, seq->count));
    }
}

/* same_value(), plus the converted table members: a spec table and a
 * struct's untagged REF are the same default when that REF is the data
 * generated from the spec that answered; their content must then agree. */
static int same_default(int i, const char *name, pico8_value_t from_spec, pico8_value_t from_struct) {
    const void *data;

    if (from_spec.kind != PICO8_VALUE_REF || from_struct.kind != PICO8_VALUE_REF || from_spec.as.ref == NULL ||
        from_struct.as.ref == NULL || pico8_type_tag_of(from_spec.as.ref) != PICO8_TYPE_TAG_OB_TABLE ||
        pico8_type_tag_of(from_struct.as.ref) != PICO8_TYPE_TAG_UNKNOWN) {
        return same_value(from_spec, from_struct);
    }
    data = generated(answering(i, name, from_spec), name);
    assert(data != NULL && "an untagged struct member stands for a generated table");
    if (from_struct.as.ref != data) {
        return 0;
    }
    assert_same_content(name, from_spec.as.ref, data);
    return 1;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t fold_ready(uint32_t h) {
    int i;
    for (i = 0; i < MODULE_COUNT; i++) {
        h = fnv(h, (uint32_t)is_ready(i));
    }
    return h;
}

/* Each module's defaults by name, its ancestors' included, absent included. */
static void test_spec_defaults(void) {
    uint32_t h = 2166136261u;
    int i;
    int n;

    for (i = 0; i < MODULE_COUNT; i++) {
        for (n = 0; n < NAME_COUNT; n++) {
            pico8_value_t v = pico8_value_fixed(0x5A5A);
            int found = spec_field(i, NAMES[n], &v);

            h = fnv(h, (uint32_t)found);
            if (found) {
                h = fold_value(h, v);
            } else {
                assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x5A5A);
            }
        }
        h = fnv(h, (uint32_t)i);
    }
    check("spec", h, SPEC_HASH);
}

/* Which names a module serves as the very value or REF its parent serves:
 * the inherited ones, never an override. */
static void test_shared_with_parent(void) {
    uint32_t h = 2166136261u;
    int i;
    int n;

    for (i = 0; i < MODULE_COUNT; i++) {
        for (n = 0; PARENT[i] >= 0 && n < NAME_COUNT; n++) {
            pico8_value_t own = pico8_value_fixed(0x5A5A);
            pico8_value_t up = pico8_value_fixed(0x5A5A);
            int in_own = spec_field(i, NAMES[n], &own);
            int in_up = spec_field(PARENT[i], NAMES[n], &up);

            h = fnv(h, (uint32_t)in_own);
            h = fnv(h, (uint32_t)in_up);
            h = fnv(h, (uint32_t)(in_own && in_up && same_value(own, up)));
        }
        h = fnv(h, (uint32_t)i);
    }
    check("share", h, SHARE_HASH);
}

/* A fresh struct of each module against its defaults: which names it
 * serves, and which of them are the very value or REF of the spec. */
static void test_struct_defaults(void) {
    uint32_t h = 2166136261u;
    int i;
    int n;

    for (i = 0; i < MODULE_COUNT; i++) {
        void *made = build(i);
        int readable = made != NULL && ((const pico8_struct_header_t *)made)->cls->field != NULL;

        h = fnv(h, (uint32_t)(made != NULL));
        h = fnv(h, (uint32_t)readable);
        for (n = 0; readable && n < NAME_COUNT; n++) {
            pico8_value_t from_spec = pico8_value_fixed(0x5A5A);
            pico8_value_t from_struct = pico8_value_fixed(0x5A5A);
            int in_spec = spec_field(i, NAMES[n], &from_spec);
            int in_struct = pico8_entity_get_field(made, NAMES[n], &from_struct);

            h = fnv(h, (uint32_t)in_spec);
            h = fnv(h, (uint32_t)in_struct);
            h = fnv(h, (uint32_t)(in_spec && in_struct && same_default(i, NAMES[n], from_spec, from_struct)));
        }
        h = fnv(h, (uint32_t)i);
    }
    check("struct", h, STRUCT_HASH);
}

/* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
 * alone: `t`, which only ke sets, and ke's `me` are still the very values
 * of before, those of `pico8_ke_start`. */
static void test_second_jl_keeps_ke(void) {
    pico8_value_t me_before;
    pico8_value_t t_before;
    pico8_value_t v;
    int i;

    assert(spec_field(M_KE, "me", &me_before) && spec_field(M_KE, "t", &t_before));
    pico8_jl();
    for (i = 0; i < MODULE_COUNT; i++) {
        assert(spec_field(i, "t", &v) && same_value(v, t_before));
    }
    assert(spec_field(M_KE, "me", &v) && same_value(v, me_before));
    assert(strcmp(pico8_ke_start.me, "ph") == 0 && pico8_ke_start.t == 0);
}

int main(void) {
    uint32_t ready = 2166136261u;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    ready = fold_ready(ready);
    pico8_cart_load();
    ready = fold_ready(ready);
    pico8_jl();
    ready = fold_ready(ready);

    test_spec_defaults();
    test_shared_with_parent();
    test_struct_defaults();
    test_second_jl_keeps_ke();

    for (i = MODULE_COUNT - 1; i >= 0; i--) {
        shutdown_of(i);
        ready = fold_ready(ready);
    }
    check("ready", ready, READY_HASH);

    printf("test_pico8_jo_spec_defaults: all tests passed\n");
    return 0;
}
