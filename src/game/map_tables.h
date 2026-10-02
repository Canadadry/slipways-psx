#ifndef GAME_MAP_TABLES_H
#define GAME_MAP_TABLES_H

#include "p8/fixed.h"

enum {
    RESOURCE_CODE_COUNT = 13,
    RESOURCE_NAMES_COUNT = 11,
    PRODUCTION_TABLES_COUNT = 6,
    PROJECT_RECORDS_COUNT = 42,
    ICON_TOOLTIPS_COUNT = 7,
    TECHNOLOGIES_COUNT = 19
};

typedef struct {
    const char *key;
    int resource;
} resource_code_t;

typedef struct {
    const char *key;
    const char *name;
} resource_names_entry_t;

/* values[0..count-1] are the entry's positionals; upgraded_entry is NULL
 * when absent. */
typedef struct {
    const char *key;
    int count;
    int values[4];
    const char *upgraded_entry;
} production_tables_entry_t;

/* An item is the string `text` when non-NULL, the row when the row field is
 * non-NULL, false when is_false is set, the number `fixed` when is_fixed is
 * set (a non-integer, as fixed-point bits), the pointer `ref` when is_ref is
 * set (NULL: absent; never set by the data tables), the integer `number`
 * otherwise. */
typedef struct data_row data_row_t;
typedef struct {
    const char *text;
    int number;
    const data_row_t *row;
    int is_false;
    int is_fixed;
    p8_fixed_t fixed;
    int is_ref;
    const void *ref;
} data_item_t;

/* `color` is only meaningful when has_color is set; draw_function is NULL
 * when absent; `resource`, which row_measure_item() draws as that resource's
 * sprite 5 pixels wide, only when has_resource is set (never by the data
 * tables). */
struct data_row {
    int count;
    const data_item_t *items;
    int has_color;
    int color;
    const char *draw_function;
    int has_resource;
    p8_fixed_t resource;
};

/* A non-NULL header makes the entry a bare string, the required_tech of the
 * records that follow, every other field zero. Otherwise positions 1..npos
 * are present: name, planet_codes, base_cost, production_key, inputs,
 * output, upgrade_tech, byproduct; upgrade_tech_false marks position 7
 * written false, upgrade_tech then being NULL. icon_sprite and play_sfx are
 * only meaningful under has_icon_sprite and has_play_sfx. */
typedef struct {
    const char *header;
    int npos;
    const char *name;
    const char *planet_codes;
    int base_cost;
    const char *production_key;
    const char *inputs;
    const char *output;
    const char *upgrade_tech;
    const char *byproduct;
    int upgrade_tech_false;
    int has_icon_sprite;
    int icon_sprite;
    const char *next_type;
    const char *class_name;
    int has_play_sfx;
    int play_sfx;
} project_records_entry_t;

/* icon_key is position 3 as written, a resource code; icon is that code's
 * resource, as resource_code_lookup() gives it. */
typedef struct {
    const char *name;
    int era;
    const char *icon_key;
    int icon;
    int cost;
    const data_row_t *tooltip_rows;
} technologies_entry_t;

extern const resource_code_t resource_codes[RESOURCE_CODE_COUNT];
extern const resource_names_entry_t resource_names_table[RESOURCE_NAMES_COUNT];
extern const production_tables_entry_t production_tables_table[PRODUCTION_TABLES_COUNT];
extern const project_records_entry_t project_records_table[PROJECT_RECORDS_COUNT];
extern const data_row_t icon_tooltips_table[ICON_TOOLTIPS_COUNT];
extern const technologies_entry_t technologies_table[TECHNOLOGIES_COUNT];

/* Returns 1 and writes *out_resource when key is present, 0 and leaves
 * *out_resource untouched otherwise. */
int resource_code_lookup(const char *key, int *out_resource);
/* NULL when key is absent. */
const char *resource_names_lookup(const char *key);
/* NULL when key is absent; otherwise a pointer into production_tables_table. */
const production_tables_entry_t *production_tables_lookup(const char *key);
/* The technologies_table entry with that name, NULL when none or when the
 * name is NULL. */
const technologies_entry_t *map_tables_technologies_lookup(const char *px);

#endif
