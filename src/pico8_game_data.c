#include "pico8_game_data.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

const int pico8_es_table[PICO8_ES_COUNT] = {1, 0, 1, 2};

const pico8_fixed_t pico8_ct_table[PICO8_CT_COUNT] = {-0x5540, 0x0, 0x4000, 0x8000};

const int pico8_b__table[PICO8_B__COUNT] = {0, 100, 200, 400};

const int pico8_co_table[PICO8_CO_COUNT] = {-5, 0, 1, 1};

const int pico8_fb_table[PICO8_FB_COUNT] = {2, 2, 5, 5, 7, 7, 10, 10, 14, 14, 14, 19, 19, 19};

const int pico8_ek_table[PICO8_EK_COUNT] = {2, 4, 7, 12};

const pico8_fixed_t pico8_ci_table[PICO8_CI_COUNT] = {0x8000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

const char *const pico8_ef_table[PICO8_EF_COUNT] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP",
    "OCT", "NOV", "DEC", "UND", "DUO", "TER"};

const pico8_df_entry_t pico8_df_table[PICO8_DF_COUNT] = {
    {"ll", 45, 0x0, NULL, "ll"},
    {"e", 160, 0x48000, "earth-like", NULL},
    {"f", 128, 0x60000, "forgeworld", NULL},
    {"m", 134, 0x40000, "mineral", NULL},
    {"o", 130, 0x30000, "ocean", NULL},
    {"r", 140, 0x28000, "remnant", NULL},
    {"x", 162, 0x30000, "xeno", NULL},
    {"j", 136, 0x20000, "jungle", NULL},
    {"i", 138, 0x20000, "iceball", NULL},
    {"s", 142, 0x20000, "barren", NULL},
    {"g", 96, 0x40000, "gas giant", NULL},
};

static const pico8_jb_item_t ix_1_lq_1_items[] = {{.n = 15}, {.n = 7}};
static const pico8_jb_row_t ix_1_lq_1 = {.count = 2, .items = ix_1_lq_1_items};
static const pico8_jb_item_t ix_1_lq_items[] = {{.row = &ix_1_lq_1}};
static const pico8_jb_row_t ix_1_lq = {.count = 1, .items = ix_1_lq_items};
static const pico8_jb_item_t ix_2_lq_1_items[] = {{.n = 37}, {.n = 7}};
static const pico8_jb_row_t ix_2_lq_1 = {.count = 2, .items = ix_2_lq_1_items};
static const pico8_jb_item_t ix_2_lq_items[] = {{.row = &ix_2_lq_1}};
static const pico8_jb_row_t ix_2_lq = {.count = 1, .items = ix_2_lq_items};
static const pico8_jb_item_t ix_3_lq_1_items[] = {{.n = 25}, {.n = 7}};
static const pico8_jb_row_t ix_3_lq_1 = {.count = 2, .items = ix_3_lq_1_items};
static const pico8_jb_item_t ix_3_lq_items[] = {{.row = &ix_3_lq_1}};
static const pico8_jb_row_t ix_3_lq = {.count = 1, .items = ix_3_lq_items};
static const pico8_jb_item_t ix_4_lq_1_items[] = {{.n = 8}, {.n = 7}};
static const pico8_jb_row_t ix_4_lq_1 = {.count = 2, .items = ix_4_lq_1_items};
static const pico8_jb_item_t ix_4_lq_items[] = {{.row = &ix_4_lq_1}};
static const pico8_jb_row_t ix_4_lq = {.count = 1, .items = ix_4_lq_items};

const pico8_ix_entry_t pico8_ix_table[PICO8_IX_COUNT] = {
    {118, 0, &ix_1_lq},
    {0, 0, &ix_2_lq},
    {47, 0, &ix_3_lq},
    {0, 117, &ix_4_lq},
};

static const pico8_jb_item_t ha_1_lq_items[] = {{.n = 31}, {.s = "restart     "}};
static const pico8_jb_row_t ha_1_lq = {.count = 2, .items = ha_1_lq_items};
static const pico8_jb_item_t ha_2_lq_items[] = {{.n = 42}, {.s = "toggle music"}};
static const pico8_jb_row_t ha_2_lq = {.count = 2, .items = ha_2_lq_items};
static const pico8_jb_item_t ha_3_lq_items[] = {{.n = 40}, {.s = "toggle sfx  "}};
static const pico8_jb_row_t ha_3_lq = {.count = 2, .items = ha_3_lq_items};
static const pico8_jb_item_t ha_4_lq_items[] = {{.n = 26}, {.s = "how to play\?"}};
static const pico8_jb_row_t ha_4_lq = {.count = 2, .items = ha_4_lq_items};

