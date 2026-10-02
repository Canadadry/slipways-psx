#ifndef PICO8_PLANET_H
#define PICO8_PLANET_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "game/layout.h"
#include "game/particle.h"
#include "game/targeting.h"
#include "engine/pointer_list.h"
#include "game/research.h"
#include "engine/entity_class.h"

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
 * struct. The struct lives in its game generation (a burst's in its
 * particle place) and goes back with it. */
typedef struct {
    entity_t header;
    const planet_archetype_t *def;
    p8_fixed_t ma;
    production_t *kw;
    const char *id;
    vec_t *p;
    vec_t *ra;
    entity_list_t *oe;
    pico8_hp_t *hn;
    const char *ep;
    pointer_list_t *jy;
    const planet_types_entry_t *nk;
    p8_fixed_t fx;
    layout_row_t *lq;
    const allocator_t *lq_place; /* where `lq` lives; NULL when `lq` is */
    entity_t *fo;
    p8_fixed_t jh;
    int has_jh;
    int p_;
    p8_fixed_t kr;
    int has_kr;
    entity_list_t *cc;
    entity_list_t *by;
    p8_fixed_t mi;
    int has_mi;
    int m_;
    production_t kw_own;
    p8_fixed_t s;           /* qh's lineage */
    p8_fixed_t qw;          /* nl */
    p8_fixed_t iu, ms;      /* ga */
    const int_seq_t *ht; /* mx */
    pico8_probe_t *probe;      /* qh's lineage */
    pico8_emitter_t *emitter;  /* hj, pm, mx, ga */
    pico8_probe_t probe_own;
    pico8_emitter_t emitter_own;
} pico8_jo_t;

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void pico8_jo_init(void);

/* Sets the `oc`/`ir` generators back to nil without reading them: their
 * zone may already be reset. Frees no `jo` instance. Idempotent, and safe
 * before the first `pico8_jo_init()`. */
void pico8_jo_shutdown(void);

/* 1 from `pico8_jo_init()` to `pico8_jo_shutdown()`, 0 otherwise. */
int pico8_jo_ready(void);

const entity_class_t *pico8_jo_struct_class(void);

/* `jo.dz` (PICO8_JO_DZ_COUNT rows), `jo.cq`, `jo.ka`, `jo.em`: jo's
 * generated data, never NULL, init or not. */
const data_row_t *pico8_jo_dz(void);
const int_seq_t *pico8_jo_cq(void);
const int_seq_t *pico8_jo_ka(void);
const data_row_t *pico8_jo_em(void);

/* The `kw` every bare planet points at; each `pico8_jo_init()` resets it to
 * `{mu=0}`. */
production_t *pico8_jo_shared_kw(void);

/* `keys` (NULL means none) is read, not kept. The new planet registers
 * itself into `hv`/`cv`/`cg` through `lt`, and lives in the construction
 * zone until that zone is given back. Requires `pico8_jo_init()`. */
pico8_jo_t *pico8_jo_new(const planet_keys_t *keys);

/* Sets up an allocated `self` as a bare planet whose descriptor is `cls`:
 * `def` kept, the members it holds copied from it, the others nil. Neither
 * runs `jo:pq` nor `lt`: a daughter's constructor does, after its own
 * defaults. Requires `pico8_jo_init()`. */
void pico8_jo_struct_init(pico8_jo_t *self, const entity_class_t *cls, const planet_archetype_t *def);

/* Bytes of the arena a held icon, a drawn slipway or a wormhole builds its
 * object in. */
#define PICO8_JO_PRISE_OCTETS 2048u

/* Where the planet constructors, `jo:pq` and `jo:lz` allocate: the grab
 * arena between `pico8_jo_prise_ouvrir` and `_fermer`, else the current
 * game generation. */
const allocator_t *pico8_jo_zone_construction(void);

/* Opens a grab arena, taken from the free grabs (emptied) or else from the
 * current generation; a placed structure keeps it until the generation is
 * given back. */
