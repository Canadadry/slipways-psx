#ifndef PICO8_PLANET_H
#define PICO8_PLANET_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_ob.h"
#include "pico8_research.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* A bare planet: jo's own defaults (`dz`, `cq`, `ka`, `em` and `kw` hold
 * the very tables of jo's class, `kw` until `lz` replaces it: never written
 * through, save `kw` in place as the cart does), its construction keys
 * `id`/`p`/`ra`, then every field jo's functions and the generic writers
 * of `cg.jo` write on it. Every planet struct begins with one, so jo's
 * functions take it for `self`. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t nu;
    pico8_value_t hf;
    pico8_value_t hs;
    pico8_value_t fa;
    pico8_value_t kq;
    pico8_value_t ma;
    pico8_value_t ka;
    pico8_value_t dz;
    pico8_value_t cq;
    pico8_value_t kw;
    pico8_value_t gq;
    pico8_value_t ca;
    pico8_value_t em;
    pico8_value_t id;
    pico8_value_t p;
    pico8_value_t ra;
    pico8_value_t oe;
    pico8_value_t hn;
    pico8_value_t ep;
    pico8_value_t jy;
    pico8_value_t nk;
    pico8_value_t fx;
    pico8_value_t lq;
    pico8_value_t fo;
    pico8_value_t jh;
    pico8_value_t p_;
    pico8_value_t kr;
    pico8_value_t cc;
    pico8_value_t by;
    pico8_value_t mi;
    pico8_value_t m_;
} pico8_jo_t;

/* TROU-8 */
/* Calls `pico8_pi_init()` first: the class spec contains `b(-7,-7,7,7)` and
 * `v(0,11)`, so the 'b'/'v' constructors must already be registered when
 * the spec string is parsed. */
void pico8_jo_init(void);

/* Frees the `jo` class and the `oc`/`ir` generators, never any `jo`
 * instance. Idempotent, and safe before the first `pico8_jo_init()`. */
void pico8_jo_shutdown(void);

/* NULL before `pico8_jo_init()` and after `pico8_jo_shutdown()`. Also
 * reachable as `pico8_class_lookup_named("jo")`; its instantiation builds
 * a pico8_jo_t. */
pico8_class_t *pico8_jo_class(void);
const pico8_struct_class_t *pico8_jo_struct_class(void);

/* `jo.dz`, `jo.cq`, `jo.ka`, `jo.em`, read on the class: the table jo's
 * `defaults` holds, borrowed. NULL before `pico8_jo_init()` and after
 * `pico8_jo_shutdown()`. */
const pico8_ob_table_t *pico8_jo_dz(void);
const pico8_ob_table_t *pico8_jo_cq(void);
const pico8_ob_table_t *pico8_jo_ka(void);
const pico8_ob_table_t *pico8_jo_em(void);

/* `overrides` (NULL means none) is read, not kept: `id`, `p` and `ra` only.
 * The new planet registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_jo_init()`. */
pico8_jo_t *pico8_jo_new(const pico8_ob_table_t *overrides);

/* Sets up an allocated `self` as a bare planet whose descriptor is `cls`:
 * the 31 members nil, then jo's defaults. Neither runs `jo:pq` nor `lt`:
 * a daughter's constructor does, after its own defaults. Requires
 * `pico8_jo_init()`. */
void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls);

void pico8_jo_pq(pico8_jo_t *self);

/* The dispatchers jo's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
pico8_value_t pico8_jo_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_gg_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_hk_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_kf_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_cn_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_ib_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_jo_ll_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* TROU-7 */
void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance);

/* TROU-3 */
/* Returns `self.hf` itself when that field is falsy (Lua's `A and B`), a
 * real boolean otherwise -- deliberately not collapsed to a C int. */
pico8_value_t pico8_jo_gg(const pico8_jo_t *self);

void pico8_jo_kd(pico8_jo_t *self);

/* No textual `:ll(` call site exists: `dg()` reaches this by state name
 * (`qc[qc.me]`), through the `LL` entry of a planet's descriptor. */
void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t);

/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh instance the caller never sees -- or on nothing,
 * when no class is registered under `hp.cw`. */
void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance);

void pico8_jo_hh(pico8_jo_t *self);

void pico8_jo_hk(const pico8_jo_t *self);

/* TROU-6 */
/* Returns a freshly built `jc` instance, Lua true, or nil; the caller
 * (`ui:bh`) keeps whatever comes back, and nothing frees the instance. */
pico8_value_t pico8_jo_ldown(pico8_jo_t *self);

/* Both take ownership and free the previous generator; NULL is the valid
 * way back to "none", and `pico8_jo_shutdown()` releases whichever is set.
 * Both start NULL -- `bw()` is what sets them. */
void pico8_jo_set_oc(pico8_gn_state_t *oc);
void pico8_jo_set_ir(pico8_jf_state_t *ir);

#endif
