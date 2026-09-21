#include "pico8_wormhole_display.h"

#include <stddef.h>
#include <string.h>

#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_wormhole.h"

/* See pico8_wormhole_display.h's own top comment for the full source/trace
 * of mz:kj. The private field/truthy helpers below are the same
 * established pattern duplicated once more per-module (pico8_probe.c's own
 * pico8_probe_*, pico8_wormhole.c's own pico8_mz_* siblings) -- an "mzd"
 * prefix (not "mz") is used here specifically so this file's own copies
 * never collide with pico8_wormhole.c's identically-shaped statics across
 * the two translation units (both are `static`, so there is no LINK-time
 * collision either way, but distinct names keep grepping for either
 * module's own helpers unambiguous). */

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

/* self.ij.ra -- self.ij is always a real pico8_instance_t* REF or Lua-nil
 * (never mz.jj's own raw-table representation, since self.ox only ever
 * holds real hv entity instances, see pico8_mz_pq's own comment,
 * pico8_wormhole.h), so this needs no representation-bridging wrapper the
 * way self.et sometimes does. */
static pico8_vec_t pico8_mzd_field_ra_of_ij(const pico8_instance_t *self) {
    pico8_value_t ij_val;
    if (self == NULL || !pico8_instance_get_field(self, "ij", &ij_val) || ij_val.kind != PICO8_VALUE_REF ||
        ij_val.as.ref == NULL) {
        return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    }
    return pico8_mzd_field_vec((const pico8_instance_t *)ij_val.as.ref, "ra");
}

/* function mz:kj(p)if self.me=="kz" then
 * if not self.et then
 * circ(p.x,p.y,self.oi,1)end
 * local cp=self.ij.ra
 * line(cp.x,cp.y,p.x,p.y,1)spr(7,p.x-4,p.y-4)else
 * for dt=0,8,2 do
 * if self.t>dt then
 * local sr=self:ej(self.t-dt)circ(p.x,p.y,sr,self.kk[flr(sr/self.oi*8)])end
 * end
 * end
 * end
 * (pico8/lua.lua:782-793.) See pico8_wormhole_display.h's own comment on
 * this declaration for the full trace, including the real, faithfully-
 * reproduced `self.kk[0]` (Lua nil color) edge case this loop hits exactly
 * when `sr==0`. */
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
        /* "kz" branch: if not self.et then circ(p.x,p.y,self.oi,1) end --
         * the expanding "approaching" ring, suppressed once a target has
         * been found (self.et truthy). */
        pico8_value_t et_val;
        int has_et = pico8_instance_get_field(self, "et", &et_val) && pico8_mzd_truthy(et_val);
        pico8_vec_t cp;

        if (!has_et) {
            pico8_fixed_t oi = pico8_mzd_field_fixed(self, "oi");
            if (gfx != NULL && mem != NULL) {
                pico8_circ(gfx, mem, px, py, (int)pico8_fixed_to_double(oi), 1);
            }
        }

        /* local cp=self.ij.ra line(cp.x,cp.y,p.x,p.y,1)spr(7,p.x-4,p.y-4)
         * -- runs unconditionally (outside the `if not self.et` above),
         * still inside the "kz" branch: a line from the nearest reachable
         * .ox point toward this draw position, plus a sprite-7 icon. */
        cp = pico8_mzd_field_ra_of_ij(self);
        if (gfx != NULL && mem != NULL) {
            pico8_line(gfx, mem, (int)pico8_fixed_to_double(cp.x), (int)pico8_fixed_to_double(cp.y), px, py, 1);
            pico8_spr_default(gfx, mem, 7, px - 4, py - 4);
        }
    } else {
        /* else branch: up to 5 concentric "ripple" rings, dt=0,2,4,6,8,
         * each gated on self.t>dt, at radius self:ej(self.t-dt) and color
         * self.kk[flr(sr/self.oi*8)] -- see this file's own top "kk[0]"
         * edge-case trace above. */
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
                    pico8_fixed_t ratio = pico8_fixed_mul(pico8_fixed_div(sr, oi), pico8_fixed_from_int(8));
                    int idx = (int)pico8_fixed_to_double(pico8_fixed_flr(ratio));
                    pico8_value_t entry;
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
