#ifndef PICO8_UI_H
#define PICO8_UI_H

#include "game/background.h"
#include "engine/entity_list.h"
#include "game/game_data.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/mouse.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL `ui`, `gfx` or `mem` argument is a safe
 * no-op, and an absent or wrong-kind field of another entity (`je`, `mo`,
 * `o_`) reads as absent rather than crashing. */

enum { PICO8_UI_IC_LAYERS = 16 };

/* `ic`: `layers[fa]` lists the hovered entities of layer `fa` in the order
 * they were found; `layer_count` is always PICO8_UI_IC_LAYERS. */
typedef struct {
    int layer_count;
    entity_list_t layers[PICO8_UI_IC_LAYERS];
} pico8_ui_ic_t;

/* `le` of a new `ui` is `pico8_ui_le_shared()`, until `cj` or `pz`
 * replaces it with a list from `pico8_ui_le_new()`, given back when it is
 * replaced in turn or when its `ui` leaves (`rendre`, from `jl()`). `ic` is
 * NULL until the first `gw`, then points at `ic_own`, whose layers every
 * `gw` remakes in the frame zone. `qa` is NULL until the first `lj`, then
 * points at `qa_own`, rewritten in place by each `lj`. Never copy the
 * struct. */
typedef struct {
    entity_t header;
    const pico8_ui_def_t *def;
    entity_list_t *le;
    vec_t *qa;
    vec_t qa_own;
    pico8_ui_ic_t *ic;
    pico8_ui_ic_t ic_own;
    entity_t *o_; /* borrowed; nil once the exit queue gives it back */
    entity_t *gj; /* borrowed; nil once the exit queue gives it back */
    entity_t *lheld; /* borrowed, never freed */
    pico8_bg_t *rheld; /* borrowed, never freed */
} pico8_ui_t;

/* Calls `pico8_pi_init()` and empties `pico8_ui_le_shared()`. Also puts
 * back the field-less placeholder `je` that `pico8_ui_set_je()` later
 * replaces. */
void pico8_ui_init(void);

/* Forgets `ok` and `je` without reading them: the zone reset that precedes
 * this call has already taken them, and the exit queue gave back `ok`'s
 * `le` before it. The shared `le` is forgotten without being read: its
 * array went with the loading sub-zone. Idempotent, and safe before the first `pico8_ui_init()`. */
void pico8_ui_shutdown(void);

/* 1 from pico8_ui_init() to pico8_ui_shutdown(), 0 otherwise. */
int pico8_ui_ready(void);

const entity_class_t *pico8_ui_struct_class(void);

/* The box of the class, read by `bv`: `&pico8_ui_def.hw`, NULL before
 * `pico8_ui_init()` and after `pico8_ui_shutdown()`. */
const box_t *pico8_ui_hw(void);

/* The empty container every new `ui` starts with as `le`: static, never
 * freed, emptied (not freed) by each `pico8_ui_init()`. */
entity_list_t *pico8_ui_le_shared(void);

/* An empty list in a place `le` of its own, its items grown there. */
entity_list_t *pico8_ui_le_new(void);

/* `le` from pico8_ui_le_new() becomes `self.le`; the list it replaces goes
 * to the exit queue (the frame still compares it in `lj`), unless it is
 * the shared one. */
void pico8_ui_set_le(pico8_ui_t *self, entity_list_t *le);

/* The `ui` registers itself through `lt` into `hv` and `cv.ev`, no `cg`; it
 * lives in the current game-zone generation. Asserts before the init. */
pico8_ui_t *pico8_ui_new(void);

/* Between `pico8_ui_init()` and `pico8_ui_set_je()` this is the placeholder
 * struct, with no member and no method; after `pico8_jl()` it is an `nf` struct
 * (pico8_mouse_reader.h). Its members are read through `pico8_je_nf()`. */
entity_t *pico8_je(void);

/* `je` as its `nf`: NULL when there is no je and on the placeholder. */
pico8_nf_t *pico8_je_nf(void);

const entity_class_t *pico8_je_placeholder_struct_class(void);

/* Makes `instance` the new `je`. The outgoing instance is NOT freed -- see
 * the comment at the definition. NULL goes back to "no je", and passing back
 * the current `pico8_je()` is a no-op. */
void pico8_ui_set_je(entity_t *instance);

/* NULL until `pico8_jl()` (pico8_bootstrap.c) calls `pico8_ui_set_ok()`, and
 * again after `pico8_ui_shutdown()`. */
pico8_ui_t *pico8_ok(void);

/* Same ownership rule as `pico8_ui_set_je()`. */
void pico8_ui_set_ok(pico8_ui_t *ui);

/* Called once per frame by `pico8__update60()` (pico8_cart.c). `mem` is an
 * explicit parameter because that call is direct, not routed through
 * a class member or `dc()` and their shared dispatch context. */
void pico8_ui_lj(p8_memory_t *mem, pico8_ui_t *self);

void pico8_ui_ev(const p8_graphics_t *gfx, p8_memory_t *mem, const pico8_ui_t *self);

void pico8_ui_gw(p8_memory_t *mem, pico8_ui_t *self);

/* `bt` is "l" or "r", anything else asserts: it picks the member
 * `lheld`/`rheld` and the method `ldown`/`rdown`. */
void pico8_ui_bh(pico8_ui_t *self, const char *bt);

/* The method `jz` asks each hovered entity: `ldown` and `rdown` get
 * `qa`/`ra`, `gv` gets `s`. */
typedef enum { PICO8_JZ_LDOWN, PICO8_JZ_RDOWN, PICO8_JZ_GV } pico8_jz_slot_t;

/* Lua's `{kv=h,kn=kn}` result, written out: returns 1 and fills `*out_kv`
 * and `*out_kn` on the first answer whose `stop` is set, or 0 leaving both
 * untouched when nothing answers. */
int pico8_ui_jz(pico8_ui_t *self, pico8_jz_slot_t rm, vec_t *qa, vec_t *ra, const entity_t *s,
                entity_t **out_kv, down_t *out_kn);

void pico8_ui_cj(pico8_ui_t *self);

#endif
