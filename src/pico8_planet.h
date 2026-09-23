#ifndef PICO8_PLANET_H
#define PICO8_PLANET_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* TROU-8 */
/* Calls `pico8_pi_init()` first: the class spec contains `b(-7,-7,7,7)` and
 * `v(0,11)`, so the 'b'/'v' constructors must already be registered when
 * the spec string is parsed. */
void pico8_jo_init(void);

/* Frees the `jo` class and the `oc`/`ir` generators, never any `jo`
 * instance. Idempotent, and safe before the first `pico8_jo_init()`. */
void pico8_jo_shutdown(void);

/* NULL before `pico8_jo_init()` and after `pico8_jo_shutdown()`. Also
 * reachable as `pico8_class_lookup_named("jo")`. */
pico8_class_t *pico8_jo_class(void);

/* `overrides` is copied, not kept. The new instance registers itself into
 * `hv`/`cv`/`cg` through `lt`, and nothing ever frees it. Called before
 * `pico8_jo_init()` it yields a class-less instance rather than NULL. */
pico8_instance_t *pico8_jo_new(const pico8_ob_table_t *overrides);

/* TROU-7 */
void pico8_jo_lz(pico8_instance_t *self, pico8_instance_t *hp_instance);

/* TROU-3 */
/* Returns `self.hf` itself when that field is falsy (Lua's `A and B`), a
 * real boolean otherwise -- deliberately not collapsed to a C int. */
pico8_value_t pico8_jo_gg(const pico8_instance_t *self);

void pico8_jo_kd(pico8_instance_t *self);

/* No textual `:ll(` call site exists: `dg()` reaches this by state name
 * (`qc[qc.me]`), through the `"ll"` field `pico8_jo_init()` installs. */
void pico8_jo_ll(pico8_instance_t *self, pico8_fixed_t t);

/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh instance the caller never sees -- or on nothing,
 * when no class is registered under `hp.cw`. */
void pico8_jo_hu(pico8_instance_t *self, pico8_instance_t *hp_instance);

void pico8_jo_hh(pico8_instance_t *self);

void pico8_jo_hk(const pico8_instance_t *self);

/* TROU-6 */
/* Returns a freshly built `jc` instance, Lua true, or nil; the caller
 * (`ui:bh`) keeps whatever comes back, and nothing frees the instance. */
pico8_value_t pico8_jo_ldown(pico8_instance_t *self);

/* Both take ownership and free the previous generator; NULL is the valid
 * way back to "none", and `pico8_jo_shutdown()` releases whichever is set.
 * Both start NULL -- `bw()` is what sets them. */
void pico8_jo_set_oc(pico8_gn_state_t *oc);
void pico8_jo_set_ir(pico8_jf_state_t *ir);

#endif
