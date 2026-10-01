#include "pico8_layout.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_hud.h"

static int g_layout_up = 0;
static const pico8_struct_class_t g_lq_struct_class;

pico8_lq_cell_t pico8_lq_cell_nothing(void) {
    pico8_lq_cell_t cell = {.kind = PICO8_LQ_CELL_NOTHING};
    return cell;
}

pico8_lq_cell_t pico8_lq_cell_fixed(pico8_fixed_t x) {
    pico8_lq_cell_t cell = {.kind = PICO8_LQ_CELL_FIXED, .as.fixed = x};
    return cell;
}

pico8_lq_cell_t pico8_lq_cell_string(const char *s) {
    pico8_lq_cell_t cell = {.kind = PICO8_LQ_CELL_STRING, .as.s = s};

    assert(s != NULL && "lq: no producer writes a NULL string; its kind would have no reader in lq:mn");
    return cell;
}

pico8_lq_cell_t pico8_lq_cell_row(const pico8_jb_row_t *row) {
    pico8_lq_cell_t cell = {.kind = PICO8_LQ_CELL_ROW, .as.row = row};

    /* Not optional: a nil read as anything but NOTHING falls through `elseif f`
     * of `lq:mn` into a sprite 0, a block of raw data on screen
     * (layout_nil_item_draws_nothing). */
    if (row == NULL) {
        return pico8_lq_cell_nothing();
    }
    return cell;
}

pico8_lq_cell_t pico8_lq_cell_item(const pico8_jb_item_t *item) {
    if (item->row != NULL) {
        return pico8_lq_cell_row(item->row);
    }
    if (item->s != NULL) {
        return pico8_lq_cell_string(item->s);
    }
    if (item->is_false) {
        return pico8_lq_cell_nothing();
    }
    if (item->is_ref) {
        return pico8_lq_cell_row(item->ref);
    }
    if (item->is_fixed) {
        return pico8_lq_cell_fixed(item->fixed);
    }
    return pico8_lq_cell_fixed(pico8_fixed_from_int(item->n));
}

/* A tuple's `f[1..4]` or an item of a line. */
static pico8_lq_cell_t pico8_lq_scalar_item(const pico8_jb_item_t *item) {
    assert(item->row == NULL && "a tuple or a line holds scalar items only; a row inside one has no reader here");
    return pico8_lq_cell_item(item);
}

/* Writes out_item plus mn's second and third Lua return values (out_w,
 * out_qv). `self` may be NULL; it is only read for the TEXT branch's colour.
 * A non-FIXED f[2]/f[3]/f[4] reads as 0. */
static void pico8_lq_mn(const pico8_lq_t *self, pico8_lq_cell_t f, pico8_lq_item_t *out_item,
                         pico8_fixed_t *out_w, pico8_fixed_t *out_qv) {
    pico8_fixed_t w = pico8_fixed_from_int(6);
    pico8_fixed_t qw = pico8_fixed_from_int(0);
    int have_qv = 0;
    pico8_fixed_t qv = pico8_fixed_from_int(0);

    if (f.kind == PICO8_LQ_CELL_ROW) {
        const pico8_jb_row_t *t = f.as.row;
        const pico8_jb_item_t wrap[] = {{.is_fixed = 1, .fixed = t->qe}, {.n = 5}};
        const pico8_jb_item_t *items = t->has_qe ? wrap : t->items;
        int count = t->has_qe ? 2 : t->count;
        pico8_lq_cell_t f1, f2, f3, f4;
        int have_f1 = count >= 1;
        int have_f2 = count >= 2;
        int have_f3 = count >= 3;
        int have_f4 = count >= 4;

        f1 = have_f1 ? pico8_lq_scalar_item(&items[0]) : pico8_lq_cell_nothing();
        f2 = have_f2 ? pico8_lq_scalar_item(&items[1]) : pico8_lq_cell_nothing();
        f3 = have_f3 ? pico8_lq_scalar_item(&items[2]) : pico8_lq_cell_nothing();
        f4 = have_f4 ? pico8_lq_scalar_item(&items[3]) : pico8_lq_cell_nothing();

        f = f1;
        if (have_f2 && f2.kind == PICO8_LQ_CELL_FIXED) {
            w = f2.as.fixed;
        }
        if (have_f3 && f3.kind == PICO8_LQ_CELL_FIXED) {
            qw = f3.as.fixed;
        }
        if (have_f4 && f4.kind != PICO8_LQ_CELL_NOTHING) {
            have_qv = 1;
            qv = (f4.kind == PICO8_LQ_CELL_FIXED) ? f4.as.fixed : pico8_fixed_from_int(0);
        }
    }

    if (f.kind == PICO8_LQ_CELL_STRING) {
        pico8_fixed_t c = pico8_fixed_from_int(6);

        if (self != NULL && self->has_c) {
            c = self->c;
        }
        out_item->kind = PICO8_LQ_ITEM_TEXT;
        out_item->text = f.as.s;
        out_item->color = c;
        *out_w = pico8_fixed_from_int((int)strlen(f.as.s) * 4 + 3);
        *out_qv = pico8_fixed_from_int(1);
        return;
    }

    if (f.kind != PICO8_LQ_CELL_NOTHING) {
        out_item->kind = PICO8_LQ_ITEM_SPRITE;
        out_item->sprite_id = (f.kind == PICO8_LQ_CELL_FIXED) ? f.as.fixed : pico8_fixed_from_int(0);
        out_item->sprite_qw = qw;
    } else {
        out_item->kind = PICO8_LQ_ITEM_NONE;
    }
    *out_w = w;
    *out_qv = have_qv ? qv : pico8_fixed_from_int(2);
}

