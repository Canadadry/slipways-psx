#include "pico8_wormhole_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_wormhole.h"

static pico8_fixed_t pico8_mzd_fixed(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_mzd_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_mzd_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return pico8_mzd_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static int pico8_mzd_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_vec_t pico8_mzd_ra_of_ij(const pico8_mz_t *self) {
    if (self->ij.kind != PICO8_VALUE_REF || self->ij.as.ref == NULL) {
        return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    }
    assert(pico8_type_tag_of(self->ij.as.ref) == PICO8_TYPE_TAG_INSTANCE &&
           "mz's ij field must hold an entity instance");
    return pico8_mzd_field_vec((const pico8_instance_t *)self->ij.as.ref, "ra");
}

void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec_t p) {
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    px = pico8_fixed_flr_int(p.x);
    py = pico8_fixed_flr_int(p.y);

    if (self->ke.me.kind == PICO8_VALUE_STRING && self->ke.me.as.string != NULL &&
        strcmp(self->ke.me.as.string, "kz") == 0) {
        pico8_vec_t cp;

        if (!pico8_mzd_truthy(self->et)) {
            pico8_fixed_t oi = pico8_mzd_fixed(self->oi);
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
        pico8_fixed_t t = pico8_mzd_fixed(self->ke.t);
        pico8_fixed_t oi = pico8_mzd_fixed(self->oi);
        int has_kk = self->kk.kind == PICO8_VALUE_REF && self->kk.as.ref != NULL;
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
                    pico8_value_t entry;

                    assert(pico8_type_tag_of(self->kk.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
                           "mz's kk colour ramp must be a spec table");
                    if (pico8_ob_table_get_index((const pico8_ob_table_t *)self->kk.as.ref, idx, &entry) &&
                        entry.kind == PICO8_VALUE_FIXED) {
                        color = pico8_fixed_flr_int(entry.as.fixed);
                    }
                }

                if (gfx != NULL && mem != NULL) {
                    pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(sr), color);
                }
            }
        }
    }
}