const pico8_ha_entry_t pico8_ha_table[PICO8_HA_COUNT] = {
    {&ha_1_lq, 0, 0},
    {&ha_2_lq, 0, 0},
    {&ha_3_lq, 0, 0},
    {&ha_4_lq, 1, 5},
};

const pico8_ha_tail_t pico8_ha_tail = {0, 107, 0, -11, 14};

static const pico8_jb_item_t bo_1_lq_items[] = {{.n = 49}, {.s = "forgiving   "}};
static const pico8_jb_row_t bo_1_lq = {.count = 2, .items = bo_1_lq_items};
static const pico8_jb_item_t bo_2_lq_items[] = {{.n = 50}, {.s = "reasonable  "}};
static const pico8_jb_row_t bo_2_lq = {.count = 2, .items = bo_2_lq_items};
static const pico8_jb_item_t bo_3_lq_items[] = {{.n = 51}, {.s = "challenging "}};
static const pico8_jb_row_t bo_3_lq = {.count = 2, .items = bo_3_lq_items};
static const pico8_jb_item_t bo_4_lq_items[] = {{.n = 52}, {.s = "tough       "}};
static const pico8_jb_row_t bo_4_lq = {.count = 2, .items = bo_4_lq_items};

const pico8_bo_entry_t pico8_bo_table[PICO8_BO_COUNT] = {
    {&bo_1_lq, "forgiving", 7, -0xfd70},
    {&bo_2_lq, "reasonable", 6, -0x999a},
    {&bo_3_lq, "challenging", 5, -0x6666},
    {&bo_4_lq, "tough", 4, -0x3334},
};

static const pico8_jb_item_t bz_1_items[] = {{.n = 26}, {.n = 86}, {.n = 0}};
static const pico8_jb_item_t bz_2_items[] = {{.n = 51}, {.n = 59}, {.n = 1}};
static const pico8_jb_item_t bz_3_items[] = {{.n = 76}, {.n = 77}, {.n = 1}};
static const pico8_jb_item_t bz_4_items[] = {{.n = 24}, {.n = 25}, {.n = 1}};
static const pico8_jb_item_t bz_5_items[] = {{.n = 33}, {.n = 41}, {.n = 1}};
static const pico8_jb_item_t bz_6_items[] = {{.s = ""}};
static const pico8_jb_item_t bz_7_items[] = {{.n = 55}, {.s = "planets:   "}};
static const pico8_jb_item_t bz_8_items[] = {{.n = 19}, {.s = "population:"}};
static const pico8_jb_item_t bz_9_items[] = {{.n = 24}, {.s = "technology:"}};
static const pico8_jb_item_t bz_10_1_items[] = {{.n = 27}, {.n = 5}};
static const pico8_jb_row_t bz_10_1 = {.count = 2, .items = bz_10_1_items};
static const pico8_jb_item_t bz_10_2_items[] = {{.is_false = 1}, {.n = 0}};
static const pico8_jb_row_t bz_10_2 = {.count = 2, .items = bz_10_2_items};
static const pico8_jb_item_t bz_10_4_items[] = {{.n = 25}, {.n = 5}};
static const pico8_jb_row_t bz_10_4 = {.count = 2, .items = bz_10_4_items};
static const pico8_jb_item_t bz_10_items[] = {{.row = &bz_10_1}, {.row = &bz_10_2}, {.is_false = 1}, {.row = &bz_10_4},
    {.s = "happiness"}};
static const pico8_jb_item_t bz_11_items[] = {{.s = "total:"}, {.s = ""}};

