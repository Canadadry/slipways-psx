/* The minimizer: the mechanical pass on made-up frames, then the tool
 * tmp/minimize (src/cmd/minimize.c) and tools/minimize.sh on made-up event
 * files, with coverage replaced by a script oracle. */

#include "devtools/minimize.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 256

static p8_input_frame_t frame(int x, int y, int buttons) {
    p8_input_frame_t f;
    memset(&f, 0, sizeof f);
    f.mouse_x = x;
    f.mouse_y = y;
    f.mouse_buttons = buttons;
    return f;
}

static int same(p8_input_frame_t a, p8_input_frame_t b) {
    return memcmp(&a, &b, sizeof a) == 0;
}

static int minimize(const p8_input_frame_t *in, int n, p8_input_frame_t *out) {
    assert(n <= MAX_FRAMES);
    return minimize_mechanical(in, n, out);
}

static void test_idle_frames_are_removed(void) {
    p8_input_frame_t in[] = {
        frame(10, 10, 0), frame(10, 10, 0), frame(10, 10, 0), frame(10, 10, 1),
        frame(10, 10, 0), frame(10, 10, 0), frame(10, 10, 0),
    };
    p8_input_frame_t out[MAX_FRAMES];
    int n = minimize(in, 7, out);
    assert(n == 2);
    assert(same(out[0], in[3]));
    assert(same(out[1], in[4]));
}

static int contains(const p8_input_frame_t *frames, int n, p8_input_frame_t f) {
    for (int i = 0; i < n; i++) {
        if (same(frames[i], f)) {
            return 1;
        }
    }
    return 0;
}

/* A press, a release (which looks like an idle frame), a d-pad press and a
 * right click during a drag: no transition disappears, even buried among
 * moves. */
static void test_button_transitions_are_always_kept(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    in[n++] = frame(0, 0, 0);
    for (int i = 1; i <= 20; i++) {
        in[n++] = frame(i, 0, 0);
    }
    in[n++] = frame(20, 0, 1);
    for (int i = 21; i <= 40; i++) {
        in[n++] = frame(i, 0, 1);
    }
    in[n++] = frame(40, 0, 3);
    in[n++] = frame(40, 0, 1);
    in[n++] = frame(40, 0, 0);
    in[n] = frame(40, 0, 0);
    in[n++].dpad[2] = 1;
    in[n++] = frame(40, 0, 0);
    int transitions[] = {21, 42, 43, 44, 45, 46};

    p8_input_frame_t out[MAX_FRAMES];
    int kept = minimize(in, n, out);
    assert(kept < n);
    for (size_t t = 0; t < sizeof transitions / sizeof transitions[0]; t++) {
        assert(contains(out, kept, in[transitions[t]]));
    }
}

/* Between two transitions: at least 80% of the moves are removed, and the
 * frame just before the transition is at the final position. */
static void test_at_least_80_percent_of_moves_between_transitions_are_removed(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    in[n++] = frame(5, 5, 1);
    in[n++] = frame(5, 5, 0);
    for (int i = 1; i <= 37; i++) {
        in[n++] = frame(5 + i, 5 + 2 * i, 0);
        in[n++] = frame(5 + i, 5 + 2 * i, 0);
    }
    in[n++] = frame(42, 79, 1);

    p8_input_frame_t out[MAX_FRAMES];
    int kept = minimize(in, n, out);
    int moves = kept - 3;
    assert(moves >= 1);
    assert(moves * 5 <= 37);
    assert(same(out[0], in[0]));
    assert(same(out[1], in[1]));
    assert(same(out[kept - 1], in[n - 1]));
    assert(same(out[kept - 2], frame(42, 79, 0)));
}

/* A drag: a button held, the mouse moving; those are moves. */
static void test_drag_moves_are_thinned_but_hold_still_frames_kept(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    in[n++] = frame(10, 10, 1);
    in[n++] = frame(10, 10, 1);
    in[n++] = frame(10, 10, 1);
    for (int i = 1; i <= 50; i++) {
        in[n++] = frame(10 + i, 10, 1);
    }
    in[n++] = frame(60, 10, 0);

    p8_input_frame_t out[MAX_FRAMES];
    int kept = minimize(in, n, out);
    assert(kept == 3 + 10 + 1);
    assert(same(out[kept - 2], frame(60, 10, 1)));
}

