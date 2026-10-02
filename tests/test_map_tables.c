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
        assert(int_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!int_seq_at(seq, count, 0, &out));
    assert(!int_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_fixed_seq(const p8_fixed_t *seq, int count) {
    p8_fixed_t out = -12345;
    int i;

    for (i = 0; i < count; i++) {
        assert(game_data_fixed_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!game_data_fixed_seq_at(seq, count, 0, &out));
    assert(!game_data_fixed_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_int_seq_at_bounds(const int_seq_t *seq) {
    int out = -12345;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(int_seq_at(seq->items, seq->count, i + 1, &out) && out == seq->items[i]);
    }
    out = -12345;
    assert(!int_seq_at(seq->items, seq->count, 0, &out));
    assert(!int_seq_at(seq->items, seq->count, seq->count + 1, &out));
    assert(out == -12345);
}

static void test_seqs_answer_each_position_and_refuse_zero_and_past_the_end(void) {
    check_int_seq(upkeep_by_tier_table, UPKEEP_BY_TIER_COUNT);
    check_fixed_seq(partner_bonus_by_tier_table, PARTNER_BONUS_BY_TIER_COUNT);
    check_int_seq(development_score_by_tier_table, DEVELOPMENT_SCORE_BY_TIER_COUNT);
    check_int_seq(happiness_by_tier_table, HAPPINESS_BY_TIER_COUNT);
    check_int_seq(palette_offsets_table, PALETTE_OFFSETS_COUNT);
    check_int_seq(tier_thresholds_table, TIER_THRESHOLDS_COUNT);
    check_fixed_seq(layer_fades_table, LAYER_FADES_COUNT);
    check_int_seq_at_bounds(&planet_ring_colors_seq);
    check_int_seq_at_bounds(&planet_made_resources_seq);
    check_int_seq_at_bounds(&structure_accepted_resources_seq);
    check_int_seq_at_bounds(&trading_hub_accepted_resources_seq);
    check_int_seq_at_bounds(&ascension_trail_colors_seq);
    check_int_seq_at_bounds(&protostar_trail_colors_seq);
    check_int_seq_at_bounds(&synthesizer_trail_colors_seq);
    check_int_seq_at_bounds(&burst_trail_colors_seq);
    check_int_seq_at_bounds(&protostar_burst_trail_colors_seq);
    check_int_seq_at_bounds(&slipgate_link_sprites_seq);
    check_int_seq_at_bounds(&infraspace_gate_link_sprites_seq);
    check_int_seq_at_bounds(&synthesizer_synthesizable_resources_seq);
    check_int_seq_at_bounds(&probe_trail_colors_seq);
    check_int_seq_at_bounds(&help_screen_trail_colors_seq);
    check_int_seq_at_bounds(&slipway_trail_colors_seq);
    check_int_seq_at_bounds(&wormhole_color_ramp_seq);
    check_int_seq_at_bounds(&background_tile_choices_seq);
}

static void test_resource_code_lookup_finds_each_key(void) {
    int qe = -1;
    int i;

    for (i = 0; i < RESOURCE_CODE_COUNT; i++) {
        int found = -1;

        assert(resource_code_lookup(resource_codes[i].key, &found));
        assert(found == resource_codes[i].resource);
    }
    assert(!resource_code_lookup("zz", &qe));
    assert(qe == -1);
}

static void test_resource_names_lookup_finds_each_key(void) {
    int i;

    for (i = 0; i < RESOURCE_NAMES_COUNT; i++) {
        assert(resource_names_lookup(resource_names_table[i].key) == resource_names_table[i].name);
    }
    assert(resource_names_lookup("zz") == NULL);
}

static void test_production_tables_lookup_finds_each_key(void) {
    int i;

    for (i = 0; i < PRODUCTION_TABLES_COUNT; i++) {
        assert(production_tables_lookup(production_tables_table[i].key) == &production_tables_table[i]);
    }
    assert(production_tables_lookup("zz") == NULL);
}

static void test_planet_types_lookup_finds_each_name(void) {
    int i;

    for (i = 0; i < PLANET_TYPES_COUNT; i++) {
        assert(planet_types_lookup(planet_types_table[i].name) == &planet_types_table[i]);
    }
    assert(planet_types_lookup("zz") == NULL);
    assert(planet_types_lookup(NULL) == NULL);
}

static p8_fixed_t never_drawn(void *ctx) {
    (void)ctx;
    assert(0 && "the pool is built without a draw");
    return 0;
}

/* bw hands gn every df entry, in order, at og = 2. */
static void test_weighted_picker_pools_planet_types_entries_by_their_r(void) {
    weighted_picker_t *state;
    int i;

    state = weighted_picker_new(frame_zone(), planet_types_table, PLANET_TYPES_COUNT, p8_fixed_from_int(2), never_drawn, NULL);
    assert(state->tickets_count == 66);
    for (i = 0; i < PLANET_TYPES_COUNT; i++) {
        int want = p8_fixed_flr_int(p8_fixed_mul(planet_types_table[i].weight, p8_fixed_from_int(2)));
        int got = 0;
        int j;

        for (j = 0; j < state->tickets_count; j++) {
            got += state->tickets[j] == &planet_types_table[i];
        }
        assert(got == want);
    }
}

/* ob() gave each string token its own allocation: no string field of the jo
 * tree shares a pointer with another string of the generated data. */
static void test_planet_tree_strings_are_distinct_objects(void) {
    const char *const tree[] = {planet_archetype.state, structure_archetype.state, structure_archetype.targeting.confirmed_state, trading_hub_archetype.required_tech,
        ascension_archetype.required_tech, protostar_archetype.class_name, protostar_archetype.state, burst_archetype.class_name, slipgate_archetype.required_tech,
        infraspace_gate_archetype.required_tech, synthesizer_archetype.required_tech};
    const char *const others[] = {background_archetype.state, wormhole_archetype.state, wormhole_archetype.targeting.confirmed_state, slipway_archetype.state,
        probe_archetype.targeting.confirmed_state, wormhole_too_far_target_status_text};
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

static void test_technologies_lookup_finds_each_name(void) {
    int i;

    for (i = 0; i < TECHNOLOGIES_COUNT; i++) {
        assert(map_tables_technologies_lookup(technologies_table[i].name) == &technologies_table[i]);
    }
    assert(map_tables_technologies_lookup("zz") == NULL && map_tables_technologies_lookup(NULL) == NULL);
}

/* Keys floored as Lua indexes a table, one entry per key, insertion order. */
static void test_resource_map_keeps_one_entry_per_key_in_order(void) {
    resource_map_t map = {0, {{0, 0}}};
    p8_fixed_t v = 0x1234;

    assert(!resource_map_get(&map, p8_fixed_from_int(21), &v) && v == 0x1234);
    resource_map_set(&map, p8_fixed_from_int(21), p8_fixed_from_int(3));
    resource_map_set(&map, p8_fixed_from_int(12), p8_fixed_from_int(5));
    resource_map_set(&map, p8_fixed_from_int(21), p8_fixed_from_int(7));
    assert(map.count == 2 && map.entries[0].resource == 21 && map.entries[1].resource == 12);
    assert(resource_map_get(&map, p8_fixed_from_int(21), &v) && v == p8_fixed_from_int(7));
    assert(resource_map_get(&map, p8_fixed_from_int(12) + 0x8000, &v) && v == p8_fixed_from_int(5));
    assert(!resource_map_get(&map, p8_fixed_from_int(13), &v) && v == p8_fixed_from_int(5));
    resource_map_set(&map, p8_fixed_from_int(99) + 0x4000, p8_fixed_from_int(1));
    assert(map.count == 3 && map.entries[2].resource == 99);
}

static void test_int_seq_has_fixed_keeps_contains_on_integers(void) {
    static const int_seq_t empty = {0, NULL};
    const int_seq_t *seq = &planet_made_resources_seq;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(int_seq_has_fixed(seq, p8_fixed_from_int(seq->items[i])));
        assert(!int_seq_has_fixed(seq, p8_fixed_from_int(seq->items[i]) + 0x8000));
    }
    assert(!int_seq_has_fixed(seq, p8_fixed_from_int(99)));
    assert(!int_seq_has_fixed(seq, (p8_fixed_t)seq->items[0]));
    assert(!int_seq_has_fixed(&empty, p8_fixed_from_int(0)));
}

int main(void) {
    test_seqs_answer_each_position_and_refuse_zero_and_past_the_end();
    test_resource_code_lookup_finds_each_key();
    test_resource_names_lookup_finds_each_key();
    test_production_tables_lookup_finds_each_key();
    test_planet_types_lookup_finds_each_name();
    test_weighted_picker_pools_planet_types_entries_by_their_r();
    test_int_seq_has_fixed_keeps_contains_on_integers();
    test_planet_tree_strings_are_distinct_objects();
    test_technologies_lookup_finds_each_name();
    test_resource_map_keeps_one_entry_per_key_in_order();

    printf("test_map_tables: all tests passed\n");
    return 0;
}
