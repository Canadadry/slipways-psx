/* tools/lever_estimate.sh on hand-written judge tables, spike frames and gain tables: no profile, no
 * emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/lever_estimate.sh"
#define SPREAD_SCRIPT "sh tools/frame_profile_check.sh spread"
#define DIR "tmp/test_lever_estimate_work"
#define JUDGE DIR "/judge.txt"
#define SPIKES DIR "/spikes.txt"
#define GAINS DIR "/gains.txt"
#define LOG DIR "/log.txt"
#define FILES JUDGE " " SPIKES " " GAINS

#define TEXT_CAP (1 << 18)

#define JUDGE_HEADER "frame update draw submit total vsyncs\n"

/* Frame 1 spikes on update 300; frames 2 and 3 leave 60 and 110 of room under 263. */
#define SMALL_JUDGE JUDGE_HEADER "0 100 100 3 203 0\n1 300 50 3 353 1\n2 100 100 3 203 0\n3 50 100 3 153 0\n4 100 100 3 203 0\n"

#define BASE_GAINS                                                                                                \
    "budget 263\nclass C1 cart_update60/ui_update\nclass C2 cart_update60/save_at_frame_end\n"                  \
    "spread S measured * responsible\n"

static void append(char *buf, size_t cap, const char *text) {
    size_t len = strlen(buf);
    assert(len + strlen(text) < cap);
    memcpy(buf + len, text, strlen(text) + 1);
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[TEXT_CAP];
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

static int run_command(const char *cmd) {
    int status = system(cmd);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void fresh_dir(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

/* Fresh inputs: <spikes> may be "" (no spike frame). */
static void write_inputs(const char *judge, const char *spikes, const char *gains) {
    fresh_dir();
    write_file(JUDGE, judge);
    write_file(SPIKES, spikes);
    write_file(GAINS, gains);
}

/* Returns the script's exit status; its output is in LOG. */
static int estimate(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " %s > " LOG " 2>&1", args);
    return run_command(cmd);
}

/* Static buffer: the first LOG line starting with <prefix> and a space, "" when none. */
static const char *output_line(const char *prefix) {
    static char line[1024];
    line[0] = '\0';
    const char *text = read_file(LOG);
    size_t len = strlen(prefix);
    for (const char *at = text; *at != '\0';) {
        const char *end = strchr(at, '\n');
        size_t line_len = end != NULL ? (size_t)(end - at) : strlen(at);
        if (line_len > len && strncmp(at, prefix, len) == 0 && at[len] == ' ') {
            assert(line_len < sizeof line);
            memcpy(line, at, line_len);
            line[line_len] = '\0';
            return line;
        }
        at += line_len + (end != NULL ? 1 : 0);
    }
    return line;
}

static const char *spike_line(int frame, int update, const char *path, const char *share) {
    static char line[512];
    snprintf(line, sizeof line, "frame %d update %d pc-update-ns 1000 path %s share %s\n", frame, update, path, share);
    return line;
}

static const char *with_rows(const char *base, const char *rows) {
    static char buf[TEXT_CAP];
    buf[0] = '\0';
    append(buf, TEXT_CAP, base);
    append(buf, TEXT_CAP, rows);
    return buf;
}

static void test_usage_errors_exit_1(void) {
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"), BASE_GAINS);
    assert(estimate("") == 1);
    assert(estimate(FILES) == 1);
    assert(estimate(DIR "/none.txt " SPIKES " " GAINS " measured") == 1);
    assert(estimate(JUDGE " " DIR "/none.txt " GAINS " measured") == 1);
    assert(estimate(JUDGE " " SPIKES " " DIR "/none.txt measured") == 1);
    assert(estimate(FILES " both") == 1);
    assert(estimate(FILES " measured speed=1") == 1);
    assert(estimate(FILES " measured allocation=random") == 1);
    assert(estimate(FILES " measured haircut=abc") == 1);
    assert(estimate(FILES " measured haircut=-5") == 1);
    assert(estimate(FILES " measured alone=") == 1);
    assert(estimate(FILES " measured alone=ZZ") == 1);
    assert(estimate(FILES " measured run=draw-5564") == 1);
    assert(estimate(FILES " measured") == 0);
    assert(estimate(FILES " estimated allocation=independent haircut=12.5") == 0);
}

/* Each <row> lands on line 4 of the gain table, after a valid one on line 3. */
static void assert_malformed_row(const char *row) {
    char gains[1024];
    snprintf(gains, sizeof gains, "budget 263\nclass C1 cart_update60/ui_update\n"
                                  "lever A measured draw work a/a ordinary 10\n%s\n", row);
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"), gains);
    assert(estimate(FILES " estimated") == 2);
    assert(strstr(read_file(LOG), GAINS ": line 4") != NULL);
}

static void test_malformed_gain_rows_exit_2_naming_the_line(void) {
    assert_malformed_row("bogus 1");
    assert_malformed_row("lever B guessed draw work b/b all 10");
    assert_malformed_row("lever B measured paint work b/b all 10");
    assert_malformed_row("lever B measured draw sideways b/b all 10");
    assert_malformed_row("lever B measured draw work b all 10");
    assert_malformed_row("lever B measured draw work b/b everywhere 10");
    assert_malformed_row("lever B measured draw work b/b all 101");
    assert_malformed_row("lever B measured draw work b/b all ten");
    assert_malformed_row("lever B measured draw work b/b class:C1 abs:5");
    assert_malformed_row("lever B measured update work b/b ordinary abs:5");
    assert_malformed_row("lever B measured update work b/b class:C9 10");
    assert_malformed_row("lever B measured update work b/b range:nowhere 10");
    assert_malformed_row("lever A measured draw work a/a ordinary 20");
    assert_malformed_row("class C1 cart_update60/save_at_frame_end");
    assert_malformed_row("override 3 cart_update60 50.0");
    assert_malformed_row("override 1 cart_update60 half");
    assert_malformed_row("ratio C9 ui_update=1");
    assert_malformed_row("ratio default ui_update");
    assert_malformed_row("spread S measured C9 responsible");
    assert_malformed_row("spread S measured * everything");
    assert_malformed_row("range r 5-2");
    assert_malformed_row("budget 300");
}

static void test_malformed_inputs_exit_2(void) {
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"), "class C1 cart_update60/ui_update\n");
    assert(estimate(FILES " measured") == 2);
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/update_entities", "50.0%"), BASE_GAINS);
    assert(estimate(FILES " measured") == 2);
    write_inputs(SMALL_JUDGE, spike_line(1, 299, "cart_update60/ui_update", "50.0%"), BASE_GAINS);
    assert(estimate(FILES " measured") == 2);
    write_inputs(SMALL_JUDGE, spike_line(9, 300, "cart_update60/ui_update", "50.0%"), BASE_GAINS);
    assert(estimate(FILES " measured") == 2);
    write_inputs(JUDGE_HEADER "0 1 2 3 6 0\n2 1 2 3 6 0\n", "", BASE_GAINS);
    assert(estimate(FILES " measured") == 2);
}

static void test_a_frame_without_lever_is_unchanged(void) {
    write_inputs(JUDGE_HEADER "0 160 100 3 263 0\n1 161 100 3 264 0\n", "",
                 with_rows(BASE_GAINS, "lever A measured draw work a/a class:C1 50   # spike frames only\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 263 after 263 update 160 draw 100 submit 3 fits") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 264 after 264 update 161 draw 100 submit 3 over 1") == 0);
}

#define DRAW_JUDGE JUDGE_HEADER "0 0 1000 0 1000 0\n1 0 1001 0 1001 0\n"

static void test_a_layer_sums_an_alternative_takes_the_largest_and_sums_the_groups(void) {
    const char *rows = "lever A measured draw work g1/a all 10\nlever B measured draw work g1/a all 5\n"
                       "lever C measured draw work g1/b all 20\nlever D measured draw work g2/x all 30\n";
    write_inputs(DRAW_JUDGE, "", with_rows(BASE_GAINS, rows));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 1000 after 500 update 0 draw 500 submit 0 over 237") == 0);
    /* 500.5 rounds half up. */
    assert(strcmp(output_line("frame 1"), "frame 1 before 1001 after 501 update 0 draw 501 submit 0 over 238") == 0);
    char capped[1024];
    snprintf(capped, sizeof capped, "%slever E measured draw work g3/y all 70\n", rows);
    write_file(GAINS, with_rows(BASE_GAINS, capped));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 1001 after 0 update 0 draw 0 submit 0 fits") == 0);
}

/* Additive would cut 35%: 650 and 651. */
static void test_layers_compose_multiplicatively(void) {
    write_inputs(DRAW_JUDGE, "",
                 with_rows(BASE_GAINS, "lever W measured draw work w/w all 20\nlever P measured draw per-call p/p all 10\n"
                                       "lever S measured draw scale s/s all 5\nlever U measured update work u/u all 50\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 1000 after 684 update 0 draw 684 submit 0 over 421") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 1001 after 685 update 0 draw 685 submit 0 over 422") == 0);
}

/* Frame 0 is in range r, frame 1 ordinary, frame 2 a C1 spike. */
static void test_the_most_specific_scope_wins_and_measured_reads_measured_rows(void) {
    const char *rows = "range r 0-0\n"
                       "lever A measured draw work a/a all 10\nlever A measured draw work a/a in-game 20\n"
                       "lever A measured draw work a/a ordinary 30\nlever A measured draw work a/a range:r 50\n"
                       "lever B measured update work b/b all 5\nlever B measured update work b/b in-game 10\n"
                       "lever B measured update work b/b class:* 20\nlever B measured update work b/b class:C1 40\n"
                       "lever E estimated draw work e/e all 50\n";
    write_inputs(JUDGE_HEADER "0 1000 1000 0 2000 0\n1 1000 1000 0 2000 0\n2 1000 1000 0 2000 0\n",
                 spike_line(2, 1000, "cart_update60/ui_update", "50.0%"), with_rows(BASE_GAINS, rows));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 2000 after 1450 update 950 draw 500 submit 0 over 1187") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 2000 after 1600 update 900 draw 700 submit 0 over 1337") == 0);
    /* Update 600 under class:C1: the work 500 shrinks to 300; draw 800 under in-game. */
    assert(strcmp(output_line("frame 2"), "frame 2 before 2000 class C1 work 300 spread 300 frames does-not-fit") == 0);
    assert(strstr(output_line("summary"), " saved 1550 ") != NULL);
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 2000 after 1100 update 900 draw 200 submit 0 over 837") == 0);
}

static void test_spike_work_takes_the_share_the_ratio_and_the_override(void) {
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.1%"), BASE_GAINS);
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 151 spread 151 frames 3") == 0);
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"),
                 with_rows(BASE_GAINS, "ratio default ui_update=0.5,save_at_frame_end=2\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 75 spread 75 frames 3") == 0);
    write_file(GAINS, with_rows(BASE_GAINS, "ratio default ui_update=0.5\nratio C1 ui_update=0.8\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 120 spread 120 frames 3") == 0);
    write_file(GAINS, with_rows(BASE_GAINS, "ratio C1 ui_update=3\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 300 spread 300 frames 3") == 0);
    /* A "#" inside a path is not a comment; the ratio is keyed by the child without its site. */
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update#2/ui_handle_button", "50.0%"),
                 "budget 263   # hblanks\nclass C1 cart_update60/ui_update#2/ui_handle_button   # C1\n"
                 "# a comment line\nratio C1 ui_update=0.8\nspread S measured * responsible\n");
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 120 spread 120 frames 3") == 0);
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"), BASE_GAINS);
    /* The override's path and share, the original path's label. */
    write_file(GAINS, with_rows(BASE_GAINS, "ratio default save_at_frame_end=0.5\n"
                                            "override 1 cart_update60/save_at_frame_end 40.0\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 60 spread 60 frames 2") == 0);
    assert(strcmp(output_line("class"), "class C1 work 60-60-60 frames 2-2-2 does-not-fit 0") == 0);
}

