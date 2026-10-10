#include "engine/dispatch_context.h"
#include "engine/entity.h"
#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "game/background.h"
#include "game/cart.h"
#include "game/economy.h"
#include "game/game_data.h"
#include "game/main_economy.h"
#include "game/probe.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "game/ui_button.h"
#include "game/wormhole.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static uint8_t g_bytes[SAVE_STATE_MAX_BYTES];
static uint8_t g_earlier[SAVE_STATE_MAX_BYTES];
static uint8_t g_encoded[SAVE_STATE_MAX_BYTES];
static save_state_t g_decoded;
static save_state_t g_captured;

enum { IDLE_FRAMES = 10, LAST_YEAR = 3426, UNFOLDING_FRAME_CAP = 150, WORMHOLE_PLACING_FRAMES = 16,
       PROBE_SCREEN_X = 40, PROBE_SCREEN_Y = 72, WORMHOLE_OFFSET_X = 30 };

static const uint32_t WORMHOLE_GAME_SEED = 0x5EED2025u;

static void play_frame(void) {
    cart_update60();
    cart_draw();
}

static void play_frames(int count) {
    for (int i = 0; i < count; i++) {
        play_frame();
    }
}

/* A count of 0 builds the title over an empty storage. */
static void load_cart_to_title_storing(const uint8_t *bytes, int count) {
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);
    cart_load();
    assert(save_storage_erase());
    if (count > 0) assert(save_storage_write(bytes, count));
    cart_init();
}

static void load_cart_to_title(void) {
    load_cart_to_title_storing(NULL, 0);
}

static void start_game(int difficulty_index) {
    button_t difficulty = {0};
    difficulty.name = difficulty_options_table[difficulty_index].name;
    difficulty.has_trade = 1;
    difficulty.trade = p8_fixed_from_int(difficulty_options_table[difficulty_index].trade);
    difficulty.has_type_bias = 1;
    difficulty.type_bias = difficulty_options_table[difficulty_index].type_bias;
    cart_start_game(&difficulty);
}

static int stored_count(void) {
    return save_storage_read(g_bytes, (int)sizeof g_bytes);
}

/* Returns the stored record's byte count; asserts that it decodes into g_decoded. */
static int read_stored_record(void) {
    int count = stored_count();
    assert(count > 0 && "a record is stored");
    assert(save_state_decode(g_bytes, count, &g_decoded) && "the stored record decodes");
    return count;
}

static int remember_stored_record(void) {
    int count = stored_count();
    memcpy(g_earlier, g_bytes, sizeof g_earlier);
    return count;
}

static int stored_record_equals_earlier(int earlier_count) {
    return stored_count() == earlier_count && memcmp(g_bytes, g_earlier, (size_t)earlier_count) == 0;
}

/* Called between cart_update60, whose end writes the record, and cart_draw. */
static int stored_record_is_the_live_game(void) {
    int count = stored_count();
    save_state_capture(&g_captured);
    return count > 0 && save_state_encode(&g_captured, g_encoded) == count && memcmp(g_bytes, g_encoded, (size_t)count) == 0;
}

static int play_frame_writing_the_live_game(void) {
    int written;

    cart_update60();
    written = stored_record_is_the_live_game();
    cart_draw();
    return written;
}

static void act(void) {
    assert(economy_spend(main_economy_typed(), p8_fixed_from_int(0)) && "a zero-cost spend always succeeds");
}

/* A game with one action written: the record of difficulty_index's game after its first end turn. */
static int start_game_with_a_save(int difficulty_index) {
    start_game(difficulty_index);
    play_frames(2);
    economy_new_year(main_economy_typed());
    play_frame();
    return read_stored_record();
}

static vec_t layer_camera(const entity_t *e) {
    p8_fixed_t parallax;
    vec_t camera;
    assert(game_data_fixed_seq_at(layer_fades_table, LAYER_FADES_COUNT, p8_fixed_flr_int(e->layer), &parallax));
    camera = vec_mul(entity_pos(main_camera()), parallax);
    return vec_make(p8_fixed_flr(camera.x), p8_fixed_flr(camera.y));
}

static entity_t *press_background(void) {
    down_t made = background_left_press((background_t *)background_live());
    assert(made.stop && "a background press makes a probe, then wormholes");
    return entity_get(made.held);
}

static mouse_pos_t mouse_at_screen(const entity_t *held, int x, int y) {
    vec_t screen = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return (mouse_pos_t){.world = vec_add(layer_camera(held), screen), .screen = screen,
                         .has = MOUSE_POS_HAS_WORLD | MOUSE_POS_HAS_SCREEN};
}

