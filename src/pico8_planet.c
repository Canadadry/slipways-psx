#include "pico8_planet.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_economy.h"
#include "pico8_entity_geom.h"
#include "pico8_fm.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_research.h"
#include "pico8_tech_gate.h"

static int g_jo_up = 0;

static pico8_gn_state_t *g_jo_oc = NULL;
static pico8_jf_state_t *g_jo_ir = NULL;

/* Shared by every bare planet: written in place by `bb` and `mx:kf` until
 * `lz` gives the planet its own. */
static pico8_kw_t g_jo_kw_shared;

int pico8_jo_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk(self, qa, ra);
}

void pico8_jo_hk_dispatch(pico8_struct_header_t *self, pico8_fixed_t yr) {
    (void)yr;
    pico8_jo_hk((const pico8_jo_t *)self);
}

int pico8_jo_is_family(const pico8_struct_header_t *e) {
    return (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_JO)) != 0;
}

void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
    pico8_jo_ll((pico8_jo_t *)self, t);
}

static const pico8_struct_class_t g_jo_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
    .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),
};

void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
    assert(g_jo_up && "pico8_jo_init() must run first");
    pico8_ke_struct_init(&self->header, cls, def->me, def->t);
    self->def = def;
    self->header.fa = &self->def->fa;
    /* Read-only: the planet modules write through their members. */
    self->header.ra = (const pico8_vec_t *const *)&self->ra;
    self->header.p = (const pico8_vec_t *const *)&self->p;
    self->header.kq = &self->def->kq;
    self->header.ma = &self->ma;
    self->header.fx = &self->fx;
    self->header.p_ = &self->p_;
    self->ma = self->def->ma;
    self->kw = &g_jo_kw_shared;
    self->id = NULL;
    self->p = NULL;
    self->ra = NULL;
    self->oe = NULL;
    self->hn = NULL;
    self->ep = NULL;
    self->jy = NULL;
    self->nk = NULL;
    self->fx = 0;
    self->lq = NULL;
    self->fo = NULL;
    self->jh = 0;
    self->has_jh = 0;
    self->p_ = 0;
    self->kr = 0;
    self->has_kr = 0;
    self->cc = NULL;
    self->by = NULL;
    self->mi = self->def->mi;
    self->has_mi = self->def->has_mi;
    self->header.m_ = &self->m_;
    self->m_ = 0;
    self->kw_own = (pico8_kw_t){0};
    self->probe = NULL;
    self->emitter = NULL;
}

static pico8_jo_t *pico8_jo_struct_new(const pico8_jo_keys_t *keys) {
    pico8_jo_t *self = malloc(sizeof *self);

    pico8_jo_struct_init(self, &g_jo_struct_class, &pico8_jo_def);
    if (keys != NULL) {
        if (keys->p != NULL) {
            self->p = keys->p;
        }
        if (keys->ra != NULL) {
            self->ra = keys->ra;
        }
        if (keys->id != NULL) {
            self->id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jo");
#endif
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_up = 1;
    g_jo_kw_shared = (pico8_kw_t){0};
}

void pico8_jo_shutdown(void) {
    g_jo_up = 0;
    if (g_jo_oc != NULL) {
        pico8_gn_free(g_jo_oc);
        g_jo_oc = NULL;
    }
    if (g_jo_ir != NULL) {
        pico8_jf_free(g_jo_ir);
        g_jo_ir = NULL;
    }
}

void pico8_jo_set_oc(pico8_gn_state_t *oc) {
    if (g_jo_oc == oc) {
        return;
    }
    pico8_gn_free(g_jo_oc);
    g_jo_oc = oc;
}

void pico8_jo_set_ir(pico8_jf_state_t *ir) {
    if (g_jo_ir == ir) {
        return;
    }
    pico8_jf_free(g_jo_ir);
    g_jo_ir = ir;
}

int pico8_jo_ready(void) {
    return g_jo_up;
}

const pico8_jb_row_t *pico8_jo_dz(void) {
    return pico8_jo_dz_rows;
}

const pico8_int_seq_t *pico8_jo_cq(void) {
    return &pico8_jo_cq_seq;
}

const pico8_int_seq_t *pico8_jo_ka(void) {
    return &pico8_jo_ka_seq;
}

const pico8_jb_row_t *pico8_jo_em(void) {
    return &pico8_jo_em_row;
}

pico8_kw_t *pico8_jo_shared_kw(void) {
    return &g_jo_kw_shared;
}

const pico8_struct_class_t *pico8_jo_struct_class(void) {
    return &g_jo_struct_class;
}

pico8_jo_t *pico8_jo_new(const pico8_jo_keys_t *keys) {
    return pico8_jo_struct_new(keys);
}

void pico8_jo_pq(pico8_jo_t *self) {
    pico8_ents_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_ents_init(oe);
    self->oe = oe;

    if (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) {
        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(self->def->hp));

        if (target != NULL) {
            pico8_jo_lz(self, target);
        }
    }
}

/* `mw(i)`: a copy of the want, never shared with `hp.i`. */
static int pico8_jo_lz_mw_step(void *e, void *ctx, void **out) {
    (void)ctx;
    if (e != NULL) {
        pico8_jy_t *copy = malloc(sizeof *copy);
        const pico8_jy_t *i = e;

        assert(copy != NULL);
        *copy = *i;
        *out = copy;
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    pico8_ptrs_t jy;
    pico8_ptrs_t *jy_heap;

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }

    self->hn = hp_instance;
    self->ep = NULL;

    pico8_ptrs_init(&jy);
    if (hp_instance != NULL && hp_instance->i != NULL) {
        jy = pico8_nv_ptrs(hp_instance->i, pico8_jo_lz_mw_step, NULL);
    }
    jy_heap = malloc(sizeof *jy_heap);
    *jy_heap = jy;
    self->jy = jy_heap;

    self->kw_own.has_qe = hp_instance != NULL && hp_instance->has_o;
    self->kw_own.qe = self->kw_own.has_qe ? hp_instance->o : 0;
    self->kw_own.mu = pico8_fixed_from_int(0);
    self->kw = &self->kw_own;
}

int pico8_jo_gg(const pico8_jo_t *self) {
    if (self == NULL) {
        return 0;
    }
    if (!self->def->hf) {
        return 0;
    }
    return self->header.me != NULL && strcmp(self->header.me, "ly") == 0;
}

void pico8_jo_kd(pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    self->ma = PICO8_FIXED_LIT(0x00014000, 1.25);
}

void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    self->p_ = t > pico8_fixed_from_int(60);
}