/* Work 150 of update 300. */
static void test_update_levers_shrink_the_work_and_the_rest(void) {
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"),
                 with_rows(BASE_GAINS, "lever U estimated update work u/u class:C1 20\n"
                                       "lever O estimated update work o/o class:* abs:100\n"));
    assert(estimate(FILES " estimated alone=U") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 120 spread 120 frames 2") == 0);
    /* The absolute gain comes off the 150 outside the work: update 150 + 50. */
    assert(estimate(FILES " estimated alone=O") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 150 spread 150 frames 1") == 0);
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("lever U alone"), "lever U alone saved 60 frames-fitting 5") == 0);
    assert(strcmp(output_line("lever O alone"), "lever O alone saved 100 frames-fitting 5") == 0);
    /* 240 left by U, work 120, rest 120 - 100. */
    assert(strstr(output_line("summary"), " saved 160 ") != NULL);
    write_file(GAINS, with_rows(BASE_GAINS, "lever O estimated update work o/o class:* abs:200\n"));
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("lever O alone"), "lever O alone saved 150 frames-fitting 5") == 0);
    /* 150 - 100.5 rounds half up to 50. */
    write_file(GAINS, with_rows(BASE_GAINS, "lever O estimated update work o/o class:* abs:100.5\n"));
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("lever O alone"), "lever O alone saved 100 frames-fitting 5") == 0);
}

