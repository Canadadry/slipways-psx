#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* `js.hd=gk` is a plain field assignment, not a `function js:hd()`: there is
 * no pico8_js_hd, the hit test is pico8_gk. Because `hd` is a `bm` name, every
 * js instance registers into cv["hd"].
 *
 * Declares the `js` class and wires `pq` plus the hd/ph/ev/kj/ldown dispatch
 * adapters onto its defaults table. */
void pico8_js_init(void);
void pico8_js_shutdown(void);

pico8_class_t *pico8_js_class(void);

pico8_instance_t *pico8_js_new(const pico8_ob_table_t *overrides);

void pico8_js_pq(pico8_instance_t *self);

void pico8_js_ph(pico8_instance_t *self);

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self);

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p);

/* Always returns true: the click is consumed whether or not a `my` callback
 * was there to run. */
pico8_value_t pico8_js_ldown(pico8_instance_t *self);

/* Declares the `fq` class and wires `ldown` onto its defaults; `hd=1` needs
 * no wiring, being plain data parsed straight from the spec string. */
void pico8_fq_init(void);
void pico8_fq_shutdown(void);
pico8_class_t *pico8_fq_class(void);

pico8_instance_t *pico8_fq_new(const pico8_ob_table_t *overrides);

/* TROU-9 */
pico8_value_t pico8_fq_ldown(pico8_instance_t *self);

#endif
