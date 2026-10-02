#include "game/galaxy.h"

#include <string.h>

#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "game/planet.h"
#include "p8/str.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

/* Adapts pico8_jf_next to pico8_gn_random_fn's ctx shape: `ctx` is always
 * the pico8_jf_state_t this file passes alongside it. */
static p8_fixed_t picker_random(void *ctx) {
    return random_walk_next((random_walk_t *)ctx);
}

void galaxy_populate(p8_fixed_t planet_count) {
    p8_fixed_t one = p8_fixed_from_int(1);
    p8_fixed_t threshold = P8_FIXED_LIT(0x00000ccc, 0.05);
    p8_fixed_t two_seventy_five = P8_FIXED_LIT(0x0002c000, 2.75);
    p8_fixed_t eighth = P8_FIXED_LIT(0x00002000, 0.125);
    p8_fixed_t planet_id;
    const pico8_js_t *difficulty;
    p8_fixed_t difficulty_type_bias = p8_fixed_from_int(0);
    random_walk_t *type_picker_random;
    weighted_picker_t *type_picker_state;
    random_walk_t *vanish_roll_state;

    for (planet_id = one; planet_id <= planet_count; planet_id = p8_fixed_add(planet_id, one)) {
        vec_t p_vec;
        vec_t *p_box;
        vec_t *pos_box;
        entity_t *nearest;
        char *id_num_str;
        char *id_str;
        planet_keys_t keys;
        planet_t *made;

        /* cg.jo is empty only on the very first trial: every accepted
         * planet below self-registers into it, so later candidates really
         * are rejection-sampled against the ones already placed. Not an
         * inert loop over a list that stays empty. */
        do {
            p8_fixed_t rx = p8_fixed_add(p8_fixed_rnd(two_seventy_five), eighth);
            p8_fixed_t ry = p8_fixed_add(p8_fixed_rnd(two_seventy_five), eighth);
            p_vec = vec_make(rx, ry);
        } while (entity_geom_nearest(p_vec, entities_by_tag("planet"), &nearest) <= threshold);

        p_box = (vec_t *)allocator_alloc(game_zone(), sizeof *p_box);
        *p_box = p_vec;
        pos_box = (vec_t *)allocator_alloc(game_zone(), sizeof *pos_box);
        *pos_box = vec_mul(p_vec, p8_fixed_from_int(128));

        id_num_str = p8_tostr_fixed(frame_zone(), planet_id);
        id_str = p8_concat2(game_zone(), "p", id_num_str);

        keys.planet_id = id_str;
        keys.map_pos = p_box;
        keys.pos = pos_box;

        /* Built for the side effect of self-registering into hv/cv/cg. */
        made = planet_new(&keys);
        (void)made;
    }

    /* A NULL dk or an absent "en" reads as 0 -- this port's usual
     * absent-field default, not a guess at which difficulty is meant. */
    difficulty = economy_score_difficulty();
    if (difficulty != NULL && difficulty->has_en) {
        difficulty_type_bias = difficulty->en;
    }

    type_picker_random = random_walk_new(game_zone(), P8_FIXED_LIT(0x00008000, 0.5), difficulty_type_bias);
    type_picker_state = weighted_picker_new(game_zone(), planet_types_table, PLANET_TYPES_COUNT, p8_fixed_from_int(2), picker_random,
                        type_picker_random);
    planet_set_type_picker(type_picker_state);

    vanish_roll_state = random_walk_new(game_zone(), P8_FIXED_LIT(0x0000cccc, 0.8), P8_FIXED_LIT(0x00008000, 0.5));
    planet_set_vanish_roll(vanish_roll_state);
}
