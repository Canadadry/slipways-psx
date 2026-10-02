#include "game/ui.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "engine/entity.h"
#include "game/mouse_button.h"
#include "game/draw_functions.h"
#include "game/planet_family.h"
#include "game/scroll.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"
#include "engine/game_zone.h"

static int g_ui_up = 0;
static entity_t *g_je = NULL;
static pico8_ui_t *g_ok = NULL;
static pico8_ents_t g_shared_le;

const pico8_qz_box_t *pico8_ui_hw(void) {
    return g_ui_up ? &pico8_ui_def.hw : NULL;
}

pico8_ents_t *pico8_ui_le_shared(void) {
    return &g_shared_le;
}

pico8_ents_t *pico8_ui_le_new(void) {
    const pico8_zone_choisie_t *place = pico8_zone_interface_place(PICO8_SORTE_LE);
    pico8_ents_t *le = (pico8_ents_t *)pico8_zone_alloc(place, sizeof *le);

    pico8_ents_init(le, place);
    return le;
}

void pico8_ui_set_le(pico8_ui_t *self, pico8_ents_t *le) {
    pico8_ents_t *old = self->le;

    self->le = le;
    if (old != NULL && old != &g_shared_le) {
        pico8_file_sortie_mettre_arene(old->zone);
    }
}

static const entity_class_t g_je_placeholder_struct_class = {0};

typedef struct {
    entity_t header;
} pico8_je_placeholder_t;

/* The only one: never allocated, never freed. */
static pico8_je_placeholder_t g_je_placeholder = {{.class = &g_je_placeholder_struct_class}};

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

static pico8_vec_t pico8_ui_vec_or_zero(const pico8_vec_t *v) {
    return v != NULL ? *v : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* je[bt] (bt "l" or "r"): NULL when `je` has no such button state,
 * which is the case for the field-less placeholder. */
static const pico8_nf_button_t *pico8_ui_je_button_table(const char *bt) {
    assert((strcmp(bt, "l") == 0 || strcmp(bt, "r") == 0) &&
           "je[bt]: bt is l or r only; another button has no state table -- stop");
    assert(g_je != NULL && "je[bt]: indexing a nil je stops the run, as in the cart");
    if (pico8_je_nf() == NULL) {
        return NULL;
    }
    return strcmp(bt, "l") == 0 ? pico8_nf_l() : pico8_nf_r();
}

static int pico8_ui_je_button_field_bool(const char *bt, const char *field) {
    const pico8_nf_button_t *b = pico8_ui_je_button_table(bt);
    if (b == NULL || !b->has_on) {
        return 0;
    }
    if (strcmp(field, "jn") == 0) {
        return b->jn;
    }
    assert(strcmp(field, "hm") == 0 && "je[bt]: only jn and hm are read");
    return b->hm;
}

static void pico8_ui_ev_dispatch(entity_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_ui_t *)self);
}

static void pico8_ui_le_rendre(pico8_ui_t *self) {
    if (self->le != NULL && self->le != &g_shared_le) {
        pico8_zone_arene_rendre(self->le->zone);
    }
    self->le = NULL;
}

static void pico8_ui_rendre_dispatch(entity_t *self) {
    pico8_ui_le_rendre((pico8_ui_t *)self);
}

static const entity_class_t g_ui_struct_class = {
    .draw_overlay = pico8_ui_ev_dispatch,
    .release = pico8_ui_rendre_dispatch,
};

