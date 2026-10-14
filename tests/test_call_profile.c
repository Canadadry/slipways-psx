#include "devtools/call_profile.h"

#include "devtools/draw_timing.h"
#include "p8/draw_backend.h"
#include "p8/gfx.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define STDERR_LOG "tmp/test_call_profile_stderr.txt"

enum {
    UPDATE = 0x1000,
    DRAW = 0x2000,
    CLS = 0x3000,
    LAYERS = 0x4000,
    LEAF = 0x5000,
    SITE_A = 0x9001,
    SITE_B = 0x9002,
    MAIN = 0x9999
};

static uint64_t now;
static uint64_t step;

static uint64_t fake_clock(void) {
    uint64_t reading = now;
    now += step;
    return reading;
}

static void set_clock(uint64_t ns) {
    now = ns;
}

static void fresh(void) {
    now = 0;
    step = 0;
    call_profile_init(fake_clock, UPDATE, DRAW);
}

static void test_a_direct_child_of_draw_gets_its_calls_and_time(void) {
    fresh();
    set_clock(100);
    call_profile_enter(DRAW, MAIN);
    set_clock(110);
    call_profile_enter(CLS, SITE_A);
    set_clock(140);
    call_profile_exit(CLS, SITE_A);
    set_clock(200);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    assert(call_profile_frame_count() == 1);
    assert(call_profile_root_ns(0, CALL_PROFILE_DRAW) == 100);
    assert(call_profile_root_ns(0, CALL_PROFILE_UPDATE) == 0);
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == 1);
    call_profile_child_t child = call_profile_child_info(CALL_PROFILE_DRAW, 0);
    assert(child.function == CLS && child.call_site == SITE_A);
    call_profile_child_figures_t figures = call_profile_child_figures(0, CALL_PROFILE_DRAW, 0);
    assert(figures.calls == 1 && figures.ns == 30 && figures.below == 0);
    assert(call_profile_self_ns(0, CALL_PROFILE_DRAW) == 70);
}

static void call(uintptr_t function, uintptr_t site) {
    call_profile_enter(function, site);
    call_profile_exit(function, site);
}

static void name_of(call_profile_root_t root, int child, const char *symbol, const char *expected) {
    char buffer[64];
    call_profile_child_name(call_profile_child_info(root, child), symbol, buffer, sizeof buffer);
    if (strcmp(buffer, expected) != 0) {
        fprintf(stderr, "child %d named %s, expected %s\n", child, buffer, expected);
        assert(0 && "child name");
    }
}

static void test_one_function_from_two_sites_is_two_children_numbered_in_first_entered_order(void) {
    fresh();
    call_profile_enter(DRAW, MAIN);
    call(CLS, SITE_A);
    call(LAYERS, SITE_B);
    call(LAYERS, SITE_A);
    call(LAYERS, SITE_B);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == 3);
    name_of(CALL_PROFILE_DRAW, 0, "p8_cls", "p8_cls");
    name_of(CALL_PROFILE_DRAW, 1, "draw_layers", "draw_layers#1");
    name_of(CALL_PROFILE_DRAW, 2, "draw_layers", "draw_layers#2");
    assert(call_profile_child_info(CALL_PROFILE_DRAW, 1).call_site == SITE_B);
    assert(call_profile_child_figures(0, CALL_PROFILE_DRAW, 1).calls == 2);
    assert(call_profile_child_figures(0, CALL_PROFILE_DRAW, 2).calls == 1);
}

static void test_the_same_function_under_the_other_root_keeps_its_bare_name(void) {
    fresh();
    call_profile_enter(UPDATE, MAIN);
    call(LAYERS, SITE_A);
    call_profile_exit(UPDATE, MAIN);
    call_profile_enter(DRAW, MAIN);
    call(LAYERS, SITE_B);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    name_of(CALL_PROFILE_UPDATE, 0, "draw_layers", "draw_layers");
    name_of(CALL_PROFILE_DRAW, 0, "draw_layers", "draw_layers");
}

static void test_deeper_calls_are_not_children_but_count_as_below(void) {
    fresh();
    call_profile_enter(DRAW, MAIN);
    call_profile_enter(LAYERS, SITE_A);
    call_profile_enter(LEAF, SITE_B);
    call(CLS, SITE_A);
    call_profile_exit(LEAF, SITE_B);
    call(LEAF, SITE_B);
    call_profile_exit(LAYERS, SITE_A);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == 1);
    call_profile_child_figures_t figures = call_profile_child_figures(0, CALL_PROFILE_DRAW, 0);
    assert(figures.calls == 1 && figures.below == 3);
}

