#include "game/message_box.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "p8/audio.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "game/hud.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "game/draw_functions.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/ui_zone.h"

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

static void pico8_mh_ev_dispatch(entity_t *self, const vec_t *ra) {
    (void)ra;
    pico8_mh_ev(dispatch_context_gfx(), dispatch_context_mem(), (pico8_mh_t *)self);
}

void pico8_mh_ev(p8_graphics_t *gfx, p8_memory_t *mem, pico8_mh_t *self) {
    p8_fixed_t self_t, rh, t, nc, t3;

    if (self == NULL) {
        return;
    }

    self_t = self->header.state_time;
    rh = self->rh;
    t = p8_fixed_sub(p8_fixed_div(self_t, rh), p8_fixed_from_int(10));

    if (t >= 0) {
        if (self->lh != NULL && self->lh(self)) {
            self->header.state_time = p8_fixed_mul(rh, p8_fixed_from_int(10));
            t = p8_fixed_from_int(0);
        }
    }

    /* `-t^3/9` is `(-(t^3))/9`: in Lua `^` binds tighter than unary `-`,
     * which in turn binds tighter than `/`. */
    t3 = p8_fixed_mul(p8_fixed_mul(t, t), t);
    nc = p8_fixed_div(p8_fixed_neg(t3), p8_fixed_from_int(9));

    if (gfx != NULL && mem != NULL) {
        draw_functions_draw_rows(gfx, mem, self->items, self->items_count, nc);
        p8_camera_reset(gfx);
    }

    if (t > P8_FIXED_LIT(0x000a8000, 10.5)) {
        self->p_ = 1;
        pico8_msgbox_set_kc(NULL);
        assert(pico8_kc() == NULL && "`self.p_,kc=true` is one value for two targets: kc gets nil, not true");
    }
}

static void pico8_mh_rendre(entity_t *self) {
    arena_release(((pico8_mh_t *)self)->place);
}

static const entity_class_t g_mh_struct_class = {
    .draw_overlay = pico8_mh_ev_dispatch,
    .tags = TAG(TAG_MESSAGE_BOX),
    .release = pico8_mh_rendre,
};

/* An article `ld` copied into a box's place, its vector by value. */
typedef struct {
    data_row_t row;
    data_item_t items[3];
    vec_t ra;
} pico8_mh_ld_t;

/* `e` itself, or its copy in `place` when it is an article `ld`. */
static const data_row_t *pico8_mh_article(const allocator_t *place, const data_row_t *e) {
    pico8_mh_ld_t *copy;
    const vec_t *ra;

    if (e->draw_function == NULL || strcmp(e->draw_function, "draw_row") != 0) {
        return e;
    }
    assert(e->count == 3 && e->items[0].is_ref && e->items[1].is_ref &&
           "an article ld is {lq, ra, a}: its lq and ra are what the box copies");
    copy = (pico8_mh_ld_t *)allocator_alloc(place, sizeof *copy);
    ra = e->items[1].ref;
    if (ra != NULL) {
        copy->ra = *ra;
    }
    copy->items[0] = (data_item_t){.is_ref = 1, .ref = layout_row_copy(place, e->items[0].ref)};
    copy->items[1] = (data_item_t){.is_ref = 1, .ref = ra != NULL ? &copy->ra : NULL};
    copy->items[2] = e->items[2];
    copy->row = *e;
    copy->row.items = copy->items;
    return &copy->row;
}

static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
    const allocator_t *place = ui_zone_slot(SLOT_KIND_MESSAGE_BOX);
    pico8_mh_t *self = (pico8_mh_t *)allocator_alloc(place, sizeof *self);
    int i;

    self->place = place;
    entity_init_instance(&self->header, &g_mh_struct_class, message_box_archetype.state, message_box_archetype.state_time);
    self->def = &message_box_archetype;
    self->header.layer = &self->def->layer;
    self->rh = self->def->duration_scale;
    self->lh = NULL;
    self->header.is_dead = &self->p_;
    self->p_ = 0;
    self->items_count = 0;
    if (keys != NULL) {
        assert(keys->count <= PICO8_MH_ITEMS_CAP &&
               "the cart's two mh() are pj:hk (3 articles at most) and pj:ph (one per bz row): a third needs more");
        for (i = 0; i < keys->count; i++) {
            self->items[self->items_count++] = pico8_mh_article(place, keys->items[i]);
        }
        if (keys->has_rh) {
            self->rh = keys->rh;
        }
        self->lh = keys->lh;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("message_box");
#endif
    pico8_mh_pq(self);
    entity_construct(&self->header);
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

static vec_t pico8_iv_vec_or_zero(const vec_t *p) {
    if (p != NULL) {
        return *p;
    }
    return vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

void pico8_iv_pq(pico8_iv_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->ra == NULL) {
        const mouse_t *je = main_mouse_typed();
        const camera_t *mo = main_camera_typed();
        vec_t je_ra = pico8_iv_vec_or_zero(je != NULL ? je->pos : NULL);
        vec_t mo_ra = pico8_iv_vec_or_zero(mo != NULL ? mo->pos : NULL);
        vec_t offset = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(5));
        self->ra_box = vec_sub(vec_add(je_ra, mo_ra), offset);
        self->ra = &self->ra_box;
    }
}

void pico8_iv_ph(pico8_iv_t *self) {
    p8_fixed_t new_l;

    if (self == NULL) {
        return;
    }

    self->ra_box = vec_add(pico8_iv_vec_or_zero(self->ra), pico8_iv_vec_or_zero(&self->def->velocity));
    self->ra = &self->ra_box;

    new_l = p8_fixed_add(self->l, P8_FIXED_LIT(0x00001999, 0.1));
    self->l = new_l;

    self->p_ = new_l >= p8_fixed_from_int(5);
}

