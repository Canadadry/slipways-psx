#ifndef PICO8_UI_H
#define PICO8_UI_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance, `gfx` or `mem` argument is a safe
 * no-op, and an absent or wrong-kind field reads as absent rather than
 * crashing. */

/* Calls `pico8_pi_init()` first: the class spec contains `b(10,10,118,118)`,
 * so the 'b' constructor must already be registered when it is parsed. The
 * `le=o()` default is then replaced by a real empty `pico8_list_t`, which is
 * what `gu()`/`nv()` need and not what `ob()` parses. Also puts back the
 * field-less placeholder `je` that `pico8_ui_set_je()` later replaces. */
void pico8_ui_init(void);

/* Frees the `ui` class and `ok`; `je` is never freed: the placeholder is
 * static, an `nf` struct in its place is dropped (TROU-7). Idempotent, and
 * safe before the first `pico8_ui_init()`. */
void pico8_ui_shutdown(void);

/* NULL before `pico8_ui_init()` and after `pico8_ui_shutdown()`. Not
 * reachable through `pico8_class_lookup()`. */
pico8_class_t *pico8_ui_class(void);

/* `overrides` is copied, not kept, and nothing ever frees the instance. */
pico8_instance_t *pico8_ui_new(const pico8_ob_table_t *overrides);

/* Between `pico8_ui_init()` and `pico8_ui_set_je()` this is the placeholder
 * struct, with no field and no method, so every `je.<field>` read falls back
 * to its default; after `pico8_jl()` it is an `nf` struct
 * (pico8_mouse_reader.h). Read it only through `pico8_entity_get_field()`. */
pico8_instance_t *pico8_je(void);

const pico8_struct_class_t *pico8_je_placeholder_struct_class(void);

/* Makes `instance` the new `je`. The outgoing instance is NOT freed -- see
 * the comment at the definition. NULL goes back to "no je", and passing back
 * the current `pico8_je()` is a no-op. */
void pico8_ui_set_je(pico8_instance_t *instance);

/* NULL until `pico8_jl()` (pico8_bootstrap.c) calls `pico8_ui_set_ok()`, and
 * again after `pico8_ui_shutdown()`. */
pico8_instance_t *pico8_ok(void);

/* Same ownership rule as `pico8_ui_set_je()`. */
void pico8_ui_set_ok(pico8_instance_t *instance);

/* Called once per frame by `pico8__update60()` (pico8_cart.c). `mem` is an
 * explicit parameter because that call is direct, not routed through
 * `mv()`/`dc()` and their shared dispatch context. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_instance_t *self);

void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self);

void pico8_ui_gw(pico8_mem_t *mem, pico8_instance_t *self);

/* `bt` is "l" or "r", concatenated at runtime into the field name
 * "lheld"/"rheld" and the method name "ldown"/"rdown". */
void pico8_ui_bh(pico8_instance_t *self, const char *bt);

/* Lua's `{kv=h,kn=kn}` result, written out: returns 1 and fills `*out_kv`
 * and `*out_kn` on the first truthy answer, or 0 leaving both untouched when
 * nothing answers. `argc`/`arg0`/`arg1` are forwarded to each candidate
 * unchanged. */
int pico8_ui_jz(pico8_instance_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_instance_t **out_kv, pico8_value_t *out_kn);

void pico8_ui_cj(pico8_instance_t *self);

#endif
