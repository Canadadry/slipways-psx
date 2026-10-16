/* tools/steady_profile.sh on hand-written TTYs, maps, group tables and PC profiles. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/steady_profile.sh"
#define DIR "tmp/test_steady_profile_work"
#define OUT DIR "/out.txt"

/* Run frames 13 and 14 are the window; frame 12 is outside it and carries other figures. */
#define MAP "cut 100 first-game-frame 13 source-first 101 window 101 102\n"

#define GROUPS                                                                                                         \
    "# a comment, then a blank line\n\n"                                                                               \
    "planet_draw_method planets\n"                                                                                     \
    "structure_icon_draw_method planets\n"                                                                             \
    "slipway_draw_method slipways\n"                                                                                   \
    "hud_select_palette_block_default hud\n"                                                                           \
    "entity_pos frame\n"                                                                                               \
    "draw_layers frame\n"                                                                                              \
    "cart_draw frame\n"

typedef struct {
    int breakdown_draw;
    int plain_draw;
    int spr_calls_pc;
    int slipway_ns;
    int ui_update_ns;
    int main_ui_ns;
    /* Non-zero: two more draw profiles, the second with this slipway time. */
    int outlier_slipway_ns;
    /* The cart profile's draw_layers figures are this many times the draw profile's sums. */
    int layers_scale;
} fixture_t;

