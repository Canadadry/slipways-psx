/* The nine planet kinds the cart handles as values -- the seven of `gx`,
 * then `pm` and `ga`, reached by `kv[hp.cw]`. test_kind_fields_and_build()
 * reads them through each kind's `rec` and kind_build() only; its frozen
 * value was taken on the code where these were still classes and must not
 * be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_vec.h"

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

static const pico8_kind_rec_t *rec_of(int i) {
    const pico8_kind_rec_t *rec = kind_of(i)->rec;

    assert(rec != NULL);
    return rec;
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

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1 };

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFFFFFFFu);
}

/* The kind word is constant by the struct's shape (`rr`/`me` strings,
 * `s`/`od` fixed), kept so that KIND_HASH still proves the typed reads. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    h = fnv(h, (uint32_t)(s != NULL));
    return s == NULL ? h : fold_string(fnv(h, (uint32_t)STRING_KIND), s);
}

static uint32_t fold_fixed_member(uint32_t h, int has, pico8_fixed_t v) {
    h = fnv(h, (uint32_t)(has != 0));
    return has ? fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)v) : h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* `rr`/`s`/`me`/`od` of each kind as `ie:hb`/`ie:kj` read them, absent
 * included (`qh`/`os` have no `rr`), and the struct each kind builds. */
static void test_kind_fields_and_build(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < KIND_COUNT; i++) {
        const pico8_kind_rec_t *rec = rec_of(i);
        void *made;

        h = fold_string_member(h, rec->rr);
        h = fold_fixed_member(h, rec->has_s, rec->s);
        h = fold_string_member(h, rec->me);
        h = fold_fixed_member(h, rec->has_od, rec->od);
        made = kind_build(i);
        assert(made != NULL &&
               ((const pico8_struct_header_t *)made)->cls == struct_class_of(i));
        h = fnv(h, (uint32_t)i);
    }
    check("kind", h, KIND_HASH);
}

/* Each kind's `rec` holds the generated default of the class that sets
 * each name. */
static void test_kind_reader_serves_the_generated_defaults(void) {
    const pico8_kind_rec_t *rec;

    rec = rec_of(0);
    assert(rec->rr == NULL);
    assert(rec->has_s && rec->s == pico8_qh_def.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_qh_def.probe.od);

    rec = rec_of(1);
    assert(rec->rr == NULL);
    assert(rec->has_s && rec->s == pico8_os_def.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_os_def.probe.od);

    rec = rec_of(2);
    assert(rec->rr == pico8_nr_start.rr);
    assert(rec->has_s && rec->s == pico8_nr_start.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_nr_start.od);

    rec = rec_of(3);
    assert(rec->rr == pico8_fr_start.rr);
    assert(rec->has_s && rec->s == pico8_fr_start.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_fr_start.od);

    rec = rec_of(4);
    assert(rec->rr == pico8_nl_def.rr);
    assert(rec->has_s && rec->s == pico8_nl_def.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_nl_def.probe.od);

    rec = rec_of(5);
    assert(rec->rr == pico8_mx_start.rr);
    assert(rec->has_s && rec->s == pico8_mx_start.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_mx_start.od);

    rec = rec_of(6);
    assert(rec->rr == pico8_hj_start.rr);
    assert(rec->has_s && rec->s == pico8_hj_start.s);
    assert(rec->me == pico8_qh_def.me);
    assert(rec->has_od && rec->od == pico8_hj_start.od);

    rec = rec_of(7);
    assert(rec->rr == pico8_hj_start.rr);
    assert(rec->has_s && rec->s == pico8_pm_start.s);
    assert(rec->me == pico8_pm_start.me);
    assert(rec->has_od && rec->od == pico8_hj_start.od);

    rec = rec_of(8);
    assert(rec->rr == NULL);
    assert(!rec->has_s && rec->s == 0);
    assert(rec->me == pico8_jo_def.me);
    assert(!rec->has_od && rec->od == 0);
}

/* `kv[hp.cw]` only ever sees "pm" or "ga": every research of `gb` carries
 * one of them as `cw`, or none. */
static void test_every_cw_is_pm_or_ga(void) {
    int with_cw = 0;
    int k;

    assert(pico8_gb_count() > 0);
    for (k = 1; k <= pico8_gb_count(); k++) {
        const pico8_hp_t *hp;
        const char *cw;

        hp = pico8_gb_at(k);
        cw = hp->cw;
        if (cw == NULL) {
            continue;
        }
        assert(strcmp(cw, "pm") == 0 || strcmp(cw, "ga") == 0);
        with_cw++;
    }
    printf("[cw] %d of %d researches\n", with_cw, pico8_gb_count());
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
    test_kind_reader_serves_the_generated_defaults();

    printf("test_pico8_entity_kind: all tests passed\n");
    return 0;
}
