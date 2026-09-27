#ifndef PICO8_PLANET_FAMILY_TRADE_H
#define PICO8_PLANET_FAMILY_TRADE_H

#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* TROU-8 */
/* Calls `pico8_pi_init()` first: `nr`'s spec contains `b(-4,-4,5,5)`, so
 * the 'b' constructor must already be registered when it is parsed. */
void pico8_planet_family_trade_init(void);

/* Frees the three specs, never any instance. Idempotent, and safe before
 * the first `pico8_planet_family_trade_init()`. */
void pico8_planet_family_trade_shutdown(void);

/* A slipgate: a laboratory, then nr's own defaults `hz` and `rr`, which no
 * ancestor has a member for. A default of nr whose name is a member of `qh`
 * or `jo` (`nu`, `kq`, `od`, `fp`, `s`, `dw`) lives in that member. `nu`,
 * `kq` and `hz` hold the very tables of nr's spec, `px` and `ba` those of
 * qh's, borrowed. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
    pico8_value_t hz;
    pico8_value_t rr;
} pico8_nr_t;

/* An infraspace gate: a slipgate, then fr's own default `mt`, which no
 * ancestor has a member for. A default of fr whose name is a member of `nr`
 * or `qh` (`rr`, `hz`, `od`, `s`) lives in that member. `hz` holds the very
 * table of fr's spec, `nu` and `kq` those of nr's, `px` and `ba` those of
 * qh's, borrowed. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_nr_t nr;
    pico8_value_t mt;
} pico8_fr_t;

/* A synthesizer: an ascension, then mx's own default `ht`, which no
 * ancestor has a member for. A default of mx whose name is a member of
 * `hj`, `qh` or `jo` (`nu`, `rr`, `hp`, `s`, `px`, `od` and the 11 emitter
 * defaults) lives in that member. `nu`, `px`, `ht` and `k_` hold the very
 * tables of mx's spec, `ba` the one of qh's, borrowed; `mx:kf` swaps `ht`
 * for a fresh table it never frees. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
    pico8_value_t ht;
} pico8_mx_t;

/* As in pico8_planet_family.h: `_ready` from
 * `pico8_planet_family_trade_init()` to its shutdown, `_default` reads the
 * kind's spec then its parent's reader (nr: qh, fr: nr, mx: hj), a REF
 * borrowed until the shutdown; each `_kind` reader is that `_default`. */
int pico8_nr_ready(void);
int pico8_nr_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_nr_kind(void);
const pico8_struct_class_t *pico8_nr_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id`, `p`, `ra`, each
 * optional. The new slipgate registers itself into `hv`/`cv`/`cg` through
 * `lt`, and nothing ever frees it. Requires
 * `pico8_planet_family_trade_init()`. */
pico8_nr_t *pico8_nr_new(const pico8_ob_table_t *overrides);

int pico8_fr_ready(void);
int pico8_fr_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_fr_kind(void);
const pico8_struct_class_t *pico8_fr_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * infraspace gate registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
pico8_fr_t *pico8_fr_new(const pico8_ob_table_t *overrides);

int pico8_mx_ready(void);
int pico8_mx_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_mx_kind(void);
const pico8_struct_class_t *pico8_mx_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * synthesizer registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
pico8_mx_t *pico8_mx_new(const pico8_ob_table_t *overrides);

/* Never stores `s`, which may be NULL: `nr:ldown` passes a static struct
 * with no field. */
pico8_value_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s);

/* TROU-6 */
/* Returns a freshly built `jc` instance, or `gv`'s own false; nothing ever
 * frees the instance. */
pico8_value_t pico8_nr_ldown(pico8_nr_t *self);

void pico8_nr_hb(pico8_nr_t *self);

/* Writes the quantity left after forwarding back onto `*lw`, so the caller
 * sees what every hop downstream consumed. */
void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw);

void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw);

#endif
