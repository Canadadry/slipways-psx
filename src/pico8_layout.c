#include "pico8_layout.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_hud.h"
#include "pico8_list.h"

static int g_layout_up = 0;
static const pico8_struct_class_t g_lq_struct_class;

/* Lua truthiness : seuls nil et false sont faux. La clause REF-NULL n'est pas
 * optionnelle — un test qui ne rejette que BOOL false rend nil vrai, et le
 * `elseif f then` de `lq:mn` transformait alors un item nil en sprite dont
 * l'identifiant retombait à 0 : un bloc de données brutes dessiné à l'écran. */
static int pico8_layout_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* A nested row has no value form here: tuples and lines hold scalars. */
static pico8_value_t pico8_jb_item_value(const pico8_jb_item_t *item) {
    assert(item->row == NULL && "a tuple or a line holds scalar items only; a row inside one has no reader here");
    if (item->s != NULL) {
        return pico8_value_string(item->s);
    }
    if (item->is_false) {
        return pico8_value_bool(0);
    }
    if (item->is_ref) {
        return pico8_value_ref((void *)item->ref);
    }
    if (item->is_fixed) {
        return pico8_value_fixed(item->fixed);
    }
    return pico8_value_fixed(pico8_fixed_from_int(item->n));
}

/* Writes out_item plus mn's second and third Lua return values (out_w,
 * out_qv). `self` may be NULL; it is only read for the TEXT branch's colour.
 * A non-FIXED f[2]/f[3]/f[4] reads as 0. */
