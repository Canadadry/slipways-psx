#ifndef PICO8_RESEARCH_H
#define PICO8_RESEARCH_H

#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* A research project of `gb`. Freed only by pico8_research_shutdown(),
 * through `gb`'s positional entries. `mr`/`gp`/`fk` are the class's REFs;
 * the other members are borrowed from `fd`'s records and from `r`, except
 * `i` and `lq`, tables built here and never freed (TROU-7). */
typedef struct {
    pico8_struct_header_t header;
    pico8_ob_table_t *mr;
    pico8_ob_table_t *gp;
    pico8_ob_table_t *fk;
    const char *px;
    const char *lr;
    pico8_fixed_t fj;
    int has_fj; /* 0: `fj` is nil, and is then never read */
    const char *ja;
    pico8_list_t *i;
    pico8_fixed_t o;
    int has_o; /* 0: `o` is nil, and is then never read */
    const char *il;
    int has_il; /* 0: `il` is nil; 1 with `il` NULL: `il` is false */
    pico8_fixed_t dm;
    int has_dm; /* 0: `dm` is nil, and is then never read */
    const char *rr;
    pico8_fixed_t ii;
    int has_ii; /* 0: `ii` is nil, and is then never read */
    const char *cw;
    const char *go;
    pico8_fixed_t rn;
    int has_rn; /* 0: `rn` is nil, and is then never read */
    pico8_ob_table_t *lq;
} pico8_hp_t;

/* Throughout this module -- this file and its `_display` sibling -- a NULL
 * project argument is a safe no-op. */

/* Calls pico8_research_shutdown() first. */
void pico8_research_init(void);

/* Frees every project `gb` still holds, then `gb` and hp's spec.
 * Walks `gb`'s positional entries only: its named `gb[px]` entries alias
 * the same projects, so freeing both would free them twice. Idempotent, and
 * safe before the first pico8_research_init(). */
void pico8_research_shutdown(void);

/* 1 from pico8_research_init() to pico8_research_shutdown(), 0 otherwise. */
int pico8_hp_ready(void);

/* `hp`'s spec default `name`; 0 when absent, or when not ready. A REF is
 * borrowed from the spec, valid until pico8_research_shutdown(). */
int pico8_hp_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_hp_struct_class(void);

/* The class's `fk`; nil before pico8_research_init() and after
 * pico8_research_shutdown(). */
pico8_value_t pico8_hp_fk(void);

/* `il` as the cart holds it: REF NULL when nil or `self` is NULL, BOOL 0, or the STRING. */
pico8_value_t pico8_hp_il(const pico8_hp_t *self);

/* Aborts unless `self` is NULL or a project. */
void pico8_hp_assert(const pico8_hp_t *self);

/* Takes only the positionals 1-8 and the keys "rr", "ii", "cw", "go",
 * "rn", each optional. Registers the new project into `gb` under its `px`,
 * so pico8_research_build_gb() must have built `gb` first. */
pico8_hp_t *pico8_hp_new(const pico8_ob_table_t *overrides);

/* Rebuilds `gb` from `fd`, freeing whatever a previous call left. A bare
 * string entry of `fd` sets the running `rr` category every later record is
 * tagged with. `fd`'s strings are borrowed, never copied, so `fd` must
 * outlive `gb` -- pico8_data_shutdown() frees them in that order. A NULL
 * `fd` leaves `gb` empty. */
void pico8_research_build_gb(const pico8_ob_table_t *fd);

/* Owned by this module; callers must not free it. */
const pico8_ob_table_t *pico8_gb(void);

/* Returns an `hc` entry borrowed from `hc`'s own storage, never a copy, or
 * NULL when `self.ja` does not resolve to one. */
const pico8_ob_table_t *pico8_hp_po(const pico8_hp_t *self);

pico8_fixed_t pico8_hp_od(const pico8_hp_t *self);

#endif
