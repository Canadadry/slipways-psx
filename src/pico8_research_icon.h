#ifndef PICO8_RESEARCH_ICON_H
#define PICO8_RESEARCH_ICON_H

#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_mem.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL `ie` struct, `gfx` or `mem` is a safe
 * no-op, and a nil member, or an absent field of `self.ke`, reads as its
 * zero/false default; a wrong-kind write, construction key or field of
 * `self.ke` stops the run.
 *
 * `self.ke` holds a `pico8_entity_kind_t`, not an entity: its fields are
 * read off a `pico8_ie_ke_t`, never pico8_entity_get_field() on the kind
 * pointer. */

/* `kq`, `ra` and `eh` point at `pico8_ie_start`'s, shared by every `ie`;
 * `hb` rebinds `ra` to the cursor. `gy` and `id` are no members: see the
 * module's shared state. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke_base;
    const pico8_qz_box_t *kq;
    pico8_fixed_t fa;
    const pico8_vec_t *ra;
    const pico8_vec_t *eh;
    const pico8_entity_kind_t *ke;
    pico8_jb_t *jb;
    int kh;
    int has_kh; /* 0: `kh` is nil, and is then never read */
    int m_;
    int has_m_; /* 0: `m_` is nil, and is then never read */
} pico8_ie_t;

/* `self.ke` read as Lua reads a class: no parent, never an entity, and no
 * writer, so a write reaching it stops the run. */
typedef struct {
    pico8_struct_header_t header;
    const char *rr;
    pico8_fixed_t s;
    int has_s; /* 0: `s` is nil, and is then never read */
    const char *me;
    pico8_fixed_t od;
    int has_od; /* 0: `od` is nil, and is then never read */
} pico8_ie_ke_t;

/* Calls pico8_research_icon_shutdown(), then pico8_pi_init(); puts `ie.id`
 * back to `pico8_ie_start.id`. */
void pico8_research_icon_init(void);

/* Clears `ie.gy`, frees nothing. Idempotent. */
void pico8_research_icon_shutdown(void);

/* 1 from pico8_research_icon_init() to pico8_research_icon_shutdown(), 0 otherwise. */
int pico8_ie_ready(void);
const pico8_struct_class_t *pico8_ie_struct_class(void);

/* The keys of `pico8_ie_new`, read, not kept; NULL is nil. `ke` (a
 * planet-upgrade kind) and `jb` (a tooltip struct) are borrowed. */
typedef struct {
    const pico8_entity_kind_t *ke;
    pico8_jb_t *jb;
} pico8_ie_keys_t;

/* `keys` NULL means none. The struct registers itself into `hv`/`cv` through
 * `lt`, and nothing frees it. */
pico8_ie_t *pico8_ie_new(const pico8_ie_keys_t *keys);

/* `ie.eh`, `pico8_ie_start.eh`, for `_init` to point `ie.gy` at: an alias,
 * not a copy. NULL before pico8_research_icon_init() and after the
 * shutdown. */
const pico8_vec_t *pico8_ie_eh(void);

/* NULL is a nil `gy`. */
void pico8_ie_set_gy(const pico8_vec_t *gy);

/* Reveals the icon once, claiming the next slot off the shared `ie.gy`
 * cursor. Requires pico8_research_icon_init(). */
void pico8_ie_hb(pico8_ie_t *self);

/* TROU-7 */
/* Returns a fresh instance of the class `self.ke` names, for the caller to
 * hold as a placement in progress, or an absent value when `self.ke` is not
 * a class; bumps the shared `ie.id` either way. Requires
 * pico8_research_icon_init(). */
pico8_down_t pico8_ie_ldown(pico8_ie_t *self);

/* By value, one per read: each member copied off the chain of the class
 * `self.ke` holds, nil when absent there or when `self.ke` is. `rr` and
 * `me` are borrowed from the kind's defaults. */
pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self);
const pico8_struct_class_t *pico8_ie_ke_view_struct_class(void);

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec_t p);

#endif
