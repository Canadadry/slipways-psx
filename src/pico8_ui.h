#ifndef PICO8_UI_H
#define PICO8_UI_H

#include "pico8_bg.h"
#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL `ui`, `gfx` or `mem` argument is a safe
 * no-op, and an absent or wrong-kind field of another entity (`je`, `mo`,
 * `o_`) reads as absent rather than crashing. */

enum { PICO8_UI_IC_LAYERS = 16 };

/* `ic`: `layers[fa]` lists the hovered entities of layer `fa` in the order
 * they were found; `layer_count` is always PICO8_UI_IC_LAYERS. */
typedef struct {
    int layer_count;
    pico8_list_t layers[PICO8_UI_IC_LAYERS];
} pico8_ui_ic_t;

/* TROU-7 */
/* `le` of a new `ui` is the class's own empty list, shared until `cj` or
 * `pz` replaces it; `hw` is the class's box, borrowed. `ic` is NULL until
 * the first `gw`, then points at `ic_own`, which every `gw` empties and
 * refills. */
typedef struct {
    pico8_ke_t ke;
    pico8_fixed_t ql;
    pico8_fixed_t fa;
    pico8_list_t *le;
    pico8_qz_box_t *hw;
    pico8_vec_t *qa;
    pico8_ui_ic_t *ic;
    pico8_ui_ic_t ic_own;
    pico8_struct_header_t *o_; /* borrowed, never freed */
    pico8_struct_header_t *gj; /* borrowed, never freed */
    pico8_struct_header_t *lheld; /* borrowed, never freed */
    pico8_bg_t *rheld; /* borrowed, never freed */
} pico8_ui_t;

/* Calls `pico8_pi_init()` first: the class spec contains `b(10,10,118,118)`,
 * so the 'b' constructor must already be registered when it is parsed. The
 * `le=o()` default is then replaced by a real empty `pico8_list_t`, which is
 * what `gu()`/`nv()` need and not what `ob()` parses. Also puts back the
 * field-less placeholder `je` that `pico8_ui_set_je()` later replaces. */
void pico8_ui_init(void);

/* Frees the `ui` spec and `ok`; `je` is never freed: the placeholder is
 * static, an `nf` struct in its place is dropped (TROU-7). Idempotent, and
 * safe before the first `pico8_ui_init()`. */
void pico8_ui_shutdown(void);

/* 1 from pico8_ui_init() to pico8_ui_shutdown(), 0 otherwise. */
int pico8_ui_ready(void);

/* `ui`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_ui_shutdown(). */
int pico8_ui_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_ui_struct_class(void);

/* The box of the class, read by `bv`; NULL before `pico8_ui_init()` and
 * after `pico8_ui_shutdown()`. */
const pico8_qz_box_t *pico8_ui_hw(void);

/* `overrides` must be NULL or empty: a key asserts. The `ui` registers
 * itself through `lt` into `hv` and `cv.ev`, no `cg`; only the current
 * `ok` is ever freed, by `pico8_ui_shutdown()`. Asserts before the init. */
pico8_ui_t *pico8_ui_new(const pico8_ob_table_t *overrides);

/* Between `pico8_ui_init()` and `pico8_ui_set_je()` this is the placeholder
 * struct, with no field and no method, so every `je.<field>` read falls back
 * to its default; after `pico8_jl()` it is an `nf` struct
 * (pico8_mouse_reader.h). Read it only through `pico8_entity_get_field()`. */
pico8_struct_header_t *pico8_je(void);

const pico8_struct_class_t *pico8_je_placeholder_struct_class(void);

/* Makes `instance` the new `je`. The outgoing instance is NOT freed -- see
 * the comment at the definition. NULL goes back to "no je", and passing back
 * the current `pico8_je()` is a no-op. */
void pico8_ui_set_je(pico8_struct_header_t *instance);

/* NULL until `pico8_jl()` (pico8_bootstrap.c) calls `pico8_ui_set_ok()`, and
 * again after `pico8_ui_shutdown()`. */
pico8_ui_t *pico8_ok(void);

/* Same ownership rule as `pico8_ui_set_je()`. */
void pico8_ui_set_ok(pico8_ui_t *ui);

/* Called once per frame by `pico8__update60()` (pico8_cart.c). `mem` is an
 * explicit parameter because that call is direct, not routed through
 * `mv()`/`dc()` and their shared dispatch context. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_ui_t *self);

void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_ui_t *self);

void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self);

/* `bt` is "l" or "r", anything else asserts: it picks the member
 * `lheld`/`rheld` and the method `ldown`/`rdown`. */
void pico8_ui_bh(pico8_ui_t *self, const char *bt);

/* Lua's `{kv=h,kn=kn}` result, written out: returns 1 and fills `*out_kv`
 * and `*out_kn` on the first truthy answer, or 0 leaving both untouched when
 * nothing answers. `argc`/`arg0`/`arg1` are forwarded to each candidate
 * unchanged. */
int pico8_ui_jz(pico8_ui_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_struct_header_t **out_kv, pico8_value_t *out_kn);

void pico8_ui_cj(pico8_ui_t *self);

#endif