/* `fs`, the last item's spacing, starts at 0 where Lua's own starts nil, so
 * a zero-item lq() yields w=0 here and would error in Lua. No call site
 * builds one. */
static void pico8_lq_pq(pico8_lq_t *self) {
    pico8_fixed_t w = pico8_fixed_from_int(0);
    pico8_fixed_t fs = pico8_fixed_from_int(0);
    int n = self->items_count;
    int i;

    /* TROU-7 */
    self->fs = (pico8_lq_item_t *)pico8_zone_alloc(self->zone, (size_t)(n > 0 ? n : 1) * sizeof *self->fs);
    for (i = 0; i < n; i++) {
        pico8_fixed_t w_i = pico8_fixed_from_int(0);
        pico8_fixed_t qv_i = pico8_fixed_from_int(0);
        pico8_fixed_t x = w;

        pico8_lq_mn(self, self->items[i], &self->fs[i], &w_i, &qv_i);
        self->fs[i].d = pico8_vec_v(x, pico8_fixed_from_int(0));
        w = pico8_fixed_add(w, pico8_fixed_add(w_i, qv_i));
        fs = qv_i;
    }
    self->fs_count = n;
    self->w = pico8_fixed_sub(w, fs);
}

static void pico8_lq_ol_item_draw(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_item_t *item,
                                   pico8_vec_t p) {
    pico8_vec_t pos = pico8_vec_add(p, item->d);

    switch (item->kind) {
    case PICO8_LQ_ITEM_TEXT:
        pico8_io_default(gfx, mem, item->text, pico8_fixed_add(pos.x, pico8_fixed_from_int(2)),
                          pico8_fixed_add(pos.y, pico8_fixed_from_int(1)),
                          pico8_fixed_flr_int(item->color));
        break;
    case PICO8_LQ_ITEM_SPRITE:
        pico8_dv(mem, item->sprite_qw);
        pico8_spr_default(gfx, mem, pico8_fixed_flr_int(item->sprite_id),
                           pico8_fixed_flr_int(pos.x),
                           pico8_fixed_flr_int(pos.y));
        pico8_dv_default(mem);
        break;
    case PICO8_LQ_ITEM_NONE:
    default:
        break;
    }
}

void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p, pico8_fixed_t a) {
    pico8_fixed_t w;
    int i;

    if (self == NULL) {
        return;
    }
    assert(self->header.cls == &g_lq_struct_class &&
           "lq:ol: every row reaching it (jo.lq, js.lq/h_, jb.ls, pi.ld) is an lq; anything else would be read "
           "through the wrong layout");
    w = self->w;
    p = pico8_vec_sub(p, pico8_vec_v(pico8_fixed_mul(a, w), pico8_fixed_from_int(0)));

    if (self->fs == NULL) {
        return;
    }
    for (i = 0; i < self->fs_count; i++) {
        pico8_lq_ol_item_draw(gfx, mem, &self->fs[i], p);
    }
}

void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p) {
    pico8_lq_ol(gfx, mem, self, p, pico8_fixed_from_int(0));
}

/* The longest line of the cart's bubbles, `nb_9_oy_1`. */
#define PICO8_JB_LINE_CAP 7

