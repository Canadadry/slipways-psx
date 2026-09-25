#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
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

/* `my` is the closure the construction table carried, which `mv` calls as
 * a field, not a method. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t hd;
    pico8_value_t my;
    pico8_value_t p_;
    pico8_value_t m_;
} pico8_fq_t;

/* Declares the `fq` class, whose instantiation builds a pico8_fq_t. */
void pico8_fq_init(void);
void pico8_fq_shutdown(void);
pico8_class_t *pico8_fq_class(void);
const pico8_struct_class_t *pico8_fq_struct_class(void);

/* `overrides` is read, not kept: `my` only. The struct registers itself
 * through `lt`, and nothing frees it. */
pico8_fq_t *pico8_fq_new(const pico8_ob_table_t *overrides);

pico8_value_t pico8_fq_ldown(pico8_fq_t *self);

#endif
