
#include <math.h>
#include <stdio.h>
#include <string.h>
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
#include "../src/pico8_ob.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fixed_test_double.h"

#define EMPTY_PLANET_EVENTS_PATH "tests/fixtures/empty_planet_cleared_events.json"

#define REVEAL_FRAME 64
#define FADE_FRAME 214
#define CONTROL_DONE_FRAME 221
#define BUILD_DONE_FRAME 228

#define CONTROL_DROP_X 88
#define CONTROL_DROP_Y 37
#define BUILD_DROP_X 110
#define BUILD_DROP_Y 55

#define CAMERA_OFFSET 192

#define QH_PRICE 15.0
#define QH_SNAP_RADIUS_SQUARED 0.025
#define QH_PLACED_STATE "ly"

#define PLANET_BOX_RADIUS 7

#define MAX_LIT_PIXELS_EMPTY_SPACE 10
#define MIN_LIT_PIXELS_VISIBLE_PLANET 30

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *state_of(const void *e) {
    pico8_value_t v;
    if (e != NULL && pico8_entity_get_field(e, "me", &v) && v.kind == PICO8_VALUE_STRING) {
        return v.as.string;
    }
    return "(none)";
}

static const pico8_vec_t *vec_field(const void *e, const char *name) {
    pico8_value_t v;
    if (e != NULL && pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_vec_t *)v.as.ref;
    }
    return NULL;
}

static double credits_now(void) {
    pico8_value_t nz;
    pico8_instance_t *fm = pico8_fm();
    if (fm != NULL && pico8_instance_get_field(fm, "nz", &nz) && nz.kind == PICO8_VALUE_FIXED) {
        return pico8_fixed_to_double(nz.as.fixed);
    }
    return 0.0;
}

static int laboratory_count(void) {
    const pico8_list_t *qh = pico8_cg("qh");
    return qh != NULL ? qh->count : 0;
}

static int lit_pixels_around(int cx, int cy) {
    int dx, dy, n = 0;
    for (dy = -PLANET_BOX_RADIUS; dy <= PLANET_BOX_RADIUS; dy++) {
        for (dx = -PLANET_BOX_RADIUS; dx <= PLANET_BOX_RADIUS; dx++) {
            int x = cx + dx;
            int y = cy + dy;
            if (x < 0 || x >= PICO8_SCREEN_WIDTH || y < 0 || y >= PICO8_SCREEN_HEIGHT) {
                continue;
            }
            if (pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x) != 0) {
                n++;
            }
        }
    }
    return n;
}

int main(void) {
    pico8_value_t difficulty_entry;
    pico8_js_t *difficulty_button;
    int frame_count;
    int i;

    double empty_x = 0.0;
    double empty_y = 0.0;
    int empty_planets = 0;
    int visible_planets = 0;
    double credits_before_control = 0.0;
    double credits_after_control = 0.0;
    double credits_after_build = 0.0;
    int labs_after_control = -1;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, EMPTY_PLANET_EVENTS_PATH));

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
    assert(frame_count == BUILD_DONE_FRAME + 1);

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == REVEAL_FRAME) {
            const pico8_list_t *jo = pico8_cg("jo");
            int j;
            double dx, dy, dist_px, snap_px;

            assert(jo != NULL);
            for (j = 0; j < jo->count; j++) {
                const void *p = jo->items[j].as.ref;
                const pico8_vec_t *ra = vec_field(p, "ra");
                if (ra == NULL) {
                    continue;
                }
                if (strcmp(state_of(p), "ll") == 0) {
                    empty_planets++;
                    empty_x = pico8_fixed_to_double(ra->x) - CAMERA_OFFSET;
                    empty_y = pico8_fixed_to_double(ra->y) - CAMERA_OFFSET;
                } else if (strcmp(state_of(p), "unknown") != 0) {
                    visible_planets++;
                }
            }

            assert(empty_planets == 1);
            assert(visible_planets > 0);

            snap_px = sqrt(QH_SNAP_RADIUS_SQUARED) * 128.0;
            dx = empty_x - BUILD_DROP_X;
            dy = empty_y - BUILD_DROP_Y;
            dist_px = sqrt(dx * dx + dy * dy);
            assert(dist_px > 0.0 && dist_px < snap_px);

            printf("[reveal] frame=%d probe revealed %d planet(s), exactly one EMPTY at screen (%.1f,%.1f), "
                   "%.1fpx from the drop spot (snap radius %.1fpx)\n",
                   i, empty_planets + visible_planets, empty_x, empty_y, dist_px, snap_px);
        } else if (i == FADE_FRAME) {
            int lit_empty = lit_pixels_around((int)(empty_x + 0.5), (int)(empty_y + 0.5));
            int lit_visible = lit_pixels_around(CONTROL_DROP_X, CONTROL_DROP_Y);

            assert(lit_empty <= MAX_LIT_PIXELS_EMPTY_SPACE);
            assert(lit_visible >= MIN_LIT_PIXELS_VISIBLE_PLANET);

            credits_before_control = credits_now();
            assert(laboratory_count() == 0);

            printf("[faded] frame=%d the empty planet's spot draws %d lit pixel(s) (blank space); a still-visible "
                   "planet draws %d; credits=%.2f\n",
                   i, lit_empty, lit_visible, credits_before_control);
        } else if (i == CONTROL_DONE_FRAME) {
            credits_after_control = credits_now();
            labs_after_control = laboratory_count();

            assert(labs_after_control == 0);
            assert(fabs(credits_after_control - credits_before_control) < 0.0001);

            printf("[control] frame=%d laboratory dropped on the VISIBLE planet at (%d,%d): refused, %d built, "
                   "credits %.2f -> %.2f\n",
                   i, CONTROL_DROP_X, CONTROL_DROP_Y, labs_after_control, credits_before_control,
                   credits_after_control);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    credits_after_build = credits_now();

    {
        const pico8_list_t *qh = pico8_cg("qh");
        const void *lab;
        const pico8_vec_t *lab_ra;

        assert(qh != NULL);
        assert(qh->count == 1);
        lab = qh->items[0].as.ref;
        assert(strcmp(state_of(lab), QH_PLACED_STATE) == 0);

        lab_ra = vec_field(lab, "ra");
        assert(lab_ra != NULL);
        assert(fabs(pico8_fixed_to_double(lab_ra->x) - (BUILD_DROP_X + CAMERA_OFFSET)) < 0.5);
        assert(fabs(pico8_fixed_to_double(lab_ra->y) - (BUILD_DROP_Y + CAMERA_OFFSET)) < 0.5);

        assert(fabs((credits_after_control - QH_PRICE) - credits_after_build) < 0.0001);
    }

    printf("[build] laboratory dropped on the vacated spot at (%d,%d): built in state \"%s\", credits %.2f -> %.2f "
           "(paid %.2f)\n",
           BUILD_DROP_X, BUILD_DROP_Y, QH_PLACED_STATE, credits_after_control, credits_after_build, QH_PRICE);

    pico8_input_free(&input);

    printf("pico8 empty planet cleared: a planet a probe reveals as empty stops blocking structure placement once it "
           "has faded out\n");
    return 0;
}
