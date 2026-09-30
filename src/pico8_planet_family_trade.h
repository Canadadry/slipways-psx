#ifndef PICO8_PLANET_FAMILY_TRADE_H
#define PICO8_PLANET_FAMILY_TRADE_H

#include "pico8_fixed.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_struct.h"

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void pico8_planet_family_trade_init(void);

/* Frees nothing, no instance. Idempotent, and safe before the first
 * `pico8_planet_family_trade_init()`. */
void pico8_planet_family_trade_shutdown(void);

/* A slipgate: a laboratory and nothing of its own; its defaults are its
 * archetype's. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
} pico8_nr_t;

/* An infraspace gate: a slipgate and nothing of its own; its defaults are
 * its archetype's. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_nr_t nr;
} pico8_fr_t;

/* A synthesizer: an ascension whose `ht` is copied from its archetype;
 * `mx:kf` points it at an empty sequence once it accepts a resource.
 * Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
} pico8_mx_t;

/* As in pico8_planet_family.h: `_ready` from
 * `pico8_planet_family_trade_init()` to its shutdown. */
int pico8_nr_ready(void);
const pico8_entity_kind_t *pico8_nr_kind(void);
const pico8_struct_class_t *pico8_nr_struct_class(void);

/* `keys` (NULL means none) is read, not kept. The new slipgate registers
 * itself into `hv`/`cv`/`cg` through `lt`, and nothing ever frees it.
 * Requires `pico8_planet_family_trade_init()`. */
pico8_nr_t *pico8_nr_new(const pico8_jo_keys_t *keys);

int pico8_fr_ready(void);
const pico8_entity_kind_t *pico8_fr_kind(void);
const pico8_struct_class_t *pico8_fr_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new infraspace gate registers itself into `hv`/`cv`/`cg` through
 * `lt`, and nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
pico8_fr_t *pico8_fr_new(const pico8_jo_keys_t *keys);

int pico8_mx_ready(void);
const pico8_entity_kind_t *pico8_mx_kind(void);
const pico8_struct_class_t *pico8_mx_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new synthesizer registers itself into `hv`/`cv`/`cg` through
 * `lt`, and nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys);

/* Never stores `s`, which may be NULL: `nr:ldown` passes a static struct
 * with no member but its header. */
pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s);

/* TROU-7 */
/* Returns a freshly built `jc` instance, or `gv`'s own false; nothing ever
 * frees the instance. */
pico8_down_t pico8_nr_ldown(pico8_nr_t *self);

void pico8_nr_hb(pico8_nr_t *self);

/* Writes the quantity left after forwarding back onto `*lw`, so the caller
 * sees what every hop downstream consumed. */
void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw);

void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw);

#endif