const pico8_bz_entry_t pico8_bz_table[PICO8_BZ_COUNT] = {
    {.row = {.count = 3, .items = bz_1_items}, .fn = "br"},
    {.row = {.count = 3, .items = bz_2_items}, .fn = "br"},
    {.row = {.count = 3, .items = bz_3_items}, .fn = "br"},
    {.row = {.count = 3, .items = bz_4_items}, .fn = "br"},
    {.row = {.count = 3, .items = bz_5_items}, .fn = "br"},
    {.row = {.count = 1, .items = bz_6_items, .has_c = 1, .c = 5}, .has_score = 1, .score = 0, .has_ra = 1, .ra_x = 64,
        .ra_y = 21},
    {.row = {.count = 2, .items = bz_7_items}, .has_score = 1, .score = 1, .has_ra = 1, .ra_x = 64, .ra_y = 34},
    {.row = {.count = 2, .items = bz_8_items}, .has_score = 1, .score = 2, .has_ra = 1, .ra_x = 64, .ra_y = 43},
    {.row = {.count = 2, .items = bz_9_items}, .has_score = 1, .score = 3, .has_ra = 1, .ra_x = 64, .ra_y = 52},
    {.row = {.count = 5, .items = bz_10_items}, .has_score = 1, .score = 4, .has_ra = 1, .ra_x = 64, .ra_y = 61},
    {.row = {.count = 2, .items = bz_11_items}, .has_score = 1, .score = 5, .has_ra = 1, .ra_x = 64, .ra_y = 74},
    {.row = {.count = 0, .items = NULL}, .has_score = 1, .score = 6, .has_ra = 1, .ra_x = 64, .ra_y = 82},
};

static const int jo_ka_items[] = {2, 3, 9, 12};
const pico8_int_seq_t pico8_jo_ka_seq = {4, jo_ka_items};

static const pico8_jb_item_t jo_dz_1_items[] = {{.s = "unhappy"}};
static const pico8_jb_item_t jo_dz_2_items[] = {{.s = "content"}};
static const pico8_jb_item_t jo_dz_3_items[] = {{.s = "prosperous"}};
static const pico8_jb_item_t jo_dz_4_items[] = {{.s = "rich"}};
const pico8_jb_row_t pico8_jo_dz_rows[PICO8_JO_DZ_COUNT] = {
    {.count = 1, .items = jo_dz_1_items, .has_c = 1, .c = 8},
    {.count = 1, .items = jo_dz_2_items, .has_c = 1, .c = 3},
    {.count = 1, .items = jo_dz_3_items, .has_c = 1, .c = 9},
    {.count = 1, .items = jo_dz_4_items, .has_c = 1, .c = 12},
};

static const int jo_cq_items[] = {12, 36, 24, 25};
const pico8_int_seq_t pico8_jo_cq_seq = {4, jo_cq_items};

const pico8_kw_t pico8_jo_kw_start = {0, 0x0, 0x0};

static const pico8_jb_item_t jo_em_items[] = {{.is_false = 1}, {.n = 3}, {.n = 0}, {.n = -1}};
const pico8_jb_row_t pico8_jo_em_row = {.count = 4, .items = jo_em_items};

static const pico8_jb_item_t qh_px_items[] = {{.s = "laboratory"}};
const pico8_jb_row_t pico8_qh_px_row = {.count = 1, .items = qh_px_items, .has_c = 1, .c = 13};

static const pico8_jb_item_t nl_px_items[] = {{.s = "trading hub"}};
const pico8_jb_row_t pico8_nl_px_row = {.count = 1, .items = nl_px_items, .has_c = 1, .c = 13};

static const pico8_jb_item_t os_px_items[] = {{.s = "processor"}};
const pico8_jb_row_t pico8_os_px_row = {.count = 1, .items = os_px_items, .has_c = 1, .c = 13};

static const pico8_jb_item_t hj_px_items[] = {{.s = "ascension nr"}};
const pico8_jb_row_t pico8_hj_px_row = {.count = 1, .items = hj_px_items, .has_c = 1, .c = 13};

static const pico8_jb_item_t pm_px_items[] = {{.s = "protostar"}};
const pico8_jb_row_t pico8_pm_px_row = {.count = 1, .items = pm_px_items, .has_c = 1, .c = 13};

static const pico8_jb_item_t mx_px_items[] = {{.s = "synthesizer"}};
const pico8_jb_row_t pico8_mx_px_row = {.count = 1, .items = mx_px_items, .has_c = 1, .c = 13};

static const int qh_ba_items[] = {28, 18, 20, 21, 22, 48};
const pico8_int_seq_t pico8_qh_ba_seq = {6, qh_ba_items};

static const int nl_ba_items[] = {28, 48, 18, 20, 21, 22};
const pico8_int_seq_t pico8_nl_ba_seq = {6, nl_ba_items};

static const int hj_k__items[] = {1, 13, 12, 12, 12, 13, 13, 5, 1, 1, 1};
const pico8_int_seq_t pico8_hj_k__seq = {11, hj_k__items};