static void pico8_lq_mn(const pico8_lq_t *self, pico8_value_t f, pico8_lq_item_t *out_item,
                         pico8_fixed_t *out_w, pico8_fixed_t *out_qv) {
    pico8_fixed_t w = pico8_fixed_from_int(6);
    pico8_fixed_t qw = pico8_fixed_from_int(0);
    int have_qv = 0;
    pico8_fixed_t qv = pico8_fixed_from_int(0);

    if (f.kind == PICO8_VALUE_REF && f.as.ref != NULL) {
        /* An item REF is a tuple `{f,w,qw,qv}` or a `{qe=x}`, read without
         * interrogating its tag: a list or a table arriving here would be
         * read at the wrong layout, quietly. */
        const pico8_jb_row_t *t = f.as.ref;
        const pico8_jb_item_t wrap[] = {{.is_fixed = 1, .fixed = t->qe}, {.n = 5}};
        const pico8_jb_item_t *items = t->has_qe ? wrap : t->items;
        int count = t->has_qe ? 2 : t->count;
        pico8_value_t f1, f2, f3, f4;
        int have_f1 = count >= 1;
        int have_f2 = count >= 2;
        int have_f3 = count >= 3;
        int have_f4 = count >= 4;

        f1 = have_f1 ? pico8_jb_item_value(&items[0]) : pico8_value_bool(0);
        f2 = have_f2 ? pico8_jb_item_value(&items[1]) : pico8_value_bool(0);
        f3 = have_f3 ? pico8_jb_item_value(&items[2]) : pico8_value_bool(0);
        f4 = have_f4 ? pico8_jb_item_value(&items[3]) : pico8_value_bool(0);

        f = f1;
        if (have_f2 && f2.kind == PICO8_VALUE_FIXED) {
            w = f2.as.fixed;
        }
        if (have_f3 && pico8_layout_truthy(f3) && f3.kind == PICO8_VALUE_FIXED) {
            qw = f3.as.fixed;
        }
        if (have_f4 && pico8_layout_truthy(f4)) {
            have_qv = 1;
            qv = (f4.kind == PICO8_VALUE_FIXED) ? f4.as.fixed : pico8_fixed_from_int(0);
        }
    }

    if (f.kind == PICO8_VALUE_STRING && f.as.string != NULL) {
        pico8_fixed_t c = pico8_fixed_from_int(6);

        if (self != NULL && self->has_c) {
            c = self->c;
        }
        out_item->kind = PICO8_LQ_ITEM_TEXT;
        out_item->text = f.as.string;
        out_item->color = c;
        *out_w = pico8_fixed_from_int((int)strlen(f.as.string) * 4 + 3);
        *out_qv = pico8_fixed_from_int(1);
        return;
    }

    if (pico8_layout_truthy(f)) {
        out_item->kind = PICO8_LQ_ITEM_SPRITE;
        out_item->sprite_id = (f.kind == PICO8_VALUE_FIXED) ? f.as.fixed : pico8_fixed_from_int(0);
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
    int n = self->items.count;
    int i;

    /* TROU-7 */
    self->fs = malloc((size_t)(n > 0 ? n : 1) * sizeof *self->fs);
    assert(self->fs != NULL);
    for (i = 0; i < n; i++) {
        pico8_fixed_t w_i = pico8_fixed_from_int(0);
        pico8_fixed_t qv_i = pico8_fixed_from_int(0);
        pico8_fixed_t x = w;

        pico8_lq_mn(self, self->items.items[i], &self->fs[i], &w_i, &qv_i);
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
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT && self->header.cls == &g_lq_struct_class &&
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

/* The line `jb:pq` builds from one item of its row. */
static pico8_lq_t *pico8_jb_line(const pico8_jb_item_t *item) {
    pico8_lq_keys_t keys = {0};
    pico8_value_t s;
    pico8_list_t items;
    pico8_lq_t *line;
    int i;

    if (item->s != NULL) {
        s = pico8_value_string(item->s);
        keys.items = &s;
        keys.count = 1;
        return pico8_lq_new(&keys);
    }
    if (item->row == NULL) {
        return pico8_lq_new(NULL);
    }
    assert(item->row->fn == NULL && "jb: a line's named keys are c only; fn has no member -- stop, do not add one");
    pico8_list_init(&items);
    for (i = 0; i < item->row->count; i++) {
        pico8_list_add(&items, pico8_jb_item_value(&item->row->items[i]));
    }
    keys.items = items.items;
    keys.count = items.count;
    keys.has_c = item->row->has_c;
    keys.c = pico8_fixed_from_int(item->row->c);
    line = pico8_lq_new(&keys);
    pico8_list_free(&items);
    return line;
}

/* `ls` is allocated even for no line: `jb:ol` draws the frame of an empty
 * `jb`, and skips one whose `ls` is NULL. */
static void pico8_jb_pq(pico8_jb_t *self, const pico8_jb_row_t *row) {
    int n = row != NULL ? row->count : 0;
    int i;

    /* TROU-7 */
    self->ls = malloc((size_t)(n > 0 ? n : 1) * sizeof *self->ls);
    assert(self->ls != NULL);
    for (i = 0; i < n; i++) {
        self->ls[i] = pico8_jb_line(&row->items[i]);
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

static void pico8_lq_ol_dispatch(void *self, pico8_vec_t p, pico8_fixed_t a) {
    pico8_lq_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_lq_t *)self, p, a);
}

/* No launcher reaches it (`jo.lq` is an `lq`); a vector `om` counts as 0. */
static void pico8_jb_ol_dispatch(void *self, pico8_vec_t p, pico8_fixed_t a) {
    (void)p;
    pico8_jb_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_jb_t *)self, pico8_fixed_from_int(0), a);
}

pico8_value_t pico8_lq_bg(const pico8_lq_t *self) {
    if (self == NULL || !self->has_bg) {
        return pico8_value_ref(NULL);
    }
    return self->bg_is_bool ? pico8_value_bool(self->bg) : pico8_value_fixed(self->bg);
}

static const pico8_vec_t *lq_get_ra(const void *self) {
    return ((const pico8_lq_t *)self)->ra;
}

static const pico8_struct_class_t g_lq_struct_class = {
    .ol = pico8_lq_ol_dispatch,
    .get_ra = lq_get_ra,
};

/* TROU-7 */
static void *pico8_lq_struct_new(const pico8_lq_keys_t *keys) {
    pico8_lq_t *self = malloc(sizeof *self);
    int k;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_lq_struct_class;
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
    pico8_list_init(&self->items);
    if (keys != NULL) {
        for (k = 0; k < keys->count; k++) {
            pico8_list_add(&self->items, keys->items[k]);
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
    pico8_lq_pq(self);
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("lq");
#endif
    return self;
}

pico8_lq_keys_t pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_value_t *items, int cap) {
    pico8_lq_keys_t keys = {0};
    int i;

    assert(row->count <= cap && row->fn == NULL && !row->has_qe &&
           "lq: a row passes positionals and c; fn or qe has no member -- stop, do not add one");
    for (i = 0; i < row->count; i++) {
        items[i] = row->items[i].row != NULL ? pico8_value_ref((void *)row->items[i].row)
                                             : pico8_jb_item_value(&row->items[i]);
    }
    keys.items = items;
    keys.count = row->count;
    keys.has_c = row->has_c;
    keys.c = pico8_fixed_from_int(row->c);
    return keys;
}

pico8_lq_keys_t pico8_lq_keys_of(const pico8_lq_t *row) {
    pico8_lq_keys_t keys;

    keys.items = row->items.items;
    keys.count = row->items.count;
    keys.has_c = row->has_c;
    keys.c = row->c;
    keys.has_bg = row->has_bg;
    keys.bg_is_bool = row->bg_is_bool;
    keys.bg = row->bg;
    keys.has_score = row->has_score;
    keys.score = row->score;
    keys.ra = row->ra;
    return keys;
}

static const pico8_struct_class_t g_jb_struct_class = {
    .ol = pico8_jb_ol_dispatch,
};

/* The `jb` itself, before its lines. */
static pico8_jb_t *pico8_jb_struct_alloc(void) {
    pico8_jb_t *self = malloc(sizeof *self);

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jb");
#endif
    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_jb_struct_class;
    self->ls = NULL;
    self->ls_count = 0;
    return self;
}

static void *pico8_jb_struct_new(const pico8_jb_row_t *row) {
    pico8_jb_t *self;

    assert((row == NULL || (!row->has_c && row->fn == NULL)) &&
           "jb: every line is positional; a named key has no member -- stop, do not add one");
    self = pico8_jb_struct_alloc();
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

pico8_lq_t *pico8_lq_new(const pico8_lq_keys_t *keys) {
    return pico8_lq_struct_new(keys);
}

pico8_jb_t *pico8_jb_new(const pico8_jb_row_t *row) {
    return pico8_jb_struct_new(row);
}
