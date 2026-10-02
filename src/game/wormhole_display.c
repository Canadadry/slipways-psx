#include "game/wormhole_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "game/wormhole.h"

static vec_t pico8_mzd_ra_of_ij(const pico8_mz_t *self) {
    const vec_t *ra = NULL;

    if (self->ij != NULL && self->ij->pos != NULL) {
        ra = *self->ij->pos;
    }
    return ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

void pico8_mz_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_mz_t *self, vec_t p) {
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    px = p8_fixed_flr_int(p.x);
    py = p8_fixed_flr_int(p.y);

    if (self->header.state != NULL && strcmp(self->header.state, "placing") == 0) {
        vec_t cp;

        if (self->probe->et == NULL) {
            p8_fixed_t oi = self->oi;
            if (gfx != NULL && mem != NULL) {
                p8_circ(gfx, mem, px, py, p8_fixed_flr_int(oi), 1);
            }
        }

        /* Runs whether or not a target was found: the `if not self.et`
         * above guards the ring alone. */
        cp = pico8_mzd_ra_of_ij(self);
        if (gfx != NULL && mem != NULL) {
            p8_line(gfx, mem, p8_fixed_flr_int(cp.x), p8_fixed_flr_int(cp.y), px, py, 1);
            p8_spr_default(gfx, mem, 7, px - 4, py - 4);
        }
    } else {
        p8_fixed_t t = self->header.state_time;
        p8_fixed_t oi = self->oi;
        int has_kk = self->def->color_ramp != NULL;
        int dt;

        for (dt = 0; dt <= 8; dt += 2) {
            if (t > p8_fixed_from_int(dt)) {
                p8_fixed_t sr = pico8_mz_ej(self, p8_fixed_sub(t, p8_fixed_from_int(dt)));
                int color = 0;

                if (has_kk) {
                    /* `flr(sr/oi*8)` is 0 whenever `sr < oi/8`, and `kk` is
                     * 1-indexed: Lua passes nil there and `circ` keeps the
                     * current pen colour, where this default draws in 0. */
                    p8_fixed_t ratio = p8_fixed_mul(p8_fixed_div(sr, oi), p8_fixed_from_int(8));
                    int idx = p8_fixed_flr_int(p8_fixed_flr(ratio));

                    int_seq_at(self->def->color_ramp->items, self->def->color_ramp->count, idx, &color);
                }

                if (gfx != NULL && mem != NULL) {
                    p8_circ(gfx, mem, px, py, p8_fixed_flr_int(sr), color);
                }
            }
        }
    }
}

void pico8_mz_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_mz_kj(dispatch_context_gfx(), dispatch_context_mem(), (pico8_mz_t *)self, p);
}
