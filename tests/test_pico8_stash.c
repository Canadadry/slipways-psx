#include "../src/pico8_stash.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_mem.h"

static void test_r_and_iz_decode_real_cart_content(void) {
    pico8_mem_t mem;
    pico8_value_t v;

    pico8_mem_init(&mem);
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_stash_init(&mem);

    assert(pico8_r()->count == 13);
    assert(pico8_ob_table_get_name(pico8_r(), "f", &v));
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(28));
    assert(pico8_ob_table_get_name(pico8_r(), "o", &v));
    assert(v.as.fixed == pico8_fixed_from_int(21));
    assert(pico8_ob_table_get_name(pico8_r(), "e", &v));
    assert(v.as.fixed == pico8_fixed_from_int(54));
    assert(!pico8_ob_table_get_name(pico8_r(), "zz", &v));

    assert(pico8_iz()->count == 11);
    assert(pico8_ob_table_get_name(pico8_iz(), "28", &v));
    assert(v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "food") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_name(pico8_iz(), "21", &v));
    assert(strcmp(v.as.string, "ore") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_name(pico8_iz(), "54", &v));
    assert(strcmp(v.as.string, "energy") == 0);
    free((void *)v.as.string);

    pico8_stash_shutdown();
}

int main(void) {
    test_r_and_iz_decode_real_cart_content();

    printf("test_pico8_stash: all tests passed\n");
    return 0;
}