static void test_calls_outside_any_root_are_ignored(void) {
    fresh();
    set_clock(10);
    call_profile_enter(LAYERS, MAIN);
    call(CLS, SITE_A);
    set_clock(50);
    call_profile_exit(LAYERS, MAIN);
    call(LEAF, MAIN);
    call_profile_frame_end();
    assert(call_profile_child_count(CALL_PROFILE_UPDATE) == 0);
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == 0);
    assert(call_profile_root_ns(0, CALL_PROFILE_UPDATE) == 0 && call_profile_root_ns(0, CALL_PROFILE_DRAW) == 0);
}

static void test_a_root_called_from_deep_in_the_harness_is_still_a_root(void) {
    fresh();
    call_profile_enter(LEAF, MAIN);
    call_profile_enter(UPDATE, SITE_A);
    call(CLS, SITE_B);
    call_profile_exit(UPDATE, SITE_A);
    call_profile_exit(LEAF, MAIN);
    call_profile_frame_end();
    assert(call_profile_child_count(CALL_PROFILE_UPDATE) == 1);
    assert(call_profile_child_figures(0, CALL_PROFILE_UPDATE, 0).calls == 1);
}

static void test_a_child_first_seen_late_has_zeros_on_earlier_frames(void) {
    fresh();
    call_profile_enter(DRAW, MAIN);
    call(CLS, SITE_A);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    call_profile_enter(DRAW, MAIN);
    call(CLS, SITE_A);
    call(LAYERS, SITE_B);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    call_profile_frame_end();
    assert(call_profile_frame_count() == 3);
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == 2);
    assert(call_profile_child_figures(0, CALL_PROFILE_DRAW, 1).calls == 0);
    assert(call_profile_child_figures(1, CALL_PROFILE_DRAW, 1).calls == 1);
    assert(call_profile_child_figures(2, CALL_PROFILE_DRAW, 0).calls == 0 && "frame end starts an empty frame");
    assert(call_profile_child_figures(2, CALL_PROFILE_DRAW, 1).calls == 0);
}

static void test_update_and_draw_times_on_one_frame(void) {
    fresh();
    set_clock(1000);
    call_profile_enter(UPDATE, MAIN);
    set_clock(1010);
    call_profile_enter(LAYERS, SITE_A);
    set_clock(1020);
    call(LEAF, SITE_B);
    set_clock(1050);
    call_profile_exit(LAYERS, SITE_A);
    set_clock(1060);
    call_profile_enter(CLS, SITE_B);
    set_clock(1065);
    call_profile_exit(CLS, SITE_B);
    set_clock(1100);
    call_profile_exit(UPDATE, MAIN);
    set_clock(2000);
    call_profile_enter(DRAW, MAIN);
    set_clock(2500);
    call_profile_exit(DRAW, MAIN);
    set_clock(3000);
    call_profile_enter(DRAW, MAIN);
    set_clock(3100);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    assert(call_profile_root_ns(0, CALL_PROFILE_UPDATE) == 100);
    assert(call_profile_child_figures(0, CALL_PROFILE_UPDATE, 0).ns == 40);
    assert(call_profile_child_figures(0, CALL_PROFILE_UPDATE, 1).ns == 5);
    assert(call_profile_self_ns(0, CALL_PROFILE_UPDATE) == 55);
    assert(call_profile_root_ns(0, CALL_PROFILE_DRAW) == 600 && "two draws in a frame add up");
    assert(call_profile_self_ns(0, CALL_PROFILE_DRAW) == 600);
}

static void test_net_time_takes_off_one_pair_cost_per_call_below(void) {
    call_profile_child_figures_t figures = { 1, 1000, 30 };
    assert(call_profile_net_ns(figures, 125) == 625 && "30 pairs at 12.5 ns");
    assert(call_profile_net_ns(figures, 0) == 1000);
    figures.below = 81;
    assert(call_profile_net_ns(figures, 125) == 0 && "floored at 0");
    figures.below = 80;
    assert(call_profile_net_ns(figures, 125) == 0);
}

static void test_the_pair_cost_is_the_clock_span_over_the_pairs_in_tenths(void) {
    fresh();
    step = 2345;
    assert(call_profile_measure_pair(100) == 234 && "2345 ns over 100 pairs is 23.4 ns");
    step = 0;
    call_profile_frame_end();
    assert(call_profile_child_count(CALL_PROFILE_UPDATE) == 0 && call_profile_child_count(CALL_PROFILE_DRAW) == 0);
    assert(call_profile_frame_count() == 1 && call_profile_root_ns(0, CALL_PROFILE_DRAW) == 0);
    call_profile_enter(DRAW, MAIN);
    call(CLS, SITE_A);
    call_profile_exit(DRAW, MAIN);
    call_profile_frame_end();
    assert(call_profile_child_figures(1, CALL_PROFILE_DRAW, 0).calls == 1 && "the stack is left empty");
}

static uint64_t backend_cost;

