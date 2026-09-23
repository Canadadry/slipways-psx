#ifndef PICO8_OB_H
#define PICO8_OB_H

#include "pico8_list.h"
#include "pico8_type_tag.h"
#include "pico8_value.h"

/* A table built by ob() mixes positional keys (1, 2, 3, ... assigned as the
 * scan goes) and named keys ("x", "y", ...) in one table, so a key here is
 * tagged as either. */
typedef enum {
    PICO8_OB_KEY_INDEX,
    PICO8_OB_KEY_NAME
} pico8_ob_key_kind_t;

typedef struct {
    pico8_ob_key_kind_t kind;
    int index; /* valid when kind == PICO8_OB_KEY_INDEX */
    char *name; /* valid when kind == PICO8_OB_KEY_NAME; owned by whichever
                 * pico8_ob_table_t this key sits in (see pico8_ob_table_free) */
} pico8_ob_key_t;

typedef struct {
    pico8_ob_key_t key;
    pico8_value_t value;
} pico8_ob_entry_t;

/* An ordered list of {key, value} pairs, appended in parse order. A value
 * of kind PICO8_VALUE_REF may point at a nested pico8_ob_table_t. The table
 * owns its key names and nothing else: what a STRING or REF value points at
 * is never freed with it. */
typedef struct {
    /* Always PICO8_TYPE_TAG_OB_TABLE -- see pico8_type_tag.h. */
    pico8_type_tag_t tag;
    pico8_ob_entry_t *entries;
    int count;
    int capacity;
} pico8_ob_table_t;

/* Builds an empty table / releases a table's own storage and key names.
 * Every value it holds, a nested table reached through a REF included, is
 * the caller's to free separately. */
pico8_ob_table_t *pico8_ob_table_new(void);
void pico8_ob_table_free(pico8_ob_table_t *table);

/* Bridges a list to a fresh table for the call sites that build a value as
 * a list and then hand it to something copying a table's pairs. Entries come
 * out at keys 1..#list, in order. The table is the caller's; a NULL list
 * yields an empty one. */
pico8_ob_table_t *pico8_ob_table_from_list(const pico8_list_t *list);

/* table[index]/table[name] = value, Lua-assignment style: an existing entry
 * with an equal key (by value, for a name by content) is overwritten in
 * place, otherwise a new entry is appended. The name is copied into the
 * table's own storage, never retained by reference. */
void pico8_ob_table_set_index(pico8_ob_table_t *table, int index, pico8_value_t value);
void pico8_ob_table_set_name(pico8_ob_table_t *table, const char *name, pico8_value_t value);

/* Return 1 and write *out_value when found, 0 and leave *out_value
 * untouched otherwise: "absent" stays distinct from "present but
 * false/zero". */
int pico8_ob_table_get_index(const pico8_ob_table_t *table, int index, pico8_value_t *out_value);
int pico8_ob_table_get_name(const pico8_ob_table_t *table, const char *name, pico8_value_t *out_value);

/* TROU-9 */
/* Takes the entry away entirely, which no set_* can do -- `t[index]=nil`. A
 * no-op when `table` is NULL or carries no indexed entry there. The removed
 * entry's value is not freed, only its slot is spliced out. */
void pico8_ob_table_remove_index(pico8_ob_table_t *table, int index);

/* A constructor reachable from a spec string. Keyed by a single char
 * because ob() only ever looks up one. */
typedef pico8_value_t (*pico8_ob_constructor_fn)(pico8_value_t a1, pico8_value_t a2,
                                                  pico8_value_t a3, pico8_value_t a4,
                                                  pico8_value_t a5);

void pico8_ob_register_constructor(char id, pico8_ob_constructor_fn fn);
pico8_ob_constructor_fn pico8_ob_lookup_constructor(char id);
/* Test support only, with no counterpart in the original: empties the
 * table so one test's registration cannot leak into another's. */
void pico8_ob_clear_constructors(void);

/* Calls fn(c, i, ctx) for every character of `qs`, `i` being 1-based. `fn`
 * writes *out_value and returns 1 to contribute a value, 0 to contribute
 * nothing. The returned list is heap-backed and must be released with
 * pico8_list_free(); ob()'s own callback contributes nothing. */
typedef int (*pico8_fi_fn)(char c, int i, void *ctx, pico8_value_t *out_value);
pico8_list_t pico8_fi(const char *s, pico8_fi_fn fn, void *ctx);

/* Parses `s` into a new table, then merges `defaults` into it. `defaults`
 * OVERWRITES what was parsed for a shared key, despite its name -- that is
 * how a call site attaches a function reference the spec grammar cannot
 * express -- and may be NULL. The returned table, nested tables included, is
 * the caller's to release with pico8_ob_table_free(). `s` must end with a
 * comma, which the assert in pico8_ob() checks. */
pico8_ob_table_t *pico8_ob(const char *s, const pico8_ob_table_t *defaults);

#endif