static const int pm_k__items[] = {1, 2, 4, 9, 10, 7};
const pico8_int_seq_t pico8_pm_k__seq = {6, pm_k__items};

static const int mx_k__items[] = {1, 1, 5, 5, 3, 3, 3, 5, 5, 1};
const pico8_int_seq_t pico8_mx_k__seq = {10, mx_k__items};

static const int ga_k__items[] = {1, 2, 4, 9, 9, 10, 10, 7, 7, 7, 7, 7};
const pico8_int_seq_t pico8_ga_k__seq = {12, ga_k__items};

static const int pm_ga_k__items[] = {1, 2, 8, 14, 14, 14, 14, 8, 8, 2, 2, 1, 1};
const pico8_int_seq_t pico8_pm_ga_k__seq = {13, pm_ga_k__items};

static const int nr_hz_items[] = {170, 170, 170, 168};
const pico8_int_seq_t pico8_nr_hz_seq = {4, nr_hz_items};

static const int fr_hz_items[] = {78, 78, 78, 76};
const pico8_int_seq_t pico8_fr_hz_seq = {4, fr_hz_items};

static const int mx_ht_items[] = {28, 20, 21, 22};
const pico8_int_seq_t pico8_mx_ht_seq = {4, mx_ht_items};

static const int g__k__items[] = {1, 1, 5, 13, 6};
const pico8_int_seq_t pico8_g__k__seq = {5, g__k__items};

static const int fv_k__items[] = {0, 1, 1, 2, 4, 9, 10, 9, 4, 2, 1, 1, 1};
const pico8_int_seq_t pico8_fv_k__seq = {13, fv_k__items};

static const int jc_k__items[] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1};
const pico8_int_seq_t pico8_jc_k__seq = {20, jc_k__items};

static const int mz_kk_items[] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
const pico8_int_seq_t pico8_mz_kk_seq = {9, mz_kk_items};

static const int bg_pl_items[] = {0, 0, 0, 72, 73, 74, 75, 88, 89, 90, 91, 88, 91, 89, 91};
const pico8_int_seq_t pico8_bg_pl_seq = {15, bg_pl_items};

static const pico8_vec_t qb_pc_items[] = {{-0x10000, 0x0}, {0x10000, 0x0}, {0x0, -0x10000}, {0x0, 0x10000}};
const pico8_vec_seq_t pico8_qb_pc_vecs = {4, qb_pc_items};

static const pico8_jb_item_t fv_rb_1_items[] = {{.n = 192}, {.n = 0}, {.n = 11}, {.n = 16}, {.n = 4}};
static const pico8_jb_row_t fv_rb_1 = {.count = 5, .items = fv_rb_1_items, .fn = "s"};
static const pico8_jb_item_t fv_rb_2_items[] = {{.n = 172}, {.n = 20}, {.n = 107}, {.n = 2}, {.n = 2}};
static const pico8_jb_row_t fv_rb_2 = {.count = 5, .items = fv_rb_2_items, .fn = "s"};
static const pico8_jb_item_t fv_rb_3_items[] = {{.s = "need help\?"}, {.n = 40}, {.n = 110}, {.n = 13}};
static const pico8_jb_row_t fv_rb_3 = {.count = 4, .items = fv_rb_3_items, .fn = "qr"};
static const pico8_jb_item_t fv_rb_4_items[] = {{.s = "slipways.net/help"}, {.n = 40}, {.n = 116}, {.n = 5}};
static const pico8_jb_row_t fv_rb_4 = {.count = 4, .items = fv_rb_4_items, .fn = "qr"};
static const pico8_jb_item_t fv_rb_items[] = {{.row = &fv_rb_1}, {.row = &fv_rb_2}, {.row = &fv_rb_3},
    {.row = &fv_rb_4}};
const pico8_jb_row_t pico8_fv_rb_row = {.count = 4, .items = fv_rb_items};

static const pico8_jb_item_t hp_fk_items[] = {{.is_false = 1}, {.n = 2}};
const pico8_jb_row_t pico8_hp_fk_row = {.count = 2, .items = hp_fk_items};

static const pico8_jb_item_t pj_gs_1_items[] = {{.s = "last year!"}, {.n = 64}, {.n = 48}, {.n = 9},
    {.is_fixed = 1, .fixed = 0x8000}};
static const pico8_jb_row_t pj_gs_1 = {.count = 5, .items = pj_gs_1_items, .fn = "qr"};
static const pico8_jb_item_t pj_gs_2_items[] = {{.s = "2 years remain"}, {.n = 64}, {.n = 48}, {.n = 2},
    {.is_fixed = 1, .fixed = 0x8000}};
