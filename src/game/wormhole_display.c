#include "game/wormhole_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "game/wormhole.h"

static vec_t nearest_anchor_pos(const wormhole_t *self) {
    const vec_t *ra = NULL;

    if (self->anchor != NULL && self->anchor->pos != NULL) {
        ra = *self->anchor->pos;
    }
    return ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

void wormhole_draw(p8_graphics_t *graphics, p8_memory_t *memory, wormhole_t *self, vec_t screen_pos) {
    int screen_x;
    int screen_y;

    if (self == NULL) {
        return;
    }

    screen_x = p8_fixed_flr_int(screen_pos.x);
    screen_y = p8_fixed_flr_int(screen_pos.y);

    if (self->header.state != NULL && strcmp(self->header.state, "placing") == 0) {
        vec_t cp;

        if (self->targeting->target == NULL) {
            p8_fixed_t oi = self->radius;
            if (graphics != NULL && memory != NULL) {
                p8_circ(graphics, memory, screen_x, screen_y, p8_fixed_flr_int(oi), 1);
            }
        }

        /* Runs whether or not a target was found: the NULL-target test
         * above guards the ring alone. */
        cp = nearest_anchor_pos(self);
        if (graphics != NULL && memory != NULL) {
            p8_line(graphics, memory, p8_fixed_flr_int(cp.x), p8_fixed_flr_int(cp.y), screen_x, screen_y, 1);
            p8_spr_default(graphics, memory, 7, screen_x - 4, screen_y - 4);
        }
    } else {
        p8_fixed_t t = self->header.state_time;
        p8_fixed_t oi = self->radius;
        int has_color_ramp = self->archetype->color_ramp != NULL;
        int dt;

        for (dt = 0; dt <= 8; dt += 2) {
            if (t > p8_fixed_from_int(dt)) {
                p8_fixed_t sr = wormhole_reveal_radius(self, p8_fixed_sub(t, p8_fixed_from_int(dt)));
                int color = 0;

                if (has_color_ramp) {
                    /* TROU-12 */
                    /* The ramp is 1-indexed: int_seq_at leaves an index of
                     * 0 at the default colour 0. */
                    p8_fixed_t ratio = p8_fixed_mul(p8_fixed_div(sr, oi), p8_fixed_from_int(8));
                    int idx = p8_fixed_flr_int(p8_fixed_flr(ratio));

                    int_seq_at(self->archetype->color_ramp->items, self->archetype->color_ramp->count, idx, &color);
                }

                if (graphics != NULL && memory != NULL) {
                    p8_circ(graphics, memory, screen_x, screen_y, p8_fixed_flr_int(sr), color);
                }
            }
        }
    }
}

void wormhole_draw_method(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    wormhole_draw(dispatch_context_gfx(), dispatch_context_mem(), (wormhole_t *)self, p);
}
