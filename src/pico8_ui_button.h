#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* A button: every key of its construction table, copied (`js(b)`), then
 * what `js:pq` built from them. `my` and `hb` are closures the table
 * carried, called as fields; `_cr_t` and `_jo_fe_pt`/`_b`/`_od` are what
 * the `my` closures of `cr` and `jo:fe` captured, not cart fields. `ra` may
 * be the vector every button of one `pz` shares: never written through.
 * Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t lq;
    pico8_value_t mb;
    pico8_value_t kq;
    pico8_value_t h_;
    pico8_value_t jb;
    pico8_value_t ra;
    pico8_value_t qf;
    pico8_value_t my;
    pico8_value_t m_;
    pico8_value_t hb;
    pico8_value_t fg;
    pico8_value_t px;
    pico8_value_t trade;
    pico8_value_t en;
    pico8_value_t _cr_t;
    pico8_value_t _jo_fe_pt;
    pico8_value_t _jo_fe_b;
    pico8_value_t _jo_fe_od;
} pico8_js_t;

void pico8_js_init(void);
void pico8_js_shutdown(void);

/* 1 from pico8_js_init() to pico8_js_shutdown(), 0 otherwise. */
int pico8_js_ready(void);

/* `js`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_js_shutdown(). */
int pico8_js_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_js_struct_class(void);

/* `overrides` is read, not kept: its measured keys only, positional ones
 * never. The struct registers itself through `lt`, and nothing frees it. */
pico8_js_t *pico8_js_new(const pico8_ob_table_t *overrides);

void pico8_js_pq(pico8_js_t *self);

void pico8_js_ph(pico8_js_t *self);

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self);

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pico8_vec_t p);

/* Always returns true: the click is consumed whether or not a `my` callback
 * was there to run. */
pico8_value_t pico8_js_ldown(pico8_js_t *self);

/* `my` is the closure the construction table carried, which `mv` calls as
 * a field, not a method. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_fixed_t fa;
    pico8_fixed_t hd;
    pico8_value_function_t my;
    int p_;
    int has_p_;
    int m_;
    int has_m_;
} pico8_fq_t;

void pico8_fq_init(void);
void pico8_fq_shutdown(void);

/* 1 from pico8_fq_init() to pico8_fq_shutdown(), 0 otherwise. */
int pico8_fq_ready(void);

/* `fq`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_fq_shutdown(). */
int pico8_fq_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_fq_struct_class(void);

/* `overrides` is read, not kept: `my` only. The struct registers itself
 * through `lt`, and nothing frees it. */
pico8_fq_t *pico8_fq_new(const pico8_ob_table_t *overrides);

pico8_value_t pico8_fq_ldown(pico8_fq_t *self);

#endif
