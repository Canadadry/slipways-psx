/* The whole compiled demo fed through the attract mode, over a player's save: two complete
 * playbacks, then one interruption inside each command range of pico8/demo.txt. Writes the
 * full-cycle fixture that make demo-check replays with ATTRACT_STATE. Run by make demo-check
 * only: thousands of frames. */

#define _POSIX_C_SOURCE 200809L

#include "devtools/replay_source.h"
#include "devtools/save_check.h"
#include "devtools/save_storage_memory.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "engine/entity.h"
#include "engine/entity_registry.h"
#include "game/bindings.h"
#include "game/cart.h"
#include "game/demo_data.h"
#include "game/demo_overlay.h"
#include "game/economy.h"
#include "game/game_data.h"
#include "game/input_step.h"
#include "game/main_economy.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "game/ui_button.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { OVERLAY_BAND_TOP = 96, OVERLAY_BAND_BOTTOM = 108, OVERLAY_TAG_LEFT = 143, OVERLAY_TAG_TOP = 11,
       OVERLAY_TAG_BOTTOM = 17, OVERLAY_CAPTION_CAP = 16 };

enum { ATTRACT_AFTER = 3, TITLE_FRAMES_AFTER_THE_RELEASE = 2, FIXTURE_TITLE_FRAMES = 10, STATE_TEXT_CAP = 65536 };

typedef struct {
    const char *command;
    int frame;
} interruption_t;

/* One frame inside each command range of pico8/demo.txt. */
static const interruption_t INTERRUPTIONS[] = {
    {"fold", 480}, {"colonise", 945}, {"slipway", 1380}, {"year", 1830}, {"research", 2280},
};

typedef struct {
    int year, colonies, slipways, techs;
} counts_t;

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;
static input_step_t g_step;
static uint8_t g_player_save[SAVE_STATE_MAX_BYTES];
static int g_player_save_count;
static int g_player_writes;
static uint8_t g_bytes[SAVE_STATE_MAX_BYTES];
static char g_state_text[STATE_TEXT_CAP];
static save_check_snapshot_t g_last_frames[2];
static int g_demo_cheats;
static p8_vram_t g_overlay_reference;
/* The band and tag regions of the first playback at each caption's first frame. */
static unsigned char g_caption_screens[OVERLAY_CAPTION_CAP][P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];

/* Returns the frame's cheat activations. */
static int play_frame(int buttons) {
    p8_input_frame_t raw;
    memset(&raw, 0, sizeof raw);
    raw.pad_buttons = buttons;
    int cheat = input_step_run(&g_step, &raw);
    cart_update60();
    cart_draw();
    return cheat;
}

/* test_demo_overlay pins this reference pixel by pixel; the band and the tag box overwrite their whole regions. */
static void draw_overlay_reference(int demo_frame) {
    static int drawn_frame = CART_NO_DEMO_FRAME;
    if (demo_frame == drawn_frame) return;
    drawn_frame = demo_frame;
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_vram_init(&g_overlay_reference);
    demo_overlay_draw(&gfx, &g_overlay_reference, demo_frame);
}

static int regions_equal(const unsigned char (*a)[P8_SCREEN_WIDTH], const unsigned char (*b)[P8_SCREEN_WIDTH], int left,
                         int top, int right, int bottom) {
    for (int y = top; y <= bottom; y++) {
        if (memcmp(&a[y][left], &b[y][left], (size_t)(right - left + 1)) != 0) return 0;
    }
    return 1;
}

static int band_equal(const unsigned char (*a)[P8_SCREEN_WIDTH], const unsigned char (*b)[P8_SCREEN_WIDTH]) {
    return regions_equal(a, b, 0, OVERLAY_BAND_TOP, P8_SCREEN_WIDTH - 1, OVERLAY_BAND_BOTTOM);
}

static int tag_equal(const unsigned char (*a)[P8_SCREEN_WIDTH], const unsigned char (*b)[P8_SCREEN_WIDTH]) {
    return regions_equal(a, b, OVERLAY_TAG_LEFT, OVERLAY_TAG_TOP, P8_SCREEN_WIDTH - 1, OVERLAY_TAG_BOTTOM);
}

static int overlay_shows_demo_frame(int demo_frame) {
    draw_overlay_reference(demo_frame);
    return band_equal(memory.screen, g_overlay_reference.screen) && tag_equal(memory.screen, g_overlay_reference.screen);
}

static int band_rows_hold_only_band_colours(void) {
    for (int y = OVERLAY_BAND_TOP; y <= OVERLAY_BAND_BOTTOM; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            if (memory.screen[y][x] != 1 && memory.screen[y][x] != 7) return 0;
        }
    }
    return 1;
}

