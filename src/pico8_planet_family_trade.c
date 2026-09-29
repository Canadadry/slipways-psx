#include "pico8_planet_family_trade.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_list.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_probe.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_type_tag.h"
#include "pico8_ref_audit.h"

/* What `mx:kf` leaves in `ht` once it accepts a resource: `o()`. */
static const pico8_int_seq_t g_mx_ht_spent = {0, NULL};

static int g_family_trade_up = 0;

pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s) {
    int qu_not_self;
    int oe_count = 0;
    pico8_down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }

    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    qu_not_self = !(s != NULL && s->cls == pico8_jc_struct_class() &&
                    ((const pico8_jc_t *)s)->qu == &self->qh.jo.ke.header);

    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;

    out.stop = qu_not_self && oe_count < 3;
    return out;
}

static pico8_down_t pico8_nr_gv_dispatch(void *self, const pico8_struct_header_t *s) {
    return pico8_nr_gv((const pico8_nr_t *)self, s);
}

static int pico8_nr_ldown_empty_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    (void)name;
    (void)out_value;
    return 0;
}

static const pico8_struct_class_t g_nr_ldown_empty_struct_class = {
    .field = pico8_nr_ldown_empty_field,
};

typedef struct {
    pico8_struct_header_t header;
} pico8_nr_ldown_empty_t;

/* The `{}` `nr:ldown` hands to `nr:gv`: never allocated, never freed; no
 * field, and no writer, so a write reaching it stops the run. */
static const pico8_nr_ldown_empty_t g_nr_ldown_empty = {{PICO8_TYPE_TAG_STRUCT, &g_nr_ldown_empty_struct_class}};

pico8_down_t pico8_nr_ldown(pico8_nr_t *self) {
    pico8_down_t gv_result = {0, NULL};
    pico8_jc_keys_t keys = {0};

    if (self == NULL) {
        return gv_result;
    }

    gv_result = pico8_nr_gv(self, (const pico8_struct_header_t *)&g_nr_ldown_empty);

    if (!gv_result.stop) {
        return gv_result;
    }

    assert(pico8_jc_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    keys.qu = (pico8_struct_header_t *)self;
    gv_result.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
    gv_result.stop = gv_result.held != NULL;
    return gv_result;
}

static pico8_down_t pico8_nr_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_nr_ldown((pico8_nr_t *)self);
}

void pico8_nr_hb(pico8_nr_t *self) {
    int oe_count = 0;
    pico8_fixed_t s_val = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;

    if (self->hz != NULL) {
        int entry;
        if (pico8_int_seq_at(self->hz->items, self->hz->count, oe_count + 1, &entry)) {
            s_val = pico8_fixed_from_int(entry);
        }
    }

    self->qh.s = s_val;
}

static void pico8_nr_hb_dispatch(void *self) {
    pico8_nr_hb((pico8_nr_t *)self);
}

void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_trade_t *t;
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    oe = self->qh.jo.oe;

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_struct_header_t *w;
        const pico8_list_t *t_rl = NULL;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_struct_header_t *)w_val.as.ref;

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && t->rl != NULL) {
            t_rl = t->rl;
        }
        if (pico8_gu(t_rl, pico8_value_ref(w))) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_trade_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(w));
            pico8_struct_header_t *other;

            gf.t = extended;
            assert(w->cls == pico8_jc_struct_class() && "nr.oe holds slipways only: jo:kf adds to it");
            other = pico8_jc_eb((const pico8_jc_t *)w, (const pico8_struct_header_t *)self);
            /* `&gf` is a stack address passed to a dispatched `cn`: safe
             * only while no `cn` implementation stores its proposal. */
            if (other != NULL && other->cls->cn != NULL) {
                other->cls->cn(other, &gf);
            }
            lw->mu = gf.mu;
        }
    }
}

static void pico8_nr_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
    pico8_nr_cn((pico8_nr_t *)self, lw);
}

