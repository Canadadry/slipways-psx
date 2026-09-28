#ifndef PICO8_PROBE_H
#define PICO8_PROBE_H

#include "pico8_economy.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), a non-NULL `self` must be a struct
 * (asserted), an absent or wrong-kind field reads as 0/false/the zero vector
 * rather than crashing, and a NULL `gfx`/`mem` skips the draw calls while
 * the rest of the body still runs. */

/* TROU-7 */
/* `nu` and `k_` are the tables of `g_`'s class, borrowed and shared with
 * `fv`, never modified; `ra` is the very REF `bv` was passed, aliased. */
typedef struct {
    pico8_ke_t ke;
    pico8_ob_table_t *nu;
    pico8_fixed_t fx;
    const char *dq;
    pico8_fixed_t od;
    int fp;
    pico8_fixed_t fa;
    pico8_fixed_t dw;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pn;
    pico8_fixed_t pk;
    pico8_fixed_t fc;
    pico8_ob_table_t *k_;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_fixed_t du;
    pico8_list_t *qy;
    pico8_vec_t *ra;
    pico8_vec_t *p;
    pico8_struct_header_t *et; /* borrowed, never freed */
    int has_et;                /* 0: nil; 1: present, and a NULL `et` is `false` */
    int p_;
    int has_p_; /* 0: `p_` is nil, and is then never read */
} pico8_g__t;

/* `et` as a value: REF NULL when `self` is NULL or `et` is absent, `false`
 * when present and NULL, a REF to the target otherwise. */
pico8_value_t pico8_g__et(const pico8_g__t *self);

/* TROU-8 */
/* Parses `g_`'s spec and puts the provisional `fm` back in the slot, its
 * `jk`/`nz` nil. Shuts the module down first, so it is idempotent and any
 * REF previously handed out by pico8_g__default() is freed by a second
 * call. */
void pico8_probe_init(void);

/* Frees the `g_` spec and the `hy` of `fm` if it holds one, never the
 * provisional, never any probe. Idempotent, and safe before the first
 * `pico8_probe_init()`. */
void pico8_probe_shutdown(void);

/* 1 from pico8_probe_init() to pico8_probe_shutdown(), 0 otherwise. */
int pico8_g__ready(void);

/* `g_`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_probe_shutdown(). */
int pico8_g__default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_g__struct_class(void);

/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 5
 * written members nil, g_'s 18 defaults. Reads no key, runs neither `pq`
 * nor `lt`, records no coverage. Requires `pico8_probe_init()`. */
void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls);

/* `overrides` (NULL means none) is read, not kept: `pn`, `t`, `du`, `py`
 * only. The new probe registers itself into `hv`/`cv`/`cg` through `lt`
 * (and gets its particle list from `f_`), and nothing ever frees it.
 * Asserts when called before `pico8_probe_init()`. */
pico8_g__t *pico8_g__new(const pico8_ob_table_t *overrides);

/* The provisional `fm`: one static struct, never allocated nor freed, with
 * no method and no writer; `jk`/`nz` are written only by the cheat code, and
 * set back to nil by every `pico8_probe_init()`. */
typedef struct {
    pico8_struct_header_t header;
    pico8_fixed_t jk;
    int has_jk; /* 0: `jk` is nil, and is then never read */
    pico8_fixed_t nz;
    int has_nz; /* 0: `nz` is nil, and is then never read */
} pico8_fm_placeholder_t;

const pico8_struct_class_t *pico8_fm_placeholder_struct_class(void);

/* `fm`: the provisional, a static struct that has only `jk`/`nz` (written by
 * the cheat code), which `pico8_probe_init()` installs, then, from `eu()` on,
 * a `hy` struct; read it through pico8_entity_get_field. NULL before init and
 * after shutdown, which several callers test for. The setter takes ownership
 * of the new occupant and frees a previous `hy` as the struct alone
 * (TROU-7); never the provisional. */
pico8_struct_header_t *pico8_fm(void);
void pico8_probe_set_fm(pico8_struct_header_t *instance);

/* The `hy` of `fm`, NULL when the slot is empty; asserts on the
 * placeholder. */
pico8_hy_t *pico8_fm_hy(void);

/* TROU-7 */
/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
 * stays aliased. `et` ends in one of three states -- a real target, a
 * static struct with no readable field but `fo`, or literal `false` -- of
 * which only `false` is falsy; collapsing the throwaway to nil would change
 * `d_`. */
void pico8_bv(pico8_struct_header_t *self, pico8_value_t mp, pico8_value_t rp);

pico8_value_t pico8_bv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

const pico8_struct_class_t *pico8_bv_et_struct_class(void);

/* Generic tooltip helper, not probe-specific: also called directly by
 * `jc:hh`, `qh:ph` and the research icon, on entities that are never `g_`.
 * `self` is read, never retained -- one caller passes a struct view on its
 * stack (`ie.ke`). */
void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);

pico8_value_t pico8_dl_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* Returns Lua true on the success branch, nil on the other; `jc:ey` consumes
 * that result. */
pico8_value_t pico8_d_(pico8_struct_header_t *self);

pico8_value_t pico8_d__dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* TROU-7 */
/* Returns a REF to a freshly allocated particle table ("p"/"v"/"l", the two
 * vectors heap-allocated too); `ez` appends it to `qy` and nothing ever frees
 * it. */
pico8_value_t pico8_g__mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