static int overlay_absent(void) {
    draw_overlay_reference(0);
    return !band_rows_hold_only_band_colours() && !tag_equal(memory.screen, g_overlay_reference.screen);
}

/* The first playback keeps each caption's first frame; the second must show the same band and tag there. */
static void check_caption_start(int playback) {
    int played = input_step_attract_demo_frame(&g_step) - 1;
    int caption = demo_overlay_find_caption(demo_data_captions, demo_data_caption_count, played);
    if (caption < 0 || demo_data_captions[caption].from != played) return;
    assert(overlay_shows_demo_frame(played) && "a caption's first frame shows it");
    if (playback == 0) {
        memcpy(g_caption_screens[caption], memory.screen, sizeof memory.screen);
        return;
    }
    if (band_equal(memory.screen, g_caption_screens[caption]) && tag_equal(memory.screen, g_caption_screens[caption])) return;
    fprintf(stderr, "test_attract_demo: caption %d at demo frame %d differs between the playbacks\n", caption, played);
    assert(0 && "the captions do not depend on the visual stream");
}

/* A count of 0 boots over an empty storage. */
static void boot_storing(const uint8_t *bytes, int count) {
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

static void play_cart_frames(int count) {
    for (int i = 0; i < count; i++) {
        cart_update60();
        cart_draw();
    }
}

/* The player's save: a forgiving game written by its first end turn. */
static void boot_over_a_player_save(void) {
    button_t difficulty = {0};
    boot_storing(NULL, 0);
    difficulty.name = difficulty_options_table[0].name;
    difficulty.has_trade = 1;
    difficulty.trade = p8_fixed_from_int(difficulty_options_table[0].trade);
    difficulty.has_type_bias = 1;
    difficulty.type_bias = difficulty_options_table[0].type_bias;
    cart_start_game(&difficulty);
    play_cart_frames(2);
    economy_new_year(main_economy_typed());
    play_cart_frames(1);
    g_player_save_count = save_storage_read(g_player_save, (int)sizeof g_player_save);
    assert(g_player_save_count > 0 && "the player's game is stored");
    boot_storing(g_player_save, g_player_save_count);
    input_step_init(&g_step, &bindings_compiled);
    input_step_set_attract_after(&g_step, ATTRACT_AFTER);
    g_player_writes = save_storage_memory_write_count();
}

/* NULL when no live button has that name. */
static const button_t *title_button(const char *name) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        const entity_t *e = bucket->items[i];
        if (e->class == button_class() && ((const button_t *)e)->name != NULL &&
            strcmp(((const button_t *)e)->name, name) == 0) {
            return (const button_t *)e;
        }
    }
    return NULL;
}

static int player_save_untouched(void) {
    int count = save_storage_read(g_bytes, (int)sizeof g_bytes);
    return count == g_player_save_count && memcmp(g_bytes, g_player_save, (size_t)count) == 0 &&
           save_storage_memory_write_count() == g_player_writes;
}

static void assert_the_title_is_back(const char *when) {
    if (cart_title_shown() && title_button("forgiving") != NULL && title_button("resume") != NULL && player_save_untouched()) {
        return;
    }
    fprintf(stderr, "test_attract_demo: %s: title shown %d, resume row %d, save untouched %d\n", when, cart_title_shown(),
            title_button("resume") != NULL, player_save_untouched());
    assert(0 && "the title is back with its resume row and the player's save byte for byte");
}

static int state_value(const char *prefix) {
    size_t length = strlen(prefix);
    for (const char *line = g_state_text; *line != '\0'; line = strchr(line, '\n') + 1) {
        if (strncmp(line, prefix, length) == 0) return atoi(line + length);
        if (strchr(line, '\n') == NULL) break;
    }
    fprintf(stderr, "test_attract_demo: no \"%s\" line in the state\n", prefix);
    assert(0 && "the state has the counted line");
    return 0;
}

static counts_t live_counts(void) {
    FILE *stream = fmemopen(g_state_text, sizeof g_state_text, "w");
    assert(stream != NULL);
    state_write(stream);
    long written = ftell(stream);
    fclose(stream);
    assert(written > 0 && written < (long)sizeof g_state_text - 1 && "test_attract_demo: the state outgrew its buffer");
    g_state_text[written] = '\0';
    return (counts_t){state_value("clock year "), state_value("count colonies "), state_value("count slipways "),
                      state_value("count techs ")};
}

/* Idle title frames until the demo starts; returns the record's counts at its first frame. */
static counts_t play_title_until_the_demo_starts(void) {
    for (int frame = 0; input_step_attract_phase(&g_step) != ATTRACT_PHASE_PLAYING; frame++) {
        assert(frame <= ATTRACT_AFTER && input_step_attract_phase(&g_step) == ATTRACT_PHASE_IDLE && cart_title_shown() &&
               "the idle title starts the demo within the threshold");
        g_demo_cheats = play_frame(0);
    }
    assert(input_step_attract_demo_frame(&g_step) == 1 && "the start frame plays the demo's first frame");
    return live_counts();
}