static int pico8_nr_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_nr_t *nr = self;

    if (strcmp(name, "hz") == 0) {
        if (nr->hz == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)nr->hz);
        return 1;
    }
    if (strcmp(name, "rr") == 0) {
        if (nr->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(nr->rr);
        return 1;
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

static void family_trade_take_fixed(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "qh.s: a write that is not a fixed does not fit the member's type -- stop, do not widen the member");
    *out = value.as.fixed;
}

/* `s`, which nr's own `hb` writes, then qh's 20 names; 0 for any other. */
static int pico8_nr_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "s") == 0) {
        family_trade_take_fixed(value, &((pico8_nr_t *)self)->qh.s);
        return 1;
    }
    return pico8_qh_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_nr_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_nr_hb_dispatch,
    .ldown = pico8_nr_ldown_dispatch,
    .gv = pico8_nr_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_nr_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .field = pico8_nr_field,
    .set_field = pico8_nr_set_field,
    .parent = pico8_qh_struct_class,
};

/* Initializes an allocated pico8_nr_t under `cls`: qh's part, nr's members
 * nil, nr's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_trade_init()`. */
static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls) {
    pico8_qh_struct_init(&self->qh, cls);
    self->hz = pico8_nr_start.hz;
    self->rr = pico8_nr_start.rr;
    self->qh.od = pico8_nr_start.od;
    self->qh.fp = pico8_nr_start.fp;
    self->qh.s = pico8_nr_start.s;
    self->qh.dw = pico8_nr_start.dw;
    self->qh.jo.nu = pico8_nr_start.nu;
    self->qh.jo.kq = &pico8_nr_start.kq;
}

static void *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_nr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_nr_struct_class);
    if (keys != NULL) {
        if (keys->id != NULL) {
            self->qh.jo.id = keys->id;
        }
        if (keys->p != NULL) {
            self->qh.jo.p = keys->p;
        }
        if (keys->ra != NULL) {
            self->qh.jo.ra = keys->ra;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    return self;
}

static int pico8_fr_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "mt") == 0) {
        *out_value = pico8_value_fixed(((const pico8_fr_t *)self)->mt);
        return 1;
    }
    return pico8_nr_field(self, name, out_value);
}

static const pico8_struct_class_t g_fr_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_nr_hb_dispatch,
    .ldown = pico8_nr_ldown_dispatch,
    .gv = pico8_nr_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_nr_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .field = pico8_fr_field,
    .set_field = pico8_nr_set_field,
    .parent = pico8_nr_struct_class,
};

static void *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_fr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(&self->nr, &g_fr_struct_class);
    self->mt = pico8_fr_start.mt;
    self->nr.hz = pico8_fr_start.hz;
    self->nr.rr = pico8_fr_start.rr;
    self->nr.qh.od = pico8_fr_start.od;
    self->nr.qh.s = pico8_fr_start.s;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "fr: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->nr.qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->nr.qh.jo.ke);
    return self;
}

void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
    pico8_struct_header_t *cp = NULL;

    if (self != NULL && sw != NULL) {
        assert(sw->cls == pico8_jc_struct_class() && "mx:kf: sw is the slipway jc:ey registers");
        cp = pico8_jc_eb((const pico8_jc_t *)sw, (const pico8_struct_header_t *)self);
    }

    if (cp != NULL) {
        const pico8_list_t *jy;
        pico8_list_iter_t it;
        pico8_value_t i_val;

        assert(pico8_jo_is_family(cp) && "mx:kf: the other end of a registered slipway is a planet");
        jy = ((const pico8_jo_t *)cp)->jy;

        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &i_val)) {
            const pico8_jy_t *i;
            pico8_fixed_t gc;

            if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                continue;
            }
            i = i_val.as.ref;

            gc = i->has_gc ? i->gc : pico8_fixed_from_int(0);
            if (gc != pico8_fixed_from_int(0)) {
                continue;
            }

            if (i->row.has_qe && self->ht != NULL && pico8_int_seq_has_fixed(self->ht, i->row.qe)) {
                pico8_kw_t *kw = self->hj.qh.jo.kw;
                if (kw != NULL) {
                    kw->has_qe = 1;
                    kw->qe = i->row.qe;
                }
                self->ht = &g_mx_ht_spent;
            }
        }
    }

    pico8_jo_kf((pico8_jo_t *)self, sw);
}

static void pico8_mx_kf_dispatch(void *self, pico8_struct_header_t *sw) {
    pico8_mx_kf((pico8_mx_t *)self, sw);
}

static int pico8_mx_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "ht") == 0) {
        const pico8_int_seq_t *ht = ((const pico8_mx_t *)self)->ht;
        if (ht == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ht);
        return 1;
    }
    return pico8_hj_struct_class()->field(self, name, out_value);
}

