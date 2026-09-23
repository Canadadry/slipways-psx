#include "pico8_wormhole_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_wormhole.h"

/* Field readers: an absent field, a wrong kind, or a NULL instance all read
 * as the zero/false default. */
static pico8_fixed_t pico8_mzd_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
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

static pico8_vec_t pico8_mzd_field_ra_of_ij(const pico8_instance_t *self) {
    pico8_value_t ij_val;
    if (self == NULL || !pico8_instance_get_field(self, "ij", &ij_val) || ij_val.kind != PICO8_VALUE_REF ||
        ij_val.as.ref == NULL) {
        return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    }
    assert(pico8_type_tag_of(ij_val.as.ref) == PICO8_TYPE_TAG_INSTANCE &&
           "mz's ij field must hold an entity instance");
    return pico8_mzd_field_vec((const pico8_instance_t *)ij_val.as.ref, "ra");
}

void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t me_val;
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    px = (int)pico8_fixed_to_double(p.x);
    py = (int)pico8_fixed_to_double(p.y);

    if (pico8_instance_get_field(self, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
        me_val.as.string != NULL && strcmp(me_val.as.string, "kz") == 0) {
        pico8_value_t et_val;
        int has_et = pico8_instance_get_field(self, "et", &et_val) && pico8_mzd_truthy(et_val);
        pico8_vec_t cp;

        if (!has_et) {
            pico8_fixed_t oi = pico8_mzd_field_fixed(self, "oi");
            if (gfx != NULL && mem != NULL) {
                pico8_circ(gfx, mem, px, py, (int)pico8_fixed_to_double(oi), 1);
            }
        }

        /* Runs whether or not a target was found: the `if not self.et`
         * above guards the ring alone. */
        cp = pico8_mzd_field_ra_of_ij(self);
        if (gfx != NULL && mem != NULL) {
            pico8_line(gfx, mem, (int)pico8_fixed_to_double(cp.x), (int)pico8_fixed_to_double(cp.y), px, py, 1);
            pico8_spr_default(gfx, mem, 7, px - 4, py - 4);
        }
    } else {
        pico8_fixed_t t = pico8_mzd_field_fixed(self, "t");
        pico8_fixed_t oi = pico8_mzd_field_fixed(self, "oi");
        pico8_value_t kk_val;
        int has_kk = pico8_instance_get_field(self, "kk", &kk_val) && kk_val.kind == PICO8_VALUE_REF &&
                     kk_val.as.ref != NULL;
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
                    int idx = (int)pico8_fixed_to_double(pico8_fixed_flr(ratio));
                    pico8_value_t entry;

                    assert(pico8_type_tag_of(kk_val.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
                           "mz's kk colour ramp must be a spec table");
                    if (pico8_ob_table_get_index((const pico8_ob_table_t *)kk_val.as.ref, idx, &entry) &&
                        entry.kind == PICO8_VALUE_FIXED) {
                        color = (int)pico8_fixed_to_double(entry.as.fixed);
                    }
                }

                if (gfx != NULL && mem != NULL) {
                    pico8_circ(gfx, mem, px, py, (int)pico8_fixed_to_double(sr), color);
                }
            }
        }
    }
}