/* The probe placed and written, then a wormhole dropped next to it, left unfolding with its drop pending. */
static void drop_a_wormhole(void) {
    entity_t *probe;
    entity_t *wormhole;
    mouse_pos_t mouse;

    p8_fixed_set_rnd_state(WORMHOLE_GAME_SEED);
    start_game(1);
    play_frames(2);
    probe = press_background();
    assert(probe->class == probe_class());
    probe->class->drag(probe, mouse_at_screen(probe, PROBE_SCREEN_X, PROBE_SCREEN_Y));
    assert(targeting_confirm(probe) && "the probe drops on an empty spot of the screen");
    assert(play_frame_writing_the_live_game() && "placing the probe is an action");

    wormhole = press_background();
    assert(wormhole->class == wormhole_class() && wormhole->state == ENTITY_STATE_PLACING);
    mouse = mouse_at_screen(wormhole, PROBE_SCREEN_X, PROBE_SCREEN_Y);
    mouse.world = vec_add(vec_mul(entity_map_pos(probe), p8_fixed_from_int(128)), vec_make(p8_fixed_from_int(WORMHOLE_OFFSET_X), 0));
    wormhole->class->drag(wormhole, mouse);
    play_frames(WORMHOLE_PLACING_FRAMES);
    wormhole_drop((wormhole_t *)wormhole);
    assert(!wormhole->is_dead && wormhole->state == ENTITY_STATE_UNFOLDING && "the wormhole unfolds next to the probe");
    assert(main_economy_typed()->save_pending && "the wormhole's drop is an action");
}

static void test_a_started_game_with_no_action_writes_nothing(void) {
    load_cart_to_title();
    start_game(1);
    play_frames(IDLE_FRAMES);
    assert(stored_count() == 0);
}

static void test_an_action_writes_the_game_at_the_end_of_its_frame(void) {
    economy_t *economy;

    load_cart_to_title();
    start_game(2);
    play_frames(2);
    economy = main_economy_typed();
    economy_new_year(economy);
    assert(stored_count() == 0 && "nothing is written before the frame ends");
    play_frame();
    read_stored_record();
    assert(g_decoded.difficulty == 3);
    assert(g_decoded.economy.year == main_economy_typed()->year);
    assert(g_decoded.economy.credits == main_economy_typed()->credits);
    assert(main_economy_typed()->save_pending == 0);
}

static void test_a_wormhole_holds_the_write_back_until_it_is_gone(void) {
    int earlier_count;
    int frames = 0;

    load_cart_to_title();
    drop_a_wormhole();
    earlier_count = remember_stored_record();
    while (wormhole_live_count() > 0) {
        assert(frames < UNFOLDING_FRAME_CAP && "the wormhole unfolds and dies");
        assert(stored_record_equals_earlier(earlier_count) && "nothing is written while a wormhole exists");
        cart_update60();
        if (wormhole_live_count() > 0) cart_draw();
        frames++;
    }
    assert(frames > 1);
    assert(!stored_record_equals_earlier(earlier_count) && stored_record_is_the_live_game() &&
           "the first frame with no wormhole writes the revealed area");
    cart_draw();
    assert(main_economy_typed()->save_pending == 0);
    read_stored_record();
    assert(g_decoded.node_count == entities_by_tag(TAG_NODE)->count);
}

static void set_last_year_eve(void) {
    main_economy_typed()->year = p8_fixed_from_int(LAST_YEAR - 1);
}

static void test_the_end_turn_that_ends_the_game_erases_the_save(void) {
    load_cart_to_title();
    (void)start_game_with_a_save(1);
    set_last_year_eve();
    economy_new_year(main_economy_typed());
    assert(strcmp(main_economy_typed()->end_status, "final score") == 0);
    play_frame();
    assert(stored_count() == 0 && "the erase wins over the frame's write");
    play_frames(IDLE_FRAMES);
    assert(stored_count() == 0);
}

static void test_the_game_end_erases_the_save_even_during_a_wormhole(void) {
    load_cart_to_title();
    drop_a_wormhole();
    assert(wormhole_live_count() > 0);
    set_last_year_eve();
    economy_new_year(main_economy_typed());
    play_frame();
    assert(wormhole_live_count() > 0 && stored_count() == 0 && "a live wormhole does not delay the erase");
}

static void test_bankruptcy_erases_the_save(void) {
    economy_t *economy;

    load_cart_to_title();
    (void)start_game_with_a_save(1);
    economy = main_economy_typed();
    assert(economy->income <= 0 && "a game with no colony has no income");
    assert(economy_spend(economy, p8_fixed_sub(economy->credits, p8_fixed_from_int(2))));
    economy_advance_clock(economy, p8_fixed_from_int(1));
    assert(strcmp(economy->end_status, "bankrupt") == 0);
    play_frame();
    assert(stored_count() == 0);
}

static void test_restart_keeps_the_save(void) {
    int earlier_count;

    load_cart_to_title();
    earlier_count = start_game_with_a_save(1);
    (void)remember_stored_record();
    cart_init();
    play_frames(IDLE_FRAMES);
    assert(stored_record_equals_earlier(earlier_count));
}

static void test_a_new_game_overwrites_the_save_at_its_first_action_only(void) {
    int earlier_count;

    load_cart_to_title();
    earlier_count = start_game_with_a_save(1);
    (void)remember_stored_record();
    cart_init();
    play_frame();
    start_game(3);
    play_frames(IDLE_FRAMES);
    assert(stored_record_equals_earlier(earlier_count) && "picking a difficulty writes nothing");
    act();
    assert(play_frame_writing_the_live_game());
    assert(!stored_record_equals_earlier(earlier_count));
    read_stored_record();
    assert(g_decoded.difficulty == 4);
}

