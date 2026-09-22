#include "pico8_collect.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_stash.h"
#include "pico8_ref_audit.h"

/* function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
 * add(rc[nt],pu)end
 * See pico8_collect.h's comment on these two entry points for why one Lua
 * function (polymorphic on nt's Lua type) becomes two here. */
void pico8_ft_index(pico8_ob_table_t *rc, int nt, pico8_value_t pu) {
    pico8_value_t bucket_value;
    pico8_list_t *bucket;
    if (pico8_ob_table_get_index(rc, nt, &bucket_value)) {
        /* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY CONSTRUCTION.
         * Producer: this function's own `else` branch four lines below,
         * which is the ONLY thing in this port that ever writes an
         * indexed entry into a table pico8_ft_index is given -- the sole
         * caller is pico8_ui_gw (src/pico8_ui.c:396), and the table it
         * passes is the `ic` built by pico8_ob_table_new() a few lines
         * earlier in that same function (src/pico8_ui.c:380-381) and
         * never written to by anything else. */
        bucket = PICO8_REF_AS_LIST(bucket_value.as.ref);
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
        /* REVERSE-CAST AUDIT (pico8_ref_audit.h): NOT COVERED. Same
         * shape as pico8_ft_index above, but this function has NO caller
         * anywhere in src/ or tests/ (the real `ft(cv,...)`/`ft(cg,...)`
         * call sites of pico8/lua.lua:156-162 are ported in
         * pico8_entity.c as direct pico8_list_add/pico8_list_del calls on
         * the static `g_cv`/`g_cg` lists, never through ft()), so no
         * producer can be named and neither the replay nor the suite
         * reaches it. Reported as NOT COVERED rather than safe: an
         * unexercised site whose only justification would be "it looks
         * like its sibling" is exactly the reasoning that marked `js:pq`
         * safe. Whoever first gives this function a caller owns
         * classifying it then. */
        bucket = PICO8_REF_AS_LIST(bucket_value.as.ref);
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
    if (qk == NULL) {
        return f;
    }
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
    /* REVERSE-CAST AUDIT (pico8_ref_audit.h): examined and NOT a REF cast
     * -- the only `(pico8_list_t *)` in src/ that is left spelled out
     * rather than routed through PICO8_REF_AS_LIST. `ctx` is not a
     * pico8_value_t REF at all; it is the `&t` pico8_la passes four lines
     * below, a pico8_list_t this same file just pico8_list_init()ed on
     * its own stack, threaded through pico8_list_foreach's void* the way
     * every callback in this port carries its closure state. Nothing
     * else can ever reach this function. */
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

/* See pico8_collect.h's own comment on this declaration for why this
 * exists alongside pico8_gu rather than changing pico8_value_equal. */
int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb) {
    pico8_list_iter_t it;
    pico8_value_t e;
    if (qk == NULL) {
        return 0;
    }
    it = pico8_list_all(qk);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_STRING && kb.kind == PICO8_VALUE_STRING) {
            if (strcmp(e.as.string, kb.as.string) == 0) {
                return 1;
            }
        } else if (pico8_value_equal(e, kb)) {
            return 1;
        }
    }
    return 0;
}

/* function ne(d,c,ku)if d>=0 then
 * d="+"..d
 * else
 * c=8
 * end
 * return d..(ku or ""),c
 * end
 * See pico8_collect.h's own comment on this declaration. */
char *pico8_ne(pico8_fixed_t d, int c, const char *ku, int *out_color) {
    char *num_str = pico8_value_tostr(pico8_value_fixed(d));
    const char *suffix = ku != NULL ? ku : "";
    char *result;
    size_t prefix_len;

    if (d >= 0) {
        *out_color = c;
        prefix_len = 1 + strlen(num_str);
        result = malloc(prefix_len + strlen(suffix) + 1);
        result[0] = '+';
        memcpy(result + 1, num_str, strlen(num_str) + 1);
    } else {
        *out_color = 8;
        prefix_len = strlen(num_str);
        result = malloc(prefix_len + strlen(suffix) + 1);
        memcpy(result, num_str, strlen(num_str) + 1);
    }
    strcat(result, suffix);
    free(num_str);
    return result;
}

/* function ho(rc,mr,gp)
 * return function(ri)for i,p in pairs(mr)do
 * ri[p]=gu(gp,p)and r[ri[i]]or ri[i]
 * end
 * rc[ri.px]=ri
 * end
 * end
 * See pico8_collect.h's own comment on this declaration for the full
 * design trace (why mr is walked positionally only, gp's NULL/pico8_gu_
 * content handling, r[ri[i]]'s polymorphic-key lookup). */
void pico8_ho_apply(pico8_ob_table_t *rc, const pico8_ob_table_t *mr, const pico8_list_t *gp, pico8_ob_table_t *ri) {
    const pico8_ob_table_t *r = pico8_r();
    pico8_value_t px_val;
    int i;

    for (i = 0; i < mr->count; i++) {
        pico8_ob_key_t key = mr->entries[i].key;
        pico8_value_t p_val;
        const char *p;
        pico8_value_t raw;
        pico8_value_t new_value;

        if (key.kind != PICO8_OB_KEY_INDEX) {
            continue;
        }
        p_val = mr->entries[i].value;
        if (p_val.kind != PICO8_VALUE_STRING) {
            continue;
        }
        p = p_val.as.string;

        if (!pico8_ob_table_get_index(ri, key.index, &raw)) {
            continue;
        }

        if (pico8_gu_content(gp, p_val)) {
            new_value = raw;
            if (raw.kind == PICO8_VALUE_FIXED) {
                pico8_ob_table_get_index(r, (int)pico8_fixed_to_double(raw.as.fixed), &new_value);
            } else if (raw.kind == PICO8_VALUE_STRING) {
                pico8_ob_table_get_name(r, raw.as.string, &new_value);
            }
        } else {
            new_value = raw;
        }
        pico8_ob_table_set_name(ri, p, new_value);
    }

    if (pico8_ob_table_get_name(ri, "px", &px_val) && px_val.kind == PICO8_VALUE_STRING) {
        pico8_ob_table_set_name(rc, px_val.as.string, pico8_value_ref(ri));
    }
}

/* pi['b'] (pico8/lua.lua:133) -- see pico8_collect.h's own comment on this
 * declaration. */
static pico8_value_t pico8_qz_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                           pico8_value_t a5) {
    (void)a5;
    return pico8_value_ref(pico8_qz(a1.as.fixed, a2.as.fixed, a3.as.fixed, a4.as.fixed));
}

void pico8_qz_register_constructor(void) {
    pico8_ob_register_constructor('b', pico8_qz_constructor);
}
