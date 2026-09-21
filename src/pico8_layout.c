#include "pico8_layout.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_hud.h"
#include "pico8_list.h"

/* See pico8_layout.h's own top comment for lq's/jb's full real Lua source
 * and the design trace. This is the first chunk to call a real class
 * "like a function" from C code of its own (jb:pq's own `lq(...)`) --
 * pico8_lq_new/pico8_jb_new (bottom of this file) are the small, thin
 * wrappers this establishes for that idiom, alongside pico8_lq_class()/
 * pico8_jb_class() (mirroring pico8_ke.h's own pico8_ke_class()
 * convention) so Chunk 4+ can subclass/call these from other modules. */

/* lq:pq's own `fs` field is a list of {fn=<draw descriptor>,d=<position
 * offset>} entries; since C has no closures, `fn` (a real Lua closure
 * capturing `f`/`qw`/`c`) is instead baked into this small tagged struct
 * at lq:mn()-time (see pico8_lq_mn below) -- PICO8_LQ_ITEM_NONE for the
 * real `function()end` no-op case, TEXT for the `io(f,...)` closure case,
 * SPRITE for the `dv(qw)spr(f,...)dv()` closure case. Private to this
 * file: nothing outside it (pico8_lq_pq/pico8_lq_ol, both below) needs
 * this shape. */
typedef enum {
    PICO8_LQ_ITEM_NONE,
    PICO8_LQ_ITEM_TEXT,
    PICO8_LQ_ITEM_SPRITE
} pico8_lq_item_kind_t;

typedef struct {
    pico8_lq_item_kind_t kind;
    const char *text;      /* TEXT only; borrowed, not owned (matches
                             * pico8_value_t's own STRING ownership rule) */
    pico8_fixed_t color;   /* TEXT only: self.c or 6 */
    pico8_fixed_t sprite_id; /* SPRITE only: f */
    pico8_fixed_t sprite_qw; /* SPRITE only: dv()'s palette-block arg */
    pico8_vec_t d;          /* position offset within the line, set by
                             * pico8_lq_pq_step (below), not by mn() itself */
} pico8_lq_item_t;

static pico8_class_t *g_lq_class = NULL;
static pico8_class_t *g_jb_class = NULL;

/* Lua truthiness (only nil/false are falsy) -- same idiom already
 * established at pico8_audio.c:32, applied here to lq:mn's own several
 * `and`/`or` truthiness checks (`f.qe`, `f3 or qw`, `qv or 2`). */
static int pico8_layout_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

/* Not a port of any named Lua function: `nv(self,...)`/`all(self)` (real
 * lq:pq/jb:pq, pico8/lua.lua:290/315) iterate `self` itself as a list --
 * lq/jb instances hold ONLY positional items (their own spec is empty, so
 * mw(overrides) at instantiation copies exactly the caller's own
 * positional entries and nothing else, see pico8_layout.h's own top
 * comment) -- but pico8_nv (pico8_collect.h) takes a pico8_list_t, not a
 * pico8_ob_table_t/pico8_instance_t. This small glue helper bridges that
 * gap: it walks `table`'s own entries in insertion order and collects
 * every PICO8_OB_KEY_INDEX one, which is exactly `all()`'s own "walk the
 * array part in order" behavior for a table built purely positionally
 * (true of every real lq()/jb() call site in scope -- see
 * pico8_layout.h's own comment on their real call sites). Returns a new,
 * heap-backed list the caller owns and must release with
 * pico8_list_free(). */
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

