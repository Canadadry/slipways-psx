#include "game/score_panel.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/collect.h"
#include "engine/dispatch_context.h"
#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "game/hud.h"
#include "engine/entity.h"
#include "game/map_tables.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "p8/str.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static int g_pj_up = 0;
static pico8_bz_row_t g_bz[SCORE_ROWS_COUNT];
static pico8_pj_t *g_ju = NULL;

static p8_fixed_t fixed_min(p8_fixed_t a, p8_fixed_t b) {
    return (a < b) ? a : b;
}

/* pj's class state `ou` and `ea`, reset by each init to their generated
 * rows and rewritten in place by `hb`/`hk`. */
static pico8_pj_ou_t g_pj_ou;
static pico8_pj_ea_t g_pj_ea;

/* Copies `src` into `dst` and its item storage (`cap` items), `fn` and `c`
 * included; nested rows stay the generated ones. */
static void pj_row_copy(data_row_t *dst, data_item_t *items, int cap, const data_row_t *src) {
    int i;

    assert(src->count <= cap && "a copied row of pj holds no more items than its generated row");
    for (i = 0; i < src->count; i++) {
        items[i] = src->items[i];
    }
    *dst = *src;
    dst->items = items;
}

static const entity_class_t g_pj_cr_struct_class = {0};

static pico8_pj_cr_t g_pj_cr = {
    {.class = &g_pj_cr_struct_class},
    &(const vec_t){0x0, 0xb0000},
    &(const vec_t){0x0, 0xb0000},
    0xa0000,
    NULL,
};

pico8_pj_ou_t *pico8_pj_ou(void) {
    return g_pj_up ? &g_pj_ou : NULL;
}

pico8_pj_ea_t *pico8_pj_ea(void) {
    return g_pj_up ? &g_pj_ea : NULL;
}

pico8_pj_cr_t *pico8_pj_cr(void) {
    return g_pj_up ? &g_pj_cr : NULL;
}

const entity_class_t *pico8_pj_cr_struct_class(void) {
    return &g_pj_cr_struct_class;
}

const data_row_t *pico8_pj_i_(void) {
    return score_panel_archetype.frame_rows;
}

const data_row_t *pico8_pj_gs(void) {
    return score_panel_archetype.countdown_rows;
}

static void pico8_pj_init_placeholder(pico8_js_t *self) {
    (void)self;
}

static pico8_js_fn g_init_dispatch = pico8_pj_init_placeholder;

pico8_js_fn pico8_init_field_value(void) {
    return g_init_dispatch;
}

void pico8_score_panel_set_init(pico8_js_fn fn) {
    g_init_dispatch = (fn != NULL) ? fn : pico8_pj_init_placeholder;
}

static int pico8_eg_gg_pred(entity_t *e, void *ctx) {
    (void)ctx;
    if (e == NULL) {
        return 0;
    }
    return planet_is_colonized((const planet_t *)e);
}

void pico8_eg(void) {
    p8_memory_t *mem = dispatch_context_mem();
    entity_list_t filtered;
    p8_fixed_t ps;
    int i;

    filtered = entity_list_filter(frame_zone(), entities_by_tag("planet"), pico8_eg_gg_pred, NULL);
    ps = p8_fixed_div(p8_fixed_from_int(filtered.count), p8_fixed_from_int(2));

    if (mem == NULL) {
        return;
    }

    for (i = 1; i <= PALETTE_OFFSETS_COUNT; i++) {
        int ny = palette_offsets_table[i - 1];
        int pd = 0x3101 + 4 * ny;
        unsigned char cur = p8_peek(mem, pd);
        int newval = (cur % 128) + ((ps < p8_fixed_from_int(i)) ? 128 : 0);
        p8_poke(mem, pd, (unsigned char)newval);
    }
}