/* The line `jb:pq` builds from one item of its row. */
static pico8_lq_t *pico8_jb_line(const pico8_zone_choisie_t *zone, const pico8_jb_item_t *item) {
    pico8_lq_keys_t keys = {0};
    pico8_lq_cell_t s;
    pico8_lq_cell_t items[PICO8_JB_LINE_CAP];
    int i;

    if (item->s != NULL) {
        s = pico8_lq_cell_string(item->s);
        keys.items = &s;
        keys.count = 1;
        return pico8_lq_new(zone, &keys);
    }
    if (item->row == NULL) {
        return pico8_lq_new(zone, NULL);
    }
    assert(item->row->fn == NULL && "jb: a line's named keys are c only; fn has no member -- stop, do not add one");
    assert(item->row->count <= PICO8_JB_LINE_CAP &&
           "jb: the cart's longest bubble line (nb_9_oy_1) has 7 items; a longer one needs a bigger array");
    for (i = 0; i < item->row->count; i++) {
        items[i] = pico8_lq_scalar_item(&item->row->items[i]);
    }
    keys.items = items;
    keys.count = item->row->count;
    keys.has_c = item->row->has_c;
    keys.c = pico8_fixed_from_int(item->row->c);
    return pico8_lq_new(zone, &keys);
}

/* `ls` is allocated even for no line: `jb:ol` draws the frame of an empty
 * `jb`, and skips one whose `ls` is NULL. */
static void pico8_jb_pq(pico8_jb_t *self, const pico8_jb_row_t *row) {
    int n = row != NULL ? row->count : 0;
    int i;

    /* TROU-7 */
    self->ls = (pico8_lq_t **)pico8_zone_alloc(self->zone, (size_t)(n > 0 ? n : 1) * sizeof *self->ls);
    for (i = 0; i < n; i++) {
        self->ls[i] = pico8_jb_line(self->zone, &row->items[i]);
    }
    self->ls_count = n;
}

/* Lua's `#self` is read here as `ls_count`: jb:pq contributes exactly one
 * `ls` entry per positional item, so the two counts never differ. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_t *self, pico8_fixed_t om,
                  pico8_fixed_t a) {
    pico8_fixed_t h;
    pico8_fixed_t dn;
    int i;

    if (self == NULL || self->ls == NULL) {
        return;
    }

    h = pico8_fixed_add(pico8_fixed_mul(pico8_fixed_from_int(self->ls_count), pico8_fixed_from_int(8)),
                         pico8_fixed_from_int(2));
    om = pico8_fixed_sub(om, pico8_fixed_mul(h, a));
    dn = pico8_fixed_add(om, h);

    pico8_fillp(gfx, 0x0F0F);
    pico8_rk(gfx, mem, om, dn, 0x10);
    pico8_fillp_reset(gfx);

    for (i = 0; i < self->ls_count; i++) {
        pico8_vec_t pos = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_add(om, pico8_fixed_from_int(2)));
        pico8_lq_ol(gfx, mem, self->ls[i], pos, PICO8_FIXED_LIT(0x00008000, 0.5));
        om = pico8_fixed_add(om, pico8_fixed_from_int(8));
    }
}

static void pico8_lq_ol_dispatch(pico8_struct_header_t *self, pico8_vec_t p, pico8_fixed_t a) {
    pico8_lq_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_lq_t *)self, p, a);
}

/* No launcher reaches it (`jo.lq` is an `lq`); a vector `om` counts as 0. */
static void pico8_jb_ol_dispatch(pico8_struct_header_t *self, pico8_vec_t p, pico8_fixed_t a) {
    (void)p;
    pico8_jb_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_jb_t *)self, pico8_fixed_from_int(0), a);
}

static const pico8_struct_class_t g_lq_struct_class = {
    .ol = pico8_lq_ol_dispatch,
};

/* TROU-7 */
/* The row with room for `n` cells, members at nil, not laid out. */
static pico8_lq_t *pico8_lq_struct_alloc(const pico8_zone_choisie_t *zone, int n) {
    pico8_lq_t *self = malloc(sizeof *self);

    assert(zone != NULL && "lq: a row without a zone would have nowhere to keep its cells");

    pico8_struct_header_init(&self->header, &g_lq_struct_class);
    self->header.ra = &self->ra;
    self->c = 0;
    self->has_c = 0;
    self->bg = 0;
    self->has_bg = 0;
    self->bg_is_bool = 0;
    self->score = 0;
    self->has_score = 0;
    self->ra = NULL;
    self->fs = NULL;
    self->fs_count = 0;
    self->w = 0;
    self->zone = zone;
    self->items = (pico8_lq_cell_t *)pico8_zone_alloc(zone, (size_t)(n > 0 ? n : 1) * sizeof *self->items);
    self->items_count = n;
    return self;
}

