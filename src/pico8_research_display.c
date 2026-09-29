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
#include "pico8_str.h"
#include "pico8_tech_gate.h"

/* Returns a newly heap-allocated, NUL-terminated string the caller owns
 * and must free(). */
static char *pico8_hp_jb_join(const pico8_hp_t *self) {
    const pico8_ptrs_t *i_list;
    size_t cap = 16;
    size_t len = 0;
    char *out = malloc(cap);
    int first = 1;
    pico8_ptrs_iter_t it;
    void *entry;

    out[0] = '\0';
    if (self->i == NULL) {
        return out;
    }
    i_list = self->i;
    it = pico8_ptrs_all(i_list);
    while (pico8_ptrs_iter_next(&it, &entry)) {
        pico8_qe_t qe = {.kind = PICO8_QE_NIL};
        char *num_str;
        const char *iz_name;
        const char *piece = "";
        size_t piece_len;
        size_t add_len;

        if (entry != NULL) {
            qe = pico8_jy_qe(entry);
        }
        num_str = qe.kind == PICO8_QE_FIXED ? pico8_tostr_fixed(qe.n) : NULL;
        iz_name = num_str != NULL ? pico8_iz_lookup(num_str) : NULL;
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

/* The `jb` of `hp:jb`, its one line and their items, in one block. */
typedef struct {
    pico8_jb_row_t outer;
    pico8_jb_item_t outer_item;
    pico8_jb_row_t inner;
    pico8_jb_item_t inner_items[3];
} pico8_hp_jb_t;

const pico8_jb_row_t *pico8_hp_jb(const pico8_hp_t *self) {
    pico8_hp_jb_t *jb;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->has_ii) {
        return NULL;
    }

    jb = calloc(1, sizeof *jb);
    assert(jb != NULL);
    jb->inner_items[0].s = pico8_hp_jb_join(self);
    jb->inner_items[1].n = 13;
    jb->inner.count = 2;
    jb->inner.items = jb->inner_items;

    if (self->has_o) {
        char *o_str = pico8_tostr_fixed(self->o);
        const char *iz_o = pico8_iz_lookup(o_str);

        if (iz_o != NULL) {
            jb->inner_items[2].s = iz_o;
            jb->inner.count = 3;
        }
        free(o_str);
    }

    jb->outer_item.row = &jb->inner;
    jb->outer.count = 1;
    jb->outer.items = &jb->outer_item;
    return &jb->outer;
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

pico8_lq_t *pico8_jo_ex(const pico8_ptrs_t *is, const pico8_fixed_t *o, pico8_fixed_t jq, pico8_fixed_t mj,
                               pico8_fixed_t d) {
    pico8_value_t *l;
    int l_count = 0;
    int l_cap;
    pico8_lq_keys_t keys = {0};
    int lc;
    int is_count = (is != NULL) ? is->count : 0;
    pico8_fixed_t ml[2];
    int ml_count;
    int pe;
    pico8_lq_t *result;

    if (o == NULL) {
        return NULL;
    }

    lc = (is_count >= 3) ? 1 : 0;

    {
        int r_w;
        int o_is_r_w = pico8_r_lookup("w", &r_w) && *o == pico8_fixed_from_int(r_w);

        if (o_is_r_w) {
            ml[0] = pico8_fixed_from_int(25);
            ml[1] = pico8_fixed_from_int(23);
            ml_count = 2;
        } else {
            ml[0] = *o;
            ml_count = 1;
        }
    }

    pe = pico8_fixed_mul(jq, pico8_fixed_from_int(ml_count)) >= pico8_fixed_from_int(3);

    {
        pico8_fixed_t jq_drawn = jq > pico8_fixed_from_int(5) ? pico8_fixed_from_int(1) : jq;
        pico8_fixed_t loop_bound = (jq_drawn > pico8_fixed_from_int(1)) ? jq_drawn : pico8_fixed_from_int(1);

        l_cap = is_count + (is_count > 0 ? 2 : 0) + (jq > pico8_fixed_from_int(5) ? 2 : 0) +
                ml_count * (pico8_fixed_flr_int(loop_bound) + 1);
    }
    l = malloc((size_t)l_cap * sizeof *l);
    assert(l != NULL);

    if (is != NULL) {
        pico8_ptrs_iter_t it = pico8_ptrs_all(is);
        void *entry;

        while (pico8_ptrs_iter_next(&it, &entry)) {
            pico8_jo_ex_tuple_t *tuple = pico8_jo_ex_tuple(3);
            pico8_qe_t qe = {.kind = PICO8_QE_NIL};
            int gc_zero = 0;

            if (entry != NULL) {
                const pico8_jy_t *i = entry;

                qe = pico8_jy_qe(i);
                gc_zero = i->has_gc && i->gc == 0;
            }
            tuple->items[0] = qe.kind == PICO8_QE_FIXED ? pico8_jo_ex_fixed(qe.n)
                                                           : (pico8_jb_item_t){.is_ref = 1};
            tuple->items[1] = pico8_jo_ex_fixed(pico8_fixed_sub(pico8_fixed_from_int(3), pico8_fixed_from_int(lc)));
            /* Lua's `i.gc~=0 and 0 or 7`: gc==0 -> 7, gc~=0 or absent -> 0.
             * 0 is truthy in Lua, so it is NOT "gc==0 -> 0". */
            tuple->items[2] = pico8_jo_ex_fixed(pico8_fixed_from_int(gc_zero ? 7 : 0));
            l[l_count++] = pico8_value_ref(&tuple->row);
        }
    }

    if (is_count > 0) {
        pico8_jo_ex_tuple_t *t1 = pico8_jo_ex_tuple(2);
        pico8_jo_ex_tuple_t *t2 = pico8_jo_ex_tuple(2);
        int use_35 = (d == pico8_fixed_from_int(1)) || pe;

        t1->items[0] = (pico8_jb_item_t){.is_false = 1};
        t1->items[1] = pico8_jo_ex_fixed(pico8_fixed_add(d, pico8_fixed_from_int(lc)));
        l[l_count++] = pico8_value_ref(&t1->row);

        if (use_35) {
            t2->items[0] = pico8_jo_ex_fixed(pico8_fixed_from_int(35));
            t2->items[1] = pico8_jo_ex_fixed(pico8_fixed_add(pico8_fixed_from_int(2), d));
        } else {
            t2->items[0] = pico8_jo_ex_fixed(pico8_fixed_from_int(32));
            t2->items[1] = pico8_jo_ex_fixed(pico8_fixed_from_int(5));
        }
        l[l_count++] = pico8_value_ref(&t2->row);
    }

    if (jq > pico8_fixed_from_int(5)) {
        char *jq_str = pico8_tostr_fixed(jq);
        static const pico8_jb_item_t t3_items[] = {{.is_false = 1}, {.n = -4}};
        static const pico8_jb_row_t t3 = {.count = 2, .items = t3_items};

        l[l_count++] = pico8_value_string(jq_str);
        l[l_count++] = pico8_value_ref((void *)&t3);
        keys.has_c = 1;
        keys.c = pico8_fixed_from_int(13);
        jq = pico8_fixed_from_int(1);
    }

    {
        int qq_i;

        for (qq_i = 0; qq_i < ml_count; qq_i++) {
            pico8_fixed_t qq = ml[qq_i];
            pico8_fixed_t loop_bound = (jq > pico8_fixed_from_int(1)) ? jq : pico8_fixed_from_int(1);
            int n = pico8_fixed_flr_int(loop_bound);
            int i;

            for (i = 1; i <= n; i++) {
                pico8_jo_ex_tuple_t *t4 = pico8_jo_ex_tuple(4);
                pico8_fixed_t i_fixed = pico8_fixed_from_int(i);
                int within = (i_fixed <= jq) && (i_fixed > mj);

                t4->items[0] = pico8_jo_ex_fixed(qq);
                t4->items[1] = pico8_jo_ex_fixed(pico8_fixed_from_int(6));
                t4->items[2] = pico8_jo_ex_fixed(pico8_fixed_from_int(within ? 0 : 7));
                t4->items[3] = pico8_jo_ex_fixed(pico8_fixed_from_int(pe ? -4 : -3));
                l[l_count++] = pico8_value_ref(&t4->row);
            }

            l[l_count++] = pico8_value_ref((void *)pico8_jo_em());
        }
    }

    assert(l_count == l_cap && "jo.ex: its row is sized at entry from #is, jq and ml; a positional more or less "
                               "is a miscount of that size");
    keys.items = l;
    keys.count = l_count;
    result = pico8_lq_new(&keys);
    free(l);
    return result;
}

pico8_lq_t *pico8_hp_lq(pico8_hp_t *self) {
    pico8_fixed_t jq;
    const pico8_ptrs_t *is = NULL;
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

    l = pico8_jo_ex(is, self->has_o ? &self->o : NULL, jq, pico8_fixed_from_int(0), pico8_fixed_from_int(1));
    if (l == NULL) {
        return NULL;
    }

    pico8_lq_add(l, pico8_value_ref((void *)pico8_hp_fk()));
    if (self->px != NULL) {
        pico8_lq_add(l, pico8_value_string(self->px));
    }

    return l;
}

/* `jo.nk.px` is one character wide by construction, so `k==jo.nk.px` is a
 * plain char comparison here -- no string identity involved. */
static int pico8_bj_predicate(const pico8_hp_t *b, const pico8_struct_header_t *jo_instance) {
    const char *px_str = NULL;
    int mg = 0;

    pico8_hp_assert(b);

    if (!pico8_oj(b->rr)) {
        return 0;
    }

    if (jo_instance != NULL) {
        const pico8_jo_t *jo;

        assert(pico8_jo_is_family(jo_instance) && "bj: jo:ex passes its own planet");
        jo = (const pico8_jo_t *)jo_instance;
        if (jo->nk != NULL) {
            px_str = jo->nk->px;
        }
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

pico8_bj_kept_t pico8_bj(const pico8_struct_header_t *jo_instance) {
    pico8_bj_kept_t result;
    int k;

    assert(pico8_gb_count() <= PICO8_FD_COUNT && "gb holds one hp per entry of fd at most: #fd bounds bj");
    result.count = 0;
    for (k = 1; k <= pico8_gb_count(); k++) {
        pico8_hp_t *b = pico8_gb_at(k);

        if (pico8_bj_predicate(b, jo_instance)) {
            result.items[result.count++] = b;
        }
    }
    return result;
}
