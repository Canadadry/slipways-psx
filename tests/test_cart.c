#define _POSIX_C_SOURCE 200809L

#include "devtools/save_check.h"
#include "devtools/save_storage_memory.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "engine/entity.h"
#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "game/background.h"
#include "game/cart.h"
#include "game/economy.h"
#include "game/bindings.h"
#include "game/game_data.h"
#include "game/input_step.h"
#include "game/main_economy.h"
#include "game/pad_nav.h"
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

enum { ATTRACT_IDLE_FRAMES = 30, ATTRACT_SECOND_START_EXTRA_IDLE = 7, ATTRACT_PLAYED_FRAMES = 30, TITLE_DUMP_CAP = 65536 };

static const char DEMO_RECORD_PATH[] = "pico8/demo_record.hex";
static const uint32_t ATTRACT_VISUAL_RANDOM_STATE = 0x0BADF00Du;

typedef struct {
    char dump[TITLE_DUMP_CAP];
    unsigned char screen[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];
    int has_resume;
} title_snapshot_t;

static input_step_t g_step;
static save_check_snapshot_t g_demo_record;
static save_check_snapshot_t g_resumed[2];
static save_check_snapshot_t g_played[2];
static title_snapshot_t g_titles[3];
static char g_raw_title_dump[2 * TITLE_DUMP_CAP];

static int hex_digit(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/* The visual stream is set to a constant: a record never holds it, and the vanishing planet's
   flicker draws from it, so two starts would draw differently. */
static void read_demo_record(save_check_snapshot_t *out) {
    FILE *file = fopen(DEMO_RECORD_PATH, "r");
    int count = 0;
    int high;

    assert(file != NULL && "pico8/demo_record.hex: the demo record is missing");
    while ((high = fgetc(file)) != '\n') {
        int low = fgetc(file);
        assert(hex_digit(high) >= 0 && hex_digit(low) >= 0 && "pico8/demo_record.hex: not one line of lowercase hex pairs and a newline");
        assert(count < SAVE_STATE_MAX_BYTES && "pico8/demo_record.hex: more than SAVE_STATE_MAX_BYTES bytes");
        out->bytes[count++] = (uint8_t)(hex_digit(high) << 4 | hex_digit(low));
    }
    assert(fgetc(file) == EOF && "pico8/demo_record.hex: something follows the newline");
    fclose(file);
    out->byte_count = count;
    out->visual_random_state = ATTRACT_VISUAL_RANDOM_STATE;
    assert(save_state_decode(out->bytes, count, &g_decoded) && "pico8/demo_record.hex: the record does not decode");
}

static void play_attract_frame(int buttons) {
    p8_input_frame_t raw;

    memset(&raw, 0, sizeof raw);
    raw.pad_buttons = buttons;
    input_step_run(&g_step, &raw);
    save_check_hide_particles(1);
    cart_update60();
    cart_draw();
    save_check_hide_particles(0);
}

static void play_empty_frames(int count) {
    for (int i = 0; i < count; i++) {
        play_attract_frame(0);
    }
}

static int is_session_line(const char *line) {
    return strncmp(line, "zone ", 5) == 0 || strncmp(line, "cap ", 4) == 0 || strncmp(line, "budget ", 7) == 0;
}

static void capture_title(title_snapshot_t *out) {
    FILE *stream = fmemopen(g_raw_title_dump, sizeof g_raw_title_dump, "w");
    size_t written;
    size_t kept = 0;

    assert(stream != NULL);
    state_write(stream);
    written = (size_t)ftell(stream);
    fclose(stream);
    assert(written < sizeof g_raw_title_dump - 1 && "test_cart: the title's state dump outgrew its buffer");
    g_raw_title_dump[written] = '\0';
    for (const char *line = g_raw_title_dump; *line != '\0';) {
        const char *end = strchr(line, '\n');
        size_t length = end != NULL ? (size_t)(end - line) + 1 : strlen(line);
        if (!is_session_line(line)) {
            assert(kept + length < TITLE_DUMP_CAP);
            memcpy(out->dump + kept, line, length);
            kept += length;
        }
        line += length;
    }
    out->dump[kept] = '\0';
    memcpy(out->screen, memory.screen, sizeof out->screen);
    out->has_resume = title_button("resume") != NULL;
}

/* Returns 1 when equal; otherwise prints the first differing dump line or pixel. */
static int titles_equal(const title_snapshot_t *a, const title_snapshot_t *b) {
    const char *left = a->dump;
    const char *right = b->dump;
    for (int line = 1; *left != '\0' || *right != '\0'; line++) {
        int left_length = (int)strcspn(left, "\n");
        int right_length = (int)strcspn(right, "\n");
        if (left_length != right_length || memcmp(left, right, (size_t)left_length) != 0) {
            fprintf(stderr, "test_cart: title dump line %d: \"%.*s\" != \"%.*s\"\n", line, left_length, left, right_length, right);
            return 0;
        }
        left += left_length + (left[left_length] == '\n');
        right += right_length + (right[right_length] == '\n');
    }
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            if (a->screen[y][x] != b->screen[y][x]) {
                fprintf(stderr, "test_cart: title pixel %d %d: color %d != %d\n", x, y, a->screen[y][x], b->screen[y][x]);
                return 0;
            }
        }
    }
    return a->has_resume == b->has_resume;
}