static void null_cls(p8_vram_t *mem) {
    (void)mem;
    now += backend_cost;
}

static void null_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    (void)gfx, (void)mem, (void)x, (void)y, (void)col;
    now += backend_cost;
}

static void null_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)gfx, (void)mem, (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
    now += backend_cost;
}

static void null_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)gfx, (void)mem, (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
    now += backend_cost;
}

static void null_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)gfx, (void)mem, (void)cx, (void)cy, (void)r, (void)col;
    now += backend_cost;
}

static void null_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    (void)gfx, (void)mem, (void)n, (void)x, (void)y, (void)w, (void)h;
    now += backend_cost;
}

static void null_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)gfx, (void)mem, (void)str, (void)x, (void)y, (void)col;
    now += backend_cost;
}

static const p8_draw_backend_t NULL_BACKEND = {
    null_cls, null_pset, null_line, null_rect, null_rect,
    null_circ, null_circ, null_spr, null_print,
};

static p8_vram_t mem;
static p8_graphics_t gfx;

static void families_over(const p8_draw_backend_t *backend) {
    fresh();
    backend_cost = 0;
    p8_vram_init(&mem);
    p8_gfx_init(&gfx);
    p8_draw_backend_set(backend);
    const p8_draw_backend_t *wrapped = call_profile_family_install();
    assert(wrapped == backend);
    assert(p8_draw_backend_get() != backend);
}

static void test_a_backend_call_adds_its_time_and_one_call_to_its_family(void) {
    families_over(&NULL_BACKEND);
    backend_cost = 7;
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    backend_cost = 3;
    p8_print(&gfx, &mem, "ab", 0, 0, 7);
    call_profile_frame_end();
    call_profile_family_figures_t spr = call_profile_family_figures(0, DRAW_FILTER_SPR);
    call_profile_family_figures_t print = call_profile_family_figures(0, DRAW_FILTER_PRINT);
    assert(spr.calls == 2 && spr.ns == 14);
    assert(print.calls == 1 && print.ns == 3);
    assert(call_profile_family_figures(0, DRAW_FILTER_CLS).calls == 0);
    assert(call_profile_in_call_ns(0) == 17);
    call_profile_frame_end();
    assert(call_profile_family_figures(1, DRAW_FILTER_SPR).calls == 0 && "frame end starts an empty frame");
    p8_draw_backend_set(NULL);
}

static void test_a_patterned_fill_also_counts_as_pattern_with_its_time_inside_its_shape(void) {
    families_over(&NULL_BACKEND);
    p8_fillp(&gfx, 0xA5A5);
    backend_cost = 30;
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    backend_cost = 40;
    p8_circfill(&gfx, &mem, 10, 10, 3, 7);
    backend_cost = 5;
    p8_rect(&gfx, &mem, 0, 0, 5, 5, 7);
    p8_fillp_reset(&gfx);
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    call_profile_frame_end();
    assert(call_profile_family_figures(0, DRAW_FILTER_RECTFILL).calls == 2);
    assert(call_profile_family_figures(0, DRAW_FILTER_RECTFILL).ns == 35);
    assert(call_profile_family_figures(0, DRAW_FILTER_CIRCFILL).ns == 40);
    call_profile_family_figures_t pattern = call_profile_family_figures(0, DRAW_FILTER_PATTERN);
    assert(pattern.calls == 2 && pattern.ns == 70);
    assert(call_profile_in_call_ns(0) == 80 && "pattern time is not added twice");
    p8_draw_backend_set(NULL);
}

static uint16_t timing_ticks(void) {
    return 0;
}

static const draw_timing_clock_t TIMING_CLOCK = { timing_ticks, timing_ticks, 200 };

static void draw_every_family_with_and_without_pattern(void) {
    p8_cls(&mem);
    for (int pass = 0; pass < 2; pass++) {
        p8_pset(&gfx, &mem, 1, 2, 3);
        p8_line(&gfx, &mem, 1, 2, 3, 4, 5);
        p8_rect(&gfx, &mem, 1, 2, 3, 4, 6);
        p8_rectfill(&gfx, &mem, 1, 2, 3, 4, 7);
        p8_circ(&gfx, &mem, 10, 11, 3, 8);
        p8_circfill(&gfx, &mem, 12, 13, 4, 9);
        p8_spr(&gfx, &mem, 5, 6, 7, 1, 2);
        p8_print(&gfx, &mem, "ab", 8, 9, 10);
        p8_circfill(&gfx, &mem, 12, 13, 0, 9);
        p8_fillp(&gfx, 0x5A5A);
    }
    p8_fillp_transparent(&gfx, 0x0F0F);
    p8_rectfill(&gfx, &mem, 1, 2, 3, 4, 7);
    p8_fillp(&gfx, 0x10000);
    p8_circfill(&gfx, &mem, 12, 13, 4, 9);
    p8_fillp_reset(&gfx);
    p8_cls(&mem);
}

