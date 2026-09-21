
#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"

#define SHOP_HOVER_EVENTS_PATH "tests/fixtures/shop_button_hover_events.json"

#define PANEL_TOP 0
#define PANEL_BOTTOM 42

#define PANEL_PRIMARY_COLOR 0
#define PANEL_SECONDARY_COLOR 1

static pico8_mem_t *g_screen_mem;

static int screen_px(int x, int y) {
    return pico8_peek(g_screen_mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

static void verify_screen_holds_only_real_colors(void) {
    int x, y;
    int bad = 0;

    for (y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            int pixel = screen_px(x, y);
            if (pixel > 0x0f) {
                if (bad < 4) {
                    fprintf(stderr, "[screen] pixel (%d,%d) = %d (0x%02x) -- not a PICO-8 colour index\n", x, y, pixel,
                            pixel);
                }
                bad++;
            }
        }
    }
    assert(bad == 0);
    printf("[screen] all %d framebuffer bytes are colour indexes 0..15\n",
           PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT);
}

static void verify_panel_is_a_two_color_stripe(void) {
    static const int probe_columns[] = {0, PICO8_SCREEN_WIDTH - 1};
    const int column_count = (int)(sizeof(probe_columns) / sizeof(probe_columns[0]));
    int i, y;
    int primary_rows = 0;
    int secondary_rows = 0;

    for (i = 0; i < column_count; i++) {
        int x = probe_columns[i];
        for (y = PANEL_TOP; y <= PANEL_BOTTOM; y++) {
            int pattern_bit_set = ((y % 4) == 1 || (y % 4) == 3);
            int expected = pattern_bit_set ? PANEL_SECONDARY_COLOR : PANEL_PRIMARY_COLOR;
            int actual = screen_px(x, y);

            if (actual != expected) {
                fprintf(stderr, "[panel] pixel (%d,%d): expected colour %d (fill pattern bit %d of 0x0F0F, colour "
                                "0x10), got %d\n",
                        x, y, expected, pattern_bit_set, actual);
            }
            assert(actual == expected);
            if (pattern_bit_set) {
                secondary_rows++;
            } else {
                primary_rows++;
            }
        }
    }

    assert(primary_rows > 0);
    assert(secondary_rows > 0);
    printf("[panel] y=%d..%d on columns 0 and %d: %d pixels in colour %d, %d in colour %d\n", PANEL_TOP, PANEL_BOTTOM,
           PICO8_SCREEN_WIDTH - 1, primary_rows, PANEL_PRIMARY_COLOR, secondary_rows, PANEL_SECONDARY_COLOR);
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_value_t difficulty_entry;
    pico8_instance_t *difficulty_button;
    int frame_count;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    g_screen_mem = &mem;

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, SHOP_HOVER_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    difficulty_button = pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    frame_count = input.frame_count;
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();
        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    verify_screen_holds_only_real_colors();
    verify_panel_is_a_two_color_stripe();

    pico8_input_free(&input);

    printf("pico8 shop tooltip fillp: %d frame(s) run against %s; the hovered item's description panel is a legal "
           "two-colour striped fill\n",
           frame_count, SHOP_HOVER_EVENTS_PATH);
    return 0;
}