void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    pico8_jo_t *working_self;
    pico8_fixed_t fp = pico8_fixed_from_int(2);

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }
    working_self = self;

    if (hp_instance != NULL && hp_instance->cw != NULL) {
        const pico8_entity_kind_t *target_kind = NULL;
        pico8_jo_keys_t keys;

        self->p_ = 1;

        target_kind = pico8_jo_kv(hp_instance->cw);

        keys.p = self->p;
        keys.ra = self->ra;
        keys.id = self->id;

        working_self = (target_kind != NULL) ? (pico8_jo_t *)target_kind->new_entity(&keys) : NULL;
    } else if (hp_instance != NULL && hp_instance->go != NULL) {
        self->nk = pico8_df_lookup(hp_instance->go);
    } else {
        pico8_ke_ks((pico8_struct_header_t *)self, "ly");
        pico8_jo_lz(self, hp_instance);
        self->fx = pico8_fixed_from_int(1);
        fp = pico8_ov("autoassemblers", pico8_fixed_from_int(3), pico8_fixed_from_int(2));
    }

    pico8_hy_nx(pico8_fm_hy(), fp);

    if (hp_instance != NULL && hp_instance->has_rn) {
        pico8_rn(hp_instance->rn);
    } else {
        pico8_rn(pico8_fixed_from_int(55));
    }

    pico8_jo_kd(working_self);
}

void pico8_jo_hh(pico8_jo_t *self) {
    const pico8_df_entry_t *nk = NULL;
    const char *ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        pico8_fixed_t roll = pico8_jf_next(g_jo_ir);
        int use_ll = 0;

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < PICO8_FIXED_LIT(0x00007333, 0.45)) {
            use_ll = pico8_df_lookup("ll") != NULL;
        }
        if (use_ll) {
            nk = pico8_df_lookup("ll");
        } else if (g_jo_oc != NULL) {
            nk = pico8_gn_next(g_jo_oc);
        }
    }
    self->nk = nk;
    self->ep = nk != NULL ? nk->d : NULL;

    if (nk != NULL && nk->me != NULL) {
        ks_arg = nk->me;
    } else {
        ks_arg = "known";
    }
    pico8_ke_ks((pico8_struct_header_t *)self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    if (!self->has_kr || self->kr == pico8_fixed_from_int(0)) {
        return;
    }
    if (!pico8_gd((const pico8_struct_header_t *)self)) {
        return;
    }
    {
        pico8_iv_keys_t keys = {0};
        int color = 0;
        char *text;

        keys.ra = self->ra;
        text = pico8_ne(self->kr, 10, "$", &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        keys.at1 = text;
        keys.at2 = pico8_fixed_from_int(color);
        pico8_iv_new(&keys);
        /* `text` is now owned by the new `iv` instance, which reads it every
         * frame it stays on screen: not freed here. */
    }
}

void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
    pico8_vec_t *ra_vec;

    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
    if (ra_vec != NULL) {
        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
    }
}
