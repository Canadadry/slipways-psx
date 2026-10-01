#include "../src/pico8_data_init.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_map_tables.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_research.h"

static void test_hc_fd_dr_nb_decode_real_cart_content(void) {
    pico8_mem_t mem;

    pico8_mem_init(&mem);
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_data_init();

    assert(PICO8_HC_COUNT == 6);
    assert(pico8_hc_lookup("f-") != NULL);
    assert(pico8_hc_lookup("qh+") != NULL);
    assert(pico8_hc_lookup("does-not-exist") == NULL);

    assert(PICO8_FD_COUNT == 42);
    assert(pico8_gb_count() == 34 && pico8_gb_at(34) != NULL);
    assert(pico8_gb_at(35) == NULL);

    assert(PICO8_DR_COUNT == 7);
    assert(pico8_dr_table[0].count > 0 && pico8_dr_table[0].items[0].row != NULL);
    {
        const pico8_jb_row_t *title = pico8_dr_table[0].items[0].row;
        assert(title->count > 0 && title->items[0].s != NULL);
        assert(strcmp(title->items[0].s, "-lab-") == 0);
    }

    assert(PICO8_NB_COUNT == 19);
    {
        const pico8_nb_entry_t *tech = &pico8_nb_table[0];

        assert(strcmp(tech->px, "space folding") == 0);
        assert(tech->lv == 1);
        assert(tech->nk == 22);
        assert(tech->od == 4);
        assert(tech->oy != NULL);
    }

    pico8_data_shutdown();
}

int main(void) {
    test_hc_fd_dr_nb_decode_real_cart_content();

    printf("test_pico8_data_init: all tests passed\n");
    return 0;
}