static void test_family_calls_equal_the_breakdown_timing_on_the_same_calls(void) {
    families_over(&NULL_BACKEND);
    p8_draw_backend_set(&NULL_BACKEND);
    draw_timing_install(&TIMING_CLOCK);
    call_profile_family_install();
    draw_every_family_with_and_without_pattern();
    draw_every_family_with_and_without_pattern();
    call_profile_frame_end();
    draw_timing_frame_end();
    draw_timing_frame_t breakdown = draw_timing_get(0);
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        uint32_t calls = call_profile_family_figures(0, (draw_filter_family_t)family).calls;
        if (calls != breakdown.calls[family]) {
            fprintf(stderr, "family %d: profile %u, breakdown %u\n", family, (unsigned)calls,
                    (unsigned)breakdown.calls[family]);
            assert(0 && "the two counting rules drifted apart");
        }
        assert(calls > 0 && "the script reaches every family");
    }
    p8_draw_backend_set(NULL);
}

static int case_frames(void) {
    fresh();
    for (int frame = 0; frame < CALL_PROFILE_FRAME_CAP; frame++) call_profile_frame_end();
    assert(call_profile_frame_count() == CALL_PROFILE_FRAME_CAP);
    call_profile_frame_end();
    return 0;
}

static int case_children(void) {
    fresh();
    call_profile_enter(DRAW, MAIN);
    for (int child = 0; child < CALL_PROFILE_CHILD_CAP; child++) call(LEAF, SITE_A + (uintptr_t)child);
    assert(call_profile_child_count(CALL_PROFILE_DRAW) == CALL_PROFILE_CHILD_CAP);
    call(CLS, SITE_A);
    return 0;
}

static int case_depth(void) {
    fresh();
    for (int level = 0; level < CALL_PROFILE_DEPTH_CAP; level++) call_profile_enter(LEAF, SITE_A);
    call_profile_enter(LEAF, SITE_A);
    return 0;
}

static int case_unbalanced(void) {
    fresh();
    call_profile_exit(LEAF, SITE_A);
    return 0;
}

static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    char *text = calloc(1, 4096);
    assert(text != NULL);
    size_t read = fread(text, 1, 4095, file);
    (void)read;
    fclose(file);
    return text;
}

static void expect_fatal(const char *self_path, const char *name, const char *message) {
    char cmd[512];
    remove(STDERR_LOG);
    snprintf(cmd, sizeof cmd, "%s %s > /dev/null 2> " STDERR_LOG, self_path, name);
    int status = system(cmd);
    assert(status != -1 && WIFEXITED(status));
    char *err = read_whole_file(STDERR_LOG);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: expected exit 3 and \"%s\", exit %d\n%s", name, message, WEXITSTATUS(status), err);
        assert(0 && "a cap overrun or an unbalanced exit must call p8_fatal");
    }
    free(err);
    remove(STDERR_LOG);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "frames") == 0) return case_frames();
        if (strcmp(argv[1], "children") == 0) return case_children();
        if (strcmp(argv[1], "depth") == 0) return case_depth();
        if (strcmp(argv[1], "unbalanced") == 0) return case_unbalanced();
        fprintf(stderr, "test_call_profile: unknown case %s\n", argv[1]);
        return 2;
    }
    test_a_direct_child_of_draw_gets_its_calls_and_time();
    test_one_function_from_two_sites_is_two_children_numbered_in_first_entered_order();
    test_the_same_function_under_the_other_root_keeps_its_bare_name();
    test_deeper_calls_are_not_children_but_count_as_below();
    test_calls_outside_any_root_are_ignored();
    test_a_root_called_from_deep_in_the_harness_is_still_a_root();
    test_a_child_first_seen_late_has_zeros_on_earlier_frames();
    test_update_and_draw_times_on_one_frame();
    test_net_time_takes_off_one_pair_cost_per_call_below();
    test_the_pair_cost_is_the_clock_span_over_the_pairs_in_tenths();
    test_a_backend_call_adds_its_time_and_one_call_to_its_family();
    test_a_patterned_fill_also_counts_as_pattern_with_its_time_inside_its_shape();
    test_family_calls_equal_the_breakdown_timing_on_the_same_calls();
    expect_fatal(argv[0], "frames", "call profile frame table full at frame 6817, capacity 6817");
    expect_fatal(argv[0], "children", "call profile: more than 16 children under one root");
    expect_fatal(argv[0], "depth", "call profile: call stack deeper than 256");
    expect_fatal(argv[0], "unbalanced", "call profile: exit without its entry");
    printf("test_call_profile: ok\n");
    return 0;
}