static pico8_ui_t *pico8_ui_struct_new(void) {
    pico8_ui_t *self;
    int i;

    assert(g_ui_up && "pico8_ui_init() must run first");
    self = (pico8_ui_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *self);
    pico8_ke_struct_init(&self->header, &g_ui_struct_class, pico8_ui_def.me, pico8_ui_def.t);
    self->def = &pico8_ui_def;
    self->header.layer = &self->def->fa;
    self->header.cursor_sprite = &self->def->ql;
    self->le = &g_shared_le;
    self->qa = NULL;
    self->ic = NULL;
    self->ic_own.layer_count = PICO8_UI_IC_LAYERS;
    for (i = 0; i < PICO8_UI_IC_LAYERS; i++) {
        pico8_ents_init(&self->ic_own.layers[i], pico8_zone_frame());
    }
    self->o_ = NULL;
    self->gj = NULL;
    self->lheld = NULL;
    self->rheld = NULL;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ui");
#endif
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_ui_init(void) {
    pico8_ui_shutdown();
    pico8_pi_init();
    pico8_ents_init(&g_shared_le, pico8_zone_chargement());
    g_ui_up = 1;

    g_je = (entity_t *)&g_je_placeholder;
}

void pico8_ui_shutdown(void) {
    g_ui_up = 0;
    pico8_ents_free(&g_shared_le);
    g_je = NULL;
    g_ok = NULL;
}

const entity_class_t *pico8_je_placeholder_struct_class(void) {
    return &g_je_placeholder_struct_class;
}

int pico8_ui_ready(void) {
    return g_ui_up;
}

const entity_class_t *pico8_ui_struct_class(void) {
    return &g_ui_struct_class;
}

pico8_ui_t *pico8_ui_new(void) {
    return pico8_ui_struct_new();
}

entity_t *pico8_je(void) {
    return g_je;
}

pico8_nf_t *pico8_je_nf(void) {
    if (g_je == NULL || g_je == &g_je_placeholder.header) {
        return NULL;
    }
    assert(g_je->class == pico8_nf_struct_class() && "je: only the placeholder or an nf is ever stored");
    return (pico8_nf_t *)g_je;
}

/* The outgoing instance lives on with its generation. `pico8_jl()` rebuilds
 * je/ok mid-frame, reachable from a click dispatched inside pico8_ui_lj,
 * whose `self` is still the old `ok` after the callback returns. */
void pico8_ui_set_je(entity_t *instance) {
    if (instance == g_je) {
        return;
    }
    g_je = instance;
}

pico8_ui_t *pico8_ok(void) {
    return g_ok;
}

/* Same non-freeing rule as pico8_ui_set_je(), and the same reason. */
void pico8_ui_set_ok(pico8_ui_t *ui) {
    if (ui == g_ok) {
        return;
    }
    g_ok = ui;
}

/* `le` is read before gw()/bh() run: the `self.le == le` test further down
 * is a pointer identity check against the list this frame started with. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_ui_t *self) {
    pico8_ents_t *le_ptr_before;
    int le_count;
    pico8_vec_t qa;
    int hit;
    const pico8_nf_t *je;
    const pico8_qb_t *mo;

    if (self == NULL) {
        return;
    }

    le_ptr_before = self->le;
    le_count = self->le != NULL ? self->le->count : 0;

    je = pico8_je_nf();
    mo = pico8_mo_qb();
    qa = pico8_vec_add(pico8_ui_vec_or_zero(je != NULL ? je->ra : NULL),
                       pico8_ui_vec_or_zero(mo != NULL ? mo->ra : NULL));
    self->qa_own = qa;
    self->qa = &self->qa_own;

    pico8_ui_gw(mem, self);

    hit = (le_count == 0) || pico8_ents_has(le_ptr_before, self->o_);
    if (hit) {
        pico8_ui_bh(self, "l");
    }

    pico8_ui_bh(self, "r");

    if (pico8_ui_je_button_field_bool("l", "jn") && self->le == le_ptr_before) {
        pico8_ui_cj(self);
    }

    self->gj = self->o_;
}

/* `(rheld and 9) or (o_ and o_.ql) or self.ql`: a hovered entity whose own
 * `ql` is absent falls through to self's own cursor sprite. */
void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_ui_t *self) {
    pico8_vec_t je_ra;
    const pico8_nf_t *je;
    int spr_id = 0;

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    if (self->rheld != NULL) {
        spr_id = 9;
    } else if (self->o_ != NULL) {
        spr_id = pico8_ui_to_int(self->o_->cursor_sprite != NULL ? *self->o_->cursor_sprite : self->def->ql);
    } else {
        spr_id = pico8_ui_to_int(self->def->ql);
    }

    je = pico8_je_nf();
    je_ra = pico8_ui_vec_or_zero(je != NULL ? je->ra : NULL);
    pico8_spr_default(gfx, mem, spr_id, pico8_ui_to_int(je_ra.x), pico8_ui_to_int(je_ra.y));
}