void pico8_pj_hb(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_fm_placeholder_t *provisional = pico8_fm_placeholder();
    p8_fixed_t fm_ge = fm != NULL ? fm->ge : p8_fixed_from_int(0);
    p8_fixed_t fm_nz = p8_fixed_from_int(0);
    data_item_t fm_jk = {.is_ref = 1};
    p8_fixed_t fm_happiness = fm != NULL && fm->has_happiness ? fm->happiness : p8_fixed_from_int(0);
    p8_fixed_t fm_lo = fm != NULL ? fm->lo : p8_fixed_from_int(0);
    data_item_t fm_yr = fm != NULL ? (data_item_t){.is_fixed = 1, .fixed = fm->yr}
                                       : (data_item_t){.is_ref = 1};
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    pico8_pj_ou_t *ou_state;
    data_item_t ou[7];
    char *cf;
    char *nz_str;
    char *happiness_str;
    char *plus_num;
    int discard_color = 0;
    int i;

    if (fm != NULL) {
        fm_nz = fm->nz;
        fm_jk = (data_item_t){.is_fixed = 1, .fixed = fm->jk};
    } else if (provisional != NULL) {
        if (provisional->has_nz) {
            fm_nz = provisional->nz;
        }
        if (provisional->has_jk) {
            fm_jk = (data_item_t){.is_fixed = 1, .fixed = provisional->jk};
        }
    }

    cf = collect_format_signed(frame_zone(), fm_ge, 0, NULL, &discard_color);

    nz_str = p8_tostr_fixed(frame_zone(), fm_nz);
    ou[0] = (data_item_t){.text = p8_concat2(frame_zone(), nz_str, "$")};

    ou[1] = (data_item_t){.text = cf};

    ou[2] = fm_jk;

    {
        int k_key;
        p8_fixed_t po_val = p8_fixed_from_int(0);
        if (resource_code_lookup("k", &k_key)) {
            pico8_qe_map_get(fm_po, p8_fixed_from_int(k_key), &po_val);
        }
        /* `..` binds looser than `+`: the Lua reads `"+"..(fm.po[r.k]+1)`. */
        plus_num =
            p8_tostr_fixed(frame_zone(), p8_fixed_add(po_val, p8_fixed_from_int(1)));
        ou[3] = (data_item_t){.text = p8_concat2(frame_zone(), "+", plus_num)};
    }

    happiness_str = p8_tostr_fixed(frame_zone(), fm_happiness);
    ou[4] = (data_item_t){.text = p8_concat2(frame_zone(), happiness_str, "%")};

    {
        int lo = p8_fixed_flr_int(fm_lo);
        ou[5] = (lo >= 1 && lo <= PERIOD_NAMES_COUNT) ? (data_item_t){.text = period_names_table[lo - 1]}
                                                  : (data_item_t){.is_ref = 1};
    }

    ou[6] = fm_yr;

    ou_state = pico8_pj_ou();
    if (ou_state != NULL) {
        for (i = 0; i < 7; i++) {
            if (ou[i].text != NULL) {
                assert(strlen(ou[i].text) <= PICO8_PJ_TEXT_CAP && "an ou text is a number with its suffix, or a month");
                strcpy(ou_state->texts[i], ou[i].text);
                ou[i].text = ou_state->texts[i];
            }
            ou_state->sub_items[i][0] = ou[i];
        }
    }

    pico8_eg();
}

static void pico8_pj_hb_dispatch(entity_t *self) {
    (void)self;
    pico8_pj_hb();
}

/* `la(pj.i_, pj.ou)`: 5 then 7 articles in the cart. */
enum { PICO8_PJ_KJ_ARTICLES_CAP = 5 + 7 };

void pico8_pj_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_pj_t *self) {
    const data_row_t *i_ = pico8_pj_i_();
    pico8_pj_ou_t *ou_state = pico8_pj_ou();
    const data_row_t *combined[PICO8_PJ_KJ_ARTICLES_CAP];
    int n = 0;
    int i;

    assert(i_->count + (ou_state != NULL ? ou_state->row.count : 0) <= PICO8_PJ_KJ_ARTICLES_CAP &&
           "la(pj.i_, pj.ou) is 5 + 7 articles in the cart");
    for (i = 0; i < i_->count; i++) {
        combined[n++] = i_->items[i].row;
    }
    for (i = 0; ou_state != NULL && i < ou_state->row.count; i++) {
        combined[n++] = ou_state->row.items[i].row;
    }

    draw_functions_draw_rows_default(gfx, mem, combined, n);

    if (self != NULL && self->has_gz) {
        p8_fixed_t gz = p8_fixed_add(self->gz, P8_FIXED_LIT(0x00003333, 0.2));
        p8_fixed_t capped = fixed_min(gz, P8_FIXED_LIT(0x00033333, 3.2));
        p8_fixed_t h = p8_fixed_mul(capped, capped);

        self->gz = gz;
        hud_draw_bar(gfx, mem, p8_fixed_from_int(0), h, 0);
        /* The cart's only colour-less `rk`: the bottom bar inherits the pen
         * colour the line above leaves behind. Passing a colour here would
         * change what is drawn. */
        hud_draw_bar_default(gfx, mem, p8_fixed_sub(p8_fixed_from_int(127), h), p8_fixed_from_int(127));
    }
}