static void test_a_save_permission_adds_the_save_capped_at_the_rest(void) {
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"),
                 with_rows(BASE_GAINS, "spread U5 estimated C1 responsible+save:20\n"));
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 150 spread 210 frames 3") == 0);
    write_file(GAINS, with_rows(BASE_GAINS, "spread U5 estimated C1 responsible+save:20\n"
                                            "lever O estimated update work o/o class:* abs:120\n"));
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 150 spread 180 frames 1") == 0);
    /* No permission: the frame fits only on its own. */
    write_inputs(SMALL_JUDGE, spike_line(1, 300, "cart_update60/ui_update", "50.0%"),
                 "budget 263\nclass C1 cart_update60/ui_update\nspread U8 estimated C2 responsible\n"
                 "class C2 cart_update60/update_entities\n");
    assert(estimate(FILES " estimated") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 353 class C1 work 150 spread 0 frames does-not-fit") == 0);
}

/* Label to path, the "before" and "spread" tokens dropped: spread's lines. */
#define NORMALIZE_AWK                                                                                             \
    "FNR == NR { sub(/(^|[ \\t])#.*/, \"\"); if ($1 == \"class\") path[$2] = $3; next }\n"                                \
    "$1 == \"frame\" && $5 == \"class\" { line = \"frame \" $2 \" class \" path[$6]\n"                          \
    "    for (i = 7; i <= NF; i++) if ($i == \"spread\") i++; else line = line \" \" $i\n"                       \
    "    print line }\n"                                                                                          \
    "$1 == \"class\" { $2 = path[$2]; print }\n"

