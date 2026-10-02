#include "game/structure_icon.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "game/probe.h"
#include "p8/str.h"
#include "game/tech_gate.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

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

/* Returns the kind held in `self.ke`, or NULL when the member is nil. The
 * result is borrowed. */
static const entity_kind_t *ie_self_ke_kind(const pico8_ie_t *self) {
    return self != NULL ? self->ke : NULL;
}

static const entity_class_t g_ie_ke_view_struct_class = {0};

pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self) {
    const entity_kind_t *kind = ie_self_ke_kind(self);
    pico8_ie_ke_t view;

    entity_init_header(&view.header, &g_ie_ke_view_struct_class);
    view.rr = NULL;
    view.s = 0;
    view.has_s = 0;
    view.me = NULL;
    view.od = 0;
    view.has_od = 0;
    if (kind != NULL) {
        view.rr = kind->record->rr;
        view.s = kind->record->s;
        view.has_s = kind->record->has_s;
        view.me = kind->record->me;
        view.od = kind->record->od;
        view.has_od = kind->record->has_od;
    }
    return view;
}

const entity_class_t *pico8_ie_ke_view_struct_class(void) {
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
        pico8_vec_t *box = (pico8_vec_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *box);
        *box = pico8_vec_sub(current, delta);
        g_ie_gy = box;
    }
}

static void pico8_ie_hb_dispatch(entity_t *self) {
    pico8_ie_hb((pico8_ie_t *)self);
}

down_t pico8_ie_ldown(pico8_ie_t *self) {
    pico8_fixed_t new_id;
    const entity_kind_t *ke_kind;
    char *id_num_str;
    char *id_str;
    planet_keys_t keys;
    down_t out = {0, NULL};

    assert(g_ie_up && "pico8_research_icon_init() must run before ie:ldown writes ie.id");
    new_id = pico8_fixed_add(g_ie_has_id ? g_ie_id : pico8_fixed_from_int(0),
                             pico8_fixed_from_int(1));
    g_ie_id = new_id;
    g_ie_has_id = 1;

    ke_kind = ie_self_ke_kind(self);
    if (ke_kind == NULL) {
        return out;
    }

    /* The grab opens before `id_str` is made: the planet keeps that exact
     * pointer as its `id`, so it must live and die with the grab's arena. */
    pico8_jo_prise_ouvrir();
    id_num_str = pico8_tostr_fixed(pico8_zone_frame(), new_id);
    id_str = pico8_concat2(pico8_jo_zone_construction(), "s", id_num_str);
    keys.planet_id = id_str;
    keys.map_pos = NULL;
    keys.pos = NULL;
    out.held = ke_kind->new_entity(&keys);
    pico8_jo_prise_fermer();
    out.stop = out.held != NULL;
    return out;
}

static down_t pico8_ie_ldown_dispatch(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
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
static void pico8_ie_kj_dispatch(entity_t *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_ie_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ie_t *)self, p);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static int pico8_ie_hd_dispatch(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk(self, qa, ra);
}

static const entity_class_t g_ie_struct_class = {
    .draw = pico8_ie_kj_dispatch,
    .hit_test = pico8_ie_hd_dispatch,
    .refresh = pico8_ie_hb_dispatch,
    .left_press = pico8_ie_ldown_dispatch,
};

static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
    pico8_ie_t *self = (pico8_ie_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *self);

    assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
    pico8_ke_struct_init(&self->header, &g_ie_struct_class, pico8_ie_def.me, pico8_ie_def.t);
    self->def = &pico8_ie_def;
    self->header.layer = &self->def->fa;
    self->header.pos = &self->ra;
    self->header.hit_box = &self->def->kq;
    self->ra = &self->def->ra;
    self->ke = NULL;
    self->jb = NULL;
    self->kh = 0;
    self->has_kh = 0;
    self->header.is_hovered = &self->m_;
    self->m_ = 0;
    if (keys != NULL) {
        self->ke = keys->ke;
        self->jb = keys->jb;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("structure_icon");
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

void pico8_research_icon_shutdown(void) {
    g_ie_up = 0;
    g_ie_gy = NULL;
}

int pico8_ie_ready(void) {
    return g_ie_up;
}

const entity_class_t *pico8_ie_struct_class(void) {
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