static void family_trade_take_seq(pico8_value_t value, const pico8_int_seq_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "mx.ht: a write neither nil nor an integer sequence ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

/* `ht`, which mx's own `kf` writes, then hj's 21 names; 0 for any other. */
static int pico8_mx_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "ht") == 0) {
        family_trade_take_seq(value, &((pico8_mx_t *)self)->ht);
        return 1;
    }
    return pico8_hj_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_mx_struct_class = {
    .kj = pico8_hj_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_mx_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .mm = pico8_g__mm,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .field = pico8_mx_field,
    .set_field = pico8_mx_set_field,
    .parent = pico8_hj_struct_class,
    .emitter_of = pico8_hj_emitter_of,
};

static void *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
    pico8_mx_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_mx_struct_class);
    self->ht = pico8_mx_start.ht;
    self->hj.rr = pico8_mx_start.rr;
    self->hj.emitter->py = pico8_mx_start.py;
    self->hj.emitter->ki = pico8_mx_start.ki;
    self->hj.emitter->pn = pico8_mx_start.pn;
    self->hj.emitter->pk = pico8_mx_start.pk;
    self->hj.emitter->pk_is_bool = pico8_mx_start.pk_is_bool;
    self->hj.emitter->fc = pico8_mx_start.fc;
    self->hj.emitter->k_ = pico8_mx_start.k_;
    self->hj.emitter->du = pico8_mx_start.du;
    self->hj.emitter->qo = pico8_mx_start.qo;
    self->hj.emitter->ro = pico8_mx_start.ro;
    self->hj.emitter->qg = pico8_mx_start.qg;
    self->hj.emitter->q_ = pico8_mx_start.q_;
    self->hj.qh.hp = pico8_mx_start.hp;
    self->hj.qh.s = pico8_mx_start.s;
    self->hj.qh.px = pico8_mx_start.px;
    self->hj.qh.od = pico8_mx_start.od;
    self->hj.qh.jo.nu = pico8_mx_start.nu;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "mx: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->hj.qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mx");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.ke);
    return self;
}

void pico8_planet_family_trade_init(void) {
    assert(pico8_qh_ready() && pico8_hj_ready() &&
           "pico8_planet_family_init() must run first");
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();
    g_family_trade_up = 1;
}

void pico8_planet_family_trade_shutdown(void) {
    g_family_trade_up = 0;
}

int pico8_nr_ready(void) {
    return g_family_trade_up;
}

static int pico8_nr_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_nr_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_nr_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_nr_start.od);
        return 1;
    }
    return pico8_qh_kind()->field(name, out_value);
}

static const pico8_entity_kind_t g_nr_kind = {pico8_nr_struct_new, pico8_nr_kind_field, &pico8_nr_kind_rec};

const pico8_entity_kind_t *pico8_nr_kind(void) {
    return &g_nr_kind;
}

const pico8_struct_class_t *pico8_nr_struct_class(void) {
    return &g_nr_struct_class;
}

pico8_nr_t *pico8_nr_new(const pico8_jo_keys_t *keys) {
    return pico8_nr_struct_new(keys);
}

int pico8_fr_ready(void) {
    return g_family_trade_up;
}

static int pico8_fr_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_fr_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_fr_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_fr_start.od);
        return 1;
    }
    return pico8_nr_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_fr_kind = {pico8_fr_struct_new, pico8_fr_kind_field, &pico8_fr_kind_rec};

const pico8_entity_kind_t *pico8_fr_kind(void) {
    return &g_fr_kind;
}

const pico8_struct_class_t *pico8_fr_struct_class(void) {
    return &g_fr_struct_class;
}

pico8_fr_t *pico8_fr_new(const pico8_jo_keys_t *keys) {
    return pico8_fr_struct_new(keys);
}

int pico8_mx_ready(void) {
    return g_family_trade_up;
}

static int pico8_mx_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_mx_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_mx_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_mx_start.od);
        return 1;
    }
    return pico8_hj_kind()->field(name, out_value);
}

static const pico8_entity_kind_t g_mx_kind = {pico8_mx_struct_new, pico8_mx_kind_field, &pico8_mx_kind_rec};

const pico8_entity_kind_t *pico8_mx_kind(void) {
    return &g_mx_kind;
}

const pico8_struct_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys) {
    return pico8_mx_struct_new(keys);
}
