#include "pico8_layout.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_hud.h"
#include "pico8_list.h"
#include "pico8_ref_audit.h"

/* C has no closures, so lq:mn bakes the draw its Lua counterpart returns as a
 * closure into this tagged struct instead. Private to this file. */
typedef enum {
    PICO8_LQ_ITEM_NONE,
    PICO8_LQ_ITEM_TEXT,
    PICO8_LQ_ITEM_SPRITE
} pico8_lq_item_kind_t;

typedef struct {
    pico8_lq_item_kind_t kind;
    const char *text;        /* TEXT only; borrowed, never owned */
    pico8_fixed_t color;   /* TEXT only: self.c or 6 */
    pico8_fixed_t sprite_id; /* SPRITE only: f */
    pico8_fixed_t sprite_qw; /* SPRITE only: dv()'s palette-block arg */
    pico8_vec_t d;           /* offset within the line; set by pico8_lq_pq_step,
                              * not by pico8_lq_mn */
} pico8_lq_item_t;

static pico8_class_t *g_lq_class = NULL;
static pico8_class_t *g_jb_class = NULL;

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

/* Collects `table`'s PICO8_OB_KEY_INDEX entries in insertion order, which is
 * what Lua's all() walks on a purely positional table. Returns a new list the
 * caller owns and must release with pico8_list_free(). */
static pico8_list_t pico8_layout_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

/* Writes out_item plus mn's second and third Lua return values (out_w,
 * out_qv). `self` may be NULL; it is only read for the TEXT branch's colour.
 * A non-FIXED f[2]/f[3]/f[4] reads as 0. */
