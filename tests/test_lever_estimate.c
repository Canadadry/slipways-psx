/* tools/lever_estimate.sh on hand-written frame and situation tables and a markdown document holding
 * a lever-table block: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/lever_estimate.sh"
#define DIR "tmp/test_lever_estimate_work"
#define FRAMES DIR "/frames.txt"
#define SITUATIONS DIR "/situations.txt"
#define DOC DIR "/levers.md"
#define PROJECTED DIR "/projected"
#define OUT DIR "/out.txt"

#define FRAMES_HEAD "frame update draw submit total vsyncs\n"
#define SITUATIONS_HEAD "frame situation family\n"

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

/* Writes the two tables and the document (raw markdown), runs the script with budget 100 and
 * one-off allowance 50, then `args`; output and errors in OUT, returns the exit status. */
static int run_doc(const char *frames, const char *situations, const char *doc, const char *args) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(FRAMES, frames);
    write_file(SITUATIONS, situations);
    write_file(DOC, doc);
    char command[1024];
    snprintf(command, sizeof command, SCRIPT " " FRAMES " " SITUATIONS " " DOC " 100 50 projected=" PROJECTED " %s > " OUT
             " 2>&1", args);
    return run_command(command);
}

/* The document is prose around one lever-table block holding `table`. */
static int run_table(const char *frames, const char *situations, const char *table, const char *args) {
    static char doc[1 << 14];
    snprintf(doc, sizeof doc, "# Levers\n\nProse.\n\n```lever-table\n%s```\n\nMore prose.\n", table);
    return run_doc(frames, situations, doc, args);
}

static int has_line(const char *path, const char *line) {
    const char *text = read_file(path);
    size_t len = strlen(line);
    for (const char *at = strstr(text, line); at != NULL; at = strstr(at + 1, line)) {
        if ((at == text || at[-1] == '\n') && (at[len] == '\n' || at[len] == '\0')) {
            return 1;
        }
    }
    return 0;
}

static void expect_line_in(const char *path, const char *line) {
    if (!has_line(path, line)) {
        fprintf(stderr, "test_lever_estimate: no line '%s' in %s:\n%s", line, path, read_file(path));
        assert(0);
    }
}

static void expect_line(const char *line) { expect_line_in(OUT, line); }

/* Frames 0 title 50, 1 map-rest 90, 2 colonise (one-off) 300, 3 map-rest 160: steady mean 100. */
#define MEAN_FRAMES FRAMES_HEAD "0 10 37 3 50 0\n1 10 77 3 90 0\n2 200 97 3 300 3\n3 10 147 3 160 1\n"
#define MEAN_SITUATIONS \
    SITUATIONS_HEAD "0 title steady\n1 map-rest steady\n2 colonise one-off\n3 map-rest steady\n"

static void test_mean_cost_and_threshold_over_steady_frames_only(void) {
    assert(run_table(MEAN_FRAMES, MEAN_SITUATIONS, "", "") == 0);
    expect_line("mean-cost steady 100.0 threshold 2.0");
    expect_line("situation-threshold map-rest mean 125.0 threshold 2.5");
    expect_line("start steady-over 1 one-off-vsyncs 3");
    expect_line("in-base none");
}

/* Frame 1 just over (draw 85 -> 76.5, rounded up to 77), frame 2 far over. */
#define TWO_FRAMES FRAMES_HEAD "0 10 37 3 50 0\n1 10 85 6 101 2\n2 10 150 5 165 2\n"
#define TWO_SITUATIONS SITUATIONS_HEAD "0 title steady\n1 map-rest steady\n2 map-rest steady\n"

static void test_one_lever_turns_a_frame_just_over_green_and_rounds_half_up(void) {
    assert(run_table(TWO_FRAMES, TWO_SITUATIONS,
                     "lever N1 estimated S basis calls measure map-move-1057\n"
                     "gain N1 map-rest draw 10.0\n",
                     "") == 0);
    expect_line("start steady-over 2 one-off-vsyncs 0");
    expect_line("lever 1 N1 estimated basis calls steady-over 1 pessimistic 1 saved 23");
    expect_line("situation map-rest over 2 end 1 pessimistic 1");
    expect_line("situation title over 0 end 0 pessimistic 0");
    expect_line("order ok");
    expect_line("end steady-over 1 pessimistic 1");
    expect_line_in(PROJECTED "/nominal-frames.txt", "0 10 37 3 50 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "1 10 77 6 93 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "2 10 135 5 150 2");
}