static pico8_lq_t *pico8_lq_struct_finish(pico8_lq_t *self) {
    pico8_lq_pq(self);
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("lq");
#endif
    return self;
}

static pico8_lq_t *pico8_lq_struct_new(const pico8_zone_choisie_t *zone, const pico8_lq_keys_t *keys) {
    pico8_lq_t *self = pico8_lq_struct_alloc(zone, keys != NULL ? keys->count : 0);
    int k;

    if (keys != NULL) {
        for (k = 0; k < keys->count; k++) {
            self->items[k] = keys->items[k];
        }
        if (keys->has_c) {
            self->c = keys->c;
            self->has_c = 1;
        }
        if (keys->has_bg) {
            self->bg = keys->bg;
            self->has_bg = 1;
            self->bg_is_bool = keys->bg_is_bool;
        }
        if (keys->has_score) {
            self->score = keys->score;
            self->has_score = 1;
        }
        self->ra = keys->ra;
    }
    return pico8_lq_struct_finish(self);
}

pico8_lq_keys_t pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_lq_cell_t *cells, int cap) {
    pico8_lq_keys_t keys = {0};
    int i;

    assert(row->count <= cap && row->fn == NULL && !row->has_qe &&
           "lq: a row passes positionals and c; fn or qe has no member -- stop, do not add one");
    for (i = 0; i < row->count; i++) {
        cells[i] = pico8_lq_cell_item(&row->items[i]);
    }
    keys.items = cells;
    keys.count = row->count;
    keys.has_c = row->has_c;
    keys.c = pico8_fixed_from_int(row->c);
    return keys;
}

pico8_lq_t *pico8_lq_copy(const pico8_zone_choisie_t *zone, const pico8_lq_t *row) {
    pico8_lq_t *self = pico8_lq_struct_alloc(zone, row->items_count);
    int k;

    for (k = 0; k < row->items_count; k++) {
        self->items[k] = row->items[k];
    }
    self->c = row->c;
    self->has_c = row->has_c;
    self->bg = row->bg;
    self->has_bg = row->has_bg;
    self->bg_is_bool = row->bg_is_bool;
    self->score = row->score;
    self->has_score = row->has_score;
    self->ra = row->ra;
    return pico8_lq_struct_finish(self);
}

/* TROU-7 */
void pico8_lq_add(pico8_lq_t *self, pico8_lq_cell_t cell) {
    self->items = (pico8_lq_cell_t *)pico8_zone_grow(self->zone, (unsigned char *)self->items,
                                                     (size_t)self->items_count * sizeof *self->items,
                                                     (size_t)(self->items_count + 1) * sizeof *self->items);
    self->items[self->items_count] = cell;
    self->items_count++;
}

static const pico8_struct_class_t g_jb_struct_class = {
    .ol = pico8_jb_ol_dispatch,
};

/* The `jb` itself, before its lines. */
static pico8_jb_t *pico8_jb_struct_alloc(const pico8_zone_choisie_t *zone) {
    pico8_jb_t *self = malloc(sizeof *self);

    assert(zone != NULL && "jb: a bubble without a zone would have nowhere to keep its lines");

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jb");
#endif
    pico8_struct_header_init(&self->header, &g_jb_struct_class);
    self->zone = zone;
    self->ls = NULL;
    self->ls_count = 0;
    return self;
}

static pico8_jb_t *pico8_jb_struct_new(const pico8_zone_choisie_t *zone, const pico8_jb_row_t *row) {
    pico8_jb_t *self;

    assert((row == NULL || (!row->has_c && row->fn == NULL)) &&
           "jb: every line is positional; a named key has no member -- stop, do not add one");
    self = pico8_jb_struct_alloc(zone);
    pico8_jb_pq(self, row);
    return self;
}

void pico8_layout_init(void) {
    pico8_layout_shutdown();
    g_layout_up = 1;
}

void pico8_layout_shutdown(void) {
    g_layout_up = 0;
}

int pico8_lq_ready(void) {
    return g_layout_up;
}

int pico8_jb_ready(void) {
    return g_layout_up;
}

const pico8_struct_class_t *pico8_lq_struct_class(void) {
    return &g_lq_struct_class;
}

const pico8_struct_class_t *pico8_jb_struct_class(void) {
    return &g_jb_struct_class;
}

pico8_lq_t *pico8_lq_new(const pico8_zone_choisie_t *zone, const pico8_lq_keys_t *keys) {
    return pico8_lq_struct_new(zone, keys);
}

pico8_jb_t *pico8_jb_new(const pico8_zone_choisie_t *zone, const pico8_jb_row_t *row) {
    return pico8_jb_struct_new(zone, row);
}
