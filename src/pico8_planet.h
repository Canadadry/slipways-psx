#ifndef PICO8_PLANET_H
#define PICO8_PLANET_H

#include "pico8_ents.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_gen.h"
#include "pico8_layout.h"
#include "pico8_particle.h"
#include "pico8_probe_comp.h"
#include "pico8_ptrs.h"
#include "pico8_research.h"
#include "pico8_struct.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* A planet of any of the ten kinds, which differ only by class table and
 * archetype: its archetype (`kw` points at the one kw every bare planet
 * shares, written in place by `bb` and `mx:kf`, until `lz` points it at the
 * planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
 * field jo's functions write on it, then its kinds' fields. A kind's field is
 * indeterminate on a planet of another kind; `probe`, `emitter` are NULL
 * there, else they point at `probe_own`, `emitter_own`: never copy the
 * struct. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_jo_def_t *def;
    pico8_fixed_t ma;
    pico8_kw_t *kw;
    const char *id;
    pico8_vec_t *p;
    pico8_vec_t *ra;
    pico8_ents_t *oe;
    pico8_hp_t *hn;
    const char *ep;
    pico8_ptrs_t *jy;
    const pico8_df_entry_t *nk;
    pico8_fixed_t fx;
    pico8_lq_t *lq;
    pico8_struct_header_t *fo;
    pico8_fixed_t jh;
    int has_jh;
    int p_;
    pico8_fixed_t kr;
    int has_kr;
    pico8_ents_t *cc;
    pico8_ents_t *by;
    pico8_fixed_t mi;
    int has_mi;
    int m_;
    pico8_kw_t kw_own;
    pico8_fixed_t s;           /* qh's lineage */
    pico8_fixed_t qw;          /* nl */
    pico8_fixed_t iu, ms;      /* ga */
    const pico8_int_seq_t *ht; /* mx */
    pico8_probe_t *probe;      /* qh's lineage */
    pico8_emitter_t *emitter;  /* hj, pm, mx, ga */
    pico8_probe_t probe_own;
    pico8_emitter_t emitter_own;
} pico8_jo_t;

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void pico8_jo_init(void);

/* Frees the `oc`/`ir` generators, never any `jo` instance. Idempotent, and
 * safe before the first `pico8_jo_init()`. */
void pico8_jo_shutdown(void);

/* 1 from `pico8_jo_init()` to `pico8_jo_shutdown()`, 0 otherwise. */
int pico8_jo_ready(void);

const pico8_struct_class_t *pico8_jo_struct_class(void);

/* `jo.dz` (PICO8_JO_DZ_COUNT rows), `jo.cq`, `jo.ka`, `jo.em`: jo's
 * generated data, never NULL, init or not. */
const pico8_jb_row_t *pico8_jo_dz(void);
const pico8_int_seq_t *pico8_jo_cq(void);
const pico8_int_seq_t *pico8_jo_ka(void);
const pico8_jb_row_t *pico8_jo_em(void);

/* The `kw` every bare planet points at; each `pico8_jo_init()` resets it to
 * `{mu=0}`. */
pico8_kw_t *pico8_jo_shared_kw(void);

/* `keys` (NULL means none) is read, not kept. The new planet registers
 * itself into `hv`/`cv`/`cg` through `lt`, and nothing ever frees it.
 * Requires `pico8_jo_init()`. */
pico8_jo_t *pico8_jo_new(const pico8_jo_keys_t *keys);

/* Sets up an allocated `self` as a bare planet whose descriptor is `cls`:
 * `def` kept, the members it holds copied from it, the others nil. Neither
 * runs `jo:pq` nor `lt`: a daughter's constructor does, after its own
 * defaults. Requires `pico8_jo_init()`. */
void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);

void pico8_jo_pq(pico8_jo_t *self);

/* The dispatchers jo's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
int pico8_jo_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
void pico8_jo_hb_dispatch(pico8_struct_header_t *self);
pico8_down_t pico8_jo_gv_dispatch(pico8_struct_header_t *self, const pico8_struct_header_t *s);
void pico8_jo_hk_dispatch(pico8_struct_header_t *self, pico8_fixed_t yr);
void pico8_jo_kj_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra);
pico8_down_t pico8_jo_ldown_dispatch(pico8_struct_header_t *self, pico8_vec_t *qa, pico8_vec_t *ra);
void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);

/* `e` is one of the ten classes of jo's tree: the only ones whose state
 * `ll` is jo's. */
int pico8_jo_is_family(const pico8_struct_header_t *e);
void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw);
void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t);

/* TROU-7 */
void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance);

/* TROU-3 */
/* 1 when `hf` is true and `me` is "ly", 0 otherwise, a NULL `self` included. */
int pico8_jo_gg(const pico8_jo_t *self);

void pico8_jo_kd(pico8_jo_t *self);

/* No textual `:ll(` call site exists: `dg()` reaches this by state name
 * (`qc[qc.me]`), through the `LL` entry of a planet's descriptor. */
void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t);

/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh planet the caller never sees -- or on nothing,
 * when `hp.cw` is neither "pm" nor "ga". */
void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance);

void pico8_jo_hh(pico8_jo_t *self);

void pico8_jo_hk(const pico8_jo_t *self);

/* TROU-7 */
/* Returns a freshly built `jc` instance in `held`, Lua true as `stop`
 * alone, or nil; `ui:bh` holds only the instance, and nothing frees it. */
pico8_down_t pico8_jo_ldown(pico8_jo_t *self);

/* Both take ownership and free the previous generator; NULL is the valid
 * way back to "none", and `pico8_jo_shutdown()` releases whichever is set.
 * Both start NULL -- `bw()` is what sets them. */
void pico8_jo_set_oc(pico8_gn_state_t *oc);
void pico8_jo_set_ir(pico8_jf_state_t *ir);

/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
 * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
 * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);

/* NULL when no kind has `fz`. */
const pico8_entity_kind_t *pico8_jo_kv(const char *fz);

#endif