static void test_gains_on_one_part_add_up_and_stop_at_100_percent(void) {
    assert(run_table(FRAMES_HEAD "0 10 200 5 215 3\n1 10 200 5 215 3\n", SITUATIONS_HEAD "0 map-move steady\n1 build steady\n",
                     "lever N1 estimated S basis share measure map-move-1057\n"
                     "gain N1 map-move draw 60.0\n"
                     "gain N1 build draw 30.0\n"
                     "lever N2 estimated S basis share measure map-move-1057\n"
                     "gain N2 map-move draw 70.0\n"
                     "gain N2 build draw 30.0\n",
                     "") != 2);
    expect_line_in(PROJECTED "/nominal-frames.txt", "0 10 0 5 15 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "1 10 80 5 95 0");
}

static void test_one_off_and_title_frames_never_change(void) {
    assert(run_table(FRAMES_HEAD "0 10 200 5 215 3\n1 300 200 5 505 6\n2 10 200 5 215 3\n3 10 80 5 95 0\n",
                     SITUATIONS_HEAD "0 map-rest steady\n1 colonise one-off\n2 build steady\n3 title steady\n",
                     "lever N1 estimated S basis share measure map-move-1057\n"
                     "gain N1 title draw 50.0\n"
                     "gain N1 map-rest update 50.0\ngain N1 map-rest draw 50.0\ngain N1 map-rest submit 50.0\n"
                     "gain N1 build update 50.0\ngain N1 build draw 50.0\ngain N1 build submit 50.0\n",
                     "") == 0);
    expect_line_in(PROJECTED "/nominal-frames.txt", "0 5 100 3 108 2");
    expect_line_in(PROJECTED "/nominal-frames.txt", "1 300 200 5 505 6");
    expect_line_in(PROJECTED "/pessimistic-frames.txt", "1 300 200 5 505 6");
    expect_line_in(PROJECTED "/nominal-frames.txt", "3 10 80 5 95 0");
    expect_line("start steady-over 2 one-off-vsyncs 6");
}

/* Draw 100 less 10% fits the budget of 100; less 8% does not. */
static void test_the_pessimistic_variant_cuts_every_gain_by_a_fifth(void) {
    assert(run_table(FRAMES_HEAD "0 10 100 0 110 2\n", SITUATIONS_HEAD "0 menu steady\n",
                     "lever N1 estimated S basis calls measure menu-5954\n"
                     "gain N1 menu draw 10.0\n",
                     "") == 0);
    expect_line("lever 1 N1 estimated basis calls steady-over 0 pessimistic 1 saved 10");
    expect_line("end steady-over 0 pessimistic 1");
    expect_line_in(PROJECTED "/pessimistic-frames.txt", "0 10 92 0 102 2");
    assert(run_table(FRAMES_HEAD "0 10 100 0 110 2\n", SITUATIONS_HEAD "0 menu steady\n",
                     "lever N1 estimated S basis calls measure menu-5954\n"
                     "gain N1 menu draw 10.0\n",
                     "haircut=0") == 0);
    expect_line("end steady-over 0 pessimistic 0");
}

/* One frame per situation, draw 100: 20% off with calls halved is 90, share 80, measured 80. */
#define FOUR_FRAMES FRAMES_HEAD "0 0 100 0 100 0\n1 0 100 0 100 0\n2 0 100 0 100 0\n3 0 100 0 100 0\n"
#define FOUR_SITUATIONS SITUATIONS_HEAD "0 map-rest steady\n1 map-move steady\n2 build steady\n3 menu steady\n"

static void test_a_scale_multiplies_only_the_estimated_gains_of_its_basis(void) {
    assert(run_table(FOUR_FRAMES, FOUR_SITUATIONS,
                     "scale calls 0.5\n"
                     "lever N1 estimated S basis calls measure map-move-1057\ngain N1 map-rest draw 20.0\n"
                     "lever N2 estimated S basis share measure map-move-1057\ngain N2 map-move draw 20.0\n"
                     "lever N3 measured S basis calls measure map-move-1057\ngain N3 build draw 20.0\n"
                     "lever N4 in-base S basis calls measure map-move-1057\ngain N4 menu draw 20.0\n",
                     "haircut=0") != 2);
    expect_line_in(PROJECTED "/nominal-frames.txt", "0 0 90 0 90 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "1 0 80 0 80 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "2 0 80 0 80 0");
    expect_line_in(PROJECTED "/nominal-frames.txt", "3 0 100 0 100 0");
}

static void test_an_in_base_lever_is_not_applied_nor_ranked_and_satisfies_after(void) {
    assert(run_table(TWO_FRAMES, TWO_SITUATIONS,
                     "lever D11 in-base M basis calls measure map-move-1057\n"
                     "gain D11 map-rest draw 90.0\n"
                     "lever N1 estimated S basis calls measure map-move-1057 after D11\n"
                     "gain N1 map-rest draw 10.0\n",
                     "") == 0);
    expect_line("in-base D11");
    expect_line("lever 1 N1 estimated basis calls steady-over 1 pessimistic 1 saved 23");
    expect_line("order ok");
    assert(strstr(read_file(OUT), "lever 2 ") == NULL);
}

/* Frame 1 map-rest 130 (draw 120): A (10%) or B (20%) alone leave it over, both fit it. Frame 2
 * build 105 (draw 95): C (10%) fits it alone. */
#define RANK_FRAMES FRAMES_HEAD "0 10 37 3 50 0\n1 10 120 0 130 2\n2 10 95 0 105 2\n"
#define RANK_SITUATIONS SITUATIONS_HEAD "0 title steady\n1 map-rest steady\n2 build steady\n"
#define LEVER_A "lever A estimated S basis calls measure map-move-1057\ngain A map-rest draw 10.0\n"
#define LEVER_B "lever B estimated S basis calls measure map-move-1057\ngain B map-rest draw 20.0\n"
#define LEVER_C "lever C estimated S basis calls measure map-move-1057\ngain C build draw 10.0\n"

static void test_greedy_ranking_counts_a_lever_by_its_effect_in_sequence(void) {
    assert(run_table(RANK_FRAMES, RANK_SITUATIONS, LEVER_C LEVER_B LEVER_A, "") == 0);
    expect_line("lever 1 C estimated basis calls steady-over 1 pessimistic 1 saved 9");
    expect_line("lever 2 B estimated basis calls steady-over 1 pessimistic 1 saved 33");
    expect_line("lever 3 A estimated basis calls steady-over 0 pessimistic 1 saved 45");
    expect_line("order ok");
}

static void test_a_stale_document_order_exits_3_naming_the_first_difference(void) {
    assert(run_table(RANK_FRAMES, RANK_SITUATIONS, LEVER_C LEVER_A LEVER_B, "") == 3);
    expect_line("order differs rank 2 document A computed B");
    expect_line("end steady-over 0 pessimistic 1");
    assert(strstr(read_file(OUT), "nominal: real-game-budget ok") != NULL);
}

static void test_ties_go_to_hblanks_saved_then_cost_then_id(void) {
    /* Each lever fits one 101 frame: Z saves the most; X and Y tie up to their id; V costs M. */
    assert(run_table(FRAMES_HEAD "0 0 101 0 101 2\n1 0 101 0 101 2\n2 0 101 0 101 2\n3 0 101 0 101 2\n", FOUR_SITUATIONS,
                     "lever Z estimated S basis calls measure map-move-1057\ngain Z map-rest draw 50.0\n"
                     "lever Y estimated S basis calls measure map-move-1057\ngain Y map-move draw 10.0\n"
                     "lever X estimated S basis calls measure map-move-1057\ngain X build draw 10.0\n"
                     "lever V estimated M basis calls measure map-move-1057\ngain V menu draw 10.0\n",
                     "") == 3);
    expect_line("lever 1 Z estimated basis calls steady-over 3 pessimistic 3 saved 50");
    assert(strstr(read_file(OUT), "lever 2 X ") != NULL);
    assert(strstr(read_file(OUT), "lever 3 Y ") != NULL);
    assert(strstr(read_file(OUT), "lever 4 V ") != NULL);
}

static void test_after_holds_a_lever_behind_another(void) {
    assert(run_table(RANK_FRAMES, RANK_SITUATIONS,
                     "lever P estimated S basis share measure map-move-1057\ngain P map-rest update 1.0\n"
                     "lever Q estimated S basis share measure map-move-1057 after P\ngain Q build draw 50.0\n",
                     "") == 0);
    expect_line("order ok");
    assert(strstr(read_file(OUT), "lever 1 P ") != NULL);
    assert(strstr(read_file(OUT), "lever 2 Q ") != NULL);
}

static void test_the_projected_tables_pass_the_real_game_budget_judge(void) {
    assert(run_table(TWO_FRAMES, TWO_SITUATIONS,
                     "lever N1 estimated S basis calls measure map-move-1057\n"
                     "gain N1 map-rest draw 10.0\n",
                     "") == 0);
    expect_line("nominal: real-game steady-over 1 frames, worst steady frame 2 total 150 situation map-rest");
    expect_line("nominal: real-game one-off-over 0 frames, vsyncs 0 of 50");
    expect_line("nominal: real-game-budget failed 1 steady frames over 100, one-off vsyncs 0 of 50");
    expect_line("pessimistic: real-game steady-over 1 frames, worst steady frame 2 total 153 situation map-rest");
    expect_line("pessimistic: real-game-budget failed 1 steady frames over 100, one-off vsyncs 0 of 50");
}

static void expect_malformed(const char *table, const char *reason) {
    assert(run_table(TWO_FRAMES, TWO_SITUATIONS, table, "") == 2);
    if (strstr(read_file(OUT), DOC ":") == NULL || strstr(read_file(OUT), reason) == NULL) {
        fprintf(stderr, "test_lever_estimate: no '%s' naming " DOC " in:\n%s", reason, read_file(OUT));
        assert(0);
    }
}

#define GOOD_LEVER "lever N1 estimated S basis calls measure map-move-1057\n"

static void test_a_malformed_block_exits_2_naming_the_document_and_line(void) {
    assert(run_doc(TWO_FRAMES, TWO_SITUATIONS, "# No block\n", "") == 2);
    assert(strstr(read_file(OUT), "no lever-table block") != NULL);
    assert(run_doc(TWO_FRAMES, TWO_SITUATIONS, "```lever-table\n```\n\n```lever-table\n```\n", "") == 2);
    assert(strstr(read_file(OUT), DOC ":4: a second lever-table block") != NULL);
    expect_malformed(GOOD_LEVER "gain N1 colonise draw 1.0\n", ":7: situation colonise is one-off");
    expect_malformed(GOOD_LEVER "gain N1 map-wander draw 1.0\n", "unknown situation map-wander");
    expect_malformed(GOOD_LEVER "gain N2 map-rest draw 1.0\n", ":7: gain for undeclared lever N2");
    expect_malformed(GOOD_LEVER "gain N1 map-rest draw 100.1\n", "percent is not from 0 to 100");
    expect_malformed("lever A estimated S basis calls measure map-move-1057 after B\n"
                     "lever B estimated S basis calls measure map-move-1057 after A\n",
                     "after cycle");
    expect_malformed("lever A estimated S basis calls measure map-move-1057 after B\n", "after unknown lever B");
    expect_malformed("lever A estimated S basis calls measure psx-full-game-check\n", "without a probe run");
    expect_malformed("lever A estimated S basis calls measure map-move-1057 probe menu-5954\n", "has a probe run");
    expect_malformed("scale calls 0.5\nscale calls 0.6\n", ":7: a second scale for calls");
    expect_malformed("scale share 0\n", "ratio is not a positive number");
    expect_malformed("lever A estimated S basis guess measure map-move-1057\n", "basis is not calls or share");
}

static void test_a_join_mismatch_exits_2_and_usage_errors_exit_1(void) {
    assert(run_table(TWO_FRAMES, SITUATIONS_HEAD "0 title steady\n1 map-rest steady\n", GOOD_LEVER, "") == 2);
    assert(strstr(read_file(OUT), SITUATIONS) != NULL);
    assert(run_table(TWO_FRAMES, SITUATIONS_HEAD "0 title steady\n2 map-rest steady\n3 map-rest steady\n", GOOD_LEVER,
                     "") == 2);
    assert(run_table(TWO_FRAMES, TWO_SITUATIONS, GOOD_LEVER, "haircut=120") == 1);
    assert(run_command(SCRIPT " " FRAMES " > " OUT " 2>&1") == 1);
    assert(strstr(read_file(OUT), "usage") != NULL);
}

int main(void) {
    test_mean_cost_and_threshold_over_steady_frames_only();
    test_one_lever_turns_a_frame_just_over_green_and_rounds_half_up();
    test_gains_on_one_part_add_up_and_stop_at_100_percent();
    test_one_off_and_title_frames_never_change();
    test_the_pessimistic_variant_cuts_every_gain_by_a_fifth();
    test_a_scale_multiplies_only_the_estimated_gains_of_its_basis();
    test_an_in_base_lever_is_not_applied_nor_ranked_and_satisfies_after();
    test_greedy_ranking_counts_a_lever_by_its_effect_in_sequence();
    test_a_stale_document_order_exits_3_naming_the_first_difference();
    test_ties_go_to_hblanks_saved_then_cost_then_id();
    test_after_holds_a_lever_behind_another();
    test_the_projected_tables_pass_the_real_game_budget_judge();
    test_a_malformed_block_exits_2_naming_the_document_and_line();
    test_a_join_mismatch_exits_2_and_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    return 0;
}
