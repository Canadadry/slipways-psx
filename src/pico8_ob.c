/* pico8/lua.lua:55-78 -- ob()/fi(), ported literally as a single linear
 * character scan (no general recursive-descent grammar), per close,
 * character-by-character reading of the real source (not just its prose
 * description elsewhere) cross-checked against every real ob()/pi call
 * site found by grepping pico8/lua.lua. The exact source:
 *
 *   function ob(qs,mr)local kn,s,n,lm=
 *   {},1,1,0
 *   fi(qs,function(c,i)local sc,rq=sub(qs,s,s),i+1
 *   if c=="(" then
 *   lm+=1
 *   elseif c==")" then
 *   lm-=1
 *   elseif lm==0 then
 *   if c=="=" then
 *   n,s=sub(qs,s,i-1),rq
 *   elseif c=="," and s<i then
 *   kn[n]=sc=='"'and sub(qs,s+1,i-2)or sub(qs,s+1,s+1)=="("and oo(pi[sc],ob(sub(qs,s+2,i-2)..","))or sc!="f"and band(sub(qs,s,i-1)+0,0xffff.fffe)s=rq
 *   if (type(n)=="number")n+=1
 *   elseif sc!='"'and c==" " or c=="\n" then
 *   s=rq
 *   end
 *   end
 *   end)
 *   return qj(kn,mr)end
 *   function fi(qs,fn)local rs={}
 *   for i=1,#qs do
 *   add(rs,fn(sub(qs,i,i),i)or nil)end
 *   return rs
 *   end
 *
 * plus, referenced by the value-computation line above:
 *   function oo(fn,a)
 *   return fn
 *   and fn(a[1],a[2],a[3],a[4],a[5])or a
 *   end
 *   pi={b=qz,v=v,br=rk,qr=io,rf=eq,s=spr}
 *   function qj(o,mr)for k,v in pairs(mr or {})do
 *   o[k]=v
 *   end
 *   return o
 *   end
 *
 * One subtlety this port deliberately preserves exactly as found, because
 * it is easy to mis-simplify when paraphrasing rather than reading the
 * Lua source's own operator precedence: the space/newline-skipping line
 *   elseif sc!='"'and c==" " or c=="\n" then
 * is `(sc~='"' and c==" ") or (c=="\n")` (Lua's `and` binds tighter than
 * `or`) -- a NEWLINE always advances the token start regardless of `sc`,
 * while a SPACE only does so when the current token isn't a quoted
 * string (sc~='"'). This is asymmetric, not "skip spaces/newlines outside
 * a quoted token" applied uniformly to both characters. No real spec
 * string in pico8/lua.lua is known to embed a literal newline character
 * inside a quoted-string token (every embedded newline found -- e.g.
 * bm's own multi-line literal, see pico8_ob.h's pico8_ob() comment and
 * this file's own test suite -- occurs between top-level comma-separated
 * fields, never inside quotes), so this distinction is not evidenced to
 * matter for any known real spec string, but it is what the literal
 * source does, so it is what this port does.
 *
 * A second subtlety, also preserved literally: `n` (the current key) is
 * only auto-incremented when it is currently a NUMBER (`if
 * (type(n)=="number")n+=1`). Once an explicit `key=` sets `n` to a
 * string, `n` stays that SAME string for every subsequent field until the
 * next explicit `key=` resets it -- there is no implicit "back to
 * positional" transition. Every real ob() spec string found in scope is
 * either purely positional or purely named, so this never actually bites
 * in practice, but a literal port preserves it rather than "fixing" it. */

#include "pico8_ob.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_fixed.h"
#include "pico8_str.h"

/* Number of C99 struct-literal fields aside, this is band(sub(...)+0,
 * 0xffff.fffe): 0xffff.fffe is a PICO-8 hex fixed-point literal, whose
 * raw 32-bit representation IS its hex digits read directly (integer part
 * 0xffff, fractional part 0xfffe) -- i.e. raw 0xFFFFFFFE, a mask that
 * clears only the least-significant bit. Applied via pico8_fixed_band
 * (already ported in pico8_fixed.c) rather than reimplemented here. */
