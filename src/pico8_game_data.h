#ifndef PICO8_GAME_DATA_H
#define PICO8_GAME_DATA_H

#include "pico8_fixed.h"
#include "pico8_geom.h"
#include "pico8_map_tables.h"
#include "pico8_vec.h"

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

typedef struct pico8_str_seq {
    int count;
    const char *const *items;
} pico8_str_seq_t;

typedef struct {
    int count;
    const pico8_vec_t *items;
} pico8_vec_seq_t;

/* A kind's `rr`, `s`, `me`, `od` as its chain of defaults resolves them: a
 * NULL string, or has_* 0, when absent. The strings are the ones the
 * pico8_X_start of the class that sets each points at. */
typedef struct pico8_kind_rec {
    const char *rr;
    int has_s;
    pico8_fixed_t s;
    const char *me;
    int has_od;
    pico8_fixed_t od;
} pico8_kind_rec_t;

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

/* The values a new instance of a class starts with: exactly the keys of the
 * class's spec, ke's left out, under the struct's own member types. */
typedef struct {
    const char *me;
    pico8_fixed_t t;
} pico8_ke_start_t;

typedef struct {
    pico8_fixed_t fa;
    pico8_fixed_t hd;
    const pico8_int_seq_t *pl;
} pico8_bg_start_t;

typedef struct {
    pico8_fixed_t ql;
    pico8_fixed_t fa;
    pico8_qz_box_t hw;
} pico8_ui_start_t;

typedef struct {
    pico8_fixed_t fa;
} pico8_js_start_t;

typedef struct {
    pico8_fixed_t fa;
    pico8_fixed_t hd;
} pico8_fq_start_t;

typedef struct {
    pico8_vec_t v;
    pico8_fixed_t ma;
    pico8_vec_t p;
    pico8_vec_t ra;
    const pico8_vec_seq_t *pc;
} pico8_qb_start_t;

typedef struct {
    pico8_vec_t ra;
} pico8_nf_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    pico8_fixed_t fa;
    pico8_fixed_t rh;
} pico8_mh_start_t;

typedef struct {
    pico8_fixed_t fa;
    pico8_fixed_t l;
    pico8_vec_t v;
} pico8_iv_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    const char *me;
    pico8_fixed_t dw;
    const char *dq;
    pico8_fixed_t spawn_snd;
    pico8_fixed_t ik;
    pico8_fixed_t oi;
    pico8_fixed_t od;
    pico8_fixed_t fp;
    pico8_fixed_t fa;
    const pico8_int_seq_t *kk;
} pico8_mz_start_t;

/* fp is the spec's false. */
typedef struct {
    const pico8_str_seq_t *nu;
    const char *me;
    pico8_fixed_t fa;
    int fp;
    pico8_fixed_t ik;
    pico8_fixed_t dw;
    const pico8_int_seq_t *k_;
    pico8_fixed_t nh;
} pico8_jc_start_t;

/* fp is the spec's false. */
typedef struct {
    const pico8_str_seq_t *nu;
    pico8_fixed_t fx;
    const char *dq;
    pico8_fixed_t od;
    int fp;
    pico8_fixed_t fa;
    pico8_fixed_t dw;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pn;
    pico8_fixed_t pk;
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_fixed_t du;
} pico8_g__start_t;

/* fv's own keys only: the others are g_'s. */
typedef struct {
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_vec_t hi;
    pico8_fixed_t ca;
    pico8_vec_t gq;
    pico8_fixed_t eo;
    pico8_fixed_t ff;
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_fixed_t pn;
    pico8_fixed_t du;
    pico8_fixed_t py;
    pico8_fixed_t pk;
    pico8_fixed_t ki;
    pico8_qz_box_t kq;
    pico8_fixed_t ql;
    pico8_fixed_t fg;
    const pico8_jb_row_t *rb;
} pico8_fv_start_t;

typedef struct {
    const pico8_jb_row_t *fk;
} pico8_hp_start_t;

typedef struct {
    pico8_qz_box_t kq;
    pico8_fixed_t fa;
    pico8_fixed_t id;
    pico8_vec_t ra;
    pico8_vec_t eh;
} pico8_ie_start_t;