static void pico8_lq_mn(const pico8_instance_t *self, pico8_value_t f, pico8_lq_item_t *out_item,
                         pico8_fixed_t *out_w, pico8_fixed_t *out_qv) {
    pico8_fixed_t w = pico8_fixed_from_int(6);
    pico8_fixed_t qw = pico8_fixed_from_int(0);
    int have_qv = 0;
    pico8_fixed_t qv = pico8_fixed_from_int(0);

    if (f.kind == PICO8_VALUE_REF && f.as.ref != NULL) {
        /* An item REF is read as an ob_table without interrogating its tag. A
         * list arriving here would be walked at the wrong stride, quietly: see
         * pico8_type_tag.h. */
        const pico8_ob_table_t *orig = f.as.ref;
        const pico8_ob_table_t *t;
        pico8_ob_table_t *wrap = NULL;
        pico8_value_t qe;
        pico8_value_t f1, f2, f3, f4;
        int have_f1, have_f2, have_f3, have_f4;

        if (pico8_ob_table_get_name(orig, "qe", &qe) && pico8_layout_truthy(qe)) {
            wrap = pico8_ob_table_new();
            pico8_ob_table_set_index(wrap, 1, qe);
            pico8_ob_table_set_index(wrap, 2, pico8_value_fixed(pico8_fixed_from_int(5)));
            t = wrap;
        } else {
            t = orig;
        }

        have_f1 = pico8_ob_table_get_index(t, 1, &f1);
        have_f2 = pico8_ob_table_get_index(t, 2, &f2);
        have_f3 = pico8_ob_table_get_index(t, 3, &f3);
        have_f4 = pico8_ob_table_get_index(t, 4, &f4);

        f = have_f1 ? f1 : pico8_value_bool(0);
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

        if (wrap != NULL) {
            pico8_ob_table_free(wrap);
        }
    }

    if (f.kind == PICO8_VALUE_STRING && f.as.string != NULL) {
        pico8_value_t c_field;
        pico8_fixed_t c = pico8_fixed_from_int(6);

        if (self != NULL && pico8_instance_get_field(self, "c", &c_field) && pico8_layout_truthy(c_field) &&
            c_field.kind == PICO8_VALUE_FIXED) {
            c = c_field.as.fixed;
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

/* The `w`/`fs` upvalues lq:pq's Lua closure captures, threaded as an
 * explicit nv() context. */
typedef struct {
    const pico8_instance_t *self;
    pico8_fixed_t w;  /* running width accumulator */
    pico8_fixed_t fs; /* last item's spacing, retained after the loop ends */
} pico8_lq_pq_ctx_t;

static int pico8_lq_pq_step(pico8_value_t f, void *ctx_ptr, pico8_value_t *out_value) {
    pico8_lq_pq_ctx_t *ctx = ctx_ptr;
    /* TROU-7 */
    pico8_lq_item_t *item = malloc(sizeof *item);
    pico8_fixed_t w_i = pico8_fixed_from_int(0);
    pico8_fixed_t qv_i = pico8_fixed_from_int(0);
    pico8_fixed_t x = ctx->w;

    pico8_lq_mn(ctx->self, f, item, &w_i, &qv_i);
    item->d = pico8_vec_v(x, pico8_fixed_from_int(0));
    ctx->w = pico8_fixed_add(ctx->w, pico8_fixed_add(w_i, qv_i));
    ctx->fs = qv_i;

    *out_value = pico8_value_ref(item);
    return 1;
}

/* ctx.fs starts at 0 where Lua's own `fs` starts nil, so a zero-item lq()
 * yields w=0 here and would error in Lua. No call site builds one. */
static void pico8_lq_pq(pico8_instance_t *self) {
    pico8_list_t items = pico8_layout_positional_values(self->fields);
    pico8_lq_pq_ctx_t ctx;
    pico8_list_t *fs_heap;

    ctx.self = self;
    ctx.w = pico8_fixed_from_int(0);
    ctx.fs = pico8_fixed_from_int(0);

    /* TROU-7 */
    fs_heap = malloc(sizeof *fs_heap);
    *fs_heap = pico8_nv(&items, pico8_lq_pq_step, &ctx);
    pico8_list_free(&items);

    pico8_ob_table_set_name(self->fields, "fs", pico8_value_ref(fs_heap));
    pico8_ob_table_set_name(self->fields, "w", pico8_value_fixed(pico8_fixed_sub(ctx.w, ctx.fs)));
}

static void pico8_lq_ol_item_draw(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_item_t *item,
                                   pico8_vec_t p) {
    pico8_vec_t pos = pico8_vec_add(p, item->d);

    switch (item->kind) {
    case PICO8_LQ_ITEM_TEXT:
        pico8_io_default(gfx, mem, item->text, pico8_fixed_add(pos.x, pico8_fixed_from_int(2)),
                          pico8_fixed_add(pos.y, pico8_fixed_from_int(1)),
                          (int)pico8_fixed_to_double(item->color));
        break;
    case PICO8_LQ_ITEM_SPRITE:
        pico8_dv(mem, item->sprite_qw);
        pico8_spr_default(gfx, mem, (int)pico8_fixed_to_double(item->sprite_id), (int)pico8_fixed_to_double(pos.x),
                           (int)pico8_fixed_to_double(pos.y));
        pico8_dv_default(mem);
        break;
    case PICO8_LQ_ITEM_NONE:
    default:
        break;
    }
}

void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p,
                  pico8_fixed_t a) {
    pico8_value_t fs_val;
    pico8_value_t w_val;
    pico8_fixed_t w = pico8_fixed_from_int(0);
    pico8_list_iter_t it;
    pico8_value_t entry;

    if (self == NULL) {
        return;
    }
    if (pico8_instance_get_field(self, "w", &w_val) && w_val.kind == PICO8_VALUE_FIXED) {
        w = w_val.as.fixed;
    }
    p = pico8_vec_sub(p, pico8_vec_v(pico8_fixed_mul(a, w), pico8_fixed_from_int(0)));

    if (!pico8_instance_get_field(self, "fs", &fs_val) || fs_val.kind != PICO8_VALUE_REF || fs_val.as.ref == NULL) {
        return;
    }
    it = pico8_list_all(PICO8_REF_AS_LIST_CONST(fs_val.as.ref));
    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_lq_ol_item_draw(gfx, mem, (const pico8_lq_item_t *)entry.as.ref, p);
        }
    }
}

void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p) {
    pico8_lq_ol(gfx, mem, self, p, pico8_fixed_from_int(0));
}

/* A bare string `ln` becomes a one-item lq; any other REF is passed to lq as
 * the override table it already is. Any other kind builds an empty lq. */