#define PICO8_OB_INITIAL_CAPACITY 4
#define PICO8_OB_NUMBER_MASK ((pico8_fixed_t)(uint32_t)0xFFFFFFFEu)

/* Not strdup() (POSIX, not C99) -- this codebase's build is -std=c99. A
 * second small private copy of this exact helper (pico8_value.c already
 * has one, static/unexported) rather than exporting pico8_value.c's own:
 * this chunk's scope statement says not to modify Phase 2's existing
 * modules' existing functions beyond genuinely-needed additions, and a
 * 4-line owned-string-copy helper is not worth widening pico8_value.h's
 * public surface for. */
static char *pico8_ob_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

pico8_ob_table_t *pico8_ob_table_new(void) {
    pico8_ob_table_t *t = malloc(sizeof *t);
    t->entries = NULL;
    t->count = 0;
    t->capacity = 0;
    return t;
}

/* Frees this table's own backing storage and its own owned key-name
 * copies. Values are never touched here (see pico8_ob.h's comment on
 * pico8_ob_table_t for why) -- a nested table reached through a
 * PICO8_VALUE_REF value must be freed separately by the caller, and a
 * PICO8_VALUE_STRING value's string likewise. */
void pico8_ob_table_free(pico8_ob_table_t *table) {
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

/* The single place every write into an ob() result table goes through:
 * kn[key]=value, Lua-table-assignment style -- overwrite in place if `key`
 * already has an entry (by value: same index, or same name content, not
 * same name pointer), append a new entry otherwise. Takes ownership of
 * `key.name` when key.kind == PICO8_OB_KEY_NAME (frees it immediately if
 * it turns out to be a duplicate of an already-stored name, since the
 * existing entry's own copy is kept) -- every caller below passes a
 * freshly-owned name it doesn't need afterward, never a borrowed one. */
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
    pico8_ob_key_t key;
    key.kind = PICO8_OB_KEY_INDEX;
    key.index = index;
    key.name = NULL;
    pico8_ob_table_set(table, key, value);
}

void pico8_ob_table_set_name(pico8_ob_table_t *table, const char *name, pico8_value_t value) {
    pico8_ob_key_t key;
    key.kind = PICO8_OB_KEY_NAME;
    key.index = 0;
    key.name = pico8_ob_strdup(name);
    pico8_ob_table_set(table, key, value);
}

/* See pico8_ob.h's own comment on this declaration for the full Chunk 18
 * trace. Splices the matching entry out (memmove-ing every later entry
 * down by one) and decrements `count` -- the direct inverse of
 * pico8_ob_table_set's own append step. */
