#ifndef GAME_RESEARCH_H
#define GAME_RESEARCH_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/map_tables.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"
#include "game/trade.h"

/* A need of a project's `inputs` or of a planet's `needs`: the resource of
 * one input code, then `satisfied` as economy_tiers writes it. `row` comes
 * first and holds the resource (`has_resource` 0: absent) and no item: a REF
 * to a need is the one-resource tuple the layout row reads. */
typedef struct {
    data_row_t row;
    int has_satisfied; /* 0: absent, and then never read */
    p8_fixed_t satisfied;
} need_t;

/* The need's resource: absent or a number, never false. */
resource_t need_resource(const need_t *i);

/* A fresh need in `zone`, never freed here: a NULL resource is absent, and
 * `satisfied` is unset. */
need_t *need_new(const allocator_t *zone, const p8_fixed_t *qe);

/* A research project, taken with its `inputs` list and the needs in it
 * from the load sub-zone: they live until the next load or shutdown, never
 * freed. The other members are borrowed from `project_records_table` and
 * from `resource_codes`, except `layout_row`, which points at
 * `layout_row_own` (its third item `layout_row_inner`) or is NULL. The
 * struct is never copied: `layout_row` points into it. */
typedef struct {
    entity_t header;
    const research_project_archetype_t *archetype;
    const char *name;
    const char *planet_codes;
    p8_fixed_t base_cost;
    int has_base_cost; /* 0: absent, and then never read */
    const char *production_key;
    pointer_list_t *inputs;
    p8_fixed_t output;
    int has_output; /* 0: absent, and then never read */
    const char *upgrade_tech;
    int has_upgrade_tech; /* 0: absent; 1 with `upgrade_tech` NULL: false */
    p8_fixed_t byproduct;
    int has_byproduct; /* 0: absent, and then never read */
    const char *required_tech;
    p8_fixed_t icon_sprite;
    int has_icon_sprite; /* 0: absent, and then never read */
    const char *class_name;
    const char *next_type;
    p8_fixed_t play_sfx;
    int has_play_sfx; /* 0: absent, and then never read */
    const data_row_t *layout_row;
    data_row_t layout_row_own;
    data_item_t layout_row_own_items[3];
    data_row_t layout_row_inner;
    data_item_t layout_row_inner_items[2];
} research_project_t;

/* Throughout this module -- this file and research_display -- a NULL
 * project argument answers NULL or 0. */

/* Calls research_shutdown() first. */
void research_init(void);

/* Forgets the projects without freeing them: their memory goes back at the
 * next load. Idempotent, and safe before the first research_init(). */
void research_shutdown(void);

/* 1 from research_init() to research_shutdown(), 0 otherwise. */
int research_project_ready(void);
const entity_class_t *research_project_class(void);

/* The class's row tail: `research_project_row_tail_row`, never NULL. */
const data_row_t *research_project_row_tail(void);

/* `upgrade_tech`, or NULL when it is absent, false or `self` is NULL. */
const char *research_project_upgrade_tech(const research_project_t *self);

/* Aborts unless `self` is NULL or a project. */
void research_project_assert(const research_project_t *self);

/* The keys of research_project_new(), read, not kept: a record of
 * `project_records_table`, never a header (NULL: no position, nor
 * `icon_sprite`, `class_name`, `next_type`, `play_sfx`), and the
 * `required_tech` category (NULL: none). Their strings are borrowed. */
typedef struct {
    const project_records_entry_t *record;
    const char *required_tech;
} research_project_keys_t;

/* `keys` NULL means none. Registers a named project under its name, so
 * research_build_projects() must have run first. */
research_project_t *research_project_new(const research_project_keys_t *keys);

/* Rebuilds the projects from `project_records_table`, forgetting whatever a
 * previous call left, whose memory goes back at the next load. A header
 * entry sets the running `required_tech` every later record is tagged with. */
void research_build_projects(void);

/* The projects, owned by this module (callers free nothing): by rank,
 * 1..research_projects_count() (NULL outside), and by name, the last
 * project registered under it (NULL when none). All empty before the first
 * research_build_projects() and after research_shutdown(). */
int research_projects_count(void);
research_project_t *research_projects_at(int k);
research_project_t *research_projects_find(const char *px);

/* Returns an entry of `production_tables_table`, never a copy, or NULL when
 * the project's `production_key` does not resolve to one. */
const production_tables_entry_t *research_project_production(const research_project_t *self);

p8_fixed_t research_project_cost(const research_project_t *self);

#endif
