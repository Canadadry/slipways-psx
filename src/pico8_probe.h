#ifndef PICO8_PROBE_H
#define PICO8_PROBE_H

#include "pico8_economy.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_particle.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), a non-NULL `self` must be a struct
 * (asserted) whose class carries the component the function reads -- a
 * probe for `bv`, `dl`, `d_`, an emitter for `g_:mm` -- and a NULL
 * `gfx`/`mem` skips the draw calls while the rest of the body still runs. */

/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
 * fields, stored in the host (`probe_own`) and never copied. `et` is
 * borrowed; with `has_et`, a NULL `et` is `false` (a `jc` keeps `has_et`
 * equal to `et != NULL`: its NULL is nil). `dq` NULL is nil. A fixed `fp` is
 * truthy even at 0, and `d_` hands it to `fm:nx`; a boolean one hands 0. */
typedef struct pico8_probe {
    pico8_struct_header_t *et;
    int has_et;
    pico8_fixed_t dw;
    pico8_fixed_t od;
    int has_od; /* 0: `od` is nil, and is then never read */
    const char *dq;
    pico8_fixed_t fp;
    int fp_is_bool; /* 1: `fp` holds a boolean (0/1), 0: a fixed */
    pico8_fixed_t ik;
    int has_ik; /* 0: `ik` is nil, and is then never read */
} pico8_probe_t;

/* TROU-7 */
/* `nu` points at `pico8_g__nu_tags` (for `fv` too), the emitter's `k_` at
 * `pico8_g__k__seq` (`pico8_fv_k__seq` for `fv`); `ra` is the very REF `bv`
 * was passed, aliased. `emitter` and `probe` point at `emitter_own` and
 * `probe_own`: never copy the struct. */
typedef struct {
    pico8_ke_t ke;
    const pico8_str_seq_t *nu;
    pico8_fixed_t fx;
    pico8_fixed_t fa;
    pico8_emitter_t *emitter;
    pico8_probe_t *probe;
    const pico8_vec_t *ra;
    pico8_vec_t *p;
    int p_;
    int has_p_; /* 0: `p_` is nil, and is then never read */
    pico8_emitter_t emitter_own;
    pico8_probe_t probe_own;
} pico8_g__t;

/* `e` itself when its class is g_'s or has g_'s up its `parent` chain; NULL
 * otherwise, and for a NULL `e`. */
pico8_g__t *pico8_g__of(pico8_struct_header_t *e);

/* The `emitter_of`, `probe_of` and field accessors of g_'s descriptor, for
 * `fv`'s to name. */
pico8_emitter_t *pico8_g__emitter_of(void *self);
pico8_probe_t *pico8_g__probe_of(void *self);
const pico8_str_seq_t *pico8_g__get_nu(const void *self);
int pico8_g__get_fa(const void *self, pico8_fixed_t *out);
const pico8_vec_t *pico8_g__get_ra(const void *self);
const pico8_vec_t *pico8_g__get_p(const void *self);
int pico8_g__get_fx(const void *self);
int pico8_g__get_p_(const void *self);

/* TROU-8 */
/* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
 * module down first, so it is idempotent. */
void pico8_probe_init(void);

/* Frees the `hy` of `fm` if it holds one, never the provisional, never any
 * probe. Idempotent, and safe before the first `pico8_probe_init()`. */
void pico8_probe_shutdown(void);

/* 1 from pico8_probe_init() to pico8_probe_shutdown(), 0 otherwise. */
int pico8_g__ready(void);
const pico8_struct_class_t *pico8_g__struct_class(void);

/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 5
 * written members nil, g_'s 18 defaults from `pico8_g__start`, 12 of them
 * in the emitter, 4 in the probe. Reads no
 * key, runs neither `pq` nor `lt`, records no coverage. */
void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls);

/* The keys of `pico8_g__new`, read, not kept. `t` is `ke.t`, whose default
 * is 0: no flag. `pn`, `du`, `py` have non-zero defaults, so a key at 0
 * needs its `has_` flag. */
typedef struct {
    pico8_fixed_t t;
    int has_pn;
    pico8_fixed_t pn;
    int has_du;
    pico8_fixed_t du;
    int has_py;
    pico8_fixed_t py;
} pico8_g__keys_t;

/* `keys` NULL means none. The new probe registers itself into `hv`/`cv`/`cg` through `lt`
 * (and gets its particle list from `f_`), and nothing ever frees it.
 * Asserts when called before `pico8_probe_init()`. */
pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys);

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
 * a `hy` struct; read it through pico8_fm_hy() and pico8_fm_placeholder().
 * NULL before init and after shutdown, which several callers test for. The
 * setter takes ownership of the new occupant and frees a previous `hy` as
 * the struct alone (TROU-7); never the provisional. */
pico8_struct_header_t *pico8_fm(void);
void pico8_probe_set_fm(pico8_struct_header_t *instance);

/* The `hy` of `fm`, NULL when the slot is empty and on the provisional. */
pico8_hy_t *pico8_fm_hy(void);

/* The provisional `fm`, NULL when the slot is empty or holds a `hy`. */
const pico8_fm_placeholder_t *pico8_fm_placeholder(void);

/* TROU-7 */
/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
 * stays aliased. `et` ends in one of three states -- a real target, a
 * static struct with no readable field but `fo`, or literal `false` -- of
 * which only `false` is falsy; collapsing the throwaway to nil would change
 * `d_`. */
void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp);

void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);

/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
 * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
 * (asserted, as is the class of `self` each accepts). */
void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self);

/* The `{}` `bv` puts in `et` over the void: one static struct, never
 * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
 * and the next `bv` overwrites it. */
typedef struct {
    pico8_struct_header_t header;
    pico8_struct_header_t *fo;
} pico8_bv_et_t;

const pico8_struct_class_t *pico8_bv_et_struct_class(void);

/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
 * read, never retained. */
void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);

/* The same, for a caller that is not a host (the research icon): `me` NULL
 * is nil, `pb` is read during the call only. */
void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);

void pico8_dl_dispatch(void *self, const pico8_vec_t *ra);

/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
 * and `mz:ey` consume it. */
int pico8_d_(pico8_struct_header_t *self);

void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);

/* TROU-7 */
/* Returns a freshly allocated `pico8_qy_t` (`has_a` 0), one allocation;
 * `ez` appends it to `qy` and nothing ever frees it. */
pico8_qy_t *pico8_g__mm(void *self);

#endif