static void test_an_action_held_back_when_restart_is_pressed_is_lost(void) {
    int earlier_count;

    load_cart_to_title();
    drop_a_wormhole();
    earlier_count = remember_stored_record();
    play_frame();
    cart_init();
    play_frame();
    assert(stored_record_equals_earlier(earlier_count));
    start_game(1);
    play_frames(IDLE_FRAMES);
    assert(stored_record_equals_earlier(earlier_count) && "the new game has no pending action");
}

/* NULL when no live button has that name. */
static button_t *title_button(const char *name) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        if (e->class == button_class() && ((const button_t *)e)->name != NULL && strcmp(((const button_t *)e)->name, name) == 0) {
            return (button_t *)e;
        }
    }
    return NULL;
}

static void test_the_title_shows_resume_only_with_a_save(void) {
    load_cart_to_title();
    assert(title_button("forgiving") != NULL && title_button("resume") == NULL);
    (void)start_game_with_a_save(1);
    cart_init();
    assert(title_button("forgiving") != NULL && title_button("resume") != NULL);
}

static void assert_an_invalid_save_is_hidden_and_kept(const uint8_t *bytes, int count) {
    memcpy(g_earlier, bytes, (size_t)count);
    load_cart_to_title_storing(g_earlier, count);
    play_frames(IDLE_FRAMES);
    assert(title_button("forgiving") != NULL && title_button("resume") == NULL);
    assert(stored_record_equals_earlier(count) && "a bad save is left alone");
    start_game(0);
    play_frames(2);
    act();
    assert(play_frame_writing_the_live_game() && "the new game's first action overwrites it");
    read_stored_record();
}

static void test_garbage_in_the_storage_hides_resume_and_is_left_alone(void) {
    static uint8_t garbage[64];

    memset(garbage, 0xA5, sizeof garbage);
    assert_an_invalid_save_is_hidden_and_kept(garbage, (int)sizeof garbage);
}

static void test_a_record_with_one_byte_flipped_hides_resume_and_is_left_alone(void) {
    static uint8_t flipped[SAVE_STATE_MAX_BYTES];
    int count;

    load_cart_to_title();
    count = start_game_with_a_save(1);
    memcpy(flipped, g_bytes, (size_t)count);
    flipped[count / 2] ^= 0x01;
    assert(!save_state_decode(flipped, count, &g_decoded));
    assert_an_invalid_save_is_hidden_and_kept(flipped, count);
}

static void test_resume_brings_back_the_saved_game_and_writes_nothing(void) {
    button_t *resume;
    int earlier_count;

    load_cart_to_title();
    earlier_count = start_game_with_a_save(2);
    (void)remember_stored_record();
    cart_init();
    play_frames(IDLE_FRAMES);
    resume = title_button("resume");
    assert(resume != NULL && resume->has_on_click);
    resume->on_click(resume);
    assert(main_economy_typed() != NULL && main_economy_typed()->save_pending == 0);
    save_state_capture(&g_captured);
    assert(save_state_encode(&g_captured, g_encoded) == earlier_count && memcmp(g_encoded, g_earlier, (size_t)earlier_count) == 0 &&
           "the resumed game is the stored game");
    play_frames(IDLE_FRAMES);
    assert(stored_record_equals_earlier(earlier_count) && "resuming writes nothing");

    cart_init();
    play_frames(IDLE_FRAMES);
    resume = title_button("resume");
    assert(resume != NULL);
    resume->on_click(resume);
    save_state_capture(&g_captured);
    assert(save_state_encode(&g_captured, g_encoded) == earlier_count && memcmp(g_encoded, g_earlier, (size_t)earlier_count) == 0 &&
           "restart then resume gives the same game again");
}

static void test_a_game_with_no_difficulty_is_never_written(void) {
    load_cart_to_title();
    cart_start_game(NULL);
    play_frame();
    act();
    play_frames(IDLE_FRAMES);
    assert(stored_count() == 0);
}

int main(void) {
    test_a_started_game_with_no_action_writes_nothing();
    test_an_action_writes_the_game_at_the_end_of_its_frame();
    test_a_wormhole_holds_the_write_back_until_it_is_gone();
    test_the_end_turn_that_ends_the_game_erases_the_save();
    test_the_game_end_erases_the_save_even_during_a_wormhole();
    test_bankruptcy_erases_the_save();
    test_restart_keeps_the_save();
    test_a_new_game_overwrites_the_save_at_its_first_action_only();
    test_an_action_held_back_when_restart_is_pressed_is_lost();
    test_a_game_with_no_difficulty_is_never_written();
    test_the_title_shows_resume_only_with_a_save();
    test_garbage_in_the_storage_hides_resume_and_is_left_alone();
    test_a_record_with_one_byte_flipped_hides_resume_and_is_left_alone();
    test_resume_brings_back_the_saved_game_and_writes_nothing();
    printf("test_cart: all tests passed\n");
    return 0;
}
