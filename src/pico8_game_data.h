#ifndef PICO8_GAME_DATA_H
#define PICO8_GAME_DATA_H

#include "pico8_fixed.h"
#include "pico8_map_tables.h"

enum {
    PICO8_ES_COUNT = 4,
    PICO8_CT_COUNT = 4,
    PICO8_B__COUNT = 4,
    PICO8_CO_COUNT = 4,
    PICO8_FB_COUNT = 14,
    PICO8_EK_COUNT = 4,
    PICO8_CI_COUNT = 15,
    PICO8_EF_COUNT = 15,
    PICO8_DF_COUNT = 11,
    PICO8_IX_COUNT = 4,
    PICO8_HA_COUNT = 4,
    PICO8_BO_COUNT = 4,
    PICO8_BZ_COUNT = 12
};

/* A pico8_fixed_t below holds the 16.16 bits ob() returned for the cart's
 * literal, 0xffff.fffe mask included: -0x5540 is -0.333. */

/* d is NULL for a record of 3 positionals, me NULL when absent. */
typedef struct {
    const char *px;
    int s;
    pico8_fixed_t r;
    const char *d;
    const char *me;
} pico8_df_entry_t;

/* ra, then lq. */
typedef struct {
    int ra_x;
    int ra_y;
    const pico8_jb_row_t *lq;
} pico8_ix_entry_t;

/* fg is only meaningful when has_fg is set. */
typedef struct {
    const pico8_jb_row_t *lq;
    int has_fg;
    int fg;
} pico8_ha_entry_t;

/* ha's named entries after its rows: hi, gq, ca. */
typedef struct {
    int hi_x;
    int hi_y;
    int gq_x;
    int gq_y;
    int ca;
} pico8_ha_tail_t;

typedef struct {
    const pico8_jb_row_t *lq;
    const char *px;
    int trade;
    pico8_fixed_t en;
} pico8_bo_entry_t;

/* row holds the positionals, its c coming last; then fn (NULL when absent),
 * score and ra, each only meaningful under its has_ flag. */
typedef struct {
    pico8_jb_row_t row;
    const char *fn;
    int has_score;
    int score;
    int has_ra;
    int ra_x;
    int ra_y;
} pico8_bz_entry_t;

extern const int pico8_es_table[PICO8_ES_COUNT];
extern const pico8_fixed_t pico8_ct_table[PICO8_CT_COUNT];
extern const int pico8_b__table[PICO8_B__COUNT];
extern const int pico8_co_table[PICO8_CO_COUNT];
extern const int pico8_fb_table[PICO8_FB_COUNT];
extern const int pico8_ek_table[PICO8_EK_COUNT];
extern const pico8_fixed_t pico8_ci_table[PICO8_CI_COUNT];
extern const char *const pico8_ef_table[PICO8_EF_COUNT];
extern const pico8_df_entry_t pico8_df_table[PICO8_DF_COUNT];
extern const pico8_ix_entry_t pico8_ix_table[PICO8_IX_COUNT];
extern const pico8_ha_entry_t pico8_ha_table[PICO8_HA_COUNT];
extern const pico8_ha_tail_t pico8_ha_tail;
extern const pico8_bo_entry_t pico8_bo_table[PICO8_BO_COUNT];
extern const pico8_bz_entry_t pico8_bz_table[PICO8_BZ_COUNT];

/* Lua's seq[i], 1-based: returns 1 and writes *out when 1 <= i <= count,
 * 0 and leaves *out untouched otherwise. */
int pico8_int_seq_at(const int *seq, int count, int i, int *out);
int pico8_fixed_seq_at(const pico8_fixed_t *seq, int count, int i, pico8_fixed_t *out);

/* Bare Lua number literals rather than ob() spec fields, hence scalars
 * here and not another table. */
pico8_fixed_t pico8_bn(void);
pico8_fixed_t pico8_bq(void);

#endif