/* Only transitions and still holds, with no idle frame and no move. */
static void test_already_minimal_frames_are_unchanged(void) {
    p8_input_frame_t in[] = {
        frame(30, 40, 1), frame(30, 40, 0), frame(70, 12, 1),
        frame(70, 12, 1), frame(90, 12, 0), frame(90, 12, 2),
    };
    int n = (int)(sizeof in / sizeof in[0]);
    p8_input_frame_t out[MAX_FRAMES];
    assert(minimize(in, n, out) == n);
    assert(memcmp(in, out, sizeof in) == 0);
}

/* A fifth of the moves kept, rounded down: none out of 4, 2 out of 10, the
 * last one among them. The transition carries its own position, so the
 * click lands on the same spot. */
static void test_segments_keep_a_fifth_of_their_moves_rounded_down(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    in[n++] = frame(0, 0, 1);
    in[n++] = frame(0, 0, 0);
    for (int i = 1; i <= 4; i++) {
        in[n++] = frame(i, 0, 0);
    }
    in[n++] = frame(5, 0, 1);
    in[n++] = frame(5, 0, 0);
    for (int i = 1; i <= 10; i++) {
        in[n++] = frame(5, i, 0);
    }
    in[n++] = frame(5, 11, 1);

    p8_input_frame_t out[MAX_FRAMES];
    int kept = minimize(in, n, out);
    assert(kept == 7);
    assert(same(out[2], in[6]));
    assert(same(out[3], in[7]));
    assert(same(out[5], frame(5, 10, 0)));
    assert(same(out[6], in[n - 1]));
}

/* A file with no transition and no hold is not emptied: its last frame
 * remains. */
static void test_a_file_of_few_moves_keeps_its_last_frame(void) {
    p8_input_frame_t in[] = {frame(1, 1, 0), frame(2, 2, 0), frame(3, 3, 0)};
    p8_input_frame_t out[MAX_FRAMES];
    assert(minimize(in, 3, out) == 1);
    assert(same(out[0], in[2]));
    assert(minimize(in, 1, out) == 1);
    assert(same(out[0], in[0]));
}

/* A path that comes back to its starting point leaves no idle frame. */
static void test_no_output_frame_is_idle_after_its_kept_predecessor(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    in[n++] = frame(0, 0, 1);
    in[n++] = frame(0, 0, 0);
    for (int i = 1; i <= 10; i++) {
        in[n++] = frame(i, 0, 0);
    }
    for (int i = 9; i >= 0; i--) {
        in[n++] = frame(i, 0, 0);
    }
    p8_input_frame_t out[MAX_FRAMES];
    int kept = minimize(in, n, out);
    for (int i = 1; i < kept; i++) {
        assert(!(out[i].mouse_x == out[i - 1].mouse_x && out[i].mouse_y == out[i - 1].mouse_y &&
                 out[i].mouse_buttons == 0 && out[i - 1].mouse_buttons == 0));
    }
}

#define TOOL "./tmp/minimize"
#define DIR "tmp/test_minimize_work"
#define LOG DIR "/log.txt"

static int run(const char *cmd) {
    char full[2048];
    snprintf(full, sizeof full, "%s > " LOG " 2>&1", cmd);
    return system(full);
}

static void write_events(const char *path, const p8_input_frame_t *frames, int n) {
    assert(p8_input_save(frames, n, path));
}

static int load_events(const char *path, p8_input_t *input) {
    p8_input_init(input);
    return p8_input_load(input, path);
}

/* The tool's output is read back by the loader, and holds the frames the
 * mechanical pass returns. */
static void test_tool_output_is_reread_by_the_loader(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    for (int i = 0; i < 30; i++) {
        in[n++] = frame(i, 100 - i, 0);
        in[n++] = frame(i, 100 - i, 0);
    }
    in[n++] = frame(29, 71, 1);
    in[n] = frame(29, 71, 0);
    in[n++].dpad[5] = 1;
    write_events(DIR "/raw.json", in, n);
    assert(run(TOOL " mechanical " DIR "/raw.json " DIR "/out.json") == 0);

    p8_input_frame_t expected[MAX_FRAMES];
    int kept = minimize(in, n, expected);
    p8_input_t loaded;
    assert(load_events(DIR "/out.json", &loaded));
    assert(loaded.frame_count == kept);
    assert(memcmp(loaded.frames, expected, sizeof expected[0] * (size_t)kept) == 0);
    p8_input_free(&loaded);
}

