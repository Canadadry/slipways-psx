/* See pico8_research_display.h's own top comment for why this module
 * exists separately from pico8_research.c (a pure module-size split, same
 * source region), and each declaration's own comment there for the full
 * Lua source/trace of every function ported here. */

#include "pico8_research_display.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_layout.h"
#include "pico8_research.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"
#include "pico8_ref_audit.h"

/* Lua truthiness (nil/false falsy, everything else truthy), extended to
 * also treat PICO8_VALUE_REF(NULL) as falsy -- same extension established
 * throughout this port (pico8_probe.c/pico8_tech_gate.c/pico8_research.c).
 * A private per-module copy, not shared with pico8_research.c's own
 * identical helper, per this file's own top comment on the size split. */
static int pico8_research_display_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Not a port of any named Lua function: private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t (used here
 * for `gb`, pico8_bj's own `jp(gb,...)`) -- a separate copy from
 * pico8_research.c's own identical helper, per this file's own top comment
 * on the size split. Returns a new, heap-backed list the caller owns and
 * must release with pico8_list_free(). */
static pico8_list_t pico8_research_display_positional_values(const pico8_ob_table_t *table) {
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

/* Not a port of any named Lua function: counts an ob_table's own POSITIONAL
 * entries only (Lua's own `#t` for a table built purely positionally) --
 * used by pico8_hp_lq below to compute the next free index before its own
 * two `add(l,...)` calls onto an already-`pq()`'d lq instance's raw fields
 * (see pico8_hp_lq's own header comment). */
static int pico8_research_display_count_positional(const pico8_ob_table_t *table) {
    int i;
    int count = 0;

    if (table == NULL) {
        return 0;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            count++;
        }
    }
    return count;
}

/* `for i in all(self.i)do it=(it and it..","or"")..iz[tostr(i.qe)]end`
 * (pico8/lua.lua:1416-1419) -- hp:jb's own comma-join loop, factored out
 * so pico8_hp_jb (below) stays focused on the surrounding table shape.
 * Growable-buffer string concatenation, matching pico8_unstash's own
 * established "incremental growable string" pattern (pico8_stash.c).
 * Returns a newly heap-allocated, NUL-terminated string the caller owns
 * and must free(). */
static char *pico8_hp_jb_join(const pico8_instance_t *self) {
    pico8_value_t i_val;
    const pico8_list_t *i_list;
    size_t cap = 16;
    size_t len = 0;
    char *out = malloc(cap);
    int first = 1;
    pico8_list_iter_t it;
    pico8_value_t entry;

    out[0] = '\0';
    if (!pico8_instance_get_field(self, "i", &i_val) || i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
        return out;
    }
    /* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY CONSTRUCTION.
     * Producer: pico8_hp_pq() (src/pico8_research.c), which ends by
     * replacing whatever the `ho()` rename pass wrote into "i" with the
     * heap pico8_list_t* pico8_fi() built. Sole writer of "i". */
    i_list = PICO8_REF_AS_LIST_CONST(i_val.as.ref);
    it = pico8_list_all(i_list);
    while (pico8_list_iter_next(&it, &entry)) {
        pico8_value_t qe = pico8_value_ref(NULL);
        char *num_str;
        pico8_value_t iz_val;
        const char *piece = "";
        size_t piece_len;
        size_t add_len;

        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_ob_table_get_name((const pico8_ob_table_t *)entry.as.ref, "qe", &qe);
        }
        num_str = pico8_value_tostr(qe);
        if (pico8_ob_table_get_name(pico8_iz(), num_str, &iz_val) && iz_val.kind == PICO8_VALUE_STRING &&
            iz_val.as.string != NULL) {
            piece = iz_val.as.string;
        }
        free(num_str);

        piece_len = strlen(piece);
        add_len = (first ? 0 : 1) + piece_len;
        if (len + add_len + 1 > cap) {
            cap = (len + add_len + 1) * 2;
            out = realloc(out, cap);
        }
        if (!first) {
            out[len++] = ',';
        }
        memcpy(out + len, piece, piece_len + 1);
        len += piece_len;
        first = 0;
    }
    return out;
}

/* function hp:jb()...end (pico8/lua.lua:1414-1421.) See
 * pico8_research_display.h's own top comment on this declaration for the
 * full trace. */