void pico8_jo_prise_ouvrir(void);
void pico8_jo_prise_fermer(void);
/* Called last by the `rendre` of an object built in `zone`, whose state is
 * `me`: a grab arena (NULL or another zone: nothing) whose object is still
 * "kz" goes on the free grabs, for the next grab to empty and reuse. */
void pico8_jo_prise_zone_rendue(const allocator_t *zone, const char *me);
/* The same whatever the state: for an object whose memory nothing borrows
 * after its death. */
void pico8_jo_prise_arene_rendue(const allocator_t *zone);
/* `pico8_jo_prise_zone_rendue` with the zone of the planet's `oe`. */
void pico8_jo_prise_rendue(const pico8_jo_t *self);
/* Sets to nil the `fo` of every `cg.np` entity that points at `e`, which is
 * being given back: `jo:kj` leaves `fo` set on a planet off screen or
 * fading, and a recycled grab arena would put a new structure behind it. */
void pico8_jo_oublier_fo(const entity_t *e);
/* Drops the free grabs, without reading them: their arenas belong to a
 * generation about to be given back or reset. */
void pico8_jo_oublier_prises(void);

/* `oe` taken from, and grown in, the construction zone. */
void pico8_jo_pq(pico8_jo_t *self);
/* `oe` taken from, and grown in, `zone`, which must outlive it. */
void pico8_jo_pq_dans(pico8_jo_t *self, const allocator_t *zone);

/* The dispatchers jo's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
int pico8_jo_hd_dispatch(entity_t *self, const vec_t *qa, const vec_t *ra);
void pico8_jo_hb_dispatch(entity_t *self);
/* Gives back the place of `lq` and sets `lq` to nil; the planet stays. */
void pico8_jo_rendre_rangee(entity_t *self);
/* `pico8_jo_rendre_rangee`, then `pico8_jo_prise_rendue`. */
void pico8_jo_rendre_dispatch(entity_t *self);
down_t pico8_jo_gv_dispatch(entity_t *self, const entity_t *s);
void pico8_jo_hk_dispatch(entity_t *self, p8_fixed_t yr);
void pico8_jo_kj_dispatch(entity_t *self, const vec_t *ra);
down_t pico8_jo_ldown_dispatch(entity_t *self, vec_t *qa, vec_t *ra);
void pico8_jo_kf_dispatch(entity_t *self, entity_t *sw);

/* `e` is one of the ten classes of jo's tree: the only ones whose state
 * `ll` is jo's. */
int pico8_jo_is_family(const entity_t *e);
void pico8_jo_cn_dispatch(entity_t *self, struct pico8_jo_trade_proposal *lw);
void pico8_jo_ll_dispatch(entity_t *self, p8_fixed_t t);

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance);

/* TROU-3 */
/* 1 when `hf` is true and `me` is "ly", 0 otherwise, a NULL `self` included. */
int pico8_jo_gg(const pico8_jo_t *self);

void pico8_jo_kd(pico8_jo_t *self);

/* No textual `:ll(` call site exists: `dg()` reaches this by state name
 * (`qc[qc.me]`), through the `LL` entry of a planet's descriptor. */
void pico8_jo_ll(pico8_jo_t *self, p8_fixed_t t);

void pico8_jo_hh(pico8_jo_t *self);

void pico8_jo_hk(const pico8_jo_t *self);

/* Returns a freshly built `jc` instance in `held`, Lua true as `stop`
 * alone, or nil; `ui:bh` holds only the instance. The `jc` lives in its
 * grab arena, which it gives back if refused. */
down_t pico8_jo_ldown(pico8_jo_t *self);

/* The state passed must live until the next call; the previous one is not
 * freed. NULL is the valid way back to "none". Both start NULL -- only
 * `bw()` sets them. */
void pico8_jo_set_oc(weighted_picker_t *oc);
void pico8_jo_set_ir(random_walk_t *ir);

/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
 * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
 * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
void pico8_na(entity_t *self, p8_fixed_t t);

/* NULL when no kind has `fz`. */
const entity_kind_t *pico8_jo_kv(const char *fz);

#endif
