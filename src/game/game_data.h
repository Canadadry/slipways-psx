#ifndef PICO8_GAME_DATA_H
#define PICO8_GAME_DATA_H

#include "p8/fixed.h"
#include "engine/geom.h"
#include "game/map_tables.h"
#include "engine/vec.h"

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
    const vec_t *items;
} pico8_vec_seq_t;

/* A kind's `rr`, `s`, `me`, `od` as its chain of defaults resolves them: a
 * NULL string, or has_* 0, when absent. The strings are the ones its
 * archetype points at. */
typedef struct pico8_kind_rec {
    const char *rr;
    int has_s;
    p8_fixed_t s;
    const char *me;
    int has_od;
    p8_fixed_t od;
} pico8_kind_rec_t;

/* qe is only meaningful when has_qe is set. */
typedef struct {
    int has_qe;
    p8_fixed_t qe;
    p8_fixed_t mu;
} pico8_kw_t;

/* A pico8_fixed_t below holds the 16.16 bits ob() returned for the cart's
 * literal, 0xffff.fffe mask included: -0x5540 is -0.333. */

/* d is NULL for a record of 3 positionals, me NULL when absent. */
typedef struct {
    const char *px;
    int s;
    p8_fixed_t r;
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
    p8_fixed_t en;
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

/* A class's archetype: its defaults resolved along its init chain, ke's
 * `me`/`t` first. A default that is the address of a module's static object
 * (ui.le, nf.ra, mz.jj) is not here: the module's init still sets it. */
typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    p8_fixed_t hd;
    const pico8_int_seq_t *pl;
} pico8_bg_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t ql;
    p8_fixed_t fa;
    box_t hw;
} pico8_ui_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
} pico8_js_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    p8_fixed_t hd;
} pico8_fq_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    vec_t v;
    p8_fixed_t ma;
    vec_t p;
    vec_t ra;
    const pico8_vec_seq_t *pc;
} pico8_qb_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
} pico8_nf_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    p8_fixed_t rh;
} pico8_mh_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    p8_fixed_t l;
    vec_t v;
} pico8_iv_def_t;

typedef struct {
    const pico8_jb_row_t *fk;
} pico8_hp_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    box_t kq;
    p8_fixed_t fa;
    vec_t ra;
    vec_t eh;
} pico8_ie_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    const pico8_jb_row_t *gs;
    const pico8_jb_row_t *i_;
} pico8_pj_def_t;

/* `dq` NULL is nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is
 * held as 0, which is what `self.fp or 0` hands. */
typedef struct {
    p8_fixed_t dw;
    p8_fixed_t od;
    int has_od;
    const char *dq;
    p8_fixed_t fp;
    p8_fixed_t ik;
    int has_ik;
} pico8_probe_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t spawn_snd;
    p8_fixed_t oi;
    p8_fixed_t fa;
    const pico8_int_seq_t *kk;
    pico8_probe_def_t probe;
} pico8_mz_def_t;

typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fa;
    const pico8_int_seq_t *k_;
    p8_fixed_t nh;
    pico8_probe_def_t probe;
} pico8_jc_def_t;

/* `pk` is the cart's 1 or false, false held as 0: only its truth is read. */
typedef struct {
    p8_fixed_t pn;
    p8_fixed_t du;
    p8_fixed_t py;
    p8_fixed_t ki;
    p8_fixed_t pk;
    p8_fixed_t fc;
    const pico8_int_seq_t *k_;
    p8_fixed_t qo;
    p8_fixed_t ro;
    p8_fixed_t qg;
    p8_fixed_t q_;
} pico8_emitter_def_t;

/* g_ and fv share this type; fv's own members are zero in g_'s. */
typedef struct {
    const char *me;
    p8_fixed_t t;
    p8_fixed_t fx;
    p8_fixed_t fa;
    vec_t ra;
    int has_ra; /* 0: the instance's `ra` is NULL */
    vec_t hi;
    p8_fixed_t ca;
    vec_t gq;
    p8_fixed_t eo;
    p8_fixed_t ff;
    box_t kq;
    p8_fixed_t ql;
    p8_fixed_t fg;
    const pico8_jb_row_t *rb;
    pico8_emitter_def_t emitter;
    pico8_probe_def_t probe;
} pico8_g__def_t;

/* The ten planet kinds share this type; a kind's value leaves at zero the
 * members of the kinds it does not descend from. `kw`, and ga's `by`, point
 * at objects their module owns, and `kw_own` only holds what `lz` writes:
 * not here. */
