#include "pico8_ob.h"

#include "pico8_ref_audit.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_fixed.h"
#include "pico8_str.h"

/* band(...,0xffff.fffe): a PICO-8 hex fixed-point literal whose raw 32-bit
 * form is its digits read straight off, i.e. a mask clearing only the
 * lowest bit. */
#define PICO8_OB_INITIAL_CAPACITY 4
#define PICO8_OB_NUMBER_MASK ((pico8_fixed_t)(uint32_t)0xFFFFFFFEu)

/* Not strdup(): POSIX, and this build is -std=c99. */
static char *pico8_ob_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

pico8_ob_table_t *pico8_ob_table_new(void) {
    pico8_ob_table_t *t = malloc(sizeof *t);
    t->tag = PICO8_TYPE_TAG_OB_TABLE;
    t->entries = NULL;
    t->count = 0;
    t->capacity = 0;
    return t;
}

pico8_ob_table_t *pico8_ob_table_from_list(const pico8_list_t *list) {
    pico8_ob_table_t *table = pico8_ob_table_new();
    pico8_list_iter_t it;
    pico8_value_t v;
    int i = 1;

    if (list == NULL) {
        return table;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &v)) {
        pico8_ob_table_set_index(table, i, v);
        i++;
    }
    return table;
}

void pico8_ob_table_free(pico8_ob_table_t *table) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    int i;
    if (table == NULL) {
        return;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_NAME) {
            free(table->entries[i].key.name);
        }
    }
    free(table->entries);
    free(table);
}

static void pico8_ob_table_grow(pico8_ob_table_t *table) {
    int new_capacity = table->capacity == 0 ? PICO8_OB_INITIAL_CAPACITY : table->capacity * 2;
    pico8_ob_entry_t *new_entries =
        realloc(table->entries, (size_t)new_capacity * sizeof(pico8_ob_entry_t));
    if (new_entries == NULL) {
        abort();
    }
    table->entries = new_entries;
    table->capacity = new_capacity;
}

static int pico8_ob_key_equal(pico8_ob_key_t a, pico8_ob_key_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    if (a.kind == PICO8_OB_KEY_INDEX) {
        return a.index == b.index;
    }
    return strcmp(a.name, b.name) == 0;
}

/* Takes ownership of `key.name` when the key is a named one, freeing it at
 * once if an entry of that name already exists. Every caller below hands it
 * a freshly owned name it does not need afterwards. */
static void pico8_ob_table_set(pico8_ob_table_t *table, pico8_ob_key_t key, pico8_value_t value) {
    int i;
    for (i = 0; i < table->count; i++) {
        if (pico8_ob_key_equal(table->entries[i].key, key)) {
            table->entries[i].value = value;
            if (key.kind == PICO8_OB_KEY_NAME) {
                free(key.name);
            }
            return;
        }
    }
    if (table->count == table->capacity) {
        pico8_ob_table_grow(table);
    }
    table->entries[table->count].key = key;
    table->entries[table->count].value = value;
    table->count++;
}

void pico8_ob_table_set_index(pico8_ob_table_t *table, int index, pico8_value_t value) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    pico8_ob_key_t key;
    key.kind = PICO8_OB_KEY_INDEX;
    key.index = index;
    key.name = NULL;
    pico8_ob_table_set(table, key, value);
}

void pico8_ob_table_set_name(pico8_ob_table_t *table, const char *name, pico8_value_t value) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    pico8_ob_key_t key;
    key.kind = PICO8_OB_KEY_NAME;
    key.index = 0;
    key.name = pico8_ob_strdup(name);
    pico8_ob_table_set(table, key, value);
}

void pico8_ob_table_remove_index(pico8_ob_table_t *table, int index) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    int i;
    if (table == NULL) {
        return;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX && table->entries[i].key.index == index) {
            int remaining = table->count - i - 1;
            if (remaining > 0) {
                memmove(&table->entries[i], &table->entries[i + 1], (size_t)remaining * sizeof(pico8_ob_entry_t));
            }
            table->count--;
            return;
        }
    }
}

int pico8_ob_table_get_index(const pico8_ob_table_t *table, int index, pico8_value_t *out_value) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    int i;
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX && table->entries[i].key.index == index) {
            *out_value = table->entries[i].value;
            return 1;
        }
    }
    return 0;
}

