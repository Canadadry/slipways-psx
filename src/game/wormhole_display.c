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

void wormhole_draw(p8_graphics_t *gfx, p8_memory_t *mem, wormhole_t *self, vec_t pos) {
    int name;
    int speed_factor;

    if (self == NULL) {
        return;
    }

    name = p8_fixed_flr_int(pos.x);
    speed_factor = p8_fixed_flr_int(pos.y);

    if (self->header.state != NULL && strcmp(self->header.state, "placing") == 0) {
        vec_t cp;

        if (self->targeting->target == NULL) {
            p8_fixed_t oi = self->radius;
            if (gfx != NULL && mem != NULL) {
                p8_circ(gfx, mem, name, speed_factor, p8_fixed_flr_int(oi), 1);
            }
        }

        /* Runs whether or not a target was found: the `if not self.et`
         * above guards the ring alone. */
        cp = nearest_anchor_pos(self);
        if (gfx != NULL && mem != NULL) {
            p8_line(gfx, mem, p8_fixed_flr_int(cp.x), p8_fixed_flr_int(cp.y), name, speed_factor, 1);
            p8_spr_default(gfx, mem, 7, name - 4, speed_factor - 4);
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
                    /* `flr(sr/oi*8)` is 0 whenever `sr < oi/8`, and `kk` is
                     * 1-indexed: Lua passes nil there and `circ` keeps the
                     * current pen colour, where this default draws in 0. */
                    p8_fixed_t ratio = p8_fixed_mul(p8_fixed_div(sr, oi), p8_fixed_from_int(8));
                    int idx = p8_fixed_flr_int(p8_fixed_flr(ratio));

                    int_seq_at(self->archetype->color_ramp->items, self->archetype->color_ramp->count, idx, &color);
                }

                if (gfx != NULL && mem != NULL) {
                    p8_circ(gfx, mem, name, speed_factor, p8_fixed_flr_int(sr), color);
                }
            }
        }
    }
}

void wormhole_draw_method(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    wormhole_draw(dispatch_context_gfx(), dispatch_context_mem(), (wormhole_t *)self, p);
}