void pico8_ob_table_remove_index(pico8_ob_table_t *table, int index) {
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

/* The `pi` constructor table (pico8/lua.lua:133), keyed by a single char
 * -- see pico8_ob.h's comment on pico8_ob_constructor_fn for why a single
 * char (not the original's occasional two-letter key) is the right
 * domain here, and for this chunk's deliberate scope decision to leave
 * this legitimately empty of real constructors. A plain 256-entry array
 * indexed by the character's value is simplest and O(1); the character
 * set in scope (ASCII identifier letters) trivially fits. */
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

/* fi(qs,fn) (pico8/lua.lua:74-78) -- see pico8_ob.h's comment on this
 * declaration for how "collect non-nil results" is represented here
 * without a nil kind in pico8_value_t. */
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

/* Parses a plain decimal fixed-point literal token ("-1", "0.5", "15",
 * ...) -- the numeric-token half of pico8/lua.lua:66's `sub(qs,s,i-1)+0`
 * (Lua's "+0" idiom coercing a numeric string to a number). Deliberately
 * NOT routed through pico8_fixed_from_double (pico8_fixed.h's own comment
 * flags that constructor as a test/debugging convenience PICO-8 itself
 * never goes through) -- this is production parsing code, not a test, so
 * it computes the raw 16.16 bits directly from the decimal digits via
 * scaled integer arithmetic instead of a host-double round trip. Scoped
 * to exactly what real ob() spec-string number tokens look like: an
 * optional leading '-', decimal digits, an optional '.' and more decimal
 * digits -- no exponent, no hex float syntax (hex fixed-point literals
 * like 0xffff.fffe appear only as C source constants in this port, never
 * inside a spec string ob() itself parses). */
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

/* Computes the value for kn[n]=... (pico8/lua.lua:66):
 *
 *   kn[n]=sc=='"'and sub(qs,s+1,i-2)
 *     or sub(qs,s+1,s+1)=="("and oo(pi[sc],ob(sub(qs,s+2,i-2)..","))
 *     or sc!="f"and band(sub(qs,s,i-1)+0,0xffff.fffe)
 *
 * Lua's `A and B or C and D or E and F` groups as `(A and B) or (C and D)
 * or (E and F)` (and binds tighter than or), so this is a 3-way
 * ternary-by-idiom over the token spanning [s,end] (end == i-1):
 *   1. sc=='"'                    -> the quoted substring between the
 *      quotes, sub(qs,s+1,i-2) (i.e. [s+1,end-1]).
 *   2. sub(qs,s+1,s+1)=="("       -> an identifier(...) call: read the
 *      SECOND character directly from the full original string at
 *      position s+1 (0-based index s) -- NOT clamped to the token's own
 *      end, exactly like the real sub() call, which has no idea where
 *      the token ends either. If it's a nested call, recurse via ob() on
 *      the parenthesized contents (with a trailing "," appended, per
 *      pico8_ob() 's own header comment on why that's required) and pass
 *      it through oo(pi[sc],...): if pi[sc] (the constructor registered
 *      for the token's first character) exists, call it with the
 *      recursive parse's first 5 positional fields unpacked as arguments
 *      (oo's own `fn(a[1],a[2],a[3],a[4],a[5])`; a missing slot is Lua
 *      nil, represented here as pico8_value_ref(NULL)); otherwise (oo's
 *      `or a` fallback) the recursive parse's table itself is the value.
 *   3. sc!="f"                    -> a plain number: sub(qs,s,i-1)+0
 *      parsed and masked with 0xffff.fffe.
 * If none of the three conditions holds (only possible when sc=='f' and
 * neither of the first two match, i.e. the "false" literal), the whole
 * Lua expression evaluates to plain `false` -- sc=='f' is used as a
 * proxy for the literal `false`, never a full string comparison, exactly
 * as the PRD's "sc=="f" trick" describes. */
static pico8_value_t pico8_ob_value_for_token(const char *qs, int s, int end, char sc) {
    int qs_len = (int)strlen(qs);
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

        nested = pico8_ob(inner_comma, NULL);
        free(inner_comma);

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
            /* `nested`'s own positional values were already copied out
             * above as independent pico8_value_t's; nothing further
             * reaches back into `nested`'s own storage, so it's safe to
             * free it here rather than leak it -- this differs from the
             * "no constructor" branch below, where `nested` itself
             * becomes the returned value and must stay alive. */
            pico8_ob_table_free(nested);
            return result;
        }
        /* No constructor registered for `ident` (the common case, e.g.
         * plain nested field lists written as o(...)) -- oo(nil,a)'s `or
         * a` fallback: the recursive parse's own result table becomes the
         * value directly, with no wrapping constructor call. Ownership of
         * `nested` passes to whatever stores this returned value. */
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

/* ob()'s local state (pico8/lua.lua:55-56's `local kn,s,n,lm=
 * {},1,1,0`), threaded through fi()'s callback via ctx instead of Lua
 * upvalue capture: kn is the result table, s the current token's start
 * index, n the current key (starts as integer 1, see pico8_ob_key_t),
 * lm the paren-nesting depth. */
typedef struct {
    const char *qs;
    pico8_ob_table_t *kn;
    int s;
    pico8_ob_key_t n;
    int lm;
} pico8_ob_ctx_t;

/* fi()'s per-character callback body (pico8/lua.lua:57-72). `sc` is
 * re-derived from the CURRENT ctx->s on every call, exactly like the
 * original's `local sc,rq=sub(qs,s,s),i+1` -- s may have moved since the
 * previous character, so this is not cached across calls. This callback
 * never produces a value (matching the real callback's implicit `return`
 * of nil -- see pico8_fi()'s own comment on this file's own fi() port);
 * ob() relies purely on its side effects on *ctx. */
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
            /* n,s=sub(qs,s,i-1),rq -- the key name is everything since
             * the token start up to (excluding) this '=', and the next
             * token starts right after it. Overwrites whatever `n` was
             * before, unconditionally (see this file's top comment on
             * why `n` doesn't reset to positional between fields on its
             * own). */
            char *name = pico8_sub(ctx->qs, ctx->s, i - 1);
            if (ctx->n.kind == PICO8_OB_KEY_NAME) {
                free(ctx->n.name);
            }
            ctx->n.kind = PICO8_OB_KEY_NAME;
            ctx->n.name = name;
            ctx->s = rq;
        } else if (c == ',' && ctx->s < i) {
            /* kn[n]=...;s=rq;if type(n)=="number" then n+=1 end -- the
             * `s<i` guard mirrors `elseif c=="," and s<i`: an empty
             * pending token (e.g. a stray double comma) is silently
             * ignored, exactly as the original does nothing for it. `n`
             * is captured into its own owned copy (key_copy) before being
             * stored, since ctx->n's string, if any, keeps living in *ctx
             * for however many further fields reuse the same key (see
             * this file's top comment on that quirk). */
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
            /* See this file's top comment for why this is NOT a
             * symmetric "skip outside quotes" rule for both characters:
             * a newline always advances s; a space only does when the
             * current token isn't a quoted string. */
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

    /* Whatever `n` was left holding (e.g. a dangling "key=" with no
     * following comma to flush it, or simply the last field's own key if
     * it was named) is local to this parse and owned by `ctx` alone --
     * never stored into ctx.kn -- so it's freed here, not leaked. */
    if (ctx.n.kind == PICO8_OB_KEY_NAME) {
        free(ctx.n.name);
    }

    /* return qj(kn,mr)end -- qj(o,mr) is:
     *   function qj(o,mr)for k,v in pairs(mr or {})do
     *   o[k]=v
     *   end
     *   return o
     *   end
     * i.e. for every key in `mr` (here, `defaults`), o[k]=v OVERWRITES
     * o's existing entry at that key, if any. Read literally, this means
     * `defaults` WINS over whatever was just parsed for any key they
     * share -- the opposite of the everyday reading of "defaults" as
     * "fall back to this only where nothing else was specified". This is
     * deliberately implemented to match qj's actual assignment direction
     * (confirmed by qj's only other two call sites in scope, mw(o) =
     * qj({},o) and hy:pq()'s qj(self,ob([[...]])), both of which are
     * "copy this INTO that, overwriting" merges, never a fill-only-
     * missing-keys merge) rather than the PRD's own prose gloss on this
     * same rule ("fields already present in defaults are overwritten by
     * anything explicitly parsed"), which describes the opposite
     * direction. Evidenced real-world impact of this direction: line 875
     * `js(ob([[...]],{my=_init}))` uses the second argument to attach an
     * actual function reference (_init) that the ob() string grammar
     * itself has no syntax to express -- and that only works at all if
     * the second argument is authoritative for the keys it sets, not a
     * fallback a same-named string field could silently shadow. */
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
