#include "../src/pico8_data_init.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_map_tables.h"
#include "../src/pico8_mem.h"

static void test_hc_fd_dr_nb_decode_real_cart_content(void) {
    pico8_mem_t mem;
    pico8_value_t v;

    pico8_mem_init(&mem);
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_data_init(&mem);

    assert(PICO8_HC_COUNT == 6);
    assert(pico8_hc_lookup("f-") != NULL);
    assert(pico8_hc_lookup("qh+") != NULL);
    assert(pico8_hc_lookup("does-not-exist") == NULL);

    assert(pico8_fd()->count == 42);

    assert(PICO8_DR_COUNT == 7);
    assert(pico8_dr_table[0].count > 0 && pico8_dr_table[0].items[0].row != NULL);
    {
        const pico8_jb_row_t *title = pico8_dr_table[0].items[0].row;
        assert(title->count > 0 && title->items[0].s != NULL);
        assert(strcmp(title->items[0].s, "-lab-") == 0);
    }

    assert(pico8_nb()->count == 38);
    assert(pico8_ob_table_get_index(pico8_nb(), 1, &v));
    assert(v.kind == PICO8_VALUE_REF);
    {
        const pico8_ob_table_t *tech = v.as.ref;
        pico8_value_t field;

        assert(pico8_ob_table_get_name(tech, "px", &field));
        assert(field.kind == PICO8_VALUE_STRING);
        assert(strcmp(field.as.string, "space folding") == 0);

        assert(pico8_ob_table_get_name(tech, "lv", &field));
        assert(field.kind == PICO8_VALUE_FIXED);
        assert(field.as.fixed == pico8_fixed_from_int(1));

        assert(pico8_ob_table_get_name(tech, "nk", &field));
        assert(field.kind == PICO8_VALUE_FIXED);
        assert(field.as.fixed == pico8_fixed_from_int(22));

        assert(pico8_ob_table_get_name(tech, "od", &field));
        assert(field.kind == PICO8_VALUE_FIXED);
        assert(field.as.fixed == pico8_fixed_from_int(4));

        assert(pico8_ob_table_get_name(tech, "oy", &field));
        assert(field.kind == PICO8_VALUE_REF);
    }

    pico8_data_shutdown();
}

int main(void) {
    test_hc_fd_dr_nb_decode_real_cart_content();

    printf("test_pico8_data_init: all tests passed\n");
    return 0;
}
