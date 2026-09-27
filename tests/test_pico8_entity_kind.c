/* The nine planet kinds the cart handles as values -- the seven of `gx`,
 * then `pm` and `ga`, reached by `kv[hp.cw]`. test_kind_fields_and_build()
 * reads them through the shared readers kind_field()/kind_build() only; its
 * frozen value was taken on the code where these were still classes and
 * must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_value.h"

#define KIND_HASH 0xBCF0E60Bu

#define KIND_COUNT 9

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const pico8_entity_kind_t *kind_of(int i) {
    switch (i) {
    case 0:
        return pico8_qh_kind();
    case 1:
        return pico8_os_kind();
    case 2:
        return pico8_nr_kind();
    case 3:
        return pico8_fr_kind();
    case 4:
        return pico8_nl_kind();
    case 5:
        return pico8_mx_kind();
    case 6:
        return pico8_hj_kind();
    case 7:
        return pico8_pm_kind();
    default:
        return pico8_ga_kind();
    }
}

static int kind_field(int i, const char *name, pico8_value_t *out) {
    return kind_of(i)->field(name, out);
}

static void *kind_build(int i) {
    return kind_of(i)->new_entity(NULL);
}

static const pico8_struct_class_t *struct_class_of(int i) {
    switch (i) {
    case 0:
        return pico8_qh_struct_class();
    case 1:
        return pico8_os_struct_class();
    case 2:
        return pico8_nr_struct_class();
    case 3:
        return pico8_fr_struct_class();
    case 4:
        return pico8_nl_struct_class();
    case 5:
        return pico8_mx_struct_class();
    case 6:
        return pico8_hj_struct_class();
    case 7:
        return pico8_pm_struct_class();
    default:
        return pico8_ga_struct_class();
    }
}

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
        return fnv(h, v.as.ref == NULL ? 0u : 1u);
    default:
        return h;
    }
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* `rr`/`s`/`me`/`od` of each kind as `ie:hb`/`ie:kj` read them, absent
 * included (`qh`/`os` have no `rr`), and the struct each kind builds. */
static void test_kind_fields_and_build(void) {
    static const char *const names[] = {"rr", "s", "me", "od"};
    uint32_t h = 2166136261u;
    int i;
    size_t n;

    for (i = 0; i < KIND_COUNT; i++) {
        void *made;

        for (n = 0; n < sizeof names / sizeof names[0]; n++) {
            pico8_value_t v = pico8_value_fixed(0x5A5A);
            int found = kind_field(i, names[n], &v);

            h = fnv(h, (uint32_t)found);
            if (found) {
                h = fold_value(h, v);
            } else {
                assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x5A5A);
            }
        }
        made = kind_build(i);
        assert(made != NULL && pico8_type_tag_of(made) == PICO8_TYPE_TAG_STRUCT &&
               ((const pico8_struct_header_t *)made)->cls == struct_class_of(i));
        h = fnv(h, (uint32_t)i);
    }
    check("kind", h, KIND_HASH);
}

typedef int (*default_fn)(const char *name, pico8_value_t *out_value);

static default_fn default_of(int i) {
    switch (i) {
    case 0:
        return pico8_qh_default;
    case 1:
        return pico8_os_default;
    case 2:
        return pico8_nr_default;
    case 3:
        return pico8_fr_default;
    case 4:
        return pico8_nl_default;
    case 5:
        return pico8_mx_default;
    case 6:
        return pico8_hj_default;
    case 7:
        return pico8_pm_default;
    default:
        return pico8_ga_default;
    }
}

/* Each kind reads its defaults through its module's own default chain, and
 * pico8_type_tag_of() reads a kind UNKNOWN. */
static void test_kind_reader_is_the_module_default(void) {
    int i;

    for (i = 0; i < KIND_COUNT; i++) {
        assert(pico8_type_tag_of(kind_of(i)) == PICO8_TYPE_TAG_UNKNOWN);
        assert(kind_of(i)->field == default_of(i));
    }
}

/* `kv[hp.cw]` only ever sees "pm" or "ga": every research of `gb` carries
 * one of them as `cw`, or none. */
static void test_every_cw_is_pm_or_ga(void) {
    const pico8_ob_table_t *gb = pico8_gb();
    int with_cw = 0;
    int k;

    assert(gb != NULL && gb->count > 0);
    for (k = 0; k < gb->count; k++) {
        const pico8_hp_t *hp;
        pico8_value_t cw;

        assert(gb->entries[k].value.kind == PICO8_VALUE_REF && gb->entries[k].value.as.ref != NULL);
        hp = gb->entries[k].value.as.ref;
        cw = hp->cw;
        if (cw.kind == PICO8_VALUE_REF && cw.as.ref == NULL) {
            continue;
        }
        assert(cw.kind == PICO8_VALUE_STRING &&
               (strcmp(cw.as.string, "pm") == 0 || strcmp(cw.as.string, "ga") == 0));
        with_cw++;
    }
    printf("[cw] %d of %d researches\n", with_cw, gb->count);
    assert(with_cw > 0);
}

int main(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    test_every_cw_is_pm_or_ga();
    pico8_jl();
    test_kind_fields_and_build();
    test_kind_reader_is_the_module_default();

    printf("test_pico8_entity_kind: all tests passed\n");
    return 0;
}
