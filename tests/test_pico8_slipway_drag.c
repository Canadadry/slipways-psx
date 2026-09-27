
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_list.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fixed_test_double.h"

#define SLIPWAY_DRAG_EVENTS_PATH "tests/fixtures/slipway_drag_events.json"

#define LAST_DRAG_FRAME 202
#define RELEASE_FRAME 203

static double credits_now(void) {
    pico8_instance_t *fm = pico8_fm();
    pico8_value_t nz;
    if (fm != NULL && pico8_entity_get_field(fm, "nz", &nz) && nz.kind == PICO8_VALUE_FIXED) {
        return pico8_fixed_to_double(nz.as.fixed);
    }
    return 0.0;
}

static const pico8_list_t *planet_oe(pico8_instance_t *planet) {
    pico8_value_t oe;
    if (planet == NULL || !pico8_entity_get_field(planet, "oe", &oe) || oe.kind != PICO8_VALUE_REF ||
        oe.as.ref == NULL) {
        return NULL;
    }
    return (const pico8_list_t *)oe.as.ref;
}

static int oe_count(pico8_instance_t *planet) {
    const pico8_list_t *oe = planet_oe(planet);
    return oe != NULL ? oe->count : 0;
}

static int total_oe(void) {
    const pico8_list_t *jo = pico8_cg("jo");
    int i, total = 0;
    if (jo == NULL) {
        return 0;
    }
    for (i = 0; i < jo->count; i++) {
        total += oe_count((pico8_instance_t *)jo->items[i].as.ref);
    }
    return total;
}

static pico8_instance_t *ui_left_held(void) {
    const void *ok = pico8_ok();
    pico8_value_t v;
    if (ok == NULL || !pico8_entity_get_field(ok, "lheld", &v) || v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }
    return (pico8_instance_t *)v.as.ref;
}

static pico8_instance_t *field_instance(pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_entity_get_field(inst, name, &v) || v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }
    return (pico8_instance_t *)v.as.ref;
}

static double field_number(pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_entity_get_field(inst, name, &v) || v.kind != PICO8_VALUE_FIXED) {
        return 0.0;
    }
    return pico8_fixed_to_double(v.as.fixed);
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    int i;

    pico8_instance_t *origin = NULL;
    pico8_instance_t *target = NULL;
    double price = 0.0;
    double credits_before = 0.0;
    int targets_found = 0;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, SLIPWAY_DRAG_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    frame_count = input.frame_count;
    assert(frame_count > RELEASE_FRAME);

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == LAST_DRAG_FRAME) {
            pico8_instance_t *held = ui_left_held();

            assert(held != NULL);
            origin = field_instance(held, "qu");
            assert(origin != NULL);
            price = field_number(held, "od");
            assert(price > 0.0);
            assert(oe_count(origin) == 0);
            assert(total_oe() == 0);
            credits_before = credits_now();

            printf("[drag] frame=%d held jc price (od)=%.2f credits (fm.nz)=%.2f, no planet connected yet\n", i,
                   price, credits_before);
        } else if (i == RELEASE_FRAME) {
            const pico8_list_t *jo = pico8_cg("jo");
            const pico8_list_t *origin_oe;
            int j;
            double credits_after = credits_now();

            assert(jo != NULL);

            assert(oe_count(origin) == 1);

            for (j = 0; j < jo->count; j++) {
                pico8_instance_t *p = (pico8_instance_t *)jo->items[j].as.ref;
                if (p == origin) {
                    continue;
                }
                if (oe_count(p) > 0) {
                    targets_found++;
                    target = p;
                }
            }
            assert(targets_found == 1);
            assert(oe_count(target) == 1);

            origin_oe = planet_oe(origin);
            assert(origin_oe != NULL);
            assert(planet_oe(target) != NULL);
            assert(origin_oe->items[0].kind == PICO8_VALUE_REF);
            assert(origin_oe->items[0].as.ref == planet_oe(target)->items[0].as.ref);
            assert(origin_oe->items[0].as.ref != NULL);

            assert(fabs((credits_before - price) - credits_after) < 0.0001);

            printf("[release] frame=%d both endpoints connected (oe=1 each, same slipway), credits %.2f -> %.2f "
                   "(paid %.2f)\n",
                   i, credits_before, credits_after, price);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 slipway drag: releasing a drag on a valid target planet builds the slipway and charges for it\n");
    return 0;
}