pico8_ob_table_t *pico8_hp_jb(const pico8_instance_t *self) {
    pico8_value_t ii_val;
    char *it;
    pico8_ob_table_t *inner;
    pico8_ob_table_t *outer;
    pico8_value_t o_val;

    if (self == NULL) {
        return NULL;
    }
    if (pico8_instance_get_field(self, "ii", &ii_val) && pico8_research_display_truthy(ii_val)) {
        return NULL;
    }

    it = pico8_hp_jb_join(self);

    inner = pico8_ob_table_new();
    pico8_ob_table_set_index(inner, 1, pico8_value_string(it));
    pico8_ob_table_set_index(inner, 2, pico8_value_fixed(pico8_fixed_from_int(13)));

    if (pico8_instance_get_field(self, "o", &o_val)) {
        char *o_str = pico8_value_tostr(o_val);
        pico8_value_t iz_o;

        if (pico8_ob_table_get_name(pico8_iz(), o_str, &iz_o)) {
            pico8_ob_table_set_index(inner, 3, iz_o);
        }
        free(o_str);
    }

    outer = pico8_ob_table_new();
    pico8_ob_table_set_index(outer, 1, pico8_value_ref(inner));
    return outer;
}

/* function jo.ex(is,o,jq,mj,d)...end (pico8/lua.lua:518-538.) See
 * pico8_research_display.h's own top comment on this declaration for the
 * full trace, including the `jo.em` forward reference. */
