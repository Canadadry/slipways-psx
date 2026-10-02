#include "game/map_tables.h"

#include <assert.h>
#include <stdio.h>

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "engine/frame_zone.h"

static void check_int_seq(const int *seq, int count) {
    int out = -12345;
    int i;

    for (i = 0; i < count; i++) {
        assert(pico8_int_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!pico8_int_seq_at(seq, count, 0, &out));
    assert(!pico8_int_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_fixed_seq(const p8_fixed_t *seq, int count) {
    p8_fixed_t out = -12345;
    int i;

    for (i = 0; i < count; i++) {
        assert(pico8_fixed_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!pico8_fixed_seq_at(seq, count, 0, &out));
    assert(!pico8_fixed_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_int_seq_at_bounds(const pico8_int_seq_t *seq) {
    int out = -12345;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(pico8_int_seq_at(seq->items, seq->count, i + 1, &out) && out == seq->items[i]);
    }
    out = -12345;
    assert(!pico8_int_seq_at(seq->items, seq->count, 0, &out));
    assert(!pico8_int_seq_at(seq->items, seq->count, seq->count + 1, &out));
    assert(out == -12345);
}

static void test_seqs_answer_each_position_and_refuse_zero_and_past_the_end(void) {
    check_int_seq(pico8_es_table, PICO8_ES_COUNT);
    check_fixed_seq(pico8_ct_table, PICO8_CT_COUNT);
    check_int_seq(pico8_b__table, PICO8_B__COUNT);
    check_int_seq(pico8_co_table, PICO8_CO_COUNT);
    check_int_seq(pico8_fb_table, PICO8_FB_COUNT);
    check_int_seq(pico8_ek_table, PICO8_EK_COUNT);
    check_fixed_seq(pico8_ci_table, PICO8_CI_COUNT);
    check_int_seq_at_bounds(&pico8_jo_ka_seq);
    check_int_seq_at_bounds(&pico8_jo_cq_seq);
    check_int_seq_at_bounds(&pico8_qh_ba_seq);
    check_int_seq_at_bounds(&pico8_nl_ba_seq);
    check_int_seq_at_bounds(&pico8_hj_k__seq);
    check_int_seq_at_bounds(&pico8_pm_k__seq);
    check_int_seq_at_bounds(&pico8_mx_k__seq);
    check_int_seq_at_bounds(&pico8_ga_k__seq);
    check_int_seq_at_bounds(&pico8_pm_ga_k__seq);
    check_int_seq_at_bounds(&pico8_nr_hz_seq);
    check_int_seq_at_bounds(&pico8_fr_hz_seq);
    check_int_seq_at_bounds(&pico8_mx_ht_seq);
    check_int_seq_at_bounds(&pico8_g__k__seq);
    check_int_seq_at_bounds(&pico8_fv_k__seq);
    check_int_seq_at_bounds(&pico8_jc_k__seq);
    check_int_seq_at_bounds(&pico8_mz_kk_seq);
    check_int_seq_at_bounds(&pico8_bg_pl_seq);
}

static void test_r_lookup_finds_each_key(void) {
    int qe = -1;
    int i;

    for (i = 0; i < PICO8_R_COUNT; i++) {
        int found = -1;

        assert(pico8_r_lookup(pico8_r_table[i].key, &found));
        assert(found == pico8_r_table[i].qe);
    }
    assert(!pico8_r_lookup("zz", &qe));
    assert(qe == -1);
}

static void test_iz_lookup_finds_each_key(void) {
    int i;

    for (i = 0; i < PICO8_IZ_COUNT; i++) {
        assert(pico8_iz_lookup(pico8_iz_table[i].key) == pico8_iz_table[i].name);
    }
    assert(pico8_iz_lookup("zz") == NULL);
}

static void test_hc_lookup_finds_each_key(void) {
    int i;

    for (i = 0; i < PICO8_HC_COUNT; i++) {
        assert(pico8_hc_lookup(pico8_hc_table[i].key) == &pico8_hc_table[i]);
    }
    assert(pico8_hc_lookup("zz") == NULL);
}

static void test_df_lookup_finds_each_px(void) {
    int i;

    for (i = 0; i < PICO8_DF_COUNT; i++) {
        assert(pico8_df_lookup(pico8_df_table[i].px) == &pico8_df_table[i]);
    }
    assert(pico8_df_lookup("zz") == NULL);
    assert(pico8_df_lookup(NULL) == NULL);
}

static p8_fixed_t never_drawn(void *ctx) {
    (void)ctx;
    assert(0 && "the pool is built without a draw");
    return 0;
}

/* bw hands gn every df entry, in order, at og = 2. */
static void test_gn_pools_df_entries_by_their_r(void) {
    pico8_gn_state_t *state;
    int i;

    state = pico8_gn(frame_zone(), pico8_df_table, PICO8_DF_COUNT, p8_fixed_from_int(2), never_drawn, NULL);
    assert(state->oc_count == 66);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        int want = p8_fixed_flr_int(p8_fixed_mul(pico8_df_table[i].r, p8_fixed_from_int(2)));
        int got = 0;
        int j;

        for (j = 0; j < state->oc_count; j++) {
            got += state->oc[j] == &pico8_df_table[i];
        }
        assert(got == want);
    }
}

/* ob() gave each string token its own allocation: no string field of the jo
 * tree shares a pointer with another string of the generated data. */
static void test_jo_tree_strings_are_distinct_objects(void) {
    const char *const tree[] = {pico8_jo_def.me, pico8_qh_def.me, pico8_qh_def.probe.dq, pico8_nl_def.rr,
        pico8_hj_def.rr, pico8_pm_def.fz, pico8_pm_def.me, pico8_ga_def.fz, pico8_nr_def.rr,
        pico8_fr_def.rr, pico8_mx_def.rr};
    const char *const others[] = {pico8_bg_def.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
        pico8_g__def.probe.dq, pico8_mz_jj_qs};
    int n = (int)(sizeof tree / sizeof tree[0]);
    int i;
    int j;

    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            assert(tree[i] != tree[j]);
        }
        for (j = 0; j < (int)(sizeof others / sizeof others[0]); j++) {
            assert(tree[i] != others[j]);
        }
    }
}

static void test_nb_lookup_finds_each_px(void) {
    int i;

    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(pico8_nb_lookup(pico8_nb_table[i].px) == &pico8_nb_table[i]);
    }
    assert(pico8_nb_lookup("zz") == NULL && pico8_nb_lookup(NULL) == NULL);
}

/* Keys floored as Lua indexes a table, one entry per key, insertion order. */
static void test_qe_map_keeps_one_entry_per_key_in_order(void) {
    pico8_qe_map_t map = {0, {{0, 0}}};
    p8_fixed_t v = 0x1234;

    assert(!pico8_qe_map_get(&map, p8_fixed_from_int(21), &v) && v == 0x1234);
    pico8_qe_map_set(&map, p8_fixed_from_int(21), p8_fixed_from_int(3));
    pico8_qe_map_set(&map, p8_fixed_from_int(12), p8_fixed_from_int(5));
    pico8_qe_map_set(&map, p8_fixed_from_int(21), p8_fixed_from_int(7));
    assert(map.count == 2 && map.entries[0].qe == 21 && map.entries[1].qe == 12);
    assert(pico8_qe_map_get(&map, p8_fixed_from_int(21), &v) && v == p8_fixed_from_int(7));
    assert(pico8_qe_map_get(&map, p8_fixed_from_int(12) + 0x8000, &v) && v == p8_fixed_from_int(5));
    assert(!pico8_qe_map_get(&map, p8_fixed_from_int(13), &v) && v == p8_fixed_from_int(5));
    pico8_qe_map_set(&map, p8_fixed_from_int(99) + 0x4000, p8_fixed_from_int(1));
    assert(map.count == 3 && map.entries[2].qe == 99);
}

static void test_int_seq_has_fixed_keeps_gu_on_integers(void) {
    static const pico8_int_seq_t empty = {0, NULL};
    const pico8_int_seq_t *seq = &pico8_jo_cq_seq;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(pico8_int_seq_has_fixed(seq, p8_fixed_from_int(seq->items[i])));
        assert(!pico8_int_seq_has_fixed(seq, p8_fixed_from_int(seq->items[i]) + 0x8000));
    }
    assert(!pico8_int_seq_has_fixed(seq, p8_fixed_from_int(99)));
    assert(!pico8_int_seq_has_fixed(seq, (p8_fixed_t)seq->items[0]));
    assert(!pico8_int_seq_has_fixed(&empty, p8_fixed_from_int(0)));
}

int main(void) {
    test_seqs_answer_each_position_and_refuse_zero_and_past_the_end();
    test_r_lookup_finds_each_key();
    test_iz_lookup_finds_each_key();
    test_hc_lookup_finds_each_key();
    test_df_lookup_finds_each_px();
    test_gn_pools_df_entries_by_their_r();
    test_int_seq_has_fixed_keeps_gu_on_integers();
    test_jo_tree_strings_are_distinct_objects();
    test_nb_lookup_finds_each_px();
    test_qe_map_keeps_one_entry_per_key_in_order();

    printf("test_pico8_map_tables: all tests passed\n");
    return 0;
}