static void test_two_complete_playbacks_reach_the_demo_end_state_and_leave_the_save(void) {
    assert(demo_data_caption_count <= OVERLAY_CAPTION_CAP);
    for (int playback = 0; playback < 2; playback++) {
        counts_t record = play_title_until_the_demo_starts();
        check_caption_start(playback);
        while (input_step_attract_demo_frame(&g_step) < demo_data_frame_count) {
            g_demo_cheats += play_frame(0);
            assert(input_step_attract_phase(&g_step) == ATTRACT_PHASE_PLAYING);
            check_caption_start(playback);
        }
        int closing = demo_data_caption_count - 1;
        assert(demo_overlay_find_caption(demo_data_captions, demo_data_caption_count, demo_data_frame_count - 1) == closing);
        assert(overlay_shows_demo_frame(demo_data_frame_count - 1) && "the last frame shows the closing caption and the tag");
        counts_t end = live_counts();
        printf("test_attract_demo: playback %d: year %d -> %d, colonies %d -> %d, slipways %d -> %d, techs %d -> %d, "
               "cheat activations %d\n", playback + 1, record.year, end.year, record.colonies, end.colonies,
               record.slipways, end.slipways, record.techs, end.techs, g_demo_cheats);
        assert(end.year == record.year + 1 && end.colonies == record.colonies + 1 && end.slipways == record.slipways + 1 &&
               end.techs == record.techs + 1 && "one more year, colony, slipway and tech than the record");
        assert(g_demo_cheats == 0 && "the demo activates no cheat");
        save_check_capture(&g_last_frames[playback]);

        play_frame(0);
        assert(input_step_attract_phase(&g_step) == ATTRACT_PHASE_IDLE && "the frame after the last one returns");
        assert(overlay_absent() && "the return frame shows the title only");
        assert_the_title_is_back("after a complete demo");
    }
    char report[512];
    if (save_check_compare(&g_last_frames[0], &g_last_frames[1], report, sizeof report)) {
        fprintf(stderr, "test_attract_demo: the second playback ends elsewhere: %s\n", report);
        assert(0 && "two playbacks end in the same game");
    }
}

static void test_an_interruption_inside_each_command_range_leaves_the_title_and_the_save(void) {
    int press = bindings_mask(&bindings_compiled, BINDINGS_MODE_TITLE, BINDINGS_ACTION_CHOOSE_ROW);
    for (size_t i = 0; i < sizeof INTERRUPTIONS / sizeof INTERRUPTIONS[0]; i++) {
        char when[64];
        (void)play_title_until_the_demo_starts();
        while (input_step_attract_demo_frame(&g_step) < INTERRUPTIONS[i].frame) {
            play_frame(0);
        }
        snprintf(when, sizeof when, "%s, demo frame %d", INTERRUPTIONS[i].command, INTERRUPTIONS[i].frame);
        play_frame(press);
        assert(input_step_attract_phase(&g_step) == ATTRACT_PHASE_SWALLOWING);
        assert(overlay_absent() && "the interrupting frame shows the title only");
        assert_the_title_is_back(when);
        for (int frame = 0; frame <= TITLE_FRAMES_AFTER_THE_RELEASE; frame++) {
            play_frame(0);
            assert(input_step_attract_phase(&g_step) == ATTRACT_PHASE_IDLE);
            assert_the_title_is_back(when);
        }
        printf("test_attract_demo: interrupted at %s: title, resume row and save bytes back\n", when);
    }
}

/* Empty frames for two complete playbacks from the title, then a few title frames. */
static void write_full_cycle_fixture(const char *path) {
    int count = 2 * (ATTRACT_AFTER + demo_data_frame_count + 1) + FIXTURE_TITLE_FRAMES;
    p8_input_frame_t *frames = calloc((size_t)count, sizeof *frames);
    char member[64];
    assert(frames != NULL);
    assert(replay_source_format_attract_after(member, sizeof member, ATTRACT_AFTER) > 0);
    assert(p8_input_save(frames, count, path, member) && "test_attract_demo: cannot write the full-cycle fixture");
    free(frames);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: test_attract_demo <full-cycle fixture.json>\n");
        return 2;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    write_full_cycle_fixture(argv[1]);
    boot_over_a_player_save();
    test_two_complete_playbacks_reach_the_demo_end_state_and_leave_the_save();
    test_an_interruption_inside_each_command_range_leaves_the_title_and_the_save();
    printf("test_attract_demo: all tests passed\n");
    return 0;
}
