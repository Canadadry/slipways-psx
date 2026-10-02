/* The nine planet kinds the cart handles as values -- the seven of `gx`,
 * then `pm` and `ga`, reached by `kv[hp.cw]`. test_kind_fields_and_build()
 * reads them through each kind's `rec` and kind_build() only; its frozen
 * value was taken on the code where these were still classes and must not
 * be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "engine/entity_class.h"

#define KIND_HASH 0x4B139A82u

#define KIND_COUNT 9

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const entity_kind_t *kind_of(int i) {
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

static const kind_record_t *record_of(int i) {
    const kind_record_t *record = kind_of(i)->record;

    assert(record != NULL);
    return record;
}

static void *kind_build(int i) {
    return kind_of(i)->new_entity(NULL);
}

static const entity_class_t *class_of(int i) {
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

static uint32_t fold_fixed_member(uint32_t h, int has, p8_fixed_t v) {
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
        const kind_record_t *record = record_of(i);
        void *made;

        h = fold_string_member(h, record->required_tech);
        h = fold_fixed_member(h, record->has_sprite, record->sprite);
        h = fold_string_member(h, record->state);
        h = fold_fixed_member(h, record->has_cost, record->cost);
        made = kind_build(i);
        assert(made != NULL &&
               ((const entity_t *)made)->class == class_of(i));
        h = fnv(h, (uint32_t)i);
    }
    check("kind", h, KIND_HASH);
}

/* Each kind's `rec` holds the generated default of the class that sets
 * each name. */
static void test_kind_reader_serves_the_generated_defaults(void) {
    const kind_record_t *record;

    record = record_of(0);
    assert(record->required_tech == NULL);
    assert(record->has_sprite && record->sprite == structure_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == structure_archetype.targeting.cost);

    record = record_of(1);
    assert(record->required_tech == NULL);
    assert(record->has_sprite && record->sprite == processor_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == processor_archetype.targeting.cost);

    record = record_of(2);
    assert(record->required_tech == slipgate_archetype.required_tech);
    assert(record->has_sprite && record->sprite == slipgate_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == slipgate_archetype.targeting.cost);

    record = record_of(3);
    assert(record->required_tech == infraspace_gate_archetype.required_tech);
    assert(record->has_sprite && record->sprite == infraspace_gate_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == infraspace_gate_archetype.targeting.cost);

    record = record_of(4);
    assert(record->required_tech == trading_hub_archetype.required_tech);
    assert(record->has_sprite && record->sprite == trading_hub_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == trading_hub_archetype.targeting.cost);

    record = record_of(5);
    assert(record->required_tech == synthesizer_archetype.required_tech);
    assert(record->has_sprite && record->sprite == synthesizer_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == synthesizer_archetype.targeting.cost);

    record = record_of(6);
    assert(record->required_tech == ascension_archetype.required_tech);
    assert(record->has_sprite && record->sprite == ascension_archetype.sprite);
    assert(record->state == structure_archetype.state);
    assert(record->has_cost && record->cost == ascension_archetype.targeting.cost);

    record = record_of(7);
    assert(record->required_tech == protostar_archetype.required_tech);
    assert(record->has_sprite && record->sprite == protostar_archetype.sprite);
    assert(record->state == protostar_archetype.state);
    assert(record->has_cost && record->cost == protostar_archetype.targeting.cost);

    record = record_of(8);
    assert(record->required_tech == NULL);
    assert(!record->has_sprite && record->sprite == 0);
    assert(record->state == planet_archetype.state);
    assert(!record->has_cost && record->cost == 0);
}

/* `kv[hp.cw]` only ever sees "pm" or "ga": every research of `gb` carries
 * one of them as `cw`, or none. */
static void test_every_class_name_is_protostar_or_burst(void) {
    int with_class_name = 0;
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
        assert(strcmp(cw, "protostar") == 0 || strcmp(cw, "burst") == 0);
        with_class_name++;
    }
    printf("[cw] %d of %d researches\n", with_class_name, pico8_gb_count());
    assert(with_class_name > 0);
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

    pico8_cart_load();
    test_every_class_name_is_protostar_or_burst();
    reset_world();
    test_kind_fields_and_build();
    test_kind_reader_serves_the_generated_defaults();

    printf("test_entity_kind: all tests passed\n");
    return 0;
}