/* Frame 1 spreads into spike frame 2; frame 5, the table's last, cannot spread. */
static void test_independent_allocation_equals_spread(void) {
    const char *spikes = "frame 1 update 300 pc-update-ns 1000 path cart_update60/ui_update share 50.0%\n"
                         "frame 2 update 100 pc-update-ns 1000 path cart_update60/save_at_frame_end share 100.0%\n"
                         "frame 5 update 300 pc-update-ns 1000 path cart_update60/ui_update share 100.0%\n";
    write_inputs(SMALL_JUDGE "5 300 200 3 503 1\n", spikes,
                 with_rows(BASE_GAINS, "range r 2-2\nratio default ui_update=0.5\n"
                                       "lever K measured draw work k/k in-game 13\nlever K measured draw work k/k range:r 50\n"));
    write_file(DIR "/normalize.awk", NORMALIZE_AWK);
    assert(estimate(FILES " measured allocation=independent") == 0);
    assert(run_command("awk -f " DIR "/normalize.awk " GAINS " " LOG " > " DIR "/estimate.txt") == 0);
    assert(run_command(SPREAD_SCRIPT " 263 " JUDGE " " SPIKES " default=13,2-2=50 ui_update=0.5 > " DIR "/spread.txt") == 0);
    const char *expected = "frame 1 class cart_update60/ui_update work 75 frames 2 overlaps 2\n"
                           "frame 2 class cart_update60/save_at_frame_end work 100 frames 1\n"
                           "frame 5 class cart_update60/ui_update work 150 frames does-not-fit\n"
                           "class cart_update60/ui_update work 75-112.5-150 frames 2-2-2 does-not-fit 1\n"
                           "class cart_update60/save_at_frame_end work 100-100-100 frames 1-1-1 does-not-fit 0\n";
    assert(strcmp(read_file(DIR "/spread.txt"), expected) == 0);
    assert(strcmp(read_file(DIR "/estimate.txt"), expected) == 0);
}