typedef struct {
    pico8_vec_t hi;
    pico8_vec_t gq;
    pico8_fixed_t ca;
} pico8_pj_cr_start_t;

typedef struct {
    pico8_fixed_t fa;
    const pico8_jb_row_t *ea;
    const pico8_jb_row_t *gs;
    const pico8_pj_cr_start_t *cr;
    const pico8_jb_row_t *i_;
    const pico8_jb_row_t *ou;
    pico8_fixed_t ed;
} pico8_pj_start_t;

/* The jo tree: each class's own keys only, the inherited ones are its parent's;
 * ga.by, an empty state list, is held by its module. */
typedef struct {
    const pico8_str_seq_t *nu;
    const char *me;
    int hf;
    pico8_fixed_t hs;
    pico8_fixed_t fa;
    pico8_qz_box_t kq;
    pico8_fixed_t ma;
    const pico8_int_seq_t *ka;
    const pico8_jb_row_t *dz;
    const pico8_int_seq_t *cq;
    const pico8_kw_t *kw;
    pico8_vec_t gq;
    pico8_fixed_t ca;
    const pico8_jb_row_t *em;
} pico8_jo_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    pico8_fixed_t hp;
    const pico8_jb_row_t *px;
    const char *me;
    const char *dq;
    int hf;
    int qh;
    pico8_fixed_t od;
    pico8_fixed_t fp;
    pico8_fixed_t dw;
    pico8_fixed_t mi;
    pico8_fixed_t s;
    const pico8_int_seq_t *ba;
} pico8_qh_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    const char *rr;
    int qh;
    pico8_fixed_t nl;
    pico8_fixed_t hp;
    pico8_fixed_t s;
    const pico8_jb_row_t *px;
    pico8_fixed_t od;
    pico8_fixed_t qw;
    const pico8_int_seq_t *ba;
} pico8_nl_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    pico8_fixed_t hp;
    const pico8_jb_row_t *px;
    int qh;
    pico8_fixed_t s;
    pico8_fixed_t od;
    pico8_fixed_t hs;
} pico8_os_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    const char *rr;
    pico8_fixed_t hp;
    int qh;
    const pico8_jb_row_t *px;
    pico8_fixed_t od;
    pico8_fixed_t s;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pn;
    pico8_fixed_t pk;
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t du;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
} pico8_hj_start_t;

typedef struct {
    const char *fz;
    const pico8_str_seq_t *nu;
    pico8_fixed_t hp;
    const pico8_jb_row_t *px;
    const char *me;
    pico8_fixed_t s;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pn;
    pico8_fixed_t pk;
    pico8_vec_t ng;
    pico8_fixed_t du;
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
} pico8_pm_start_t;

typedef struct {
    const char *fz;
    const pico8_str_seq_t *nu;
    int hf;
    pico8_qz_box_t kq;
    pico8_fixed_t fa;
    pico8_fixed_t fc;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    const pico8_int_seq_t *k_;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_fixed_t ms;
} pico8_ga_start_t;

/* The keys pm:pq passes to the ga it spawns, besides `ra`. */
typedef struct {
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_fixed_t ms;
    const pico8_int_seq_t *k_;
} pico8_ga_pq_keys_t;

typedef struct {
    const pico8_str_seq_t *nu;
    pico8_fixed_t od;
    pico8_fixed_t fp;
    pico8_qz_box_t kq;
    pico8_fixed_t s;
    const pico8_int_seq_t *hz;
    pico8_fixed_t dw;
    const char *rr;
} pico8_nr_start_t;

typedef struct {
    pico8_fixed_t mt;
    const char *rr;
    pico8_fixed_t od;
    pico8_fixed_t s;
    const pico8_int_seq_t *hz;
} pico8_fr_start_t;