/* function lq:mn(f)local w,qw,qv,fn=6,0
 * if type(f)=="table" then
 * if (f.qe)f={f.qe,5}
 * f,w,qw,qv=f[1],f[2],f[3]or qw,f[4]
 * end
 * if type(f)=="string" then
 * local c=self.c or 6
 * return function(p)io(f,p.x+2,p.y+1,c)end,#f*4+3,1
 * elseif f then
 * fn=function(p)dv(qw)spr(f,p.x,p.y)dv()end
 * end
 * return fn or function()end,w,qv or 2
 * end
 * (pico8/lua.lua:300-312.) Ported as an item-descriptor builder (writing
 * out_item, plus mn's own 2nd/3rd Lua return values via out_w/out_qv)
 * rather than returning a real Lua closure -- see this file's own top
 * comment on pico8_lq_item_t. `self` is only consulted for the TEXT
 * branch's `self.c or 6` (self may be NULL for a non-instance caller,
 * though every real caller in scope is lq:pq's own `self`).
 *
 * The `type(f)=="table"` branch's own `if (f.qe)f={f.qe,5}` reassigns `f`
 * to a BRAND NEW 2-element table before the shared 4-way destructure that
 * follows -- so this port first decides which table (`orig` or a freshly
 * built `wrap`) to destructure FROM, then applies ONE shared
 * get_index(1..4) pass to whichever it picked, exactly mirroring the
 * single Lua destructuring statement `f,w,qw,qv=f[1],f[2],f[3]or qw,f[4]`
 * running once against whichever table `f` now is. `f[3] or qw`/`f[4]`
 * (then, at the very end, `qv or 2`) all use Lua's own truthiness
 * short-circuit (pico8_layout_truthy), not just "is the key present" --
 * ported precisely rather than approximated, even though no real call
 * site in this chunk's own scope (only lq:pq's own string/plain-number
 * items) ever exercises the table-item branch at all (Chunk 4+'s js/hp
 * are the first real table-shaped `f`, not yet ported -- a non-FIXED
 * f[2]/f[3]/f[4] defensively reads as 0, matching
 * pico8_entity_geom.c's own established "wrong-kind field reads as zero"
 * divergence, until a real call site can be checked against). */
static void pico8_lq_mn(const pico8_instance_t *self, pico8_value_t f, pico8_lq_item_t *out_item,
                         pico8_fixed_t *out_w, pico8_fixed_t *out_qv) {
    pico8_fixed_t w = pico8_fixed_from_int(6);
    pico8_fixed_t qw = pico8_fixed_from_int(0);
    int have_qv = 0;
    pico8_fixed_t qv = pico8_fixed_from_int(0);

    if (f.kind == PICO8_VALUE_REF && f.as.ref != NULL) {
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

/* nv()'s own ctx for lq:pq's closure below (`function(f)f,fw,fs=self:mn(f)
 * local x=w
 * w+=fw+fs
 * return {fn=f,d=v(x,0)}
 * end` -- the running `w`/`fs` upvalues captured by that real Lua closure,
 * threaded here as an explicit ctx per pico8_nv_fn's own established
 * convention, matching pico8_ho's/pico8_jf's own "closure -> context
 * struct" precedent (pico8_collect.h/pico8_gen.h). */
typedef struct {
    const pico8_instance_t *self;
    pico8_fixed_t w;  /* running width accumulator */
    pico8_fixed_t fs; /* last item's own spacing (kept after the loop
                        * ends, matching the real Lua's own upvalue that
                        * simply retains its last-assigned value) */
} pico8_lq_pq_ctx_t;

static int pico8_lq_pq_step(pico8_value_t f, void *ctx_ptr, pico8_value_t *out_value) {
    pico8_lq_pq_ctx_t *ctx = ctx_ptr;
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

/* function lq:pq()local w,s,fs,f,fw=0,0
 * self.fs=nv(self,function(f)f,fw,fs=self:mn(f)local x=w
 * w+=fw+fs
 * return {fn=f,d=v(x,0)}
 * end)self.w=w-fs
 * end
 * (pico8/lua.lua:289-294.) `s` (declared but never read anywhere in the
 * real body) has no C equivalent -- dead Lua local, nothing to port. The
 * real `local w,s,fs,f,fw=0,0` only initializes `w`/`s`; `fs` starts Lua
 * nil, not 0 -- this port's own ctx.fs starts at pico8_fixed_from_int(0)
 * instead (a defensive default, since a genuinely 0-item lq() is not
 * evidenced by any real call site -- see pico8_layout.h's own comment on
 * lq()'s real call sites, all of which pass at least one item), documented
 * here rather than silently matched to Lua's own would-be-a-crash nil
 * case. */
static void pico8_lq_pq(pico8_instance_t *self) {
    pico8_list_t items = pico8_layout_positional_values(self->fields);
    pico8_lq_pq_ctx_t ctx;
    pico8_list_t *fs_heap;

    ctx.self = self;
    ctx.w = pico8_fixed_from_int(0);
    ctx.fs = pico8_fixed_from_int(0);

    fs_heap = malloc(sizeof *fs_heap);
    *fs_heap = pico8_nv(&items, pico8_lq_pq_step, &ctx);
    pico8_list_free(&items);

    pico8_ob_table_set_name(self->fields, "fs", pico8_value_ref(fs_heap));
    pico8_ob_table_set_name(self->fields, "w", pico8_value_fixed(pico8_fixed_sub(ctx.w, ctx.fs)));
}

/* `f.fn(p+f.d)` (real lq:ol's own loop body, pico8/lua.lua:297-298) for
 * one item -- calls whichever real closure pico8_lq_mn baked into `item`
 * (see this file's own top comment), at position `p+item->d`. Not a
 * literal Lua source line of its own (the real closures ARE lq:mn's own
 * inline `function(p)...end` bodies, already traced there); this is
 * where they actually get "invoked" against a concrete gfx/mem, since
 * this port has no first-class closure value to store and call instead. */
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

/* See pico8_layout.h's own comment on this declaration. */
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
    it = pico8_list_all((const pico8_list_t *)fs_val.as.ref);
    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_lq_ol_item_draw(gfx, mem, (const pico8_lq_item_t *)entry.as.ref, p);
        }
    }
}