/* Frames 0 and 1 spike with work 150; every later frame leaves 50 of room. */
#define SHARED_JUDGE                                                                                              \
    JUDGE_HEADER "0 300 0 0 300 1\n1 300 0 0 300 1\n2 213 0 0 213 0\n3 213 0 0 213 0\n4 213 0 0 213 0\n"

static void test_sequential_allocation_consumes_the_room(void) {
    char spikes[1024];
    snprintf(spikes, sizeof spikes, "%s", spike_line(0, 300, "cart_update60/ui_update", "50.0%"));
    append(spikes, sizeof spikes, spike_line(1, 300, "cart_update60/ui_update", "50.0%"));
    write_inputs(SHARED_JUDGE, spikes, BASE_GAINS);
    /* Independent: frame 1 counted at its full update by frame 0, its own 113 of room by itself. */
    assert(estimate(FILES " measured allocation=independent") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 300 class C1 work 150 spread 150 frames 3 overlaps 1") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 300 class C1 work 150 spread 150 frames 2") == 0);
    /* Sequential: frame 0 takes 37 of frame 1's 113, frame 1 then needs 76 + 50 + 50. */
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 300 class C1 work 150 spread 150 frames 2 overlaps 1") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 300 class C1 work 150 spread 150 frames 3") == 0);
}

/* Frame 0 spikes with work <update>; it keeps 263 - 3 - 1 = 259 of room, every later frame 1. */
static void write_long_judge(int frames, int spike_update) {
    static char text[TEXT_CAP];
    char line[64];
    text[0] = '\0';
    append(text, TEXT_CAP, JUDGE_HEADER);
    for (int frame = 0; frame < frames; frame++) {
        int update = frame == 0 ? spike_update : 258;
        snprintf(line, sizeof line, "%d %d 1 3 %d 0\n", frame, update, update + 4);
        append(text, TEXT_CAP, line);
    }
    write_inputs(text, spike_line(0, spike_update, "cart_update60/save_at_frame_end", "100.0%"), BASE_GAINS);
}

static void test_does_not_fit_past_600_frames_or_the_table(void) {
    const char *allocations[] = {"measured allocation=independent", "measured allocation=sequential"};
    for (int i = 0; i < 2; i++) {
        char args[256];
        snprintf(args, sizeof args, FILES " %s", allocations[i]);
        write_long_judge(700, 858);
        assert(estimate(args) == 0);
        assert(strcmp(output_line("frame 0"), "frame 0 before 862 class C2 work 858 spread 858 frames 600") == 0);
        write_long_judge(700, 859);
        assert(estimate(args) == 0);
        assert(strcmp(output_line("frame 0"), "frame 0 before 863 class C2 work 859 spread 859 frames does-not-fit") == 0);
        assert(strcmp(output_line("class"), "class C2 work 859-859-859 frames - does-not-fit 1") == 0);
        write_long_judge(10, 268);
        assert(estimate(args) == 0);
        assert(strcmp(output_line("frame 0"), "frame 0 before 272 class C2 work 268 spread 268 frames 10") == 0);
        write_long_judge(10, 269);
        assert(estimate(args) == 0);
        assert(strcmp(output_line("frame 0"), "frame 0 before 273 class C2 work 269 spread 269 frames does-not-fit") == 0);
    }
}

