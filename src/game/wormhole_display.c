#include "game/wormhole_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "game/wormhole.h"

static pico8_vec_t pico8_mzd_ra_of_ij(const pico8_mz_t *self) {
    const pico8_vec_t *ra = NULL;

    if (self->ij != NULL && self->ij->pos != NULL) {
        ra = *self->ij->pos;
    }
    return ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec_t p) {
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    px = pico8_fixed_flr_int(p.x);
    py = pico8_fixed_flr_int(p.y);

    if (self->header.state != NULL && strcmp(self->header.state, "kz") == 0) {
        pico8_vec_t cp;

        if (self->probe->et == NULL) {
            pico8_fixed_t oi = self->oi;
            if (gfx != NULL && mem != NULL) {
                pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(oi), 1);
            }
        }

        /* Runs whether or not a target was found: the `if not self.et`
         * above guards the ring alone. */
        cp = pico8_mzd_ra_of_ij(self);
        if (gfx != NULL && mem != NULL) {
            pico8_line(gfx, mem, pico8_fixed_flr_int(cp.x), pico8_fixed_flr_int(cp.y), px, py, 1);
            pico8_spr_default(gfx, mem, 7, px - 4, py - 4);
        }
    } else {
        pico8_fixed_t t = self->header.state_time;
        pico8_fixed_t oi = self->oi;
        int has_kk = self->def->kk != NULL;
        int dt;

        for (dt = 0; dt <= 8; dt += 2) {
            if (t > pico8_fixed_from_int(dt)) {
                pico8_fixed_t sr = pico8_mz_ej(self, pico8_fixed_sub(t, pico8_fixed_from_int(dt)));
                int color = 0;

                if (has_kk) {
                    /* `flr(sr/oi*8)` is 0 whenever `sr < oi/8`, and `kk` is
                     * 1-indexed: Lua passes nil there and `circ` keeps the
                     * current pen colour, where this default draws in 0. */
                    pico8_fixed_t ratio = pico8_fixed_mul(pico8_fixed_div(sr, oi), pico8_fixed_from_int(8));
                    int idx = pico8_fixed_flr_int(pico8_fixed_flr(ratio));

                    pico8_int_seq_at(self->def->kk->items, self->def->kk->count, idx, &color);
                }

                if (gfx != NULL && mem != NULL) {
                    pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(sr), color);
                }
            }
        }
    }
}

void pico8_mz_kj_dispatch(entity_t *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
}