pico8_instance_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
                               pico8_fixed_t d) {
    pico8_ob_table_t *l;
    int lc;
    int is_count = (is != NULL) ? is->count : 0;
    pico8_value_t ml[2];
    int ml_count;
    int pe;
    int idx = 1;
    pico8_instance_t *result;

    if (!pico8_research_display_truthy(o)) {
        return NULL;
    }

    l = pico8_ob_table_new();
    lc = (is_count >= 3) ? 1 : 0;

    {
        pico8_value_t r_w;
        int o_is_r_w = pico8_ob_table_get_name(pico8_r(), "w", &r_w) && pico8_value_equal(o, r_w);

        if (o_is_r_w) {
            ml[0] = pico8_value_fixed(pico8_fixed_from_int(25));
            ml[1] = pico8_value_fixed(pico8_fixed_from_int(23));
            ml_count = 2;
        } else {
            ml[0] = o;
            ml_count = 1;
        }
    }

    pe = pico8_fixed_mul(jq, pico8_fixed_from_int(ml_count)) >= pico8_fixed_from_int(3);

    if (is != NULL) {
        pico8_list_iter_t it = pico8_list_all(is);
        pico8_value_t entry;

        while (pico8_list_iter_next(&it, &entry)) {
            pico8_ob_table_t *tuple = pico8_ob_table_new();
            pico8_value_t qe = pico8_value_ref(NULL);
            pico8_value_t gc;
            int gc_zero = 0;

            if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
                const pico8_ob_table_t *i_table = entry.as.ref;

                pico8_ob_table_get_name(i_table, "qe", &qe);
                if (pico8_ob_table_get_name(i_table, "gc", &gc) && gc.kind == PICO8_VALUE_FIXED &&
                    gc.as.fixed == 0) {
                    gc_zero = 1;
                }
            }
            pico8_ob_table_set_index(tuple, 1, qe);
            pico8_ob_table_set_index(
                tuple, 2, pico8_value_fixed(pico8_fixed_sub(pico8_fixed_from_int(3), pico8_fixed_from_int(lc))));
            /* Lua's `i.gc~=0 and 0 or 7`: gc==0 -> 7, gc~=0 (or absent) -> 0
             * (0 is truthy in Lua, so this is NOT the naive "gc==0 -> 0"
             * reading -- fixed here after a verifier caught the inverted
             * ternary). */
            pico8_ob_table_set_index(tuple, 3, pico8_value_fixed(pico8_fixed_from_int(gc_zero ? 7 : 0)));
            pico8_ob_table_set_index(l, idx++, pico8_value_ref(tuple));
        }
    }

    if (is_count > 0) {
        pico8_ob_table_t *t1 = pico8_ob_table_new();
        pico8_ob_table_t *t2 = pico8_ob_table_new();
        int use_35 = (d == pico8_fixed_from_int(1)) || pe;

        pico8_ob_table_set_index(t1, 1, pico8_value_bool(0));
        pico8_ob_table_set_index(t1, 2, pico8_value_fixed(pico8_fixed_add(d, pico8_fixed_from_int(lc))));
        pico8_ob_table_set_index(l, idx++, pico8_value_ref(t1));

        if (use_35) {
            pico8_ob_table_set_index(t2, 1, pico8_value_fixed(pico8_fixed_from_int(35)));
            pico8_ob_table_set_index(t2, 2, pico8_value_fixed(pico8_fixed_add(pico8_fixed_from_int(2), d)));
        } else {
            pico8_ob_table_set_index(t2, 1, pico8_value_fixed(pico8_fixed_from_int(32)));
            pico8_ob_table_set_index(t2, 2, pico8_value_fixed(pico8_fixed_from_int(5)));
        }
        pico8_ob_table_set_index(l, idx++, pico8_value_ref(t2));
    }

    if (jq > pico8_fixed_from_int(5)) {
        char *jq_str = pico8_value_tostr(pico8_value_fixed(jq));
        pico8_ob_table_t *t3 = pico8_ob_table_new();

        pico8_ob_table_set_index(l, idx++, pico8_value_string(jq_str));
        pico8_ob_table_set_index(t3, 1, pico8_value_bool(0));
        pico8_ob_table_set_index(t3, 2, pico8_value_fixed(pico8_fixed_from_int(-4)));
        pico8_ob_table_set_index(l, idx++, pico8_value_ref(t3));
        pico8_ob_table_set_name(l, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        jq = pico8_fixed_from_int(1);
    }

    {
        int qq_i;

        for (qq_i = 0; qq_i < ml_count; qq_i++) {
            pico8_value_t qq = ml[qq_i];
            pico8_fixed_t loop_bound = (jq > pico8_fixed_from_int(1)) ? jq : pico8_fixed_from_int(1);
            int n = (int)pico8_fixed_to_double(loop_bound);
            int i;
            pico8_class_t *jo_class;
            pico8_value_t em_val;

            for (i = 1; i <= n; i++) {
                pico8_ob_table_t *t4 = pico8_ob_table_new();
                pico8_fixed_t i_fixed = pico8_fixed_from_int(i);
                int within = (i_fixed <= jq) && (i_fixed > mj);

                pico8_ob_table_set_index(t4, 1, qq);
                pico8_ob_table_set_index(t4, 2, pico8_value_fixed(pico8_fixed_from_int(6)));
                pico8_ob_table_set_index(t4, 3, pico8_value_fixed(pico8_fixed_from_int(within ? 0 : 7)));
                pico8_ob_table_set_index(t4, 4, pico8_value_fixed(pico8_fixed_from_int(pe ? -4 : -3)));
                pico8_ob_table_set_index(l, idx++, pico8_value_ref(t4));
            }

            jo_class = pico8_class_lookup_named("jo");
            if (jo_class != NULL && pico8_ob_table_get_name(jo_class->defaults, "em", &em_val)) {
                pico8_ob_table_set_index(l, idx++, em_val);
            } else {
                pico8_ob_table_set_index(l, idx++, pico8_value_ref(NULL));
            }
        }
    }

    result = pico8_lq_new(l);
    pico8_ob_table_free(l);
    return result;
}

/* function hp:lq()...end (pico8/lua.lua:1409-1413.) See
 * pico8_research_display.h's own top comment on this declaration for the
 * full trace, including the deliberately preserved `self.lq` field-vs-
 * method naming collision. */
