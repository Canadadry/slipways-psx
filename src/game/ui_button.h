#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "game/game_data.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/research.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

typedef struct pico8_js pico8_js_t;

/* `js.my`, `js.hb`: the cart calls them as `self:my()`, `self:hb()`, and
 * drops what they return. */
typedef void (*pico8_js_fn)(pico8_js_t *self);

/* A button: every key of its construction table, copied (`js(b)`), then
 * what `js:pq` built from them. `my` and `hb` are closures the table
 * carried, called as fields; `_cr_t` and `_jo_fe_pt`/`_b`/`_od` are what
 * the `my` closures of `cr` and `jo:fe` captured, not cart fields. `ra` and
 * `qf` are NULL (nil) or point at `ra_box`/`qf_box`, copies of the values the
 * keys carried. The struct, its `kq`, `lq`, `h_` and `jb` live in `place`,
 * given back by the exit queue once `he` takes the button out -- but for an
 * `lq` a `hb` rebuilt, which lives in `lq_place` and goes with the button. */
struct pico8_js {
    entity_t header;
    const pico8_zone_choisie_t *place;
    pico8_vec_t ra_box;
    pico8_vec_t qf_box;
    const pico8_js_def_t *def;
    pico8_fixed_t fa;
    pico8_lq_t *lq;
    const pico8_zone_choisie_t *lq_place; /* NULL: `lq` is in `place` */
    pico8_fixed_t mb;
    pico8_lq_t *h_;
    int has_h_; /* 0: nil; 1: present, and a NULL `h_` is `false` */
    pico8_jb_t *jb;
    const pico8_vec_t *ra;
    pico8_vec_t *qf;
    pico8_js_fn my;
    int has_my; /* 0: nil; 1: present, and a NULL `my` is `false` */
    int m_;
    pico8_js_fn hb;
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
};

void pico8_js_init(void);
void pico8_js_shutdown(void);

/* 1 from pico8_js_init() to pico8_js_shutdown(), 0 otherwise. */
int pico8_js_ready(void);
const entity_class_t *pico8_js_struct_class(void);

/* The cart's keys of a button, each typed as the member it fills (NULL
 * or an unset flag: absent). The rows `lq_row`, `h_row`, `jb_row` and the
 * `lq` `lq_copy` are read during the construction, not kept. `js.lq`: the
 * row `lq_row`, else a copy of `lq_copy`, else an empty row. */
typedef struct {
    int has_fa; /* 0: `fa` keeps the archetype's */
    pico8_fixed_t fa;
    const pico8_lq_t *lq_copy;
    const pico8_lq_keys_t *lq_row;
    int has_h_; /* present without `h_row` nor `h_is_false`: an empty row (TROU-23) */
    int h_is_false;
    const pico8_lq_keys_t *h_row;
    int has_jb; /* present without `jb_row`: an empty `jb` (TROU-23) */
    const pico8_jb_row_t *jb_row;
    const pico8_vec_t *ra;
    const pico8_vec_t *qf;
    int has_my; /* present, and a NULL `my` is `false` */
    pico8_js_fn my;
    pico8_js_fn hb;
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
 * through `lt`. */
pico8_js_t *pico8_js_new(const pico8_js_keys_t *keys);

/* `keys` is read, not kept (NULL: none). */
void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys);

void pico8_js_ph(pico8_js_t *self);

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self);

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pico8_vec_t p);

/* Always returns true: the click is consumed whether or not a `my` callback
 * was there to run. */
down_t pico8_js_ldown(pico8_js_t *self);

typedef struct pico8_fq pico8_fq_t;

/* `fq.my`: the Lua truth of what it returns, which `fq:ldown` keeps as `p_`. */
typedef int (*pico8_fq_my_fn)(pico8_fq_t *self);

/* `my` is the closure the construction table carried, which `mv` calls as
 * a field, not a method. The struct lives in `place`, given back by the
 * exit queue once `he` takes the veil out. */
struct pico8_fq {
    entity_t header;
    const pico8_zone_choisie_t *place;
    const pico8_fq_def_t *def;
    pico8_fq_my_fn my;
    int p_;
    int m_;
};

void pico8_fq_init(void);
void pico8_fq_shutdown(void);

/* 1 from pico8_fq_init() to pico8_fq_shutdown(), 0 otherwise. */
int pico8_fq_ready(void);
const entity_class_t *pico8_fq_struct_class(void);

/* The cart's key `my` of a veil: NULL is nil. */
typedef struct {
    pico8_fq_my_fn my;
} pico8_fq_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through `lt`. */
pico8_fq_t *pico8_fq_new(const pico8_fq_keys_t *keys);

down_t pico8_fq_ldown(pico8_fq_t *self);

#endif
