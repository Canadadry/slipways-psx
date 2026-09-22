#ifndef PICO8_PROBE_H
#define PICO8_PROBE_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), an absent or wrong-kind field reads as
 * 0/false/the zero vector rather than crashing, and a NULL `gfx`/`mem` skips
 * the draw calls while the rest of the body still runs. */

/* TROU-8 */
/* Builds the `g_` class and installs the `fm` placeholder. Shuts the module
 * down first, so it is idempotent and any previously handed out
 * `pico8_g__class()` pointer is freed by a second call. */
void pico8_probe_init(void);

/* Frees the `g_` class and whichever instance `fm` currently holds, never any
 * `g_` instance. Idempotent, and safe before the first `pico8_probe_init()`. */
void pico8_probe_shutdown(void);

/* NULL before `pico8_probe_init()` and after `pico8_probe_shutdown()`. Also
 * reachable as `pico8_class_lookup_named("g_")`. */
pico8_class_t *pico8_g__class(void);

/* `overrides` is copied, not kept. The new instance registers itself into
 * `hv`/`cv`/`cg` through `lt` (and gets its particle list from `f_`), and
 * nothing ever frees it. Called before `pico8_probe_init()` it yields a
 * class-less instance rather than NULL. */
pico8_instance_t *pico8_g__new(const pico8_ob_table_t *overrides);

/* `fm`. `pico8_probe_init()` installs a class-less placeholder here;
 * `pico8_cart` replaces it with the real `hy` instance. The setter frees the
 * previous instance and takes ownership of the new one; NULL before init and
 * after shutdown, which several callers test for. */
pico8_instance_t *pico8_fm(void);
void pico8_probe_set_fm(pico8_instance_t *instance);

/* TROU-7 */
/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
 * stays aliased. `et` ends in one of three states -- a real target, a
 * class-less throwaway instance, or literal `false` -- of which only `false`
 * is falsy; collapsing the throwaway to nil would change `d_`. */
void pico8_bv(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

pico8_value_t pico8_bv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* Generic tooltip helper, not probe-specific: also called directly by
 * `jc:hh`, `qh:ph` and the research icon, on entities that are never `g_`.
 * `self` is read, never retained -- one caller passes a stack instance. */
void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

pico8_value_t pico8_dl_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* Returns Lua true on the success branch, nil on the other; `jc:ey` consumes
 * that result. */
pico8_value_t pico8_d_(pico8_instance_t *self);

pico8_value_t pico8_d__dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* TROU-7 */
/* Returns a REF to a freshly allocated particle table ("p"/"v"/"l", the two
 * vectors heap-allocated too); `ez` appends it to `qy` and nothing ever frees
 * it. */
pico8_value_t pico8_g__mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