int pico8_ob_table_get_name(const pico8_ob_table_t *table, const char *name, pico8_value_t *out_value) {
    PICO8_REF_AUDIT_EXPECT_OB_TABLE(table);
    int i;
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_NAME &&
            strcmp(table->entries[i].key.name, name) == 0) {
            *out_value = table->entries[i].value;
            return 1;
        }
    }
    return 0;
}

static pico8_ob_constructor_fn pico8_ob_constructors[256];

void pico8_ob_register_constructor(char id, pico8_ob_constructor_fn fn) {
    pico8_ob_constructors[(unsigned char)id] = fn;
}

pico8_ob_constructor_fn pico8_ob_lookup_constructor(char id) {
    return pico8_ob_constructors[(unsigned char)id];
}

void pico8_ob_clear_constructors(void) {
    memset(pico8_ob_constructors, 0, sizeof pico8_ob_constructors);
}

pico8_list_t pico8_fi(const char *s, pico8_fi_fn fn, void *ctx) {
    pico8_list_t rs;
    int i;
    int len = (int)strlen(s);

    pico8_list_init(&rs);
    for (i = 1; i <= len; i++) {
        pico8_value_t v;
        if (fn(s[i - 1], i, ctx, &v)) {
            pico8_list_add(&rs, v);
        }
    }
    return rs;
}

/* Handles exactly what a spec-string number token can be: an optional '-',
 * digits, an optional '.' and more digits. No exponent, no hex. */
static pico8_fixed_t pico8_ob_parse_number(const char *tok) {
    const char *p = tok;
    int neg = 0;
    int64_t int_part = 0;
    int64_t frac_part = 0;
    int64_t frac_scale = 1;
    int64_t raw;

    if (*p == '-') {
        neg = 1;
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        int_part = int_part * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            frac_part = frac_part * 10 + (*p - '0');
            frac_scale *= 10;
            p++;
        }
    }

    raw = int_part << PICO8_FIXED_SHIFT;
    if (frac_scale > 1) {
        raw += (frac_part * 65536) / frac_scale;
    }
    if (neg) {
        raw = -raw;
    }
    return (pico8_fixed_t)(uint32_t)raw;
}

/* The value of one token: a quoted string, an `identifier(...)` call, the
 * literal false when the token starts with 'f', or a decimal number. */
static pico8_value_t pico8_ob_value_for_token(const char *qs, int s, int end, char sc) {
    int qs_len = (int)strlen(qs);
    /* The second character is read from the whole string, not clamped to the
     * token's end, exactly like the sub() it ports. */
    char second = ((size_t)s < (size_t)qs_len) ? qs[s] : '\0';

    if (sc == '"') {
        char *str = pico8_sub(qs, s + 1, end - 1);
        return pico8_value_string(str);
    }

    if (second == '(') {
        char ident = sc;
        char *inner = pico8_sub(qs, s + 2, end - 1);
        size_t inner_len = strlen(inner);
        char *inner_comma = malloc(inner_len + 2);
        pico8_ob_table_t *nested;
        pico8_ob_constructor_fn ctor;

        memcpy(inner_comma, inner, inner_len);
        inner_comma[inner_len] = ',';
        inner_comma[inner_len + 1] = '\0';
        free(inner);

        /* The recursive parse gets the trailing comma the inner text never
         * carries. */
        nested = pico8_ob(inner_comma, NULL);
        free(inner_comma);

        /* TROU-4 */
        ctor = pico8_ob_lookup_constructor(ident);
        if (ctor != NULL) {
            pico8_value_t absent = pico8_value_ref(NULL);
            pico8_value_t a1 = absent, a2 = absent, a3 = absent, a4 = absent, a5 = absent;
            pico8_value_t result;

            pico8_ob_table_get_index(nested, 1, &a1);
            pico8_ob_table_get_index(nested, 2, &a2);
            pico8_ob_table_get_index(nested, 3, &a3);
            pico8_ob_table_get_index(nested, 4, &a4);
            pico8_ob_table_get_index(nested, 5, &a5);

            result = ctor(a1, a2, a3, a4, a5);
            /* The five values were copied out above and nothing reaches back
             * into `nested`, so it is freed here rather than leaked. */
            pico8_ob_table_free(nested);
            return result;
        }
        /* No constructor for `ident`, the common case (a plain nested field
         * list, o(...)): `nested` itself is the value, and its ownership
         * passes to whatever stores it. */
        return pico8_value_ref(nested);
    }

    if (sc == 'f') {
        return pico8_value_bool(0);
    }

    {
        char *tok = pico8_sub(qs, s, end);
        pico8_fixed_t raw = pico8_ob_parse_number(tok);
        free(tok);
        return pico8_value_fixed(pico8_fixed_band(raw, PICO8_OB_NUMBER_MASK));
    }
}

