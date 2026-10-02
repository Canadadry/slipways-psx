#ifndef GAME_RESEARCH_H
#define GAME_RESEARCH_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/map_tables.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"
#include "game/trade.h"

/* A want of `hp.i` or `jo.jy`, the cart's `{qe=r[c]}`, then `gc` as
 * `economy_tiers` writes it. `row` comes first and holds `qe` (has_qe 0:
 * nil) and no item: a REF to a want is the `{qe=x}` tuple `lq:mn` reads. */
typedef struct {
    data_row_t row;
    int has_satisfied; /* 0: `gc` is nil, and is then never read */
    p8_fixed_t satisfied;
} need_t;

/* `i.qe`: nil or a number, never false. */
resource_t need_resource(const need_t *i);

/* A fresh want in `zone`, never freed here: `qe` NULL is nil, no `gc`. */
need_t *need_new(const allocator_t *zone, const p8_fixed_t *qe);

/* A research project of `gb`, taken with its list `i` and the wants in it
 * from the load sub-zone: they live until the next load or shutdown, never
 * freed. The other members are borrowed from `pico8_fd_table` and from `r`,
 * except `lq`, which points at `lq_own` (its third item `lq_inner`) or is
 * NULL. The struct is never copied: `lq` points into it. */
typedef struct {
    entity_t header;
    const research_project_archetype_t *archetype;
    const char *name;
    const char *planet_codes;
    p8_fixed_t base_cost;
    int has_base_cost; /* 0: `fj` is nil, and is then never read */
    const char *production_key;
    pointer_list_t *inputs;
    p8_fixed_t output;
    int has_output; /* 0: `o` is nil, and is then never read */
    const char *upgrade_tech;
    int has_upgrade_tech; /* 0: `il` is nil; 1 with `il` NULL: `il` is false */
    p8_fixed_t byproduct;
    int has_byproduct; /* 0: `dm` is nil, and is then never read */
    const char *required_tech;
    p8_fixed_t icon_sprite;
    int has_icon_sprite; /* 0: `ii` is nil, and is then never read */
    const char *class_name;
    const char *next_type;
    p8_fixed_t play_sfx;
    int has_play_sfx; /* 0: `rn` is nil, and is then never read */
    const data_row_t *layout_row;
    data_row_t layout_row_own;
    data_item_t layout_row_own_items[3];
    data_row_t layout_row_inner;
    data_item_t layout_row_inner_items[2];
} research_project_t;

/* Throughout this module -- this file and its `_display` sibling -- a NULL
 * project argument is a safe no-op. */

/* Calls pico8_research_shutdown() first. */
void research_init(void);

/* Forgets `gb` without freeing its projects: their memory goes back at the
 * next load. Idempotent, and safe before the first pico8_research_init(). */
void research_shutdown(void);

/* 1 from pico8_research_init() to pico8_research_shutdown(), 0 otherwise. */
int research_project_ready(void);
const entity_class_t *research_project_class(void);

/* The class's `fk`: `pico8_hp_fk_row`, never NULL. */
const data_row_t *research_project_row_tail(void);

/* `il` as the cart reads it: NULL when nil, false or `self` is NULL. */
const char *research_project_upgrade_tech(const research_project_t *self);

/* Aborts unless `self` is NULL or a project. */
void research_project_assert(const research_project_t *self);

/* The keys of `pico8_hp_new`, read, not kept: a record of `pico8_fd_table`,
 * never a header (NULL: no positional, nor `ii`, `cw`, `go`, `rn`), and the
 * `rr` category (NULL: nil). Their strings are borrowed. */
typedef struct {
    const project_records_entry_t *record;
    const char *required_tech;
} research_project_keys_t;

/* `keys` NULL means none. Registers the new project into `gb` under its
 * `px`, so pico8_research_build_gb() must have built `gb` first. */
research_project_t *research_project_new(const research_project_keys_t *keys);

/* Rebuilds `gb` from `pico8_fd_table`, forgetting whatever a previous call
 * left, whose memory goes back at the next load. A header entry sets the
 * running `rr` category every later record is tagged with. */
void research_build_projects(void);

/* `gb`, owned by this module (callers free nothing): its projects by rank,
 * 1..pico8_gb_count() (NULL outside), and by `px`, the last project
 * registered under it (NULL when none). All empty before the first
 * pico8_research_build_gb() and after pico8_research_shutdown(). */
int research_projects_count(void);
research_project_t *research_projects_at(int k);
research_project_t *research_projects_find(const char *px);

/* Returns an entry of pico8_hc_table, never a copy, or NULL when `self.ja`
 * does not resolve to one. */
const production_tables_entry_t *research_project_production(const research_project_t *self);

p8_fixed_t research_project_cost(const research_project_t *self);

#endif
