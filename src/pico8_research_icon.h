#ifndef PICO8_RESEARCH_ICON_H
#define PICO8_RESEARCH_ICON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL `self`, `gfx` or `mem` is a safe no-op, and
 * an absent or wrong-kind field reads as its zero/false default.
 *
 * `self.ke` holds a CLASS, not an instance: reading a field off it goes
 * through the transient wrapper in pico8_research_icon.c, never
 * pico8_instance_get_field() on the class pointer itself. */

/* Calls pico8_research_icon_shutdown(), then pico8_pi_init(): `ie`'s spec
 * has `b(...)`/`v(...)` constructors that must already be registered. */
void pico8_research_icon_init(void);

void pico8_research_icon_shutdown(void);

/* NULL before pico8_research_icon_init() and after the shutdown. */
pico8_class_t *pico8_ie_class(void);

/* Every real call passes `ke` (a planet-upgrade class) and `jb` (a tooltip
 * instance); both are borrowed, and nothing ever frees the icon. */
pico8_instance_t *pico8_ie_new(const pico8_ob_table_t *overrides);

/* Reveals the icon once, claiming the next slot off the class-wide `ie.gy`
 * cursor. Requires pico8_research_icon_init(): it writes `ie`'s defaults. */
void pico8_ie_hb(pico8_instance_t *self);

/* TROU-7 */
/* Returns a fresh instance of the class `self.ke` names, for the caller to
 * hold as a placement in progress, or an absent value when `self.ke` is not
 * a class. Requires pico8_research_icon_init(). `mp` is unused, as in Lua. */
pico8_value_t pico8_ie_ldown(pico8_instance_t *self, pico8_value_t mp);

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