static void pico8_iv_ph_dispatch(entity_t *self, p8_fixed_t t) {
    (void)t;
    pico8_iv_ph((pico8_iv_t *)self);
}

void pico8_iv_kj(const p8_graphics_t *gfx, p8_memory_t *mem, const pico8_iv_t *self, vec_t p) {
    const char *text;
    int color;
    p8_fixed_t a_center = P8_FIXED_LIT(0x00008000, 0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    hud_select_palette_block(mem, self->l);

    text = self->at1 != NULL ? self->at1 : "";
    color = p8_fixed_flr_int(self->at2);

    hud_print_outlined(gfx, mem, text, p.x, p.y, color, &a_center);

    hud_select_palette_block_default(mem);
}

static void pico8_iv_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_iv_kj(dispatch_context_gfx(), dispatch_context_mem(), (const pico8_iv_t *)self, p);
}

static void pico8_iv_rendre(entity_t *self) {
    arena_release(((pico8_iv_t *)self)->place);
}

static const entity_class_t g_iv_struct_class = {
    .draw = pico8_iv_kj_dispatch,
    .states = {[ENTITY_STATE_NORMAL] = pico8_iv_ph_dispatch},
    .release = pico8_iv_rendre,
};

static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
    const allocator_t *place = ui_zone_slot(SLOT_KIND_TOAST);
    pico8_iv_t *self = (pico8_iv_t *)allocator_alloc(place, sizeof *self);

    self->place = place;

    entity_init_instance(&self->header, &g_iv_struct_class, toast_archetype.state, toast_archetype.state_time);
    self->def = &toast_archetype;
    self->header.layer = &self->def->layer;
    self->header.pos = &self->ra;
    self->l = self->def->fade;
    self->ra = NULL;
    self->header.is_dead = &self->p_;
    self->p_ = 0;
    self->at1 = NULL;
    self->at2 = p8_fixed_from_int(0);
    if (keys != NULL) {
        self->ra = keys->ra;
        if (keys->at1 != NULL) {
            assert(strlen(keys->at1) <= PICO8_IV_TEXT_CAP &&
                   "a toast's text is a `ne` price or \"-no money-\": a longer one needs a bigger at1_buf and iv place");
            strcpy(self->at1_buf, keys->at1);
            self->at1 = self->at1_buf;
        }
        self->at2 = keys->at2;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("toast");
#endif
    pico8_iv_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_iv_init(void) {
    pico8_iv_shutdown();
    draw_functions_init();
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
    vec_t bp;
    vec_t ra;
} pico8_pz_ctx_t;

/* One row of `bs` onto a copy of its keys: the common `ra`, its own `qf`,
 * `fa` from `p.ca`; then the button. */
static button_t *pico8_pz_step(pico8_pz_ctx_t *ctx, const button_keys_t *row) {
    button_keys_t b = *row;
    vec_t qf = ctx->bp;
    vec_t gq;

    b.pos = &ctx->ra;
    b.slide_target = &qf;

    /* TROU-9 */
    if (ctx->frame->has_ca) {
        b.layer = ctx->frame->ca;
        b.has_layer = 1;
    }

    gq = ctx->frame->gq != NULL ? *ctx->frame->gq : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    ctx->bp = vec_add(ctx->bp, gq);

    return button_new(&b);
}

/* With no `ok` there is no list to hold the button. */
static void pz_add(entity_list_t *le, button_t *button) {
    if (le != NULL) {
        entity_list_add(le, &button->header);
    }
}

void pico8_pz(const pico8_pz_frame_t *frame, const button_keys_t *bs, int n) {
    layout_row_cell_t ep_cell;
    vec_t ra;
    pico8_pz_ctx_t ctx;
    entity_list_t *le;
    ui_t *ok;
    p8_fixed_t rn_arg;
    button_keys_t ep_row = {0};
    layout_row_keys_t ep_lq = {0};
    int has_ep;
    int i;

    if (frame == NULL) {
        return;
    }

    has_ep = frame->ep != NULL;
    if (has_ep) {
        ep_cell = layout_row_cell_string(frame->ep);
        ep_lq.items = &ep_cell;
        ep_lq.count = 1;
        ep_lq.has_color = 1;
        ep_lq.color = p8_fixed_from_int(13);
        ep_row.layout_row_keys = &ep_lq;
    }

    if (frame->has_eo) {
        rn_arg = frame->eo;
    } else {
        rn_arg = p8_fixed_from_int(62);
    }
    p8_play_sfx(rn_arg);

    if (frame->hi != NULL) {
        ra = *frame->hi;
    } else if (frame->ra != NULL) {
        ra = *frame->ra;
    } else {
        ra = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    }

    ctx.frame = frame;
    ctx.bp = vec_add(ra, vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0)));
    ctx.ra = ra;

    ok = main_ui();
    ui_clear_ephemerals(ok);

    le = ok != NULL ? ui_ephemerals_new() : NULL;
    if (has_ep) {
        pz_add(le, pico8_pz_step(&ctx, &ep_row));
    }
    for (i = 0; i < n; i++) {
        pz_add(le, pico8_pz_step(&ctx, &bs[i]));
    }
    if (ok != NULL) {
        ui_set_ephemerals(ok, le);
    }

    /* The `js` buttons built just above stay registered in `hv`/`cv`/`cg`
     * once `le` no longer holds them, exactly as in the cart. */
    if (frame->ff && ok != NULL) {
        ui_set_ephemerals(ok, ui_ephemerals_new());
    }
}
