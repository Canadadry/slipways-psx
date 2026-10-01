#include "pico8_research_icon.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_str.h"
#include "pico8_tech_gate.h"

static int g_ie_up = 0;

/* `ie.gy`, `ie.id`: mutated on the class by the cart, so one cursor and one
 * counter shared by every icon, never a member. `gy` is rebound, never
 * written through: an icon's `ra`, and `eh`, may hold its REF. Only `_init`
 * resets `gy`, only pico8_research_icon_init() resets `id`. */
static const pico8_vec_t *g_ie_gy = NULL;
static pico8_fixed_t g_ie_id = 0;
static int g_ie_has_id = 0;

static int ie_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* Returns a newly heap-allocated string the caller owns and must free(). */
static char *ie_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* Returns the kind held in `self.ke`, or NULL when the member is nil. The
 * result is borrowed. */
static const pico8_entity_kind_t *ie_self_ke_kind(const pico8_ie_t *self) {
    return self != NULL ? self->ke : NULL;
}

static const pico8_struct_class_t g_ie_ke_view_struct_class = {0};

pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self) {
    const pico8_entity_kind_t *kind = ie_self_ke_kind(self);
    pico8_ie_ke_t view;

    pico8_struct_header_init(&view.header, &g_ie_ke_view_struct_class);
    view.rr = NULL;
    view.s = 0;
    view.has_s = 0;
    view.me = NULL;
    view.od = 0;
    view.has_od = 0;
    if (kind != NULL) {
        view.rr = kind->rec->rr;
        view.s = kind->rec->s;
        view.has_s = kind->rec->has_s;
        view.me = kind->rec->me;
        view.od = kind->rec->od;
        view.has_od = kind->rec->has_od;
    }
    return view;
}

const pico8_struct_class_t *pico8_ie_ke_view_struct_class(void) {
    return &g_ie_ke_view_struct_class;
}

void pico8_ie_hb(pico8_ie_t *self) {
    const char *rr;

    if (self == NULL) {
        return;
    }

    if (self->has_kh && self->kh) {
        return;
    }

    /* `qh`/`os` never set `rr`, so `rr` stays absent for them and
     * pico8_oj() then answers true unconditionally. */
    rr = pico8_ie_ke_view(self).rr;

    if (!pico8_oj(rr)) {
        return;
    }

    assert(g_ie_up && "pico8_research_icon_init() must run before ie:hb writes ie.gy");

    /* `self.ra` points at the vector `ie.gy` points at now. The `-=` below rebinds
     * `ie.gy` to a fresh vector rather than writing through the old one, so
     * this snapshot survives it. */
    self->ra = g_ie_gy;
    self->kh = 1;
    self->has_kh = 1;

    {
        pico8_vec_t current =
            g_ie_gy != NULL ? *g_ie_gy : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        pico8_vec_t delta = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
        pico8_vec_t *heap = malloc(sizeof *heap);
        *heap = pico8_vec_sub(current, delta);
        g_ie_gy = heap;
    }
}

static void pico8_ie_hb_dispatch(void *self) {
    pico8_ie_hb((pico8_ie_t *)self);
}

pico8_down_t pico8_ie_ldown(pico8_ie_t *self) {
    pico8_fixed_t new_id;
    const pico8_entity_kind_t *ke_kind;
    char *id_num_str;
    char *id_str;
    pico8_jo_keys_t keys;
    pico8_down_t out = {0, NULL};

    assert(g_ie_up && "pico8_research_icon_init() must run before ie:ldown writes ie.id");
    new_id = pico8_fixed_add(g_ie_has_id ? g_ie_id : pico8_fixed_from_int(0),
                             pico8_fixed_from_int(1));
    g_ie_id = new_id;
    g_ie_has_id = 1;

    ke_kind = ie_self_ke_kind(self);
    if (ke_kind == NULL) {
        return out;
    }

    id_num_str = pico8_tostr_fixed(new_id);
    id_str = ie_concat2("s", id_num_str);
    free(id_num_str);

    /* `id_str` is not freed: the new planet's `id` keeps that exact
     * pointer. */
    keys.id = id_str;
    keys.p = NULL;
    keys.ra = NULL;
    out.held = ke_kind->new_entity(&keys);
    out.stop = out.held != NULL;
    return out;
}