static int pico8_jb_pq_step(pico8_value_t ln, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *wrapped = NULL;
    const pico8_ob_table_t *overrides;
    pico8_instance_t *line;

    (void)ctx;

    if (ln.kind == PICO8_VALUE_STRING) {
        wrapped = pico8_ob_table_new();
        pico8_ob_table_set_index(wrapped, 1, ln);
        overrides = wrapped;
    } else if (ln.kind == PICO8_VALUE_REF) {
        overrides = (const pico8_ob_table_t *)ln.as.ref;
    } else {
        overrides = NULL;
    }

    line = pico8_lq_new(overrides);
    if (wrapped != NULL) {
        pico8_ob_table_free(wrapped);
    }

    *out_value = pico8_value_ref(line);
    return 1;
}

static void pico8_jb_pq(pico8_instance_t *self) {
    pico8_list_t items = pico8_layout_positional_values(self->fields);
    /* TROU-7 */
    pico8_list_t *ls_heap = malloc(sizeof *ls_heap);

    *ls_heap = pico8_nv(&items, pico8_jb_pq_step, NULL);
    pico8_list_free(&items);

    pico8_ob_table_set_name(self->fields, "ls", pico8_value_ref(ls_heap));
}

/* Lua's `#self` is read here as `ls->count`: jb:pq contributes exactly one
 * `ls` entry per positional item, so the two counts never differ. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t om,
                  pico8_fixed_t a) {
    pico8_value_t ls_val;
    const pico8_list_t *ls;
    pico8_fixed_t h;
    pico8_fixed_t dn;
    pico8_list_iter_t it;
    pico8_value_t entry;

    if (self == NULL || !pico8_instance_get_field(self, "ls", &ls_val) || ls_val.kind != PICO8_VALUE_REF ||
        ls_val.as.ref == NULL) {
        return;
    }
    ls = PICO8_REF_AS_LIST_CONST(ls_val.as.ref);

    h = pico8_fixed_add(pico8_fixed_mul(pico8_fixed_from_int(ls->count), pico8_fixed_from_int(8)),
                         pico8_fixed_from_int(2));
    om = pico8_fixed_sub(om, pico8_fixed_mul(h, a));
    dn = pico8_fixed_add(om, h);

    pico8_fillp(gfx, 0x0F0F);
    pico8_rk(gfx, mem, om, dn, 0x10);
    pico8_fillp_reset(gfx);

    it = pico8_list_all(ls);
    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_vec_t pos = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_add(om, pico8_fixed_from_int(2)));
            pico8_lq_ol(gfx, mem, (const pico8_instance_t *)entry.as.ref, pos, pico8_fixed_from_double(0.5));
            om = pico8_fixed_add(om, pico8_fixed_from_int(8));
        }
    }
}

/* The only mv() call site passes argc 2, so this bridge ignores argc; an
 * arg1 that is not FIXED falls back to 0, which is `a` omitted. */
static pico8_value_t pico8_lq_ol_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_fixed_t a = pico8_fixed_from_int(0);
    (void)argc;

    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    if (arg1.kind == PICO8_VALUE_FIXED) {
        a = arg1.as.fixed;
    }
    pico8_lq_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self, p, a);
    return pico8_value_ref(NULL);
}

/* "ol" is wired onto lq's defaults as a real function field as well: a
 * grep for pico8_lq_ol misses the pico8_mv("ol") route that jo:kj uses. */
void pico8_layout_init(void) {
    pico8_layout_shutdown();
    g_lq_class = pico8_class_declare(NULL, NULL);
    pico8_class_set_pq(g_lq_class, pico8_lq_pq);
    pico8_ob_table_set_name(g_lq_class->defaults, "ol", pico8_value_function(pico8_lq_ol_dispatch));
    g_jb_class = pico8_class_declare(NULL, NULL);
    pico8_class_set_pq(g_jb_class, pico8_jb_pq);
}

void pico8_layout_shutdown(void) {
    if (g_lq_class != NULL) {
        pico8_class_free(g_lq_class);
        g_lq_class = NULL;
    }
    if (g_jb_class != NULL) {
        pico8_class_free(g_jb_class);
        g_jb_class = NULL;
    }
}

pico8_class_t *pico8_lq_class(void) {
    return g_lq_class;
}

pico8_class_t *pico8_jb_class(void) {
    return g_jb_class;
}

pico8_instance_t *pico8_lq_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_lq_class, overrides);
}

pico8_instance_t *pico8_jb_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_jb_class, overrides);
}