typedef struct {
    const pico8_str_seq_t *nu;
    const char *rr;
    pico8_fixed_t hp;
    pico8_fixed_t s;
    const pico8_jb_row_t *px;
    pico8_fixed_t od;
    const pico8_int_seq_t *ht;
    pico8_fixed_t py;
    pico8_fixed_t ki;
    pico8_fixed_t pn;
    pico8_fixed_t pk;
    pico8_fixed_t fc;
    const pico8_int_seq_t *k_;
    pico8_fixed_t du;
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
} pico8_mx_start_t;

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

/* The table members of the other classes, one object per class that sets it. */
extern const pico8_str_seq_t pico8_mh_nu_tags;
/* g_'s nu is fv's too: fv never sets its own. */
extern const pico8_str_seq_t pico8_g__nu_tags;
extern const pico8_int_seq_t pico8_g__k__seq;
extern const pico8_int_seq_t pico8_fv_k__seq;
extern const pico8_str_seq_t pico8_jc_nu_tags;
extern const pico8_int_seq_t pico8_jc_k__seq;
extern const pico8_str_seq_t pico8_mz_nu_tags;
extern const pico8_int_seq_t pico8_mz_kk_seq;
extern const pico8_int_seq_t pico8_bg_pl_seq;
extern const pico8_vec_seq_t pico8_qb_pc_vecs;
extern const pico8_jb_row_t pico8_hp_fk_row;
/* Draw rows: each item a row of positionals then its fn; gs has two false. */
extern const pico8_jb_row_t pico8_fv_rb_row;
extern const pico8_jb_row_t pico8_pj_gs_row;
extern const pico8_jb_row_t pico8_pj_i__row;
/* pj's ea and ou: the state tables pico8_score_panel_init() builds from them. */
extern const pico8_jb_row_t pico8_pj_ea_row;
extern const pico8_jb_row_t pico8_pj_ou_row;
extern const pico8_pj_cr_start_t pico8_pj_cr_start;

/* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
 * nf.r are empty tables, held by their module. */
extern const pico8_ke_start_t pico8_ke_start;
extern const pico8_bg_start_t pico8_bg_start;
extern const pico8_ui_start_t pico8_ui_start;
extern const pico8_js_start_t pico8_js_start;
extern const pico8_fq_start_t pico8_fq_start;
extern const pico8_qb_start_t pico8_qb_start;
extern const pico8_nf_start_t pico8_nf_start;
extern const pico8_mh_start_t pico8_mh_start;
extern const pico8_iv_start_t pico8_iv_start;
extern const pico8_mz_start_t pico8_mz_start;
extern const pico8_jc_start_t pico8_jc_start;
extern const pico8_g__start_t pico8_g__start;
extern const pico8_fv_start_t pico8_fv_start;
extern const pico8_hp_start_t pico8_hp_start;
/* ie's class keys, and the kq, ra, eh every icon points at. */
extern const pico8_ie_start_t pico8_ie_start;
extern const pico8_pj_start_t pico8_pj_start;
extern const pico8_jo_start_t pico8_jo_start;
extern const pico8_qh_start_t pico8_qh_start;
extern const pico8_nl_start_t pico8_nl_start;
extern const pico8_os_start_t pico8_os_start;
extern const pico8_hj_start_t pico8_hj_start;
extern const pico8_pm_start_t pico8_pm_start;
extern const pico8_ga_start_t pico8_ga_start;
extern const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys;
extern const pico8_nr_start_t pico8_nr_start;
extern const pico8_fr_start_t pico8_fr_start;
extern const pico8_mx_start_t pico8_mx_start;
extern const pico8_kind_rec_t pico8_qh_kind_rec;
extern const pico8_kind_rec_t pico8_nl_kind_rec;
extern const pico8_kind_rec_t pico8_os_kind_rec;
extern const pico8_kind_rec_t pico8_hj_kind_rec;
extern const pico8_kind_rec_t pico8_pm_kind_rec;
extern const pico8_kind_rec_t pico8_ga_kind_rec;
extern const pico8_kind_rec_t pico8_nr_kind_rec;
extern const pico8_kind_rec_t pico8_fr_kind_rec;
extern const pico8_kind_rec_t pico8_mx_kind_rec;
/* mz.jj's qs: the table jj=o(qs=...) of mz's spec. */
extern const char pico8_mz_jj_qs[];

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
