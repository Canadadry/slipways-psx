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
    PICO8_BZ_COUNT = 12,
    PICO8_JO_DZ_COUNT = 4
};

/* items is NULL when count is 0. */
typedef struct {
    int count;
    const int *items;
} pico8_int_seq_t;

typedef struct {
    int count;
    const char *const *items;
} pico8_str_seq_t;

/* qe is only meaningful when has_qe is set. */
typedef struct {
    int has_qe;
    pico8_fixed_t qe;
    pico8_fixed_t mu;
} pico8_kw_t;

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

/* The table members of the jo tree, one object per class that sets it. */
extern const pico8_str_seq_t pico8_jo_nu_tags;
extern const pico8_str_seq_t pico8_qh_nu_tags;
extern const pico8_str_seq_t pico8_nl_nu_tags;
extern const pico8_str_seq_t pico8_os_nu_tags;
extern const pico8_str_seq_t pico8_hj_nu_tags;
extern const pico8_str_seq_t pico8_pm_nu_tags;
extern const pico8_str_seq_t pico8_ga_nu_tags;
extern const pico8_str_seq_t pico8_nr_nu_tags;
extern const pico8_str_seq_t pico8_mx_nu_tags;
extern const pico8_int_seq_t pico8_jo_ka_seq;
extern const pico8_jb_row_t pico8_jo_dz_rows[PICO8_JO_DZ_COUNT];
extern const pico8_int_seq_t pico8_jo_cq_seq;
extern const pico8_kw_t pico8_jo_kw_start;
extern const pico8_jb_row_t pico8_jo_em_row;
extern const pico8_jb_row_t pico8_qh_px_row;
extern const pico8_jb_row_t pico8_nl_px_row;
extern const pico8_jb_row_t pico8_os_px_row;
extern const pico8_jb_row_t pico8_hj_px_row;
extern const pico8_jb_row_t pico8_pm_px_row;
extern const pico8_jb_row_t pico8_mx_px_row;
extern const pico8_int_seq_t pico8_qh_ba_seq;
extern const pico8_int_seq_t pico8_nl_ba_seq;
extern const pico8_int_seq_t pico8_hj_k__seq;
extern const pico8_int_seq_t pico8_pm_k__seq;
extern const pico8_int_seq_t pico8_mx_k__seq;
extern const pico8_int_seq_t pico8_ga_k__seq;
/* The k_ key pm:pq passes to the ga it spawns. */
extern const pico8_int_seq_t pico8_pm_ga_k__seq;
extern const pico8_int_seq_t pico8_nr_hz_seq;
extern const pico8_int_seq_t pico8_fr_hz_seq;
extern const pico8_int_seq_t pico8_mx_ht_seq;

/* Lua's seq[i], 1-based: returns 1 and writes *out when 1 <= i <= count,
 * 0 and leaves *out untouched otherwise. */
int pico8_int_seq_at(const int *seq, int count, int i, int *out);
int pico8_fixed_seq_at(const pico8_fixed_t *seq, int count, int i, pico8_fixed_t *out);

/* 1 when v is pico8_fixed_from_int of an element: the cart's `gu` on integers. */
int pico8_int_seq_has_fixed(const pico8_int_seq_t *seq, pico8_fixed_t v);

/* The df entry whose px is `px`, NULL when none or `px` is NULL: `df[px]`
 * after the px/s/r/d pass. */
const pico8_df_entry_t *pico8_df_lookup(const char *px);

/* Bare Lua number literals rather than ob() spec fields, hence scalars
 * here and not another table. */
pico8_fixed_t pico8_bn(void);
pico8_fixed_t pico8_bq(void);

#endif
