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

static const char *const jo_nu_items[] = {"jo", "np", "prober"};
const pico8_str_seq_t pico8_jo_nu_tags = {3, jo_nu_items};

static const char *const qh_nu_items[] = {"qh", "jo", "np", "prober"};
const pico8_str_seq_t pico8_qh_nu_tags = {4, qh_nu_items};

static const char *const nl_nu_items[] = {"jo", "np", "prober"};
const pico8_str_seq_t pico8_nl_nu_tags = {3, nl_nu_items};

static const char *const os_nu_items[] = {"jo", "np", "prober"};
const pico8_str_seq_t pico8_os_nu_tags = {3, os_nu_items};

static const char *const hj_nu_items[] = {"hj", "jo", "np", "prober"};
const pico8_str_seq_t pico8_hj_nu_tags = {4, hj_nu_items};

static const char *const pm_nu_items[] = {"jo", "np", "prober"};
const pico8_str_seq_t pico8_pm_nu_tags = {3, pm_nu_items};

const pico8_str_seq_t pico8_ga_nu_tags = {0, NULL};

static const char *const nr_nu_items[] = {"nr", "np", "prober"};
const pico8_str_seq_t pico8_nr_nu_tags = {3, nr_nu_items};

static const char *const mx_nu_items[] = {"jo", "np", "prober"};
const pico8_str_seq_t pico8_mx_nu_tags = {3, mx_nu_items};

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

static const char *const mh_nu_items[] = {"mh"};
const pico8_str_seq_t pico8_mh_nu_tags = {1, mh_nu_items};

static const char *const g__nu_items[] = {"g_", "prober"};
const pico8_str_seq_t pico8_g__nu_tags = {2, g__nu_items};

static const int g__k__items[] = {1, 1, 5, 13, 6};
const pico8_int_seq_t pico8_g__k__seq = {5, g__k__items};

static const int fv_k__items[] = {0, 1, 1, 2, 4, 9, 10, 9, 4, 2, 1, 1, 1};
const pico8_int_seq_t pico8_fv_k__seq = {13, fv_k__items};

static const char *const jc_nu_items[] = {"jc"};
const pico8_str_seq_t pico8_jc_nu_tags = {1, jc_nu_items};

static const int jc_k__items[] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1};
const pico8_int_seq_t pico8_jc_k__seq = {20, jc_k__items};

static const char *const mz_nu_items[] = {"mz"};
const pico8_str_seq_t pico8_mz_nu_tags = {1, mz_nu_items};

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

static const char *const hp_mr_items[] = {"px", "lr", "fj", "ja", "i", "o", "il", "dm"};
const pico8_str_seq_t pico8_hp_mr_names = {8, hp_mr_items};

static const char *const hp_gp_items[] = {"o", "dm"};
const pico8_str_seq_t pico8_hp_gp_names = {2, hp_gp_items};

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
