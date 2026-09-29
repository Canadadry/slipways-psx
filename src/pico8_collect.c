#include "pico8_collect.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_str.h"

pico8_ptrs_t pico8_jp_ptrs(const pico8_ptrs_t *qk, pico8_jp_ptrs_predicate_fn ow, void *ctx) {
    pico8_ptrs_t f;
    pico8_ptrs_iter_t it;
    void *e;
    pico8_ptrs_init(&f);
    if (qk == NULL) {
        return f;
    }
    it = pico8_ptrs_all(qk);
    while (pico8_ptrs_iter_next(&it, &e)) {
        if (ow(e, ctx)) {
            pico8_ptrs_add(&f, e);
        }
    }
    return f;
}

int pico8_gu_ptrs(const pico8_ptrs_t *qk, const void *kb) {
    pico8_ptrs_iter_t it;
    void *e;
    if (qk == NULL) {
        return 0;
    }
    it = pico8_ptrs_all(qk);
    while (pico8_ptrs_iter_next(&it, &e)) {
        if (e == kb) {
            return 1;
        }
    }
    return 0;
}

pico8_ptrs_t pico8_nv_ptrs(const pico8_ptrs_t *qk, pico8_nv_ptrs_fn kp, void *ctx) {
    pico8_ptrs_t m;
    pico8_ptrs_iter_t it;
    void *e;
    assert(qk != NULL && "nv: a NULL list is not tolerated here, unlike gu and jp");
    pico8_ptrs_init(&m);
    it = pico8_ptrs_all(qk);
    while (pico8_ptrs_iter_next(&it, &e)) {
        void *v;
        if (kp(e, ctx, &v)) {
            pico8_ptrs_add(&m, v);
        }
    }
    return m;
}

static void pico8_la_ptrs_append(void *e, void *ctx) {
    pico8_ptrs_add(ctx, e);
}

pico8_ptrs_t pico8_la_ptrs(const pico8_ptrs_t *a, const pico8_ptrs_t *b) {
    pico8_ptrs_t t;
    assert(a != NULL && b != NULL && "la: a NULL list is not tolerated here, unlike gu and jp");
    pico8_ptrs_init(&t);
    pico8_ptrs_foreach(a, pico8_la_ptrs_append, &t);
    pico8_ptrs_foreach(b, pico8_la_ptrs_append, &t);
    return t;
}

/* TROU-3 */
int pico8_gu_content_ptrs(const pico8_ptrs_t *qk, const char *kb) {
    pico8_ptrs_iter_t it;
    void *e;
    if (qk == NULL) {
        return 0;
    }
    it = pico8_ptrs_all(qk);
    while (pico8_ptrs_iter_next(&it, &e)) {
        if (e != NULL && strcmp(e, kb) == 0) {
            return 1;
        }
    }
    return 0;
}

char *pico8_ne(pico8_fixed_t d, int c, const char *ku, int *out_color) {
    char *num_str = pico8_tostr_fixed(d);
    const char *suffix = ku != NULL ? ku : "";
    char *result;
    size_t prefix_len;

    /* d < 0 keeps the '-' pico8_tostr_fixed already wrote; only the '+' of a
     * positive is added here. */
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

int pico8_ho_gp_has(const pico8_str_seq_t *gp, const char *p) {
    int j;

    for (j = 0; gp != NULL && j < gp->count; j++) {
        if (strcmp(gp->items[j], p) == 0) {
            return 1;
        }
    }
    return 0;
}
