#include "pico8_research_display.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_entity.h"
#include "pico8_layout.h"
#include "pico8_map_tables.h"
#include "pico8_planet.h"
#include "pico8_research.h"
#include "pico8_tech_gate.h"

/* Lua truthiness, widened so this port's "absent" filler REF(NULL) is
 * falsy too. */
static int pico8_research_display_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Returns a newly heap-allocated, NUL-terminated string the caller owns
 * and must free(). */
static char *pico8_hp_jb_join(const pico8_hp_t *self) {
    const pico8_list_t *i_list;
    size_t cap = 16;
    size_t len = 0;
    char *out = malloc(cap);
    int first = 1;
    pico8_list_iter_t it;
    pico8_value_t entry;

    out[0] = '\0';
    if (self->i == NULL) {
        return out;
    }
    i_list = self->i;
    it = pico8_list_all(i_list);
    while (pico8_list_iter_next(&it, &entry)) {
        pico8_value_t qe = pico8_value_ref(NULL);
        char *num_str;
        const char *iz_name;
        const char *piece = "";
        size_t piece_len;
        size_t add_len;

        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            qe = pico8_jy_qe(entry.as.ref);
        }
        num_str = pico8_value_tostr(qe);
        iz_name = pico8_iz_lookup(num_str);
        if (iz_name != NULL) {
            piece = iz_name;
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

pico8_ob_table_t *pico8_hp_jb(const pico8_hp_t *self) {
    char *it;
    pico8_ob_table_t *inner;
    pico8_ob_table_t *outer;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->has_ii) {
        return NULL;
    }

    it = pico8_hp_jb_join(self);

    inner = pico8_ob_table_new();
    pico8_ob_table_set_index(inner, 1, pico8_value_string(it));
    pico8_ob_table_set_index(inner, 2, pico8_value_fixed(pico8_fixed_from_int(13)));

    if (self->has_o) {
        char *o_str = pico8_value_tostr(pico8_value_fixed(self->o));
        const char *iz_o = pico8_iz_lookup(o_str);

        if (iz_o != NULL) {
            pico8_ob_table_set_index(inner, 3, pico8_value_string(iz_o));
        }
        free(o_str);
    }

    outer = pico8_ob_table_new();
    pico8_ob_table_set_index(outer, 1, pico8_value_ref(inner));
    return outer;
}

/* A tuple of `lq:mn` and its items, in one block, never freed (TROU-7). */
typedef struct {
    pico8_jb_row_t row;
    pico8_jb_item_t items[4];
} pico8_jo_ex_tuple_t;

static pico8_jo_ex_tuple_t *pico8_jo_ex_tuple(int count) {
    pico8_jo_ex_tuple_t *t = calloc(1, sizeof *t);

    assert(t != NULL);
    t->row.count = count;
    t->row.items = t->items;
    return t;
}

static pico8_jb_item_t pico8_jo_ex_fixed(pico8_fixed_t x) {
    return (pico8_jb_item_t){.is_fixed = 1, .fixed = x};
}

pico8_lq_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
                               pico8_fixed_t d) {
    pico8_ob_table_t *l;
    int lc;
    int is_count = (is != NULL) ? is->count : 0;
    pico8_value_t ml[2];
    int ml_count;
    int pe;
    int idx = 1;
    pico8_lq_t *result;

    if (!pico8_research_display_truthy(o)) {
        return NULL;
    }

    l = pico8_ob_table_new();
    lc = (is_count >= 3) ? 1 : 0;

    {
        int r_w;
        /* TROU-3 */
        int o_is_r_w = pico8_r_lookup("w", &r_w) && pico8_value_equal(o, pico8_value_fixed(pico8_fixed_from_int(r_w)));

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
            pico8_jo_ex_tuple_t *tuple = pico8_jo_ex_tuple(3);
            pico8_value_t qe = pico8_value_ref(NULL);
            int gc_zero = 0;

            if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
                const pico8_jy_t *i = entry.as.ref;

                qe = pico8_jy_qe(i);
                gc_zero = i->has_gc && i->gc == 0;
            }
            assert((qe.kind == PICO8_VALUE_FIXED || (qe.kind == PICO8_VALUE_REF && qe.as.ref == NULL)) &&
                   "a want record's qe is a resource of r or nil");
            tuple->items[0] = qe.kind == PICO8_VALUE_FIXED ? pico8_jo_ex_fixed(qe.as.fixed)
                                                           : (pico8_jb_item_t){.is_ref = 1};
            tuple->items[1] = pico8_jo_ex_fixed(pico8_fixed_sub(pico8_fixed_from_int(3), pico8_fixed_from_int(lc)));
            /* Lua's `i.gc~=0 and 0 or 7`: gc==0 -> 7, gc~=0 or absent -> 0.
             * 0 is truthy in Lua, so it is NOT "gc==0 -> 0". */
            tuple->items[2] = pico8_jo_ex_fixed(pico8_fixed_from_int(gc_zero ? 7 : 0));
            pico8_ob_table_set_index(l, idx++, pico8_value_ref(&tuple->row));
        }
    }

    if (is_count > 0) {
        pico8_jo_ex_tuple_t *t1 = pico8_jo_ex_tuple(2);
        pico8_jo_ex_tuple_t *t2 = pico8_jo_ex_tuple(2);
        int use_35 = (d == pico8_fixed_from_int(1)) || pe;

        t1->items[0] = (pico8_jb_item_t){.is_false = 1};
        t1->items[1] = pico8_jo_ex_fixed(pico8_fixed_add(d, pico8_fixed_from_int(lc)));
        pico8_ob_table_set_index(l, idx++, pico8_value_ref(&t1->row));

        if (use_35) {
            t2->items[0] = pico8_jo_ex_fixed(pico8_fixed_from_int(35));
            t2->items[1] = pico8_jo_ex_fixed(pico8_fixed_add(pico8_fixed_from_int(2), d));
        } else {
            t2->items[0] = pico8_jo_ex_fixed(pico8_fixed_from_int(32));
            t2->items[1] = pico8_jo_ex_fixed(pico8_fixed_from_int(5));
        }
        pico8_ob_table_set_index(l, idx++, pico8_value_ref(&t2->row));
    }

    if (jq > pico8_fixed_from_int(5)) {
        char *jq_str = pico8_value_tostr(pico8_value_fixed(jq));
        static const pico8_jb_item_t t3_items[] = {{.is_false = 1}, {.n = -4}};
        static const pico8_jb_row_t t3 = {.count = 2, .items = t3_items};

        pico8_ob_table_set_index(l, idx++, pico8_value_string(jq_str));
        pico8_ob_table_set_index(l, idx++, pico8_value_ref((void *)&t3));
        pico8_ob_table_set_name(l, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        jq = pico8_fixed_from_int(1);
    }

    {
        int qq_i;

        for (qq_i = 0; qq_i < ml_count; qq_i++) {
            pico8_value_t qq = ml[qq_i];
            pico8_fixed_t loop_bound = (jq > pico8_fixed_from_int(1)) ? jq : pico8_fixed_from_int(1);
            int n = pico8_fixed_flr_int(loop_bound);
            int i;

            for (i = 1; i <= n; i++) {
                pico8_jo_ex_tuple_t *t4 = pico8_jo_ex_tuple(4);
                pico8_fixed_t i_fixed = pico8_fixed_from_int(i);
                int within = (i_fixed <= jq) && (i_fixed > mj);

                assert(qq.kind == PICO8_VALUE_FIXED && "ml holds sprite numbers");
                t4->items[0] = pico8_jo_ex_fixed(qq.as.fixed);
                t4->items[1] = pico8_jo_ex_fixed(pico8_fixed_from_int(6));
                t4->items[2] = pico8_jo_ex_fixed(pico8_fixed_from_int(within ? 0 : 7));
                t4->items[3] = pico8_jo_ex_fixed(pico8_fixed_from_int(pe ? -4 : -3));
                pico8_ob_table_set_index(l, idx++, pico8_value_ref(&t4->row));
            }

            pico8_ob_table_set_index(l, idx++, pico8_value_ref((void *)pico8_jo_em()));
        }
    }

    result = pico8_lq_new_table(l);
    pico8_ob_table_free(l);
    return result;
}