/* The parse state fi()'s callback carries: `kn` the result table, `s` the
 * current token's 1-based start, `n` the current key, `lm` the paren depth.
 * `n.name`, when there is one, belongs to this struct alone. */
typedef struct {
    const char *qs;
    pico8_ob_table_t *kn;
    int s;
    pico8_ob_key_t n;
    int lm;
} pico8_ob_ctx_t;

/* `sc`, the current token's first character, is re-derived from ctx->s at
 * every character and must not be cached across calls: ctx->s moves. The
 * callback contributes no value, it only mutates *ctx. */
static int pico8_ob_step(char c, int i, void *ctx_ptr, pico8_value_t *out_value) {
    pico8_ob_ctx_t *ctx = ctx_ptr;
    char sc = ctx->qs[ctx->s - 1];
    int rq = i + 1;

    (void)out_value;

    if (c == '(') {
        ctx->lm++;
    } else if (c == ')') {
        ctx->lm--;
    } else if (ctx->lm == 0) {
        if (c == '=') {
            /* The key is everything from the token start up to this '='.
             * Once named, `n` stays that same name for every later field
             * until the next `key=`: there is no implicit return to
             * positional keys. */
            char *name = pico8_sub(ctx->qs, ctx->s, i - 1);
            if (ctx->n.kind == PICO8_OB_KEY_NAME) {
                free(ctx->n.name);
            }
            ctx->n.kind = PICO8_OB_KEY_NAME;
            ctx->n.name = name;
            ctx->s = rq;
        } else if (c == ',' && ctx->s < i) {
            /* The comma is what flushes a field. The `ctx->s < i` guard
             * drops an empty pending token (two commas in a row). The key
             * is stored as its own copy, since ctx->n keeps its name for the
             * fields that follow. */
            pico8_value_t value = pico8_ob_value_for_token(ctx->qs, ctx->s, i - 1, sc);
            pico8_ob_key_t key_copy = ctx->n;

            if (key_copy.kind == PICO8_OB_KEY_NAME) {
                key_copy.name = pico8_ob_strdup(ctx->n.name);
            }
            pico8_ob_table_set(ctx->kn, key_copy, value);
            ctx->s = rq;
            if (ctx->n.kind == PICO8_OB_KEY_INDEX) {
                ctx->n.index++;
            }
        } else if ((sc != '"' && c == ' ') || c == '\n') {
            /* Not a symmetric "skip whitespace outside quotes" rule: a
             * newline always advances the token start, a space only does so
             * outside a quoted token. */
            ctx->s = rq;
        }
    }

    return 0;
}

pico8_ob_table_t *pico8_ob(const char *s, const pico8_ob_table_t *defaults) {
    pico8_ob_ctx_t ctx;
    pico8_list_t discard;
    int i;

    ctx.qs = s;
    ctx.kn = pico8_ob_table_new();
    ctx.s = 1;
    ctx.n.kind = PICO8_OB_KEY_INDEX;
    ctx.n.index = 1;
    ctx.n.name = NULL;
    ctx.lm = 0;

    discard = pico8_fi(s, pico8_ob_step, &ctx);
    pico8_list_free(&discard);

    /* ctx.n's name is owned by this parse alone -- ctx.kn always got a copy
     * -- so it is freed here. */
    if (ctx.n.kind == PICO8_OB_KEY_NAME) {
        free(ctx.n.name);
    }

    /* Nothing flushes a token that no comma follows, so a spec that does not
     * end with one loses its last field, silently. Not asserted: the specs
     * decoded from cart memory end with two commas, and the pending-token
     * index lands differently there than on a source literal. */

    if (defaults != NULL) {
        for (i = 0; i < defaults->count; i++) {
            pico8_ob_key_t key_copy = defaults->entries[i].key;
            if (key_copy.kind == PICO8_OB_KEY_NAME) {
                key_copy.name = pico8_ob_strdup(defaults->entries[i].key.name);
            }
            pico8_ob_table_set(ctx.kn, key_copy, defaults->entries[i].value);
        }
    }

    return ctx.kn;
}