static void test_tool_drop_removes_a_range_and_never_empties_the_file(void) {
    p8_input_frame_t in[] = {frame(1, 1, 0), frame(2, 2, 0), frame(3, 3, 1), frame(4, 4, 0)};
    write_events(DIR "/four.json", in, 4);
    assert(run(TOOL " drop " DIR "/four.json " DIR "/two.json 1 2") == 0);
    p8_input_t loaded;
    assert(load_events(DIR "/two.json", &loaded));
    assert(loaded.frame_count == 2);
    assert(same(loaded.frames[0], in[0]));
    assert(same(loaded.frames[1], in[3]));
    p8_input_free(&loaded);

    assert(run(TOOL " drop " DIR "/four.json " DIR "/none.json 0 4") != 0);
    assert(run(TOOL " drop " DIR "/four.json " DIR "/none.json 3 2") != 0);
    assert(run("test -e " DIR "/none.json") != 0);
    assert(run(TOOL " mechanical " DIR "/absent.json " DIR "/none.json") != 0);
}

#define SCRIPT "sh tools/minimize.sh " TOOL
/* Script oracles in place of line_coverage.sh cover: the set of lines of an
 * event file on stdout, a non-zero exit status if it fails. */
#define CLICKS_ORACLE DIR "/clicks.sh"
#define CONSTANT_ORACLE DIR "/constant.sh"
#define AT_LEAST_3_ORACLE DIR "/at_least_3.sh"
#define X7_ORACLE DIR "/x7.sh"
#define DPAD_WIDTH_ORACLE DIR "/dpad_width.sh"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

static void write_oracles(void) {
    write_file(CLICKS_ORACLE, "grep '\"mouse_buttons\": 1' \"$1\" | sed 's/, \"mouse_buttons.*//' | sort -u\n");
    write_file(CONSTANT_ORACLE, "echo src/cmd/replay.c:1\n");
    write_file(AT_LEAST_3_ORACLE, "[ $(grep -c mouse_x \"$1\") -ge 3 ] && echo src/cmd/replay.c:1\n");
    write_file(X7_ORACLE, "grep -c '\"mouse_x\": 7,' \"$1\"\n");
    write_file(DPAD_WIDTH_ORACLE, "grep -q 'false, false, false, false, false' \"$1\" && echo six || echo four\n");
}

/* Two clicks held at two spots in a session of 80 frames. */
static int write_session(const char *path) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    for (int i = 0; i < 30; i++) {
        in[n++] = frame(i, i, 0);
    }
    for (int i = 0; i < 5; i++) {
        in[n++] = frame(30, 30, 1);
    }
    for (int i = 0; i < 40; i++) {
        in[n++] = frame(30 - i / 2, 30, 0);
    }
    for (int i = 0; i < 5; i++) {
        in[n++] = frame(10, 30, 1);
    }
    write_events(path, in, n);
    return n;
}

static int frame_count(const char *path) {
    p8_input_t loaded;
    assert(load_events(path, &loaded));
    int n = loaded.frame_count;
    p8_input_free(&loaded);
    return n;
}

static int same_oracle_output(const char *oracle, const char *a, const char *b) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "sh %s %s > " DIR "/a.lines && sh %s %s > " DIR "/b.lines && cmp -s " DIR
             "/a.lines " DIR "/b.lines", oracle, a, oracle, b);
    return system(cmd) == 0;
}

/* Without an oracle, the script only runs the mechanical pass. */
static void test_script_without_oracle_is_the_mechanical_stage(void) {
    write_session(DIR "/session.json");
    assert(run(SCRIPT " " DIR "/session.json " DIR "/mech.json") == 0);
    assert(run(TOOL " mechanical " DIR "/session.json " DIR "/expected.json") == 0);
    assert(run("cmp " DIR "/mech.json " DIR "/expected.json") == 0);
}

/* The guided pass keeps exactly the lines of the raw session and goes down
 * to the minimum the oracle accepts. */