static void assert_starts_equal(const save_check_snapshot_t *first, const save_check_snapshot_t *second) {
    char report[512];
    if (save_check_compare(first, second, report, sizeof report)) {
        fprintf(stderr, "test_cart: the second start differs from the first: %s\n", report);
        assert(0 && "two attract starts from the record give the same game");
    }
}

/* Neither cart_resume_game nor cart_init touches the input step: without these re-inits, the pad
   navigation of one phase leaks into the next. */
static void start_the_demo_record(void) {
    input_step_init(&g_step, &bindings_compiled);
    save_check_resume(&g_demo_record);
}

static void return_to_the_title(void) {
    cart_return_to_title();
    input_step_init(&g_step, &bindings_compiled);
}

/* first_frame_buttons: held on the first played frame only. */
static void play_the_demo_record(int start, int first_frame_buttons) {
    start_the_demo_record();
    save_check_capture(&g_resumed[start]);
    play_attract_frame(first_frame_buttons);
    play_empty_frames(ATTRACT_PLAYED_FRAMES - 1);
    save_check_capture(&g_played[start]);
    memcpy(g_played[start].screen, memory.screen, sizeof g_played[start].screen);
    g_played[start].has_screen = 1;
}

/* Boots over a player's save, then plays the record twice, each time back to the title. */
static void run_two_attract_cycles(int first_frame_buttons) {
    int player_count;
    int writes;

    assert(!bindings_compiled_personal && "the test binaries link the default bindings table");
    read_demo_record(&g_demo_record);
    load_cart_to_title();
    player_count = start_game_with_a_save(1);
    (void)remember_stored_record();
    load_cart_to_title_storing(g_earlier, player_count);
    input_step_init(&g_step, &bindings_compiled);
    writes = save_storage_memory_write_count();

    play_empty_frames(ATTRACT_IDLE_FRAMES);
    capture_title(&g_titles[0]);
    assert(g_titles[0].has_resume && "the boot title offers the player's game");
    for (int start = 0; start < 2; start++) {
        if (start > 0) play_empty_frames(ATTRACT_SECOND_START_EXTRA_IDLE);
        play_the_demo_record(start, first_frame_buttons);
        if (first_frame_buttons != 0) assert(pad_nav_tech_open(&g_step.pad_nav) && "the press opened the tech panel");
        return_to_the_title();
        play_empty_frames(ATTRACT_IDLE_FRAMES);
        capture_title(&g_titles[start + 1]);
        assert(titles_equal(&g_titles[0], &g_titles[start + 1]) && "the title after a return is the boot title");
    }
    assert_starts_equal(&g_resumed[0], &g_resumed[1]);
    assert_starts_equal(&g_played[0], &g_played[1]);
    assert(stored_record_equals_earlier(player_count) && "the player's save is untouched");
    assert(save_storage_memory_write_count() == writes && "the attract cycle writes nothing");
}

static void test_two_attract_starts_play_the_same_game_and_return_to_the_boot_title(void) {
    run_two_attract_cycles(0);
}

static void test_a_return_with_the_tech_panel_open_builds_the_boot_title(void) {
    int open_tech_panel = bindings_mask(&bindings_compiled, BINDINGS_MODE_JUMP, BINDINGS_ACTION_OPEN_TECH_PANEL);
    assert(open_tech_panel != 0 && "the default table binds open_tech_panel in jump mode");
    run_two_attract_cycles(open_tech_panel);
}

static void test_over_an_empty_storage_the_returned_title_has_no_resume_row(void) {
    read_demo_record(&g_demo_record);
    load_cart_to_title();
    input_step_init(&g_step, &bindings_compiled);
    play_empty_frames(ATTRACT_IDLE_FRAMES);
    capture_title(&g_titles[0]);
    play_the_demo_record(0, 0);
    return_to_the_title();
    play_empty_frames(ATTRACT_IDLE_FRAMES);
    capture_title(&g_titles[1]);
    assert(!g_titles[1].has_resume && titles_equal(&g_titles[0], &g_titles[1]) && "the title after a return is the boot title");
    assert(stored_count() == 0 && "the demo record is never stored");
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
    test_two_attract_starts_play_the_same_game_and_return_to_the_boot_title();
    test_a_return_with_the_tech_panel_open_builds_the_boot_title();
    test_over_an_empty_storage_the_returned_title_has_no_resume_row();
    printf("test_cart: all tests passed\n");
    return 0;
}
