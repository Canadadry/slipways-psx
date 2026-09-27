/* A planet running a project whose production table is `hc["-"]` (here
 * "mine") keeps producing once it turns rich: `bb()` sets `kw.mu` to
 * `po[min(mi,#po)]`, and `#po` is the sequence length, not counting the named
 * `mc` key that table also holds. Its panel therefore still shows an
 * "exports:" row at tier 4, as it does at tier 3.
 *
 * Reuses tests/fixtures/built_planet_panel_events.json (see
 * tests/test_pico8_built_planet_panel_nil_row.c) to reach a colonized "p27"
 * through the real update/draw chain, then installs the project and drives
 * the economy tick the way src/pico8_economy.c does. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_research.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define EVENTS_PATH "tests/fixtures/built_planet_panel_events.json"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static pico8_struct_header_t *planet_by_id(const char *id) {
    const pico8_list_t *jo = pico8_cg("jo");
    int i;
    assert(jo != NULL);
    for (i = 0; i < jo->count; i++) {
        pico8_struct_header_t *p = (pico8_struct_header_t *)jo->items[i].as.ref;
        pico8_value_t v;
        if (pico8_entity_get_field(p, "id", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, id) == 0) {
            return p;
        }
    }
    return NULL;
}

static int kw_mu(const pico8_struct_header_t *p) {
    pico8_value_t kw, mu;
    assert(pico8_entity_get_field(p, "kw", &kw) && kw.kind == PICO8_VALUE_REF && kw.as.ref != NULL);
    assert(pico8_ob_table_get_name((const pico8_ob_table_t *)kw.as.ref, "mu", &mu));
    assert(mu.kind == PICO8_VALUE_FIXED);
    return pico8_fixed_flr_int(mu.as.fixed);
}

/* The row's first item, as `lq[1]` finds it. */
static int row_first(const pico8_lq_t *lq, pico8_value_t *out) {
    if (lq->items.count < 1) {
        return 0;
    }
    *out = lq->items.items[0];
    return 1;
}

/* Sets the tier, runs the economy tick to a fixed point, marks every need as
 * received and rebuilds the panel. Returns kw.mu; *has_exports tells whether
 * some panel row leads with "exports:". */
static int promote_and_open_panel(pico8_struct_header_t *planet, int tier, int *has_exports) {
    pico8_value_t jy, le;
    const pico8_list_t *needs;
    const pico8_list_t *rows;
    int i;

    pico8_entity_set_field(planet, "mi", pico8_value_fixed(pico8_fixed_from_int(tier)));
    do {
        pico8_bd();
    } while (pico8_bb());

    assert(pico8_entity_get_field(planet, "jy", &jy) && jy.kind == PICO8_VALUE_REF && jy.as.ref != NULL);
    needs = (const pico8_list_t *)jy.as.ref;
    for (i = 0; i < needs->count; i++) {
        pico8_ob_table_set_name((pico8_ob_table_t *)needs->items[i].as.ref, "gc",
                                pico8_value_fixed(pico8_fixed_from_int(1)));
    }

    pico8_jo_gh((const pico8_jo_t *)planet);

    assert(pico8_entity_get_field(pico8_ok(), "le", &le) && le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    rows = (const pico8_list_t *)le.as.ref;
    *has_exports = 0;
    for (i = 0; i < rows->count; i++) {
        const void *button = rows->items[i].as.ref;
        pico8_value_t lq, first;
        if (button != NULL && pico8_entity_get_field(button, "lq", &lq) && lq.kind == PICO8_VALUE_REF &&
            lq.as.ref != NULL && row_first(lq.as.ref, &first) &&
            first.kind == PICO8_VALUE_STRING && strcmp(first.as.string, "exports:") == 0) {
            *has_exports = 1;
        }
    }
    return kw_mu(planet);
}

int main(void) {
    pico8_value_t difficulty_entry, mine;
    pico8_struct_header_t *p27;
    int i, mu3, mu4, exports3, exports4;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    assert(pico8_input_load(&input, EVENTS_PATH));
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (i < input.frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    p27 = planet_by_id("p27");
    assert(p27 != NULL);
    assert(pico8_ob_table_get_name(pico8_gb(), "mine", &mine) && mine.kind == PICO8_VALUE_REF);
    pico8_jo_lz((pico8_jo_t *)p27, (pico8_hp_t *)mine.as.ref);

    mu3 = promote_and_open_panel(p27, 3, &exports3);
    mu4 = promote_and_open_panel(p27, 4, &exports4);
    printf("[mine on p27] tier 3: kw.mu=%d exports row=%d; tier 4: kw.mu=%d exports row=%d\n", mu3, exports3, mu4,
           exports4);

    assert(mu3 == 3 && exports3);
    /* hc["-"] is {1,2,3,mc="+"}: past the last level, production stays at 3. */
    assert(mu4 == 3);
    assert(exports4);

    pico8_input_free(&input);
    printf("pico8 rich planet exports: a rich mine keeps producing 3 and its panel keeps its exports row\n");
    return 0;
}