void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p) {
    pico8_lq_ol(gfx, mem, self, p, pico8_fixed_from_int(0));
}

/* `function(ln)return lq(type(ln)=="string" and {ln}or ln)end` -- jb:pq's
 * own nv() closure (see pico8_jb_pq below): a bare string `ln` becomes a
 * one-item lq() (`{ln}`), any other `ln` (already a positional item-list
 * table, e.g. `{"a",5}`) is passed to lq() as-is. `ln.kind` other than
 * STRING/REF is not evidenced by any real jb() call site in scope (Chunk
 * 4/8/19's `jb(self.jb)`/`jb(dr[i])`, not yet ported) -- defensively
 * builds an empty-overrides lq() rather than guessing further. */
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

/* jb=kv:jr()function jb:pq()self.ls=nv(self,function(ln)
 * return lq(type(ln)=="string" and {ln}or ln)end)end
 * (pico8/lua.lua:314-315.) */
static void pico8_jb_pq(pico8_instance_t *self) {
    pico8_list_t items = pico8_layout_positional_values(self->fields);
    pico8_list_t *ls_heap = malloc(sizeof *ls_heap);

    *ls_heap = pico8_nv(&items, pico8_jb_pq_step, NULL);
    pico8_list_free(&items);

    pico8_ob_table_set_name(self->fields, "ls", pico8_value_ref(ls_heap));
}

/* See pico8_layout.h's own comment on this declaration. `#self` (real
 * Lua's own length of `self`, the jb instance's own positional item
 * count) is read here as `ls->count` instead -- identical in every real
 * case, since jb:pq's own nv() call always contributes exactly one `ls`
 * entry per positional `ln` item (its callback never omits a value), so
 * this doesn't need a second walk of self->fields just to recompute the
 * same count `ls` already has. */
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
    ls = (const pico8_list_t *)ls_val.as.ref;

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

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_lq_ol's own real signature -- see pico8_layout.h's own top
 * "CORRECTION" note on pico8_lq_ol for why this registration exists (a
 * real gap found while porting `jo:kj`, Chunk 13, sub-lot 5: the real
 * source's `mv(self.lq,"ol",p+v(0,7),0.5)` needs `lq`'s class-level "ol"
 * field to actually be PICO8_VALUE_FUNCTION-kind, which this module never
 * wired -- the two real call sites known when this module was first built,
 * `self.jb:ol(...)`/`c:ol(...)` in pico8_ui_button.c, are both plain `:`
 * method-call sugar, never `mv()`, so no gap was visible until now). `argc`
 * is always 2 at this real call site (`p`, `a`) -- an omitted `a` (this
 * port's own `pico8_lq_ol_default`) has no real `mv()` call site anywhere
 * in scope, so this bridge doesn't special-case `argc<2`; `arg1` not being
 * PICO8_VALUE_FIXED (never evidenced) falls back to 0, matching `a`
 * omitted numerically (see pico8_layout.h's own comment on pico8_lq_ol for
 * why that's an exact equivalence, not an approximation). gfx/mem come
 * from pico8_dispatch_context.h (pico8_dispatch_gfx/pico8_dispatch_mem),
 * the same shared "current frame's gfx/mem" convention every other
 * gfx/mem-needing dispatch bridge in this port already uses (see that
 * header's own top comment). */
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

/* lq=kv:jr()... / jb=kv:jr()... (pico8/lua.lua:289/314): both root
 * classes, no spec string (`kv:jr()`, NULL here, mirroring `rj or ""`'s
 * "no argument" case -- pico8_class_declare's own comment). Neither gets
 * an `ns`-metatable-based `__eq`/dispatch entry -- only their own `pq` is
 * set, exactly matching the real `function lq:pq()...end`/`function
 * jb:pq()...end` statements. `lq`'s own "ol" field is ALSO wired here as a
 * real `mv()`-routed field (`pico8_lq_ol_dispatch` above) -- see that
 * function's own comment for the full trace of this real, previously-
 * unwired gap. */
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
