#include "game/slipway_display.h"

#include <assert.h>
#include <stddef.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "game/planet.h"
#include "game/slipway.h"
#include "engine/vec.h"
#include "game/particle_zone.h"

typedef char pico8_ts_tient_sa_place[sizeof(pico8_ts_t) <= PICO8_TS_PLACE ? 1 : -1];

static int pico8_jc_display_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

void pico8_jc_ch(pico8_jc_t *self, const entity_t *f, const entity_t *t) {
    if (self == NULL) {
        return;
    }
    /* The random draw happens before the `hg` test, as in Lua: swapping the
     * two would consume the RNG differently and desynchronise every replay. */
    if (p8_fixed_rnd_default() >= P8_FIXED_LIT(0x0000147a, 0.08)) {
        return;
    }

    if (self->hg != NULL) {
        /* The mouse point jc:hq hangs on `re` has no `id`. */
        const char *t_id = t != NULL && pico8_jo_is_family(t) ? ((const pico8_jo_t *)t)->id : NULL;
        if (pico8_jc_hg_has(self->hg, t_id)) {
            if (self->ts != NULL) {
                pico8_vec_t f_ra = pico8_jc_end_ra(f);
                pico8_vec_t t_ra = pico8_jc_end_ra(t);
                pico8_vec_t d = pico8_vec_sub(t_ra, f_ra);
                pico8_vec_t v = pico8_vec_nn(d);
                pico8_ts_t *particle = (pico8_ts_t *)pico8_zone_alloc(pico8_zone_particules(PICO8_PARTICULES_TS),
                                                                      sizeof *particle);

                particle->ra = pico8_vec_add(f_ra, pico8_vec_pa(v));
                particle->v = v;
                particle->l = pico8_vec_qx(d);

                pico8_ptrs_add(self->ts, particle);
                pico8_zone_particules_noter_elements(PICO8_PARTICULES_TS, self->ts->count);
            }
        }
    }
}

void pico8_jc_lg(const p8_graphics_t *gfx, p8_memory_t *mem, const pico8_jc_t *self, p8_fixed_t l,
                  p8_fixed_t h, p8_fixed_t s, int c) {
    entity_t *qu;
    entity_t *re;
    p8_fixed_t t;
    pico8_vec_t qu_pt;
    pico8_vec_t re_pt;
    p8_fixed_t dx;

    if (self == NULL) {
        return;
    }

    assert(s > p8_fixed_from_int(0) && "jc:lg only ascends: a zero step hangs, a negative one draws nothing");
    assert(gfx != NULL && mem != NULL && "pico8_line dereferences gfx and mem: neither may be NULL here");

    qu = self->qu;
    re = self->re;
    t = self->header.state_time;

    qu_pt = pico8_vec_add(pico8_jc_end_ra(qu),
                           pico8_vec_qm(P8_FIXED_LIT(0x00018000, 1.5), p8_fixed_div(t, p8_fixed_from_int(126))));
    re_pt = pico8_vec_add(pico8_jc_end_ra(re),
                           pico8_vec_qm(P8_FIXED_LIT(0x00018000, 1.5), p8_fixed_div(t, p8_fixed_from_int(176))));

    for (dx = l; dx <= h; dx = p8_fixed_add(dx, s)) {
        p8_fixed_t dy;
        for (dy = l; dy <= h; dy = p8_fixed_add(dy, s)) {
            int x1 = pico8_jc_display_to_int(p8_fixed_add(qu_pt.x, dx));
            int y1 = pico8_jc_display_to_int(p8_fixed_add(qu_pt.y, dy));
            int x2 = pico8_jc_display_to_int(p8_fixed_add(re_pt.x, dx));
            int y2 = pico8_jc_display_to_int(p8_fixed_add(re_pt.y, dy));
            p8_line(gfx, mem, x1, y1, x2, y2, c);
        }
    }
}

void pico8_jc_kj(const p8_graphics_t *gfx, p8_memory_t *mem, pico8_jc_t *self) {
    entity_t *qu;
    entity_t *re;

    if (self == NULL) {
        return;
    }

    qu = self->qu;
    re = self->re;

    if (pico8_gd(qu) || pico8_gd(re)) {
        p8_fixed_t nh = self->nh;
        int nh_i = pico8_jc_display_to_int(nh);
        p8_fixed_t jv = p8_fixed_from_int(0);
        int et_or_fo;
        int mt_truthy;
        p8_fixed_t c;
        p8_fixed_t bc;
        p8_fixed_t tc;

        assert(nh_i >= 1 && "k_ is 1-based: an nh of 0 would silently draw the slipway black");

        if (self->def->k_ != NULL) {
            int entry;
            if (pico8_int_seq_at(self->def->k_->items, self->def->k_->count, nh_i, &entry)) {
                jv = p8_fixed_from_int(entry);
            }
        }

        if (nh < p8_fixed_from_int(20)) {
            self->nh = p8_fixed_add(nh, p8_fixed_from_int(1));
        }

        et_or_fo = self->probe->et != NULL || self->fo != NULL;
        mt_truthy = self->has_mt;

        c = et_or_fo ? p8_fixed_from_int(2) : (mt_truthy ? p8_fixed_from_int(0) : jv);
        bc = mt_truthy ? jv : p8_fixed_from_int(0);
        tc = mt_truthy ? p8_fixed_from_int(5) : p8_fixed_from_int(13);

        pico8_jc_lg(gfx, mem, self, p8_fixed_from_int(-1), p8_fixed_from_int(2), p8_fixed_from_int(3),
                    pico8_jc_display_to_int(bc));
        pico8_jc_lg(gfx, mem, self, p8_fixed_from_int(0), p8_fixed_from_int(1), p8_fixed_from_int(1),
                    pico8_jc_display_to_int(c));

        {
            pico8_ptrs_t *ts = self->ts;
            if (ts != NULL) {
                int i;
                for (i = 0; i < ts->count; i++) {
                    pico8_ts_t *t = ts->items[i];
                    pico8_vec_t new_ra;
                    p8_fixed_t new_l;

                    if (t == NULL) {
                        continue;
                    }

                    new_ra = pico8_vec_add(t->ra, t->v);
                    t->ra = new_ra;

                    new_l = p8_fixed_sub(t->l, p8_fixed_from_int(1));
                    t->l = new_l;

                    /* No `else` in the Lua: an expired particle is removed
                     * and still drawn once more at its final position. */
                    if (new_l <= p8_fixed_from_int(0)) {
                        pico8_ptrs_del(ts, t);
                        pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_TS), (unsigned char *)t);
                    }

                    {
                        int x = pico8_jc_display_to_int(new_ra.x);
                        int y = pico8_jc_display_to_int(new_ra.y);
                        p8_rectfill(gfx, mem, x, y, x + 1, y + 1, 0);
                        p8_pset(gfx, mem, x, y, pico8_jc_display_to_int(tc));
                    }
                }
            }
        }

        pico8_jc_ch(self, qu, re);
        pico8_jc_ch(self, re, qu);
    }

    self->fo = NULL;
}

void pico8_jc_kj_dispatch(entity_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
}
