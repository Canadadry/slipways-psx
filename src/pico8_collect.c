#include "pico8_collect.h"

#include <stdlib.h>

/* function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
 * add(rc[nt],pu)end
 * See pico8_collect.h's comment on these two entry points for why one Lua
 * function (polymorphic on nt's Lua type) becomes two here. */
void pico8_ft_index(pico8_ob_table_t *rc, int nt, pico8_value_t pu) {
    pico8_value_t bucket_value;
    pico8_list_t *bucket;
    if (pico8_ob_table_get_index(rc, nt, &bucket_value)) {
        bucket = bucket_value.as.ref;
    } else {
        bucket = malloc(sizeof *bucket);
        pico8_list_init(bucket);
        pico8_ob_table_set_index(rc, nt, pico8_value_ref(bucket));
    }
    pico8_list_add(bucket, pu);
}

/* function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
 * add(rc[nt],pu)end
 * String-keyed sibling of pico8_ft_index above. */
void pico8_ft_name(pico8_ob_table_t *rc, const char *nt, pico8_value_t pu) {
    pico8_value_t bucket_value;
    pico8_list_t *bucket;
    if (pico8_ob_table_get_name(rc, nt, &bucket_value)) {
        bucket = bucket_value.as.ref;
    } else {
        bucket = malloc(sizeof *bucket);
        pico8_list_init(bucket);
        pico8_ob_table_set_name(rc, nt, pico8_value_ref(bucket));
    }
    pico8_list_add(bucket, pu);
}

/* function qj(o,mr)for k,v in pairs(mr or {})do
 * o[k]=v
 * end
 * return o
 * end
 * See pico8_collect.h's comment on this function for the merge direction
 * (mr wins) and why this duplicates pico8_ob.c's own inline defaults-merge
 * rather than sharing code with it. */
pico8_ob_table_t *pico8_qj(pico8_ob_table_t *o, const pico8_ob_table_t *mr) {
    int i;
    if (mr != NULL) {
        for (i = 0; i < mr->count; i++) {
            pico8_ob_key_t key = mr->entries[i].key;
            if (key.kind == PICO8_OB_KEY_NAME) {
                pico8_ob_table_set_name(o, key.name, mr->entries[i].value);
            } else {
                pico8_ob_table_set_index(o, key.index, mr->entries[i].value);
            }
        }
    }
    return o;
}

/* function mw(o)
 * return qj({},o)end */
pico8_ob_table_t *pico8_mw(const pico8_ob_table_t *o) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    return pico8_qj(t, o);
}

/* function gu(qk,kb)for e in all(qk)do
 * if (e==kb)return true
 * end
 * end
 * See pico8_collect.h's comment on this function for the flagged
 * pointer-vs-content string equality gap left open for whichever chunk
 * first needs it (Chunk 11's ho()). */
int pico8_gu(const pico8_list_t *qk, pico8_value_t kb) {
    pico8_list_iter_t it;
    pico8_value_t e;
    if (qk == NULL) {
        return 0;
    }
    it = pico8_list_all(qk);
    while (pico8_list_iter_next(&it, &e)) {
        if (pico8_value_equal(e, kb)) {
            return 1;
        }
    }
    return 0;
}

/* function jp(qk,ow)local f={}
 * for e in all(qk)do
 * if (ow(e))add(f,e)
 * end
 * return f
 * end */
pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx) {
    pico8_list_t f;
    pico8_list_iter_t it;
    pico8_value_t e;
    pico8_list_init(&f);
    it = pico8_list_all(qk);
    while (pico8_list_iter_next(&it, &e)) {
        if (ow(e, ctx)) {
            pico8_list_add(&f, e);
        }
    }
    return f;
}

/* function nv(qk,kp)local m={}
 * for e in all(qk)do
 * add(m,kp(e)or nil)end
 * return m
 * end
 * See pico8_collect.h's comment on this function for the "or nil"/
 * collect-non-nil-results translation, matching pico8_fi()'s convention. */
pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx) {
    pico8_list_t m;
    pico8_list_iter_t it;
    pico8_value_t e;
    pico8_list_init(&m);
    it = pico8_list_all(qk);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_value_t v;
        if (kp(e, ctx, &v)) {
            pico8_list_add(&m, v);
        }
    }
    return m;
}

/* local kg=function(e)add(t,e)end -- la()'s own inline closure, ported as
 * a plain pico8_list_foreach_fn taking the target list `t` as ctx. */
static void pico8_la_append(pico8_value_t value, void *ctx) {
    pico8_list_add((pico8_list_t *)ctx, value);
}

/* function la(a,b)local t={}
 * local kg=function(e)add(t,e)end
 * foreach(a,kg)foreach(b,kg)
 * return t
 * end */
pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b) {
    pico8_list_t t;
    pico8_list_init(&t);
    pico8_list_foreach(a, pico8_la_append, &t);
    pico8_list_foreach(b, pico8_la_append, &t);
    return t;
}

/* function oo(fn,a)
 * return fn
 * and fn(a[1],a[2],a[3],a[4],a[5])or a
 * end
 * See pico8_collect.h's comment on this function for the accepted
 * duplication against pico8_ob.c's own identical inline logic. */
pico8_value_t pico8_oo(pico8_ob_constructor_fn fn, const pico8_ob_table_t *a) {
    if (fn != NULL) {
        pico8_value_t absent = pico8_value_ref(NULL);
        pico8_value_t a1 = absent, a2 = absent, a3 = absent, a4 = absent, a5 = absent;
        pico8_ob_table_get_index(a, 1, &a1);
        pico8_ob_table_get_index(a, 2, &a2);
        pico8_ob_table_get_index(a, 3, &a3);
        pico8_ob_table_get_index(a, 4, &a4);
        pico8_ob_table_get_index(a, 5, &a5);
        return fn(a1, a2, a3, a4, a5);
    }
    return pico8_value_ref((void *)a);
}
