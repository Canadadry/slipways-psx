#ifndef PICO8_COLLECT_H
#define PICO8_COLLECT_H

#include "pico8_geom.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Appends `pu` to the bucket `rc[nt]`, creating it if absent. A bucket is a
 * heap-allocated pico8_list_t that `rc` only borrows, through a
 * PICO8_VALUE_REF, and never frees. Two entry points because `nt` is a
 * number at some call sites and a string at others. */
void pico8_ft_index(pico8_ob_table_t *rc, int nt, pico8_value_t pu);
void pico8_ft_name(pico8_ob_table_t *rc, const char *nt, pico8_value_t pu);

/* Merges `mr` into `o` in place and returns `o`; `mr` may be NULL (nothing
 * to merge). The merge is shallow, so `o` and `mr` then share whatever
 * their REF and STRING values point at. */
pico8_ob_table_t *pico8_qj(pico8_ob_table_t *o, const pico8_ob_table_t *mr);

/* Returns a new table, shallow, that the caller owns and releases with
 * pico8_ob_table_free(). */
pico8_ob_table_t *pico8_mw(const pico8_ob_table_t *o);

/* TROU-3 */
/* A NULL `qk` is a member of nothing rather than an error, mirroring
 * all(nil). */
int pico8_gu(const pico8_list_t *qk, pico8_value_t kb);

/* `ow` gets each element plus `ctx`, this port's stand-in for the captured
 * upvalue of the original's closure. Returns a new heap-backed list the
 * caller releases with pico8_list_free(); a NULL `qk` gives an empty one. */
typedef int (*pico8_jp_predicate_fn)(pico8_value_t e, void *ctx);
pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx);

/* `kp` writes *out_value and returns 1 to contribute an element, 0 to
 * contribute nothing -- this port has no nil for `add(m,kp(e)or nil)` to
 * drop. Returns a new heap-backed list the caller releases with
 * pico8_list_free(). `qk` must not be NULL. */
typedef int (*pico8_nv_fn)(pico8_value_t e, void *ctx, pico8_value_t *out_value);
pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx);

/* Returns a new heap-backed list, `a`'s elements then `b`'s, that the
 * caller releases with pico8_list_free(). Neither argument may be NULL. */
pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b);

/* TROU-4 */
/* With `fn` NULL the result is `a` itself wrapped as a REF, so `a`'s
 * identity and lifetime stay the caller's; otherwise it is whatever `fn`
 * returns, called with a[1]..a[5], each missing slot being
 * pico8_value_ref(NULL). */
pico8_value_t pico8_oo(pico8_ob_constructor_fn fn, const pico8_ob_table_t *a);

/* TROU-3 */
/* Membership as pico8_gu, except that two PICO8_VALUE_STRINGs are compared
 * by content. */
int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb);

/* Returns a newly malloc'd string the caller must free(), and writes the
 * color into *out_color: `c` when d >= 0, 8 when d < 0. `ku` may be NULL
 * (no suffix). A caller that wants only the string may pass any `c`. */
char *pico8_ne(pico8_fixed_t d, int c, const char *ku, int *out_color);

/* The body of the closure `ho()` returns, applied to one record `ri`: names
 * `ri`'s positional fields after `mr`'s own positional entries, then
 * registers `ri` into `rc` under its `.px`. A field listed in `gp` (which
 * may be NULL) takes its value from the global `r` table. `rc` borrows
 * `ri`; a positional slot absent from `ri` leaves its named field unset. */
void pico8_ho_apply(pico8_ob_table_t *rc, const pico8_ob_table_t *mr, const pico8_list_t *gp, pico8_ob_table_t *ri);

/* Registers the `b` entry of `pi`; pico8_pi_init() calls it alongside the
 * other constructor registrations. */
void pico8_qz_register_constructor(void);

#endif