static const pico8_jb_row_t pj_gs_2 = {.count = 5, .items = pj_gs_2_items, .fn = "qr"};
static const pico8_jb_item_t pj_gs_5_items[] = {{.s = "5 years remain"}, {.n = 64}, {.n = 48}, {.n = 2},
    {.is_fixed = 1, .fixed = 0x8000}};
static const pico8_jb_row_t pj_gs_5 = {.count = 5, .items = pj_gs_5_items, .fn = "qr"};
static const pico8_jb_item_t pj_gs_items[] = {{.row = &pj_gs_1}, {.row = &pj_gs_2}, {.is_false = 1}, {.is_false = 1},
    {.row = &pj_gs_5}};
const pico8_jb_row_t pico8_pj_gs_row = {.count = 5, .items = pj_gs_items};

static const pico8_jb_item_t pj_i__1_items[] = {{.n = 126}, {.n = 127}, {.n = 1}};
static const pico8_jb_row_t pj_i__1 = {.count = 3, .items = pj_i__1_items, .fn = "br"};
static const pico8_jb_item_t pj_i__2_items[] = {{.n = 0}, {.n = 1}, {.n = 1}};
static const pico8_jb_row_t pj_i__2 = {.count = 3, .items = pj_i__2_items, .fn = "br"};
static const pico8_jb_item_t pj_i__3_items[] = {{.n = 8}, {.n = 118}, {.n = 34}};
static const pico8_jb_row_t pj_i__3 = {.count = 3, .items = pj_i__3_items, .fn = "rf"};
static const pico8_jb_item_t pj_i__4_items[] = {{.n = 0}, {.n = -3}, {.n = 26}};
static const pico8_jb_row_t pj_i__4 = {.count = 3, .items = pj_i__4_items, .fn = "rf"};
static const pico8_jb_item_t pj_i__5_items[] = {{.n = 81}, {.n = -3}, {.n = 117}};
static const pico8_jb_row_t pj_i__5 = {.count = 3, .items = pj_i__5_items, .fn = "rf"};
static const pico8_jb_item_t pj_i__items[] = {{.row = &pj_i__1}, {.row = &pj_i__2}, {.row = &pj_i__3},
    {.row = &pj_i__4}, {.row = &pj_i__5}};
const pico8_jb_row_t pico8_pj_i__row = {.count = 5, .items = pj_i__items};

static const pico8_jb_item_t pj_ea_1_items[] = {{.n = 42}, {.n = 43}, {.n = 1}};
static const pico8_jb_row_t pj_ea_1 = {.count = 3, .items = pj_ea_1_items, .fn = "br"};
static const pico8_jb_item_t pj_ea_2_items[] = {{.s = ""}, {.n = 64}, {.n = 40}, {.n = 6},
    {.is_fixed = 1, .fixed = 0x8000}};
static const pico8_jb_row_t pj_ea_2 = {.count = 5, .items = pj_ea_2_items, .fn = "qr"};
static const pico8_jb_item_t pj_ea_items[] = {{.row = &pj_ea_1}, {.row = &pj_ea_2}};
const pico8_jb_row_t pico8_pj_ea_row = {.count = 2, .items = pj_ea_items};

