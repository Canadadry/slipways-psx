#ifndef PICO8_RESEARCH_H
#define PICO8_RESEARCH_H

#include "p8/fixed.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"
#include "pico8_trade.h"

/* A want of `hp.i` or `jo.jy`, the cart's `{qe=r[c]}`, then `gc` as
 * `economy_tiers` writes it. `row` comes first and holds `qe` (has_qe 0:
 * nil) and no item: a REF to a want is the `{qe=x}` tuple `lq:mn` reads. */
typedef struct {
    pico8_jb_row_t row;
    int has_gc; /* 0: `gc` is nil, and is then never read */
    pico8_fixed_t gc;
} pico8_jy_t;

/* `i.qe`: nil or a number, never false. */
pico8_qe_t pico8_jy_qe(const pico8_jy_t *i);

/* A fresh want in `zone`, never freed here: `qe` NULL is nil, no `gc`. */
pico8_jy_t *pico8_jy_new(const pico8_zone_choisie_t *zone, const pico8_fixed_t *qe);

/* A research project of `gb`, taken with its list `i` and the wants in it
 * from the load sub-zone: they live until the next load or shutdown, never
 * freed. The other members are borrowed from `pico8_fd_table` and from `r`,
 * except `lq`, which points at `lq_own` (its third item `lq_inner`) or is
 * NULL. The struct is never copied: `lq` points into it. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_hp_def_t *def;
    const char *px;
    const char *lr;
    pico8_fixed_t fj;
    int has_fj; /* 0: `fj` is nil, and is then never read */
    const char *ja;
    pico8_ptrs_t *i;
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
    const pico8_jb_row_t *lq;
    pico8_jb_row_t lq_own;
    pico8_jb_item_t lq_own_items[3];
    pico8_jb_row_t lq_inner;
    pico8_jb_item_t lq_inner_items[2];
} pico8_hp_t;

/* Throughout this module -- this file and its `_display` sibling -- a NULL
 * project argument is a safe no-op. */

/* Calls pico8_research_shutdown() first. */
void pico8_research_init(void);

/* Forgets `gb` without freeing its projects: their memory goes back at the
 * next load. Idempotent, and safe before the first pico8_research_init(). */
void pico8_research_shutdown(void);

/* 1 from pico8_research_init() to pico8_research_shutdown(), 0 otherwise. */
int pico8_hp_ready(void);
const pico8_struct_class_t *pico8_hp_struct_class(void);

/* The class's `fk`: `pico8_hp_fk_row`, never NULL. */
const pico8_jb_row_t *pico8_hp_fk(void);

/* `il` as the cart reads it: NULL when nil, false or `self` is NULL. */
const char *pico8_hp_il(const pico8_hp_t *self);

/* Aborts unless `self` is NULL or a project. */
void pico8_hp_assert(const pico8_hp_t *self);

/* The keys of `pico8_hp_new`, read, not kept: a record of `pico8_fd_table`,
 * never a header (NULL: no positional, nor `ii`, `cw`, `go`, `rn`), and the
 * `rr` category (NULL: nil). Their strings are borrowed. */
typedef struct {
    const pico8_fd_entry_t *ri;
    const char *rr;
} pico8_hp_keys_t;

/* `keys` NULL means none. Registers the new project into `gb` under its
 * `px`, so pico8_research_build_gb() must have built `gb` first. */
pico8_hp_t *pico8_hp_new(const pico8_hp_keys_t *keys);

/* Rebuilds `gb` from `pico8_fd_table`, forgetting whatever a previous call
 * left, whose memory goes back at the next load. A header entry sets the
 * running `rr` category every later record is tagged with. */
void pico8_research_build_gb(void);

/* `gb`, owned by this module (callers free nothing): its projects by rank,
 * 1..pico8_gb_count() (NULL outside), and by `px`, the last project
 * registered under it (NULL when none). All empty before the first
 * pico8_research_build_gb() and after pico8_research_shutdown(). */
int pico8_gb_count(void);
pico8_hp_t *pico8_gb_at(int k);
pico8_hp_t *pico8_gb_find(const char *px);

/* Returns an entry of pico8_hc_table, never a copy, or NULL when `self.ja`
 * does not resolve to one. */
const pico8_hc_entry_t *pico8_hp_po(const pico8_hp_t *self);

pico8_fixed_t pico8_hp_od(const pico8_hp_t *self);

#endif