pico8_lq_t *pico8_hp_lq(pico8_hp_t *self) {
    pico8_fixed_t jq;
    const pico8_list_t *is = NULL;
    const pico8_hc_entry_t *po;
    pico8_lq_t *l;

    pico8_hp_assert(self);
    if (self == NULL || !pico8_hp_ready()) {
        return NULL;
    }

    if (self->i != NULL) {
        is = self->i;
    }

    po = pico8_hp_po(self);
    jq = pico8_fixed_from_int(1);
    if (po != NULL && po->count >= 1) {
        pico8_fixed_t po1 = pico8_fixed_from_int(po->values[0]);
        jq = (po1 > pico8_fixed_from_int(1)) ? po1 : pico8_fixed_from_int(1);
    }

    l = pico8_jo_ex(is, self->has_o ? pico8_value_fixed(self->o) : pico8_value_ref(NULL), jq, pico8_fixed_from_int(0), pico8_fixed_from_int(1));
    if (l == NULL) {
        return NULL;
    }

    pico8_list_add(&l->items, pico8_value_ref((void *)pico8_hp_fk()));
    if (self->px != NULL) {
        pico8_list_add(&l->items, pico8_value_string(self->px));
    }

    return l;
}

/* The filter closure's captured upvalue. */
typedef struct {
    const pico8_struct_header_t *jo_instance;
} pico8_bj_ctx_t;

/* `jo.nk.px` is one character wide by construction, so `k==jo.nk.px` is a
 * plain char comparison here -- no string identity involved. */
static int pico8_bj_predicate(pico8_value_t e, void *ctx_ptr) {
    pico8_bj_ctx_t *ctx = ctx_ptr;
    const pico8_hp_t *b;
    pico8_value_t nk_val;
    const char *px_str = NULL;
    int mg = 0;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    b = e.as.ref;
    pico8_hp_assert(b);

    if (!pico8_oj(b->rr != NULL ? pico8_value_string(b->rr) : pico8_value_ref(NULL))) {
        return 0;
    }

    if (ctx->jo_instance != NULL && pico8_entity_get_field(ctx->jo_instance, "nk", &nk_val) &&
        nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL) {
        px_str = ((const pico8_df_entry_t *)nk_val.as.ref)->px;
    }

    if (px_str != NULL && px_str[0] != '\0' && px_str[1] == '\0' && b->lr != NULL) {
        size_t n = strlen(b->lr);
        size_t i;

        for (i = 0; i < n; i++) {
            if (b->lr[i] == px_str[0]) {
                mg = 1;
                break;
            }
        }
    }
    return mg;
}

pico8_list_t pico8_bj(const pico8_struct_header_t *jo_instance) {
    pico8_list_t items;
    pico8_bj_ctx_t ctx;
    pico8_list_t result;
    int k;

    pico8_list_init(&items);
    for (k = 1; k <= pico8_gb_count(); k++) {
        pico8_list_add(&items, pico8_value_ref(pico8_gb_at(k)));
    }
    ctx.jo_instance = jo_instance;
    result = pico8_jp(&items, pico8_bj_predicate, &ctx);
    pico8_list_free(&items);
    return result;
}