static void pico8_pj_kj_dispatch(entity_t *self, const vec_t *ra) {
    (void)ra;
    pico8_pj_kj(dispatch_context_gfx(), dispatch_context_mem(), (pico8_pj_t *)self);
}

static const entity_class_t g_pj_struct_class = {
    .draw = pico8_pj_kj_dispatch,
    .new_year = pico8_pj_hk_dispatch,
    .refresh = pico8_pj_hb_dispatch,
    .states = {[ENTITY_STATE_NORMAL] = pico8_pj_ph_dispatch},
};

static pico8_pj_t *pico8_pj_struct_new(void) {
    pico8_pj_t *self;

    assert(g_pj_up && "pico8_score_panel_init() must run first");
    self = (pico8_pj_t *)allocator_alloc(game_zone(), sizeof *self);
    entity_init_instance(&self->header, &g_pj_struct_class, score_panel_archetype.state, score_panel_archetype.state_time);
    self->def = &score_panel_archetype;
    self->fa = self->def->layer;
    self->header.layer = &self->fa;
    self->ed = NULL;
    self->c_ = 0;
    self->has_c_ = 0;
    self->gz = 0;
    self->has_gz = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("score_panel");
#endif
    entity_construct(&self->header);
    return self;
}

void pico8_score_panel_init(void) {
    int i;

    pico8_score_panel_shutdown();

    draw_functions_init();

    for (i = 0; i < SCORE_ROWS_COUNT; i++) {
        const score_rows_entry_t *e = &score_rows_table[i];
        pico8_bz_row_t *b = &g_bz[i];
        int k;

        pj_row_copy(&b->row, b->items, 5, &e->row);
        b->row.draw_function = e->draw_function;
        for (k = 0; k < 5; k++) {
            b->has[k] = k < e->row.count;
        }
        b->ra = vec_make(p8_fixed_from_int(e->pos_x), p8_fixed_from_int(e->pos_y));
        b->e = e;
    }

    g_pj_up = 1;
    pj_row_copy(&g_pj_ou.row, g_pj_ou.items, 7, &score_panel_readouts_row);
    for (i = 0; i < g_pj_ou.row.count; i++) {
        pj_row_copy(&g_pj_ou.subs[i], g_pj_ou.sub_items[i], 4, score_panel_readouts_row.items[i].row);
        g_pj_ou.items[i].row = &g_pj_ou.subs[i];
    }
    pj_row_copy(&g_pj_ea.row, g_pj_ea.items, 3, &score_panel_year_banner_row);
    pj_row_copy(&g_pj_ea.second, g_pj_ea.second_items, 5, score_panel_year_banner_row.items[1].row);
    g_pj_ea.items[1].row = &g_pj_ea.second;
    g_pj_ea.lh = NULL;
    g_pj_cr.ep = NULL;

    pico8_score_panel_set_init(NULL);
    g_ju = NULL;

    pico8_score_panel_display_init();
}

void pico8_score_panel_shutdown(void) {
    pico8_score_panel_display_shutdown();
    g_pj_up = 0;
    pico8_score_panel_set_init(NULL);
    g_ju = NULL;
}

int pico8_pj_ready(void) {
    return g_pj_up;
}

const entity_class_t *pico8_pj_struct_class(void) {
    return &g_pj_struct_class;
}

pico8_pj_t *pico8_pj_new(void) {
    return pico8_pj_struct_new();
}

pico8_bz_row_t *pico8_pj_bz(void) {
    return g_pj_up ? g_bz : NULL;
}

pico8_pj_t *pico8_ju(void) {
    return g_ju;
}

void pico8_pj_set_ju(pico8_pj_t *instance) {
    g_ju = instance;
}
