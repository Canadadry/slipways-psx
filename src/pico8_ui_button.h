#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_map_tables.h"
#include "pico8_mem.h"
#include "pico8_planet.h"
#include "pico8_research.h"
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
    pico8_fixed_t fa;
    pico8_lq_t *lq;
    pico8_fixed_t mb;
    pico8_qz_box_t *kq;
    pico8_lq_t *h_;
    int has_h_; /* 0: nil; 1: present, and a NULL `h_` is `false` */
    pico8_jb_t *jb;
    pico8_vec_t *ra;
    pico8_vec_t *qf;
    pico8_value_function_t my;
    int has_my; /* 0: nil; 1: present, and a NULL `my` is `false` */
    int m_;
    int has_m_; /* 0: `m_` is nil, and is then never read */
    pico8_value_function_t hb;
    pico8_fixed_t fg;
    int has_fg; /* 0: `fg` is nil, and is then never read */
    const char *px;
    pico8_fixed_t trade;
    int has_trade; /* 0: `trade` is nil, and is then never read */
    pico8_fixed_t en;
    int has_en; /* 0: `en` is nil, and is then never read */
    const pico8_nb_entry_t *_cr_t; /* an entry of pico8_nb_table */
    pico8_jo_t *_jo_fe_pt;
    pico8_hp_t *_jo_fe_b;
    pico8_fixed_t _jo_fe_od;
    int has__jo_fe_od; /* 0: `_jo_fe_od` is nil, and is then never read */
} pico8_js_t;

void pico8_js_init(void);
void pico8_js_shutdown(void);

/* 1 from pico8_js_init() to pico8_js_shutdown(), 0 otherwise. */
int pico8_js_ready(void);
const pico8_struct_class_t *pico8_js_struct_class(void);

/* The cart's keys of a button, each typed as the member it fills (NULL
 * or an unset flag: absent). `lq`, `h_` and `jb` are the sources `js:pq`
 * copies a row or a `jb` from: the row `lq_row`, `h_row`, `jb_row` when set
 * (read during the construction, not kept), else a list, an `lq` row, true,
 * false or nil. */
typedef struct {
    int has_fa; /* 0: `fa` keeps `pico8_js_start.fa` */
    pico8_fixed_t fa;
    pico8_value_t lq;
    const pico8_lq_keys_t *lq_row;
    int has_h_; /* present: a nil `h_` builds an empty row (TROU-23), false none */
    pico8_value_t h_;
    const pico8_lq_keys_t *h_row;
    int has_jb; /* present: a nil `jb` builds an empty `jb` (TROU-23) */
    pico8_value_t jb;
    const pico8_jb_row_t *jb_row;
    pico8_vec_t *ra;
    pico8_vec_t *qf;
    int has_my; /* present, and a NULL `my` is `false` */
    pico8_value_function_t my;
    pico8_value_function_t hb;
    int has_fg;
    pico8_fixed_t fg;
    const char *px;
    int has_trade;
    pico8_fixed_t trade;
    int has_en;
    pico8_fixed_t en;
    const pico8_nb_entry_t *_cr_t;
    pico8_jo_t *_jo_fe_pt;
    pico8_hp_t *_jo_fe_b;
    int has__jo_fe_od;
    pico8_fixed_t _jo_fe_od;
} pico8_js_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through `lt`, and nothing frees it. */
pico8_js_t *pico8_js_new(const pico8_js_keys_t *keys);

/* `h_` as a value: REF NULL when `self` is NULL or `h_` is absent, `false`
 * when present and NULL, a REF to the row otherwise. */
pico8_value_t pico8_js_h_(const pico8_js_t *self);

/* `my` as a value: REF NULL when `self` is NULL or `my` is absent, `false`
 * when present and NULL, the function otherwise. */
pico8_value_t pico8_js_my(const pico8_js_t *self);

/* `lq`, `h_`, `jb` are the row's construction values `js:pq` builds the
 * members from, TROU-23 already applied; `lq_row`, `h_row`, `jb_row`, when
 * set, the rows it builds them from instead. All borrowed, never kept. */
void pico8_js_pq(pico8_js_t *self, pico8_value_t lq, const pico8_lq_keys_t *lq_row, pico8_value_t h_,
                 const pico8_lq_keys_t *h_row, pico8_value_t jb, const pico8_jb_row_t *jb_row);

void pico8_js_ph(pico8_js_t *self);

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self);

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pico8_vec_t p);

/* Always returns true: the click is consumed whether or not a `my` callback
 * was there to run. */
pico8_down_t pico8_js_ldown(pico8_js_t *self);

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
const pico8_struct_class_t *pico8_fq_struct_class(void);

/* The cart's key `my` of a veil: NULL is nil. */
typedef struct {
    pico8_value_function_t my;
} pico8_fq_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through `lt`, and nothing frees it. */
pico8_fq_t *pico8_fq_new(const pico8_fq_keys_t *keys);

pico8_down_t pico8_fq_ldown(pico8_fq_t *self);

#endif