static void test_haircut_divides_every_gain(void) {
    write_inputs(JUDGE_HEADER "0 0 300 0 300 0\n", "", with_rows(BASE_GAINS, "lever K measured draw work k/k all 13\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 300 after 261 update 0 draw 261 submit 0 fits") == 0);
    /* 13 / 1.2 = 10.83: 267.5 rounds to 268. */
    assert(estimate(FILES " measured haircut=20") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 300 after 268 update 0 draw 268 submit 0 over 5") == 0);
    assert(strstr(output_line("summary"), " haircut 20 ") != NULL);
}

static void test_alone_keeps_one_lever_and_lever_lines_rank_each(void) {
    write_inputs(JUDGE_HEADER "0 0 280 0 280 0\n1 0 1000 0 1000 0\n", "",
                 with_rows(BASE_GAINS, "lever A measured draw work a/a all 10\nlever B measured draw work b/b all 20\n"));
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 280 after 196 update 0 draw 196 submit 0 fits") == 0);
    assert(strcmp(output_line("lever A alone"), "lever A alone saved 128 frames-fitting 1") == 0);
    assert(strcmp(output_line("lever B alone"), "lever B alone saved 256 frames-fitting 1") == 0);
    assert(strstr(read_file(LOG), "lever A alone saved 128 frames-fitting 1\nlever B alone ") != NULL);
    assert(estimate(FILES " measured alone=A") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 1000 after 900 update 0 draw 900 submit 0 over 637") == 0);
    assert(strcmp(output_line("lever"), "") == 0);
    assert(strstr(output_line("summary"), " saved 128 ") != NULL);
}

static void test_summary_counts_frames_and_spikes(void) {
    char spikes[1024];
    snprintf(spikes, sizeof spikes, "%s", spike_line(3, 300, "cart_update60/ui_update", "50.0%"));
    append(spikes, sizeof spikes, spike_line(5, 300, "cart_update60/ui_update", "50.0%"));
    write_inputs(JUDGE_HEADER "0 100 97 3 200 0\n1 100 160 3 263 0\n2 100 167 3 270 1\n3 300 50 3 353 1\n"
                              "4 100 50 3 153 0\n5 300 0 0 300 1\n",
                 spikes, BASE_GAINS);
    assert(estimate(FILES " measured") == 0);
    assert(strcmp(output_line("summary"), "summary end measured allocation sequential haircut 0 run - frames 6 over 1 "
                                          "worst 2 270 mean 221.50 saved 0 spikes 2 does-not-fit 1") == 0);
    assert(strcmp(output_line("frame 5"), "frame 5 before 300 class C1 work 150 spread 150 frames does-not-fit") == 0);
    assert(strcmp(output_line("class"), "class C1 work 150-150-150 frames 2-2-2 does-not-fit 1") == 0);
}

/* in-game@<run> rows apply only under run=<run>, over in-game rows. */
static void test_run_selects_the_in_game_rows_of_one_draw_run(void) {
    write_inputs(JUDGE_HEADER "0 0 1000 0 1000 0\n1 0 1000 0 1000 0\n", "",
                 with_rows(BASE_GAINS, "range t 0-0\nlever A measured draw work a/a range:t 50\n"
                                       "lever A measured draw work a/a in-game@r1 10\nlever A measured draw work a/a in-game@r2 20\n"
                                       "lever B measured draw work b/b in-game 1\nlever B measured draw work b/b in-game@r1 15\n"));
    assert(estimate(FILES " measured run=r1") == 0);
    assert(strcmp(output_line("frame 0"), "frame 0 before 1000 after 500 update 0 draw 500 submit 0 over 237") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 1000 after 750 update 0 draw 750 submit 0 over 487") == 0);
    assert(strstr(output_line("summary"), " run r1 ") != NULL);
    assert(estimate(FILES " measured run=r2") == 0);
    assert(strcmp(output_line("frame 1"), "frame 1 before 1000 after 790 update 0 draw 790 submit 0 over 527") == 0);
    assert(estimate(FILES " measured") == 1);
    assert(strstr(read_file(LOG), "run=") != NULL);
    assert(estimate(FILES " measured run=r3") == 1);
}

int main(void) {
    test_usage_errors_exit_1();
    test_malformed_gain_rows_exit_2_naming_the_line();
    test_malformed_inputs_exit_2();
    test_a_frame_without_lever_is_unchanged();
    test_a_layer_sums_an_alternative_takes_the_largest_and_sums_the_groups();
    test_layers_compose_multiplicatively();
    test_the_most_specific_scope_wins_and_measured_reads_measured_rows();
    test_spike_work_takes_the_share_the_ratio_and_the_override();
    test_update_levers_shrink_the_work_and_the_rest();
    test_a_save_permission_adds_the_save_capped_at_the_rest();
    test_independent_allocation_equals_spread();
    test_sequential_allocation_consumes_the_room();
    test_does_not_fit_past_600_frames_or_the_table();
    test_haircut_divides_every_gain();
    test_alone_keeps_one_lever_and_lever_lines_rank_each();
    test_summary_counts_frames_and_spikes();
    test_run_selects_the_in_game_rows_of_one_draw_run();
    assert(system("rm -rf " DIR) == 0);
    printf("test_lever_estimate: all tests passed\n");
    return 0;
}