static const pico8_jb_item_t pj_ou_1_items[] = {{.is_false = 1}, {.n = 13}, {.n = 121}, {.n = 9}};
static const pico8_jb_row_t pj_ou_1 = {.count = 4, .items = pj_ou_1_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_2_items[] = {{.is_false = 1}, {.n = 37}, {.n = 121}, {.n = 2}};
static const pico8_jb_row_t pj_ou_2 = {.count = 4, .items = pj_ou_2_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_3_items[] = {{.is_false = 1}, {.n = 13}, {.n = 2}, {.n = 9}};
static const pico8_jb_row_t pj_ou_3 = {.count = 4, .items = pj_ou_3_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_4_items[] = {{.is_false = 1}, {.n = 29}, {.n = 2}, {.n = 2}};
static const pico8_jb_row_t pj_ou_4 = {.count = 4, .items = pj_ou_4_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_5_items[] = {{.is_false = 1}, {.n = 59}, {.n = 2}, {.n = 2}};
static const pico8_jb_row_t pj_ou_5 = {.count = 4, .items = pj_ou_5_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_6_items[] = {{.is_false = 1}, {.n = 85}, {.n = 2}, {.n = 13}};
static const pico8_jb_row_t pj_ou_6 = {.count = 4, .items = pj_ou_6_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_7_items[] = {{.is_false = 1}, {.n = 99}, {.n = 2}, {.n = 13}};
static const pico8_jb_row_t pj_ou_7 = {.count = 4, .items = pj_ou_7_items, .fn = "qr"};
static const pico8_jb_item_t pj_ou_items[] = {{.row = &pj_ou_1}, {.row = &pj_ou_2}, {.row = &pj_ou_3},
    {.row = &pj_ou_4}, {.row = &pj_ou_5}, {.row = &pj_ou_6}, {.row = &pj_ou_7}};
const pico8_jb_row_t pico8_pj_ou_row = {.count = 7, .items = pj_ou_items};

const pico8_pj_cr_start_t pico8_pj_cr_start = {
    .hi = {0x0, 0xb0000},
    .gq = {0x0, 0xb0000},
    .ca = 0xa0000,
};

const char pico8_mz_jj_qs[] = "-too far-";

const pico8_ke_start_t pico8_ke_start = {
    .me = "ph",
    .t = 0x0,
};

const pico8_bg_start_t pico8_bg_start = {
    .fa = 0x10000,
    .hd = 0x10000,
    .pl = &pico8_bg_pl_seq,
};

const pico8_ui_start_t pico8_ui_start = {
    .ql = 0x3a0000,
    .fa = 0xf0000,
    .hw = {0xa0000, 0xa0000, 0x760000, 0x760000},
};

const pico8_js_start_t pico8_js_start = {
    .fa = 0xb0000,
};

const pico8_fq_start_t pico8_fq_start = {
    .fa = 0xe0000,
    .hd = 0x10000,
};

const pico8_qb_start_t pico8_qb_start = {
    .v = {0x0, 0x0},
    .ma = 0x0,
    .p = {0xc00000, 0xc00000},
    .ra = {0xc00000, 0xc00000},
    .pc = &pico8_qb_pc_vecs,
};

const pico8_nf_start_t pico8_nf_start = {
    .ra = {0x0, 0x0},
};

const pico8_mh_start_t pico8_mh_start = {
    .fa = 0xe0000,
    .rh = 0x50000,
};

const pico8_iv_start_t pico8_iv_start = {
    .fa = 0x90000,
    .l = 0x0,
    .v = {0x0, -0x570a},
};

const pico8_mz_start_t pico8_mz_start = {
    .me = "kz",
    .dw = 0x0,
    .dq = "pr",
    .spawn_snd = 0x340000,
    .ik = 0x310000,
    .oi = 0x0,
    .od = 0x30000,
    .fp = 0x10000,
    .fa = 0x50000,
    .kk = &pico8_mz_kk_seq,
};

const pico8_jc_start_t pico8_jc_start = {
    .me = "kz",
    .fa = 0x20000,
    .fp = 0,
    .ik = 0x300000,
    .dw = 0x106,
    .k_ = &pico8_jc_k__seq,
    .nh = 0x140000,
};

const pico8_g__start_t pico8_g__start = {
    .fx = 0x10000,
    .dq = "ly",
    .od = 0x0,
    .fp = 0,
    .fa = 0x20000,
    .dw = 0x666,
    .py = 0xf5c,
    .ki = 0x90000,
    .pn = 0x30000,
    .pk = 0x10000,
    .fc = 0x10000,
    .k_ = &pico8_g__k__seq,
    .qo = 0x1051e,
    .ro = -0xd70,
    .qg = 0xd70,
    .q_ = 0x1051e,
    .du = 0x20000,
};

const pico8_fv_start_t pico8_fv_start = {
    .fa = 0xa0000,
    .ra = {0x400000, 0x1c0000},
    .hi = {0x220000, 0x380000},
    .ca = 0xb0000,
    .gq = {0x0, 0xb0000},
    .eo = -0x10000,
    .ff = 0x10000,
    .fc = 0x50000,
    .k_ = &pico8_fv_k__seq,
    .qo = 0x1051e,
    .ro = -0x1ca,
    .qg = 0x1ca,
    .q_ = 0x1051e,
    .pn = 0x50000,
    .du = 0xf0000,
    .py = 0x1b6,
    .pk = 0x10000,
    .ki = 0x110000,
    .kq = {-0x280000, 0x4e0000, 0x280000, 0x620000},
    .ql = 0xb0000,
    .fg = 0x50000,
    .rb = &pico8_fv_rb_row,
};

const pico8_hp_start_t pico8_hp_start = {
    .fk = &pico8_hp_fk_row,
};

const pico8_ie_start_t pico8_ie_start = {
    .kq = {0x0, 0x0, 0x90000, 0x80000},
    .fa = 0xc0000,
    .id = 0x0,
    .ra = {0x3e70000, 0x3e70000},
    .eh = {0x760000, 0x780000},
};

const pico8_pj_start_t pico8_pj_start = {
    .fa = 0xa0000,
    .ea = &pico8_pj_ea_row,
    .gs = &pico8_pj_gs_row,
    .cr = &pico8_pj_cr_start,
    .i_ = &pico8_pj_i__row,
    .ou = &pico8_pj_ou_row,
    .ed = 0x10000,
};

static const char jo_me_text[] = "unknown";
static const char qh_me_text[] = "kz";
static const char qh_dq_text[] = "ly";
static const char nl_rr_text[] = "trade league";
static const char hj_rr_text[] = "ascension";
static const char pm_fz_text[] = "pm";
static const char pm_me_text[] = "ly";
static const char ga_fz_text[] = "ga";
static const char nr_rr_text[] = "slipgates";
static const char fr_rr_text[] = "infraspace";
static const char mx_rr_text[] = "void synthesis";

const pico8_jo_start_t pico8_jo_start = {
    .me = jo_me_text,
    .hf = 1,
    .hs = 0x10000,
    .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0,
    .ka = &pico8_jo_ka_seq,
    .dz = pico8_jo_dz_rows,
    .cq = &pico8_jo_cq_seq,
    .kw = &pico8_jo_kw_start,
    .gq = {0x0, 0xb0000},
    .ca = 0x90000,
    .em = &pico8_jo_em_row,
};

const pico8_qh_start_t pico8_qh_start = {
    .hp = 0x10000,
    .px = &pico8_qh_px_row,
    .me = qh_me_text,
    .dq = qh_dq_text,
    .hf = 0,
    .qh = 1,
    .od = 0xf0000,
    .fp = 0x20000,
    .dw = 0x666,
    .mi = 0x10000,
    .s = 0xae0000,
    .ba = &pico8_qh_ba_seq,
};

const pico8_nl_start_t pico8_nl_start = {
    .rr = nl_rr_text,
    .qh = 0,
    .nl = 0x10000,
    .hp = 0x60000,
    .s = 0x440000,
    .px = &pico8_nl_px_row,
    .od = 0x140000,
    .qw = 0xf0000,
    .ba = &pico8_nl_ba_seq,
};

const pico8_os_start_t pico8_os_start = {
    .hp = 0x40000,
    .px = &pico8_os_px_row,
    .qh = 0,
    .s = 0x640000,
    .od = 0xf0000,
    .hs = 0x8000,
};

const pico8_hj_start_t pico8_hj_start = {
    .rr = hj_rr_text,
    .hp = 0x20000,
    .qh = 0,
    .px = &pico8_hj_px_row,
    .od = 0x3c0000,
    .s = 0xa60000,
    .py = 0x1eb8,
    .ki = 0x90000,
    .pn = 0x50000,
    .pk = 0x10000,
    .fc = 0x8000,
    .k_ = &pico8_hj_k__seq,
    .du = 0x78000,
    .qo = 0xfae0,
    .ro = -0x11ec,
    .qg = 0x11ea,
    .q_ = 0xfae0,
};

const pico8_pm_start_t pico8_pm_start = {
    .fz = pm_fz_text,
    .hp = 0x30000,
    .px = &pico8_pm_px_row,
    .me = pm_me_text,
    .s = 0x620000,
    .py = 0xf5c,
    .ki = 0x90000,
    .pn = 0x80000,
    .pk = 0,
    .ng = {0x10000, 0x0},
    .du = 0x20000,
    .fc = 0x10000,
    .k_ = &pico8_pm_k__seq,
    .qo = 0x1051e,
    .ro = -0xd70,
    .qg = 0xd70,
    .q_ = 0x1051e,
};

const pico8_ga_start_t pico8_ga_start = {
    .fz = ga_fz_text,
    .hf = 0,
    .kq = {0x0, 0x0, -0x10000, -0x10000},
    .fa = 0x30000,
    .fc = 0x0,
    .qo = 0xe666,
    .ro = 0x0,
    .qg = 0x0,
    .q_ = 0xe666,
    .k_ = &pico8_ga_k__seq,
    .iu = -0xf5c,
    .py = 0x60000,
    .ms = 0x30000,
};

const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys = {
    .qo = 0xf850,
    .ro = -0x2020,
    .qg = 0x2020,
    .q_ = 0xf850,
    .iu = -0x24e,
    .py = 0x90000,
    .ms = 0x18000,
    .k_ = &pico8_pm_ga_k__seq,
};

const pico8_nr_start_t pico8_nr_start = {
    .od = 0xa0000,
    .fp = 0x10000,
    .kq = {-0x40000, -0x40000, 0x50000, 0x50000},
    .s = 0xaa0000,
    .hz = &pico8_nr_hz_seq,
    .dw = 0x28e,
    .rr = nr_rr_text,
};

const pico8_fr_start_t pico8_fr_start = {
    .mt = 0x10000,
    .rr = fr_rr_text,
    .od = 0x190000,
    .s = 0x4e0000,
    .hz = &pico8_fr_hz_seq,
};

const pico8_mx_start_t pico8_mx_start = {
    .rr = mx_rr_text,
    .hp = 0x50000,
    .s = 0x420000,
    .px = &pico8_mx_px_row,
    .od = 0x280000,
    .ht = &pico8_mx_ht_seq,
    .py = 0x1eb8,
    .ki = 0x90000,
    .pn = 0x50000,
    .pk = 0x10000,
    .fc = 0x4ccc,
    .k_ = &pico8_mx_k__seq,
    .du = 0x50000,
    .qo = 0xfae0,
    .ro = -0x11ec,
    .qg = 0x11ea,
    .q_ = 0xfae0,
};

const pico8_kind_rec_t pico8_qh_kind_rec = {NULL, 1, 0xae0000, qh_me_text, 1, 0xf0000};
const pico8_kind_rec_t pico8_nl_kind_rec = {nl_rr_text, 1, 0x440000, qh_me_text, 1, 0x140000};
const pico8_kind_rec_t pico8_os_kind_rec = {NULL, 1, 0x640000, qh_me_text, 1, 0xf0000};
const pico8_kind_rec_t pico8_hj_kind_rec = {hj_rr_text, 1, 0xa60000, qh_me_text, 1, 0x3c0000};
const pico8_kind_rec_t pico8_pm_kind_rec = {hj_rr_text, 1, 0x620000, pm_me_text, 1, 0x3c0000};
const pico8_kind_rec_t pico8_ga_kind_rec = {NULL, 0, 0x0, jo_me_text, 0, 0x0};
const pico8_kind_rec_t pico8_nr_kind_rec = {nr_rr_text, 1, 0xaa0000, qh_me_text, 1, 0xa0000};
const pico8_kind_rec_t pico8_fr_kind_rec = {fr_rr_text, 1, 0x4e0000, qh_me_text, 1, 0x190000};
const pico8_kind_rec_t pico8_mx_kind_rec = {mx_rr_text, 1, 0x420000, qh_me_text, 1, 0x280000};

int pico8_int_seq_at(const int *seq, int count, int i, int *out) {
    assert(seq != NULL && count >= 0 && "a sequence is a table and its length");
    if (i < 1 || i > count) {
        return 0;
    }
    *out = seq[i - 1];
    return 1;
}

int pico8_fixed_seq_at(const pico8_fixed_t *seq, int count, int i, pico8_fixed_t *out) {
    assert(seq != NULL && count >= 0 && "a sequence is a table and its length");
    if (i < 1 || i > count) {
        return 0;
    }
    *out = seq[i - 1];
    return 1;
}

int pico8_int_seq_has_fixed(const pico8_int_seq_t *seq, pico8_fixed_t v) {
    int i;

    for (i = 0; i < seq->count; i++) {
        if (pico8_fixed_from_int(seq->items[i]) == v) {
            return 1;
        }
    }
    return 0;
}

const pico8_df_entry_t *pico8_df_lookup(const char *px) {
    int i;

    if (px == NULL) {
        return NULL;
    }
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        if (strcmp(pico8_df_table[i].px, px) == 0) {
            return &pico8_df_table[i];
        }
    }
    return NULL;
}

pico8_fixed_t pico8_bn(void) {
    return pico8_fixed_from_int(6);
}

pico8_fixed_t pico8_bq(void) {
    return PICO8_FIXED_LIT(0x00001902, 0.0977);
}
