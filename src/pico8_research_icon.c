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
#include "pico8_tech_gate.h"

static int g_ie_up = 0;

/* `ie.gy`, `ie.id`: mutated on the class by the cart, so one cursor and one
 * counter shared by every icon, never a member. `gy` is rebound, never
 * written through: an icon's `ra`, and `eh`, may hold its REF. Only `_init`
 * resets `gy`, only pico8_research_icon_init() resets `id`. */
static const pico8_vec_t *g_ie_gy = NULL;
static pico8_value_t g_ie_id = {PICO8_VALUE_REF, {.ref = NULL}};

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

static int pico8_ie_ke_view_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ie_ke_t *view = self;

    if (strcmp(name, "rr") == 0) {
        if (view->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(view->rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        if (!view->has_s) {
            return 0;
        }
        *out_value = pico8_value_fixed(view->s);
        return 1;
    }
    if (strcmp(name, "me") == 0) {
        if (view->me == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(view->me);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        if (!view->has_od) {
            return 0;
        }
        *out_value = pico8_value_fixed(view->od);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_ie_ke_view_struct_class = {
    .field = pico8_ie_ke_view_field,
};

pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self) {
    const pico8_entity_kind_t *kind = ie_self_ke_kind(self);
    pico8_ie_ke_t view;

    view.header.tag = PICO8_TYPE_TAG_STRUCT;
    view.header.cls = &g_ie_ke_view_struct_class;
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

    if (!pico8_oj(rr != NULL ? pico8_value_string(rr) : pico8_value_ref(NULL))) {
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
    new_id = pico8_fixed_add(g_ie_id.kind == PICO8_VALUE_FIXED ? g_ie_id.as.fixed : pico8_fixed_from_int(0),
                             pico8_fixed_from_int(1));
    g_ie_id = pico8_value_fixed(new_id);

    ke_kind = ie_self_ke_kind(self);
    if (ke_kind == NULL) {
        return out;
    }

    id_num_str = pico8_value_tostr(pico8_value_fixed(new_id));
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
    if (self->has_m_ && self->m_) {
        if (ie_self_ke_kind(self) != NULL) {
            pico8_probe_t pb = {0};

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
static void pico8_ie_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_ie_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ie_t *)self, p);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
}

/* `gy` and `id` read off an icon answer the shared state, as the class
 * chain did. */
static int pico8_ie_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ie_t *ie = self;
    pico8_value_t value;

    if (strcmp(name, "gy") == 0) {
        value = pico8_value_ref((void *)g_ie_gy);
    } else if (strcmp(name, "id") == 0) {
        value = g_ie_id;
    } else if (strcmp(name, "kq") == 0) {
        if (ie->kq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ie->kq);
        return 1;
    } else if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(ie->fa);
        return 1;
    } else if (strcmp(name, "ra") == 0) {
        if (ie->ra == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ie->ra);
        return 1;
    } else if (strcmp(name, "eh") == 0) {
        if (ie->eh == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ie->eh);
        return 1;
    } else if (strcmp(name, "ke") == 0) {
        if (ie->ke == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ie->ke);
        return 1;
    } else if (strcmp(name, "jb") == 0) {
        if (ie->jb == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(ie->jb);
        return 1;
    } else if (strcmp(name, "kh") == 0) {
        if (!ie->has_kh) {
            return 0;
        }
        *out_value = pico8_value_bool(ie->kh);
        return 1;
    } else if (strcmp(name, "m_") == 0) {
        if (!ie->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(ie->m_);
        return 1;
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static void ie_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "ie.m_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean;
    *has = 1;
}

/* `m_` (the hover loop, on `ok.o_` and `ok.gj`): the only name a generic
 * writer ever writes on an `ie`. */
static int pico8_ie_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_ie_t *ie = self;

    if (strcmp(name, "m_") == 0) {
        ie_take_bool(value, &ie->m_, &ie->has_m_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_ie_struct_class = {
    .kj = pico8_ie_kj_dispatch,
    .hd = pico8_ie_hd_dispatch,
    .hb = pico8_ie_hb_dispatch,
    .ldown = pico8_ie_ldown_dispatch,
    .field = pico8_ie_field,
    .set_field = pico8_ie_set_field,
};

static void *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
    pico8_ie_t *self = malloc(sizeof *self);

    assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
    pico8_ke_struct_init(&self->ke_base, &g_ie_struct_class);
    self->kq = &pico8_ie_start.kq;
    self->fa = pico8_ie_start.fa;
    self->ra = &pico8_ie_start.ra;
    self->eh = &pico8_ie_start.eh;
    self->ke = NULL;
    self->jb = NULL;
    self->kh = 0;
    self->has_kh = 0;
    self->m_ = 0;
    self->has_m_ = 0;
    if (keys != NULL) {
        self->ke = keys->ke;
        self->jb = keys->jb;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ie");
#endif
    pico8_ke_struct_pq(&self->ke_base);
    return self;
}

void pico8_research_icon_init(void) {
    pico8_research_icon_shutdown();

    pico8_pi_init();

    g_ie_up = 1;
    g_ie_gy = NULL;
    g_ie_id = pico8_value_fixed(pico8_ie_start.id);
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
    return g_ie_up ? &pico8_ie_start.eh : NULL;
}

void pico8_ie_set_gy(const pico8_vec_t *gy) {
    g_ie_gy = gy;
}
