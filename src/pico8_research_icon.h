#ifndef PICO8_RESEARCH_ICON_H
#define PICO8_RESEARCH_ICON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL `ie` struct, `gfx` or `mem` is a safe
 * no-op, and a nil or wrong-kind member, or an absent field of `self.ke`,
 * reads as its zero/false default.
 *
 * `self.ke` holds a CLASS, not an instance: its fields are read off a
 * `pico8_ie_ke_t`, never pico8_instance_get_field() on the class pointer. */

/* `kq`, `ra` and `eh` point at the objects the module owns, shared by every
 * `ie` and never modified in place; `hb` rebinds `ra` to the cursor. `gy`
 * and `id` are no members: see the module's shared state. Nothing frees the
 * struct: TROU-7. */
typedef struct {
    pico8_ke_t ke_base;
    pico8_value_t kq;
    pico8_value_t fa;
    pico8_value_t ra;
    pico8_value_t eh;
    pico8_value_t ke;
    pico8_value_t jb;
    pico8_value_t kh;
    pico8_value_t m_;
} pico8_ie_t;

/* `self.ke` read as Lua reads a class: no parent, never an entity, and no
 * writer, so a write reaching it stops the run. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t rr;
    pico8_value_t s;
    pico8_value_t me;
    pico8_value_t od;
} pico8_ie_ke_t;

/* Calls pico8_research_icon_shutdown(), then pico8_pi_init(): `ie`'s spec
 * has `b(...)`/`v(...)` constructors that must already be registered. */
void pico8_research_icon_init(void);

/* Frees the `ie` class and the module's `kq` box and `ra`/`eh` vectors,
 * never any `ie`, and clears `ie.gy`. Idempotent. */
void pico8_research_icon_shutdown(void);

/* NULL before pico8_research_icon_init() and after the shutdown. */
pico8_class_t *pico8_ie_class(void);
const pico8_struct_class_t *pico8_ie_struct_class(void);

/* `overrides` is read, not kept, and may hold `ke` (a planet-upgrade class)
 * and `jb` (a tooltip struct) only, both borrowed. The struct registers
 * itself into `hv`/`cv` through `lt`, and nothing frees it. */
pico8_ie_t *pico8_ie_new(const pico8_ob_table_t *overrides);

/* `ie.eh`, the module's vector, for `_init` to point `ie.gy` at: an alias,
 * not a copy. Absent before pico8_research_icon_init(). */
pico8_value_t pico8_ie_eh(void);

void pico8_ie_set_gy(pico8_value_t gy);

/* Reveals the icon once, claiming the next slot off the shared `ie.gy`
 * cursor. Requires pico8_research_icon_init(). */
void pico8_ie_hb(pico8_ie_t *self);

/* TROU-7 */
/* Returns a fresh instance of the class `self.ke` names, for the caller to
 * hold as a placement in progress, or an absent value when `self.ke` is not
 * a class; bumps the shared `ie.id` either way. Requires
 * pico8_research_icon_init(). `mp` is unused, as in Lua. */
pico8_value_t pico8_ie_ldown(pico8_ie_t *self, pico8_value_t mp);

/* By value, one per read: each member copied off the chain of the class
 * `self.ke` holds, nil when absent there or when `self.ke` is. */
pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self);
const pico8_struct_class_t *pico8_ie_ke_view_struct_class(void);

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec_t p);

#endif
