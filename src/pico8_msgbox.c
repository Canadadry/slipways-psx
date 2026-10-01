#include "pico8_msgbox.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_fixed.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_scroll.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"
#include "pico8_zone_transitoire.h"

static int g_mh_up = 0;
static int g_iv_up = 0;
static pico8_mh_t *g_kc = NULL;

pico8_mh_t *pico8_kc(void) {
    return g_kc;
}

void pico8_msgbox_set_kc(pico8_mh_t *mh) {
    g_kc = mh;
}

void pico8_mh_pq(pico8_mh_t *self) {
    pico8_mh_t *previous;

    if (self == NULL) {
        return;
    }

    previous = pico8_kc();
    if (previous != NULL) {
        previous->p_ = 1;
    }
    pico8_msgbox_set_kc(self);
}

static void pico8_mh_ev_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_mh_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mh_t *)self);
}

void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self) {
    pico8_fixed_t self_t, rh, t, nc, t3;

    if (self == NULL) {
        return;
    }

    self_t = self->header.t;
    rh = self->rh;
    t = pico8_fixed_sub(pico8_fixed_div(self_t, rh), pico8_fixed_from_int(10));

    if (t >= 0) {
        if (self->lh != NULL && self->lh(self)) {
            self->header.t = pico8_fixed_mul(rh, pico8_fixed_from_int(10));
            t = pico8_fixed_from_int(0);
        }
    }

    /* `-t^3/9` is `(-(t^3))/9`: in Lua `^` binds tighter than unary `-`,
     * which in turn binds tighter than `/`. */
    t3 = pico8_fixed_mul(pico8_fixed_mul(t, t), t);
    nc = pico8_fixed_div(pico8_fixed_neg(t3), pico8_fixed_from_int(9));

    if (gfx != NULL && mem != NULL) {
        pico8_bl(gfx, mem, self->items, self->items_count, nc);
        pico8_camera_reset(gfx);
    }

    if (t > PICO8_FIXED_LIT(0x000a8000, 10.5)) {
        self->p_ = 1;
        pico8_msgbox_set_kc(NULL);
        assert(pico8_kc() == NULL && "`self.p_,kc=true` is one value for two targets: kc gets nil, not true");
    }
}

static const pico8_struct_class_t g_mh_struct_class = {
    .ev = pico8_mh_ev_dispatch,
    .tags = PICO8_TAG(PICO8_TAG_MH),
};

static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
    pico8_mh_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->header, &g_mh_struct_class, pico8_mh_def.me, pico8_mh_def.t);
    self->def = &pico8_mh_def;
    self->header.fa = &self->def->fa;
    self->rh = self->def->rh;
    self->lh = NULL;
    self->header.p_ = &self->p_;
    self->p_ = 0;
    self->items_count = 0;
    if (keys != NULL) {
        assert(keys->count <= PICO8_MH_ITEMS_CAP &&
               "the cart's two mh() are pj:hk (3 articles at most) and pj:ph (one per bz row): a third needs more");
        for (i = 0; i < keys->count; i++) {
            self->items[self->items_count++] = keys->items[i];
        }
        if (keys->has_rh) {
            self->rh = keys->rh;
        }
        self->lh = keys->lh;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mh");
#endif
    pico8_mh_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_mh_init(void) {
    pico8_mh_shutdown();
    g_mh_up = 1;
}

void pico8_mh_shutdown(void) {
    g_mh_up = 0;
    pico8_msgbox_set_kc(NULL);
}

int pico8_mh_ready(void) {
    return g_mh_up;
}

pico8_mh_t *pico8_mh_new(const pico8_mh_keys_t *keys) {
    return pico8_mh_struct_new(keys);
}

static pico8_vec_t pico8_iv_vec_or_zero(const pico8_vec_t *p) {
    if (p != NULL) {
        return *p;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

void pico8_iv_pq(pico8_iv_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->ra == NULL) {
        const pico8_nf_t *je = pico8_je_nf();
        const pico8_qb_t *mo = pico8_mo_qb();
        pico8_vec_t je_ra = pico8_iv_vec_or_zero(je != NULL ? je->ra : NULL);
        pico8_vec_t mo_ra = pico8_iv_vec_or_zero(mo != NULL ? mo->ra : NULL);
        pico8_vec_t offset = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(5));
        pico8_vec_t new_ra = pico8_vec_sub(pico8_vec_add(je_ra, mo_ra), offset);
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        self->ra = heap_ra;
    }
}

void pico8_iv_ph(pico8_iv_t *self) {
    pico8_vec_t new_ra;
    pico8_fixed_t new_l;
    pico8_vec_t *heap_ra;

    if (self == NULL) {
        return;
    }

    new_ra = pico8_vec_add(pico8_iv_vec_or_zero(self->ra), pico8_iv_vec_or_zero(&self->def->v));
    /* TROU-7 */
    heap_ra = malloc(sizeof *heap_ra);
    *heap_ra = new_ra;
    self->ra = heap_ra;

    new_l = pico8_fixed_add(self->l, PICO8_FIXED_LIT(0x00001999, 0.1));
    self->l = new_l;

    self->p_ = new_l >= pico8_fixed_from_int(5);
}

static void pico8_iv_ph_dispatch(pico8_struct_header_t *self, pico8_fixed_t t) {
    (void)t;
    pico8_iv_ph((pico8_iv_t *)self);
}

void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_iv_t *self, pico8_vec_t p) {
    const char *text;
    int color;
    pico8_fixed_t a_center = PICO8_FIXED_LIT(0x00008000, 0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    pico8_dv(mem, self->l);

    text = self->at1 != NULL ? self->at1 : "";
    color = pico8_fixed_flr_int(self->at2);

    pico8_io(gfx, mem, text, p.x, p.y, color, &a_center);

    pico8_dv_default(mem);
}

static void pico8_iv_kj_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_iv_t *)self, p);
}