static fixture_t fixture_default(void) {
    return (fixture_t){
        .breakdown_draw = 100, .plain_draw = 80, .spr_calls_pc = 10, .slipway_ns = 400, .ui_update_ns = 100, .layers_scale = 1};
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return buf;
    }
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int run_command(const char *command) {
    int status = system(command);
    assert(status != -1 && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/* Calibration 64: one tick is one hblank. Per frame: spr 10 calls 40 ticks, line 10 calls 20 ticks. */
static void write_ttys(const fixture_t *fixture) {
    char text[4096];
    char *at = text;
    at += sprintf(at, "psx-frame: hblanks-per-vsync 263\r\npsx-draw: clk8-per-64-hblanks 64\r\n");
    for (int frame = 12; frame <= 14; frame++) {
        int draw = frame == 12 ? 999 : fixture->breakdown_draw;
        at += sprintf(at, "psx-frame: frame %d update 30 draw %d submit 5 total %d vsyncs 1\r\n", frame, draw,
                      35 + draw);
        at += sprintf(at,
                      "psx-draw: frame %d cls 0 0 pset 0 0 line 20 10 rect 0 0 rectfill 0 0 circ 0 0 circfill 0 0 "
                      "spr 40 10 print 0 0 pattern 0 0 wrapped 0\r\n",
                      frame);
    }
    write_file(DIR "/breakdown.tty.txt", text);
    at = text;
    at += sprintf(at, "psx-frame: hblanks-per-vsync 263\n");
    for (int frame = 12; frame <= 14; frame++) {
        int draw = frame == 12 ? 999 : fixture->plain_draw;
        at += sprintf(at, "psx-frame: frame %d update 30 draw %d submit 5 total %d vsyncs 1\n", frame, draw, 35 + draw);
    }
    write_file(DIR "/plain.tty.txt", text);
}

static char *write_families(char *at, int frame, int spr_calls, int spr_ns, int line_ns) {
    static const char *const others[] = {"cls", "pset", "rect", "rectfill", "circ", "circfill", "print", "pattern"};
    at += sprintf(at, "frame %d family spr %d %d units %d switches 0\n", frame, spr_calls, spr_ns, spr_calls);
    at += sprintf(at, "frame %d family line 10 %d units 10 switches 0\n", frame, line_ns);
    for (size_t i = 0; i < sizeof others / sizeof others[0]; i++) {
        at += sprintf(at, "frame %d family %s 0 0 units 0 switches 0\n", frame, others[i]);
    }
    return at;
}

/* Rooted at draw_layers and cart_update60: planet_draw_method draws 6 spr in 300 ns, the
 * structure icons 4 spr in 100 ns, the slipways 10 line in 100 ns. */
static void write_draw_profile(const char *path, const fixture_t *fixture) {
    char text[8192];
    char *at = text;
    at += sprintf(at, "profile run.json frames 15 hook-pair-ns 4.0 clock monotonic-ns\n");
    at += sprintf(at, "root draw draw_layers\nroot update cart_update60\n");
    for (int frame = 12; frame <= 14; frame++) {
        int scale = frame == 12 ? 7 : 1;
        at += sprintf(at, "frame %d update 400 draw 1000\n", frame);
        at += sprintf(at, "frame %d child update update_entities 1 %d 150 3\n", frame, 200 * scale);
        at += sprintf(at, "frame %d child update ui_update 1 %d 50 3\n", frame, fixture->ui_update_ns);
        at += sprintf(at, "frame %d child update main_ui 1 %d 0 0\n", frame, fixture->main_ui_ns);
        at += sprintf(at, "frame %d child draw entity_pos@1c590#1 3 0 0 0\n", frame);
        at += sprintf(at, "frame %d child draw planet_draw_method 6 %d 400 20\n", frame, 500 * scale);
        at += sprintf(at, "frame %d child draw structure_icon_draw_method 4 100 80 4\n", frame);
        at += sprintf(at, "frame %d child draw slipway_draw_method 2 %d 300 10\n", frame, fixture->slipway_ns);
        at += sprintf(at, "frame %d self update 0\n", frame);
        at += sprintf(at, "frame %d self draw 100\n", frame);
        at = write_families(at, frame, fixture->spr_calls_pc, 400, 100);
        at += sprintf(at, "frame %d in-call 500\n", frame);
        at += sprintf(at, "frame %d child-family draw planet_draw_method spr 6 %d\n", frame, 300 * scale);
        at += sprintf(at, "frame %d child-family draw structure_icon_draw_method spr 4 100\n", frame);
        at += sprintf(at, "frame %d child-family draw slipway_draw_method line 10 100\n", frame);
    }
    write_file(path, text);
}

/* Rooted at cart_draw and input_step_run: draw_layers holds what the other profile splits. */
static void write_cart_profile(const char *path, const fixture_t *fixture) {
    char text[8192];
    char *at = text;
    at += sprintf(at, "profile run.json frames 15 hook-pair-ns 4.0 clock monotonic-ns\n");
    at += sprintf(at, "root draw cart_draw\nroot update input_step_run\n");
    for (int frame = 12; frame <= 14; frame++) {
        at += sprintf(at, "frame %d update 100 draw 1500\n", frame);
        at += sprintf(at, "frame %d child update pad_nav_step 1 90 80 0\n", frame);
        at += sprintf(at, "frame %d child draw draw_layers 1 %d 900 40\n", frame, 1100 * fixture->layers_scale);
        at += sprintf(at, "frame %d child draw hud_select_palette_block_default 1 200 150 2\n", frame);
        at += sprintf(at, "frame %d self update 10\n", frame);
        at += sprintf(at, "frame %d self draw 200\n", frame);
        at = write_families(at, frame, fixture->spr_calls_pc, 400, 100);
        at += sprintf(at, "frame %d in-call 500\n", frame);
        at += sprintf(at, "frame %d child-family draw draw_layers spr 10 %d\n", frame, 400 * fixture->layers_scale);
        at += sprintf(at, "frame %d child-family draw draw_layers line 10 %d\n", frame, 100 * fixture->layers_scale);
    }
    write_file(path, text);
}

static int run_profile(const fixture_t *fixture, const char *overhead, const char *groups) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(DIR "/map.txt", MAP);
    write_file(DIR "/groups.txt", groups);
    write_ttys(fixture);
    write_draw_profile(DIR "/draw.txt", fixture);
    write_cart_profile(DIR "/cart.txt", fixture);
    const char *draws = DIR "/draw.txt";
    if (fixture->outlier_slipway_ns != 0) {
        fixture_t outlier = *fixture;
        outlier.slipway_ns = fixture->outlier_slipway_ns;
        write_draw_profile(DIR "/draw2.txt", &outlier);
        write_draw_profile(DIR "/draw3.txt", fixture);
        draws = DIR "/draw.txt," DIR "/draw2.txt," DIR "/draw3.txt";
    }
    char command[1024];
    snprintf(command, sizeof command,
             SCRIPT " profile %s " DIR "/groups.txt " DIR "/map.txt " DIR "/plain.tty.txt " DIR "/breakdown.tty.txt %s " DIR
                    "/cart.txt > " OUT " 2>&1",
             overhead, draws);
    return run_command(command);
}

static int has_line(const char *line) {
    const char *text = read_file(OUT);
    size_t len = strlen(line);
    for (const char *at = strstr(text, line); at != NULL; at = strstr(at + 1, line)) {
        if ((at == text || at[-1] == '\n') && (at[len] == '\n' || at[len] == '\0')) {
            return 1;
        }
    }
    return 0;
}

static void expect_line(const char *line) {
    if (!has_line(line)) {
        fprintf(stderr, "test_steady_profile: no line '%s' in:\n%s", line, read_file(OUT));
        assert(0);
    }
}

/* Per frame: spr 40 - 10 x 1 = 30 shared 300:100 by PC ns, line 20 - 10 = 10 all to the slipways;
 * outside 100 - 60 = 40 shared 200:0:300:0:100:200:200 by PC time outside primitives; update 30
 * shared 100:200:100 (main_ui's 0 folded); plain total 80 + 30 + 5 = 115. */
static void test_a_profile_splits_the_measured_frame(void) {
    fixture_t fixture = fixture_default();
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("window run-frames 13-14 frames 2 overhead-per-call 1 calls 40");
    expect_line("group planets in-primitives 30.0 game-code 8.0 total 38.0 share 33.0%");
    expect_line("group slipways in-primitives 10.0 game-code 12.0 total 22.0 share 19.1%");
    expect_line("group hud in-primitives 0.0 game-code 8.0 total 8.0 share 7.0%");
    expect_line("group frame in-primitives 0.0 game-code 12.0 total 12.0 share 10.4%");
    expect_line("group particles in-primitives 0.0 game-code 0.0 total 0.0 share 0.0%");
    expect_line("function planet_draw_method group planets in-primitives 22.5 game-code 8.0 share 26.5%");
    expect_line("function structure_icon_draw_method group planets in-primitives 7.5 game-code 0.0 share 6.5%");
    expect_line("function slipway_draw_method group slipways in-primitives 10.0 game-code 12.0 share 19.1%");
    expect_line("function draw_layers group frame in-primitives 0.0 game-code 4.0 share 3.5%");
    expect_line("function cart_draw group frame in-primitives 0.0 game-code 8.0 share 7.0%");
    expect_line("function entity_pos group frame in-primitives 0.0 game-code 0.0 share 0.0%");
    expect_line("update input_step_run 7.5 share 6.5%");
    expect_line("update update_entities 15.0 share 13.0%");
    expect_line("update ui_update 7.5 share 6.5%");
    expect_line("update other 0.0 share 0.0%");
    expect_line("submit 5.0 share 4.3%");
    expect_line("adds-up ok sum 115.0 measured 115.0 residual 0.0 noise-floor 3.0");
}

/* Without the overhead, in-calls are 60 a frame, 20 more than plain draw: far past the floor of 3. */
static void test_the_overhead_is_removed_per_call(void) {
    fixture_t fixture = fixture_default();
    assert(run_profile(&fixture, "0", GROUPS) == 0);
    expect_line("group planets in-primitives 40.0 game-code 8.0 total 48.0 share 41.7%");
    expect_line("adds-up failed sum 135.0 measured 115.0 residual 20.0 noise-floor 3.0");
}

static void test_adds_up_within_3_hblanks_a_frame(void) {
    fixture_t fixture = fixture_default();
    fixture.plain_draw = 77;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("adds-up ok sum 115.0 measured 112.0 residual 3.0 noise-floor 3.0");
    fixture.plain_draw = 76;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("adds-up failed sum 115.0 measured 111.0 residual 4.0 noise-floor 3.0");
}

/* Breakdown draw 980: the sum is 35 + 980 - 20 = 995 a frame. */
static void test_the_floor_is_1_percent_of_a_large_frame(void) {
    fixture_t fixture = fixture_default();
    fixture.breakdown_draw = 980;
    fixture.plain_draw = 965;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("adds-up ok sum 995.0 measured 1000.0 residual -5.0 noise-floor 10.0");
    fixture.plain_draw = 976;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("adds-up failed sum 995.0 measured 1011.0 residual -16.0 noise-floor 10.1");
}

/* A third profile 5 times slower on the slipways moves nothing: the median is taken. */
static void test_pc_figures_are_medians_over_the_profiles(void) {
    fixture_t fixture = fixture_default();
    fixture.outlier_slipway_ns = 2000;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("function slipway_draw_method group slipways in-primitives 10.0 game-code 12.0 share 19.1%");
    expect_line("adds-up ok sum 115.0 measured 115.0 residual 0.0 noise-floor 3.0");
}

/* main_ui's 4 ns out of 400 is 0.3 hblanks, under 1% of 115. */
static void test_small_update_pieces_fold_into_other(void) {
    fixture_t fixture = fixture_default();
    fixture.ui_update_ns = 96;
    fixture.main_ui_ns = 4;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("update ui_update 7.2 share 6.3%");
    expect_line("update other 0.3 share 0.3%");
    assert(strstr(read_file(OUT), "update main_ui") == NULL);
}

/* draw_layers twice as slow in the cart profile: its functions weigh 1200 outside against 400 for
 * hud and cart_draw, so outside 40 goes 10:0:15:5 under it and 5:5 beside it. */
static void test_draw_layers_functions_are_scaled_to_the_cart_profile(void) {
    fixture_t fixture = fixture_default();
    fixture.layers_scale = 2;
    assert(run_profile(&fixture, "1", GROUPS) == 0);
    expect_line("function planet_draw_method group planets in-primitives 22.5 game-code 10.0 share 28.3%");
    expect_line("function slipway_draw_method group slipways in-primitives 10.0 game-code 15.0 share 21.7%");
    expect_line("function draw_layers group frame in-primitives 0.0 game-code 5.0 share 4.3%");
    expect_line("function hud_select_palette_block_default group hud in-primitives 0.0 game-code 5.0 share 4.3%");
    expect_line("function cart_draw group frame in-primitives 0.0 game-code 5.0 share 4.3%");
}

/* Frame 13 alone is a slipway-aim frame over 263; its draw 260 is shared by PC time, 500:100:400:0:100
 * under draw_layers and 200:200 beside it. */
static void test_a_situation_with_no_run_is_shared_by_pc_time_over_its_full_game_frames(void) {
    fixture_t fixture = fixture_default();
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(DIR "/groups.txt", GROUPS);
    write_file(DIR "/judge.txt", "frame update draw submit total vsyncs\n12 40 250 10 300 1\n13 30 260 10 300 1\n"
                                 "14 30 160 10 200 1\n");
    write_file(DIR "/situations.txt",
               "frame situation family\n12 map-rest steady\n13 slipway-aim steady\n14 slipway-aim steady\n");
    write_draw_profile(DIR "/draw.txt", &fixture);
    write_cart_profile(DIR "/cart.txt", &fixture);
    assert(run_command(SCRIPT " no-run slipway-aim 263 " DIR "/judge.txt " DIR "/situations.txt " DIR "/groups.txt " DIR
                              "/draw.txt " DIR "/cart.txt > " OUT " 2>&1") == 0);
    expect_line("no-run slipway-aim frames 1 budget 263 shares pc-time");
    expect_line("group planets total 104.0 share 34.7%");
    expect_line("function planet_draw_method group planets total 86.7 share 28.9%");
    expect_line("function hud_select_palette_block_default group hud total 34.7 share 11.6%");
    expect_line("update update_entities 15.0 share 5.0%");
    expect_line("submit 10.0 share 3.3%");
    expect_line("adds-up n/a no-run sum 300.0 measured 300.0");
}

/* Two runs of 2 window frames and 20 backend calls a frame (pattern apart): (40 + 60) / 80. */
static void test_the_overhead_is_pooled_over_runs(void) {
    fixture_t fixture = fixture_default();
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(DIR "/map.txt", MAP);
    write_ttys(&fixture);
    assert(system("mkdir -p " DIR "/second && mv " DIR "/*.tty.txt " DIR "/second/") == 0);
    fixture.breakdown_draw = 110;
    write_ttys(&fixture);
    assert(run_command(SCRIPT " overhead " DIR "/breakdown.tty.txt:" DIR "/plain.tty.txt:" DIR "/map.txt " DIR
                              "/second/breakdown.tty.txt:" DIR "/second/plain.tty.txt:" DIR "/map.txt > " OUT
                              " 2>&1") == 0);
    expect_line("overhead-per-call 1.2500 runs 2 calls 80 breakdown-draw 420 plain-draw 320");
}

static void test_a_symbol_missing_from_the_group_table_fails(void) {
    fixture_t fixture = fixture_default();
    assert(run_profile(&fixture, "1",
                       "planet_draw_method planets\nslipway_draw_method slipways\nentity_pos frame\n"
                       "hud_select_palette_block_default hud\ndraw_layers frame\ncart_draw frame\n") == 2);
    assert(strstr(read_file(OUT), "no group for symbol structure_icon_draw_method") != NULL);
    assert(run_profile(&fixture, "1", GROUPS "p8_cls nowhere\n") == 2);
}

static void test_a_calls_mismatch_fails(void) {
    fixture_t fixture = fixture_default();
    fixture.spr_calls_pc = 9;
    assert(run_profile(&fixture, "1", GROUPS) == 2);
    expect_line("calls-match differs frame 13 family spr pc 9 psx 10");
}

static void test_usage_errors_exit_1(void) {
    fixture_t fixture = fixture_default();
    assert(run_profile(&fixture, "x", GROUPS) == 1);
    assert(run_command(SCRIPT " profile 1 > " OUT " 2>&1") == 1);
    assert(strstr(read_file(OUT), "usage") != NULL);
    assert(run_command(SCRIPT " nothing > " OUT " 2>&1") == 1);
}

int main(void) {
    test_a_profile_splits_the_measured_frame();
    test_the_overhead_is_removed_per_call();
    test_adds_up_within_3_hblanks_a_frame();
    test_the_floor_is_1_percent_of_a_large_frame();
    test_pc_figures_are_medians_over_the_profiles();
    test_small_update_pieces_fold_into_other();
    test_draw_layers_functions_are_scaled_to_the_cart_profile();
    test_a_situation_with_no_run_is_shared_by_pc_time_over_its_full_game_frames();
    test_the_overhead_is_pooled_over_runs();
    test_a_symbol_missing_from_the_group_table_fails();
    test_a_calls_mismatch_fails();
    test_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    return 0;
}