static pico8_down_t pico8_ie_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_ie_ldown((pico8_ie_t *)self);
}

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec_t p) {
    pico8_ie_ke_t view;

    /* An icon not yet revealed by `ie:hb` draws nothing at all. */
    if (self == NULL || !(self->has_kh && self->kh)) {
        return;
    }
    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_palt(mem, 3, 0);

    view = pico8_ie_ke_view(self);
    pico8_spr_default(gfx, mem, view.has_s ? ie_to_int(view.s) : 0, ie_to_int(p.x),
                      ie_to_int(p.y));

    /* `dl(self.ke)` is not inert: every kind `self.ke` can hold inherits
     * `qh`'s `me="kz"`, so `dl` passes its own state check and draws the
     * kind-level build cost. */
    if (self->m_) {
        if (ie_self_ke_kind(self) != NULL) {
            pico8_probe_t pb = {.def = &pico8_probe_def_none};

            pb.has_od = view.has_od;
            pb.od = view.od;
            pico8_dl_show(gfx, mem, view.me, &pb);
        }
        if (self->jb != NULL) {
            pico8_jb_ol(gfx, mem, self->jb, pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        }
    }

    pico8_palt(mem, 3, 1);
}

/* `dc("kj")` calls `qc[nt](qc,qc.ra)`, so `ra` is always `self.ra`. */
static void pico8_ie_kj_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_ie_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ie_t *)self, p);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static int pico8_ie_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk(self, qa, ra);
}

static const pico8_struct_class_t g_ie_struct_class = {
    .kj = pico8_ie_kj_dispatch,
    .hd = pico8_ie_hd_dispatch,
    .hb = pico8_ie_hb_dispatch,
    .ldown = pico8_ie_ldown_dispatch,
};

static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
    pico8_ie_t *self = malloc(sizeof *self);

    assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
    pico8_ke_struct_init(&self->header, &g_ie_struct_class, pico8_ie_def.me, pico8_ie_def.t);
    self->def = &pico8_ie_def;
    self->header.fa = &self->def->fa;
    self->header.ra = &self->ra;
    self->header.kq = &self->def->kq;
    self->ra = &self->def->ra;
    self->ke = NULL;
    self->jb = NULL;
    self->kh = 0;
    self->has_kh = 0;
    self->header.m_ = &self->m_;
    self->m_ = 0;
    if (keys != NULL) {
        self->ke = keys->ke;
        self->jb = keys->jb;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ie");
#endif
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_research_icon_init(void) {
    pico8_research_icon_shutdown();

    pico8_pi_init();

    g_ie_up = 1;
    g_ie_gy = NULL;
    g_ie_id = pico8_fixed_from_int(0);
    g_ie_has_id = 1;
}

/* TROU-7 */
void pico8_research_icon_shutdown(void) {
    g_ie_up = 0;
    g_ie_gy = NULL;
}

int pico8_ie_ready(void) {
    return g_ie_up;
}

const pico8_struct_class_t *pico8_ie_struct_class(void) {
    return &g_ie_struct_class;
}

pico8_ie_t *pico8_ie_new(const pico8_ie_keys_t *keys) {
    return pico8_ie_struct_new(keys);
}

const pico8_vec_t *pico8_ie_eh(void) {
    return g_ie_up ? &pico8_ie_def.eh : NULL;
}

const pico8_vec_t *pico8_ie_gy(void) {
    return g_ie_gy;
}

int pico8_ie_id(pico8_fixed_t *out) {
    if (!g_ie_has_id) {
        return 0;
    }
    *out = g_ie_id;
    return 1;
}

void pico8_ie_set_gy(const pico8_vec_t *gy) {
    g_ie_gy = gy;
}