/* Lua's `(self.gj or {}).m_=false` writes onto a throwaway table when the
 * field is absent, with no observable effect, so the write is simply
 * skipped here rather than allocating a table nothing reads. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
    const pico8_nf_t *je;
    const pico8_vec_t *je_ra;
    const pico8_ents_t *hd_bucket;
    int i;
    int poke_val;

    if (self == NULL || mem == NULL) {
        return;
    }

    for (i = 0; i < PICO8_UI_IC_LAYERS; i++) {
        pico8_ents_init(&self->ic_own.layers[i], pico8_zone_frame());
    }
    self->ic = &self->ic_own;
    self->o_ = NULL;

    je = pico8_je_nf();
    je_ra = je != NULL ? je->ra : NULL;

    hd_bucket = pico8_cv("hd");
    if (hd_bucket != NULL) {
        for (i = 0; i < hd_bucket->count; i++) {
            entity_t *e = hd_bucket->items[i];

            if (e->class->hit_test != NULL && e->class->hit_test(e, self->qa, je_ra)) {
                if (e->layer != NULL) {
                    int fa = pico8_ui_to_int(*e->layer);

                    assert(fa >= 0 && fa < PICO8_UI_IC_LAYERS &&
                           "ui.ic: a hovered entity's fa outside 0..15, the layers jz walks -- stop, do not widen");
                    pico8_ents_add(&self->ic->layers[fa], e);
                }
                self->o_ = e;
            }
        }
    }

    if (self->gj != NULL) {
        assert(self->gj->is_hovered != NULL &&
               "entity: a generic write names a field this struct has no member for -- stop, do not add a member");
        *self->gj->is_hovered = 0;
    }

    if (self->o_ != NULL) {
        assert(self->o_->is_hovered != NULL &&
               "entity: a generic write names a field this struct has no member for -- stop, do not add a member");
        *self->o_->is_hovered = 1;
    }

    if (self->o_ != NULL) {
        poke_val = pico8_ui_to_int(self->o_->gpio_signal != NULL ? *self->o_->gpio_signal : pico8_fixed_from_int(0));
    } else {
        int prev = pico8_peek(mem, 0x5f80);
        poke_val = (prev - 1 > 0) ? prev - 1 : 0;
    }
    pico8_poke(mem, 0x5f80, (unsigned char)poke_val);
}

static int ui_is_held_class(const entity_class_t *cls) {
    return cls == pico8_jc_struct_class() || cls == pico8_g__struct_class() || cls == pico8_mz_struct_class() ||
           (cls->states[ENTITY_STATE_SETTLED] != NULL && cls != pico8_pm_struct_class());
}

void pico8_ui_bh(pico8_ui_t *self, const char *bt) {
    int l;
    entity_t *held_inst;
    pico8_nf_t *je;
    pico8_vec_t *je_ra;

    if (self == NULL) {
        return;
    }

    assert((strcmp(bt, "l") == 0 || strcmp(bt, "r") == 0) &&
           "ui:bh: bt is l or r only; another button has no state table -- stop");
    l = bt[0] == 'l';
    je = pico8_je_nf();
    je_ra = je != NULL ? je->ra : NULL;

    if (pico8_ui_je_button_field_bool(bt, "jn")) {
        entity_t *kv = NULL;
        down_t kn;

        if (pico8_ui_jz(self, l ? PICO8_JZ_LDOWN : PICO8_JZ_RDOWN, self->qa, je_ra, NULL, &kv, &kn) &&
            kn.held != NULL) {
            if (l) {
                assert(ui_is_held_class(kn.held->class) &&
                       "ui.lheld: what an ldown hands over is a jc; a g_ or an mz from bg; a qh, os, nr, fr, nl, mx "
                       "or hj from an ie");
                self->lheld = kn.held;
            } else {
                assert(kn.held->class == pico8_bg_struct_class() && "ui.rheld: what an rdown hands over is the bg");
                self->rheld = (pico8_bg_t *)kn.held;
            }
        }
    }

    held_inst = l ? self->lheld : (self->rheld != NULL ? &self->rheld->header : NULL);

    if (held_inst != NULL && held_inst->class->drag != NULL) {
        held_inst->class->drag(held_inst, self->qa, je_ra);
    }

    if (pico8_ui_je_button_field_bool(bt, "hm") && held_inst != NULL) {
        entity_t *nw_kv = NULL;
        down_t nw_kn;

        /* `ey`'s argument is the entity `jz` found, not a field read on
         * `held`: no object in this cart carries a "kv" field. */
        (void)pico8_ui_jz(self, PICO8_JZ_GV, NULL, NULL, held_inst, &nw_kv, &nw_kn);

        if (held_inst->class->drop != NULL) {
            held_inst->class->drop(held_inst, nw_kv);
        }
        if (l) {
            self->lheld = NULL;
        } else {
            self->rheld = NULL;
        }
    }
}

/* Layers descend from 15 to 0 and the first truthy answer wins: that order
 * is the dispatch priority between overlapping entities, not an arbitrary
 * walk. */
int pico8_ui_jz(pico8_ui_t *self, pico8_jz_slot_t rm, pico8_vec_t *qa, pico8_vec_t *ra, const entity_t *s,
                entity_t **out_kv, down_t *out_kn) {
    int mf;

    if (self == NULL) {
        return 0;
    }

    if (self->ic == NULL) {
        return 0;
    }

    for (mf = PICO8_UI_IC_LAYERS - 1; mf >= 0; mf--) {
        const pico8_ents_t *bucket = &self->ic->layers[mf];
        int i;

        for (i = 0; i < bucket->count; i++) {
            entity_t *h = bucket->items[i];
            down_t kn = {0, NULL};

            switch (rm) {
            case PICO8_JZ_LDOWN:
                if (h->class->left_press != NULL) {
                    kn = h->class->left_press(h, qa, ra);
                }
                break;
            case PICO8_JZ_RDOWN:
                if (h->class->right_press != NULL) {
                    kn = h->class->right_press(h, qa, ra);
                }
                break;
            case PICO8_JZ_GV:
                if (h->class->accept_drop != NULL) {
                    kn = h->class->accept_drop(h, s);
                }
                break;
            }
            if (kn.stop) {
                *out_kv = h;
                *out_kn = kn;
                return 1;
            }
        }
    }

    return 0;
}

void pico8_ui_cj(pico8_ui_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->le != NULL) {
        pico8_ents_iter_t it = pico8_ents_all(self->le);
        entity_t *e;
        while (pico8_ents_iter_next(&it, &e)) {
            if (e != NULL) {
                pico8_he(e);
            }
        }
    }

    pico8_ui_set_le(self, pico8_ui_le_new());
}