pico8_instance_t *pico8_hp_lq(pico8_instance_t *self) {
    pico8_value_t i_val;
    pico8_value_t o_val;
    pico8_value_t po1;
    pico8_fixed_t jq;
    const pico8_list_t *is = NULL;
    const pico8_ob_table_t *po;
    pico8_instance_t *l;
    pico8_value_t fk_val;
    pico8_value_t px_val;
    int next_index;

    if (self == NULL || pico8_hp_class() == NULL) {
        return NULL;
    }

    if (pico8_instance_get_field(self, "i", &i_val) && i_val.kind == PICO8_VALUE_REF && i_val.as.ref != NULL) {
        /* REVERSE-CAST AUDIT: SAFE BY CONSTRUCTION -- "i", producer
         * pico8_hp_pq() (src/pico8_research.c), sole writer. */
        is = PICO8_REF_AS_LIST_CONST(i_val.as.ref);
    }
    if (!pico8_instance_get_field(self, "o", &o_val)) {
        o_val = pico8_value_ref(NULL);
    }

    po = pico8_hp_po(self);
    jq = pico8_fixed_from_int(1);
    if (po != NULL && pico8_ob_table_get_index(po, 1, &po1) && po1.kind == PICO8_VALUE_FIXED) {
        jq = (po1.as.fixed > pico8_fixed_from_int(1)) ? po1.as.fixed : pico8_fixed_from_int(1);
    }

    l = pico8_jo_ex(is, o_val, jq, pico8_fixed_from_int(0), pico8_fixed_from_int(1));
    if (l == NULL) {
        return NULL;
    }

    next_index = pico8_research_display_count_positional(l->fields) + 1;
    if (pico8_ob_table_get_name(pico8_hp_class()->defaults, "fk", &fk_val)) {
        pico8_ob_table_set_index(l->fields, next_index++, fk_val);
    }
    if (pico8_instance_get_field(self, "px", &px_val)) {
        pico8_ob_table_set_index(l->fields, next_index++, px_val);
    }

    return l;
}

/* `bj(jo)`'s own ctx for its filter predicate below -- `jo` (renamed here
 * to `jo_instance`), the real Lua closure's own captured upvalue. */
typedef struct {
    const pico8_instance_t *jo_instance;
} pico8_bj_ctx_t;

/* `function(b)if oj(b.rr)then local mg fi(b.lr,function(k)mg=mg or
 * k==jo.nk.px end) return mg end end` (pico8/lua.lua:1392-1397) -- `bj`'s
 * own real Lua closure. See pico8_research_display.h's own top comment on
 * pico8_bj for the full trace, including why the `k==jo.nk.px` comparison
 * is a direct C-character comparison, not routed through pico8_gu/
 * pico8_gu_content at all. */
static int pico8_bj_predicate(pico8_value_t e, void *ctx_ptr) {
    pico8_bj_ctx_t *ctx = ctx_ptr;
    pico8_instance_t *b;
    pico8_value_t rr_val;
    pico8_value_t lr_val;
    pico8_value_t nk_val;
    pico8_value_t px_val;
    const char *px_str = NULL;
    int mg = 0;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    b = e.as.ref;

    if (!pico8_instance_get_field(b, "rr", &rr_val)) {
        rr_val = pico8_value_ref(NULL);
    }
    if (!pico8_oj(rr_val)) {
        return 0;
    }

    if (ctx->jo_instance != NULL && pico8_instance_get_field(ctx->jo_instance, "nk", &nk_val) &&
        nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL &&
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "px", &px_val) &&
        px_val.kind == PICO8_VALUE_STRING) {
        px_str = px_val.as.string;
    }

    if (px_str != NULL && px_str[0] != '\0' && px_str[1] == '\0' &&
        pico8_instance_get_field(b, "lr", &lr_val) && lr_val.kind == PICO8_VALUE_STRING &&
        lr_val.as.string != NULL) {
        size_t n = strlen(lr_val.as.string);
        size_t i;

        for (i = 0; i < n; i++) {
            if (lr_val.as.string[i] == px_str[0]) {
                mg = 1;
                break;
            }
        }
    }
    return mg;
}

/* function bj(jo)
 * return jp(gb,function(b)...end)
 * end
 * (pico8/lua.lua:1391-1398.) See pico8_research_display.h's own top
 * comment on this declaration for the full trace. */
pico8_list_t pico8_bj(const pico8_instance_t *jo_instance) {
    pico8_list_t items = pico8_research_display_positional_values(pico8_gb());
    pico8_bj_ctx_t ctx;
    pico8_list_t result;

    ctx.jo_instance = jo_instance;
    result = pico8_jp(&items, pico8_bj_predicate, &ctx);
    pico8_list_free(&items);
    return result;
}