static const pico8_struct_class_t g_iv_struct_class = {
    .kj = pico8_iv_kj_dispatch,
    .states = {[PICO8_STATE_PH] = pico8_iv_ph_dispatch},
};

static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
    pico8_iv_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->header, &g_iv_struct_class, pico8_iv_def.me, pico8_iv_def.t);
    self->def = &pico8_iv_def;
    self->header.fa = &self->def->fa;
    self->header.ra = &self->ra;
    self->l = self->def->l;
    self->ra = NULL;
    self->header.p_ = &self->p_;
    self->p_ = 0;
    self->at1 = NULL;
    self->at2 = pico8_fixed_from_int(0);
    if (keys != NULL) {
        self->ra = keys->ra;
        self->at1 = keys->at1;
        self->at2 = keys->at2;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("iv");
#endif
    pico8_iv_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_iv_init(void) {
    pico8_iv_shutdown();
    pico8_pi_init();
    g_iv_up = 1;
}

void pico8_iv_shutdown(void) {
    g_iv_up = 0;
}

int pico8_iv_ready(void) {
    return g_iv_up;
}

pico8_iv_t *pico8_iv_new(const pico8_iv_keys_t *keys) {
    return pico8_iv_struct_new(keys);
}

typedef struct {
    const pico8_pz_frame_t *frame;
    pico8_vec_t bp;
    pico8_vec_t *ra_heap;
} pico8_pz_ctx_t;

/* One row of `bs` onto a copy of its keys: the shared `ra`, its own `qf`,
 * `fa` from `p.ca`; then the button. */
static pico8_js_t *pico8_pz_step(pico8_pz_ctx_t *ctx, const pico8_js_keys_t *row) {
    pico8_js_keys_t b = *row;
    pico8_vec_t gq;

    /* Every item's "ra" is the same vector object, as in Lua -- not a copy
     * per item. */
    b.ra = ctx->ra_heap;

    b.qf = malloc(sizeof *b.qf);
    *b.qf = ctx->bp;

    /* TROU-9 */
    if (ctx->frame->has_ca) {
        b.fa = ctx->frame->ca;
        b.has_fa = 1;
    }

    gq = ctx->frame->gq != NULL ? *ctx->frame->gq : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    ctx->bp = pico8_vec_add(ctx->bp, gq);

    return pico8_js_new(&b);
}

void pico8_pz(const pico8_pz_frame_t *frame, const pico8_js_keys_t *bs, int n) {
    pico8_lq_cell_t ep_cell;
    pico8_vec_t ra;
    pico8_pz_ctx_t ctx;
    pico8_ents_t *le_heap;
    pico8_ui_t *ok;
    pico8_fixed_t rn_arg;
    pico8_js_keys_t ep_row = {0};
    pico8_lq_keys_t ep_lq = {0};
    int has_ep;
    int i;

    if (frame == NULL) {
        return;
    }

    has_ep = frame->ep != NULL;
    if (has_ep) {
        ep_cell = pico8_lq_cell_string(frame->ep);
        ep_lq.items = &ep_cell;
        ep_lq.count = 1;
        ep_lq.has_c = 1;
        ep_lq.c = pico8_fixed_from_int(13);
        ep_row.lq_row = &ep_lq;
    }

    if (frame->has_eo) {
        rn_arg = frame->eo;
    } else {
        rn_arg = pico8_fixed_from_int(62);
    }
    pico8_rn(rn_arg);

    if (frame->hi != NULL) {
        ra = *frame->hi;
    } else if (frame->ra != NULL) {
        ra = *frame->ra;
    } else {
        ra = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    }

    ctx.frame = frame;
    ctx.bp = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)));
    ctx.ra_heap = malloc(sizeof *ctx.ra_heap);
    *ctx.ra_heap = ra;

    ok = pico8_ok();
    pico8_ui_cj(ok);

    /* TROU-7 */
    le_heap = malloc(sizeof *le_heap);
    pico8_ents_init(le_heap, &pico8_zone_transitoire);
    if (has_ep) {
        pico8_ents_add(le_heap, &pico8_pz_step(&ctx, &ep_row)->header);
    }
    for (i = 0; i < n; i++) {
        pico8_ents_add(le_heap, &pico8_pz_step(&ctx, &bs[i])->header);
    }
    if (ok != NULL) {
        ok->le = le_heap;
    } else {
        pico8_ents_free(le_heap);
        free(le_heap);
    }

    /* TROU-7 */
    /* The `js` buttons built just above stay registered in `hv`/`cv`/`cg`
     * once `le` no longer holds them, exactly as in the cart. */
    if (frame->ff && ok != NULL) {
        pico8_ents_t *empty_heap = malloc(sizeof *empty_heap);
        pico8_ents_init(empty_heap, &pico8_zone_transitoire);
        ok->le = empty_heap;
    }
}