static void test_coverage_pass_keeps_raw_lines_and_goes_to_the_minimum(void) {
    int raw = write_session(DIR "/session.json");
    assert(run(SCRIPT " " DIR "/session.json " DIR "/clicks.json sh " CLICKS_ORACLE) == 0);
    assert(same_oracle_output(CLICKS_ORACLE, DIR "/session.json", DIR "/clicks.json"));
    assert(frame_count(DIR "/clicks.json") == 2);
    assert(frame_count(DIR "/clicks.json") < raw);

    assert(run(SCRIPT " " DIR "/session.json " DIR "/constant.json sh " CONSTANT_ORACLE) == 0);
    assert(frame_count(DIR "/constant.json") == 1);
}

/* A candidate the oracle fails on is rejected; a raw session it fails on
 * makes the script fail without writing the output file. */
static void test_coverage_pass_rejects_candidates_the_oracle_fails_on(void) {
    write_session(DIR "/session.json");
    assert(run(SCRIPT " " DIR "/session.json " DIR "/three.json sh " AT_LEAST_3_ORACLE) == 0);
    assert(frame_count(DIR "/three.json") == 3);

    p8_input_frame_t two[] = {frame(1, 1, 0), frame(2, 2, 0)};
    write_events(DIR "/two.json", two, 2);
    assert(run(SCRIPT " " DIR "/two.json " DIR "/none.json sh " AT_LEAST_3_ORACLE) != 0);
    assert(run("test -e " DIR "/none.json") != 0);
}

/* The mechanical pass removes frames at mouse_x 7, and so changes the
 * lines: the guided pass starts again from the raw session and keeps them
 * all. */
static void test_coverage_pass_restarts_from_raw_when_mechanical_stage_changes_lines(void) {
    p8_input_frame_t in[MAX_FRAMES];
    int n = 0;
    for (int i = 0; i < 40; i++) {
        in[n++] = frame(i % 2 ? 7 : 8, i, 0);
    }
    write_events(DIR "/x7_session.json", in, n);
    assert(run(SCRIPT " " DIR "/x7_session.json " DIR "/x7.json sh " X7_ORACLE) == 0);
    assert(same_oracle_output(X7_ORACLE, DIR "/x7_session.json", DIR "/x7.json"));
    assert(frame_count(DIR "/x7.json") == 20);
}

/* A session with a d-pad of 4 booleans: the writer always writes 6, so the
 * loader line that pads the 4 is never covered again. The reference lines
 * are those of the rewritten raw session. */
static void test_coverage_pass_judges_against_the_rewritten_raw_session(void) {
    write_file(DIR "/four.json", "{ \"frames\": [\n"
               "{ \"mouse_x\": 1, \"mouse_y\": 1, \"mouse_buttons\": 0, \"dpad\": [false, false, false, false] },\n"
               "{ \"mouse_x\": 2, \"mouse_y\": 1, \"mouse_buttons\": 0, \"dpad\": [false, false, false, false] },\n"
               "{ \"mouse_x\": 2, \"mouse_y\": 1, \"mouse_buttons\": 1, \"dpad\": [false, false, false, false] }\n"
               "] }\n");
    assert(run(SCRIPT " " DIR "/four.json " DIR "/four_min.json sh " DPAD_WIDTH_ORACLE) == 0);
    assert(frame_count(DIR "/four_min.json") == 1);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

int main(void) {
    reset();
    test_tool_output_is_reread_by_the_loader();
    test_tool_drop_removes_a_range_and_never_empties_the_file();
    write_oracles();
    test_script_without_oracle_is_the_mechanical_stage();
    test_coverage_pass_keeps_raw_lines_and_goes_to_the_minimum();
    test_coverage_pass_rejects_candidates_the_oracle_fails_on();
    test_coverage_pass_restarts_from_raw_when_mechanical_stage_changes_lines();
    test_coverage_pass_judges_against_the_rewritten_raw_session();
    test_idle_frames_are_removed();
    test_button_transitions_are_always_kept();
    test_at_least_80_percent_of_moves_between_transitions_are_removed();
    test_drag_moves_are_thinned_but_hold_still_frames_kept();
    test_already_minimal_frames_are_unchanged();
    test_segments_keep_a_fifth_of_their_moves_rounded_down();
    test_a_file_of_few_moves_keeps_its_last_frame();
    test_no_output_frame_is_idle_after_its_kept_predecessor();
    printf("test_minimize: ok\n");
    return 0;
}
