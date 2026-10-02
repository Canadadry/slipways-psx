#ifndef PICO8_HELP_H
#define PICO8_HELP_H

#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/probe.h"
#include "engine/entity_class.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op, and an absent or wrong-kind field reads as absent. */

/* Builds `bo`; `pico8_help_shutdown()` frees it and is called first by an
 * init, so both are idempotent. Both also reset `eu` to the placeholder, so
 * the real one must be installed after every init. */
void pico8_help_init(void);
void pico8_help_shutdown(void);

/* 1 from pico8_help_init() to pico8_help_shutdown(), 0 otherwise. */
int pico8_fv_ready(void);
const entity_class_t *pico8_fv_struct_class(void);

/* TROU-8 */
/* The help's class carries `g_`'s tags, so `lt` registers it into `cg.g_` and
 * `cg.prober` too; it lives in the current game-zone generation. Its `ra` points at `pico8_fv_def`'s,
 * shared by every help. Asserts when called before `pico8_help_init()`. */
probe_t *pico8_fv_new(void);

/* `pico8_eu_field_value()` never returns NULL: an inert placeholder until
 * `pico8_help_set_eu()` installs the real `eu`, which
 * `pico8_help_set_eu(NULL)` puts back. */
button_fn_t pico8_eu_field_value(void);
void pico8_help_set_eu(button_fn_t fn);

/* Stamps the `eu` value current at call time onto every row of `bo`: rows
 * stamped before a `pico8_help_set_eu()` keep what they were given. A NULL
 * `self` still stamps them and only skips the `pz()` call. */
void pico8_fv_pq(probe_t *self);

void pico8_fv_kj(p8_graphics_t *gfx, p8_memory_t *mem, probe_t *self, vec_t p);

void pico8_fv_kj_dispatch(entity_t *self, const vec_t *ra);

#endif