typedef struct {
    const char *me;
    p8_fixed_t t;
    int hf;
    p8_fixed_t hs;
    p8_fixed_t fa;
    box_t kq;
    p8_fixed_t ma;
    const pico8_int_seq_t *ka;
    const pico8_jb_row_t *dz; /* PICO8_JO_DZ_COUNT rows */
    const pico8_int_seq_t *cq;
    vec_t gq;
    p8_fixed_t ca;
    const pico8_jb_row_t *em;
    p8_fixed_t mi;
    int has_mi;
    p8_fixed_t hp;
    const pico8_jb_row_t *px;
    int qh;
    p8_fixed_t s;
    const pico8_int_seq_t *ba;
    const char *rr;
    p8_fixed_t nl;
    p8_fixed_t qw;
    vec_t ng;
    const char *fz;
    p8_fixed_t iu;
    p8_fixed_t ms;
    const pico8_int_seq_t *hz;
    p8_fixed_t mt;
    const pico8_int_seq_t *ht;
    pico8_probe_def_t probe;
    pico8_emitter_def_t emitter;
} pico8_jo_def_t;

/* The keys pm:pq passes to the ga it spawns, besides `ra`. */
typedef struct {
    p8_fixed_t qo;
    p8_fixed_t ro;
    p8_fixed_t qg;
    p8_fixed_t q_;
    p8_fixed_t iu;
    p8_fixed_t py;
    p8_fixed_t ms;
    const pico8_int_seq_t *k_;
} pico8_ga_pq_keys_t;

extern const int pico8_es_table[PICO8_ES_COUNT];
extern const p8_fixed_t pico8_ct_table[PICO8_CT_COUNT];
extern const int pico8_b__table[PICO8_B__COUNT];
extern const int pico8_co_table[PICO8_CO_COUNT];
extern const int pico8_fb_table[PICO8_FB_COUNT];
extern const int pico8_ek_table[PICO8_EK_COUNT];
extern const p8_fixed_t pico8_ci_table[PICO8_CI_COUNT];
extern const char *const pico8_ef_table[PICO8_EF_COUNT];
extern const pico8_df_entry_t pico8_df_table[PICO8_DF_COUNT];
extern const pico8_ix_entry_t pico8_ix_table[PICO8_IX_COUNT];
extern const pico8_ha_entry_t pico8_ha_table[PICO8_HA_COUNT];
extern const pico8_ha_tail_t pico8_ha_tail;
extern const pico8_bo_entry_t pico8_bo_table[PICO8_BO_COUNT];
extern const pico8_bz_entry_t pico8_bz_table[PICO8_BZ_COUNT];

/* The table members of the jo tree, one object per class that sets it. */
extern const pico8_int_seq_t pico8_jo_ka_seq;
extern const pico8_jb_row_t pico8_jo_dz_rows[PICO8_JO_DZ_COUNT];
extern const pico8_int_seq_t pico8_jo_cq_seq;
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
extern const pico8_int_seq_t pico8_g__k__seq;
extern const pico8_int_seq_t pico8_fv_k__seq;
extern const pico8_int_seq_t pico8_jc_k__seq;
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

/* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
 * nf.r are empty tables, held by their module. */
extern const pico8_ga_pq_keys_t pico8_pm_ga_pq_keys;
extern const pico8_bg_def_t pico8_bg_def;
extern const pico8_ui_def_t pico8_ui_def;
extern const pico8_js_def_t pico8_js_def;
extern const pico8_fq_def_t pico8_fq_def;
extern const pico8_qb_def_t pico8_qb_def;
extern const pico8_nf_def_t pico8_nf_def;
extern const pico8_mh_def_t pico8_mh_def;
extern const pico8_iv_def_t pico8_iv_def;
extern const pico8_hp_def_t pico8_hp_def;
extern const pico8_ie_def_t pico8_ie_def;
extern const pico8_pj_def_t pico8_pj_def;
extern const pico8_mz_def_t pico8_mz_def;
extern const pico8_jc_def_t pico8_jc_def;
/* The probe of research_icon's `pb`, which has no class: every field 0. */
extern const pico8_probe_def_t pico8_probe_def_none;
extern const pico8_g__def_t pico8_g__def;
extern const pico8_g__def_t pico8_fv_def;
extern const pico8_jo_def_t pico8_jo_def;
extern const pico8_jo_def_t pico8_qh_def;
extern const pico8_jo_def_t pico8_nl_def;
extern const pico8_jo_def_t pico8_os_def;
extern const pico8_jo_def_t pico8_hj_def;
extern const pico8_jo_def_t pico8_pm_def;
extern const pico8_jo_def_t pico8_mx_def;
extern const pico8_jo_def_t pico8_ga_def;
extern const pico8_jo_def_t pico8_nr_def;
extern const pico8_jo_def_t pico8_fr_def;
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
int pico8_fixed_seq_at(const p8_fixed_t *seq, int count, int i, p8_fixed_t *out);

/* 1 when v is pico8_fixed_from_int of an element: the cart's `gu` on integers. */
int pico8_int_seq_has_fixed(const pico8_int_seq_t *seq, p8_fixed_t v);

/* The df entry whose px is `px`, NULL when none or `px` is NULL: `df[px]`
 * after the px/s/r/d pass. */
const pico8_df_entry_t *pico8_df_lookup(const char *px);

/* Bare Lua number literals rather than ob() spec fields, hence scalars
 * here and not another table. */
p8_fixed_t pico8_bn(void);
p8_fixed_t pico8_bq(void);

#endif
