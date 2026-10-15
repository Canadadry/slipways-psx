/* tmp/frame_profile run on a title-start e2e and on a resume-start recording built from the
 * committed demo record, its outputs read back as files. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TOOL "./tmp/frame_profile"
#define SYMBOLS "tmp/frame_profile.syms"
#define DRIVER "./tmp/replay"
#define TITLE_EVENTS "tests/e2e/pad_title.json"
#define RECORD "pico8/demo_record.hex"
#define RESUME_EVENTS "tmp/test_frame_profile_resume.json"
#define OUT "tmp/test-frame-profile"
#define STDERR_LOG "tmp/test-frame-profile/stderr.txt"

enum { RESUME_FRAMES = 40, LINE_CAP = 512 };

static const char *const DRAW_CHILDREN[] = {
    "p8_cls", "hud_select_palette_block_default", "draw_layers#1", "draw_layers#2", "demo_overlay_draw",
};
static const char *const UPDATE_CHILDREN[] = {"update_entities", "ui_update"};
static const char *const FAMILIES[] = {
    "cls", "pset", "line", "rect", "rectfill", "circ", "circfill", "spr", "print", "pattern",
};

#define COUNT(array) ((int)(sizeof(array) / sizeof(array)[0]))

/* Allocated buffer, NUL-terminated, freed by the caller. */
static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size >= 0);
    rewind(file);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    buf[size] = '\0';
    fclose(file);
    return buf;
}

static int run(const char *command) {
    int status = system(command);
    assert(status != -1 && WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void write_resume_events(void) {
    char *hex = read_whole_file(RECORD);
    hex[strcspn(hex, "\n")] = '\0';
    FILE *out = fopen(RESUME_EVENTS, "w");
    assert(out != NULL);
    fprintf(out, "{\n  \"input\": \"pad\",\n  \"start\": \"resume\",\n  \"initial_save\": \"%s\",\n  \"frames\": [\n", hex);
    for (int i = 0; i < RESUME_FRAMES; i++) {
        fprintf(out, "    { \"pad\": [%s] }%s\n", i % 8 == 4 ? "1" : "", i < RESUME_FRAMES - 1 ? "," : "");
    }
    fputs("  ]\n}\n", out);
    fclose(out);
    free(hex);
}

/* The replay's frame count, from its own stdout. */
static int replay_frames(const char *events, const char *state) {
    char command[LINE_CAP];
    snprintf(command, sizeof command, "STATE=%s " DRIVER " %s /dev/null " OUT "/replay_dump.txt > " OUT "/replay.log",
             state, events);
    assert(run(command) == 0);
    char *log = read_whole_file(OUT "/replay.log");
    int frames = -1;
    const char *line = strstr(log, "replay: ");
    assert(line != NULL && sscanf(line, "replay: %d frame(s)", &frames) == 1);
    free(log);
    return frames;
}

/* roots NULL profiles the default draw and update roots; an update root follows its draw root. */
static void profile_rooted(const char *events, const char *dir, const char *state, const char *roots) {
    char command[LINE_CAP];
    snprintf(command, sizeof command, "mkdir -p %s && STATE=%s " TOOL " %s %s " SYMBOLS " %s > %s/log.txt", dir, state,
             events, dir, roots != NULL ? roots : "", dir);
    int status = run(command);
    if (status != 0) fprintf(stderr, "test_frame_profile: %s exited %d\n", events, status);
    assert(status == 0);
}

static void profile(const char *events, const char *dir, const char *state) {
    profile_rooted(events, dir, state, NULL);
}

static char *profile_file(const char *dir, const char *name) {
    char path[LINE_CAP];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    return read_whole_file(path);
}

static int has_line_starting(const char *text, const char *prefix) {
    size_t length = strlen(prefix);
    for (const char *line = text; *line != '\0';) {
        if (strncmp(line, prefix, length) == 0) return 1;
        const char *end = strchr(line, '\n');
        if (end == NULL) break;
        line = end + 1;
    }
    return 0;
}

static void expect_line(const char *text, const char *prefix) {
    if (!has_line_starting(text, prefix)) {
        fprintf(stderr, "test_frame_profile: no line starting with \"%s\"\n", prefix);
        assert(0 && "missing line");
    }
}

/* Every frame line but its times: the call counts and below columns stay. */
static char *counts_only(const char *frames) {
    char *out = calloc(1, strlen(frames) + 1);
    assert(out != NULL);
    const char *line = frames;
    while (*line != '\0') {
        const char *end = strchr(line, '\n');
        size_t length = end != NULL ? (size_t)(end - line) : strlen(line);
        char text[LINE_CAP];
        assert(length < sizeof text);
        memcpy(text, line, length);
        text[length] = '\0';
        int frame;
        char root[16], name[128], family[16];
        unsigned calls, below;
        unsigned long long ns, net;
        char kept[LINE_CAP] = "";
        if (sscanf(text, "frame %d child %15s %127s %u %llu %llu %u", &frame, root, name, &calls, &ns, &net, &below) == 7) {
            snprintf(kept, sizeof kept, "%d %s %s %u %u\n", frame, root, name, calls, below);
        } else if (sscanf(text, "frame %d family %15s %u", &frame, family, &calls) == 3) {
            snprintf(kept, sizeof kept, "%d %s %u\n", frame, family, calls);
        }
        strcat(out, kept);
        line += length + (end != NULL);
    }
    return out;
}

/* self = root - children, child <= root, net <= raw, in-call = families but pattern. */
static void check_times_add_up(const char *frames, int frame_count) {
    unsigned long long root[2] = {0, 0}, children[2] = {0, 0}, self[2] = {0, 0};
    unsigned long long families = 0, in_call = 0;
    int frame = -1, done = 0;
    const char *line = frames;
    while (1) {
        const char *end = strchr(line, '\n');
        int at = -1;
        char text[LINE_CAP] = "";
        if (end != NULL) {
            size_t length = (size_t)(end - line);
            assert(length < sizeof text);
            memcpy(text, line, length);
            text[length] = '\0';
            sscanf(text, "frame %d", &at);
        }
        if (at != frame && frame >= 0) {
            for (int r = 0; r < 2; r++) {
                assert(children[r] + self[r] == root[r]);
                assert(children[r] <= root[r]);
            }
            assert(in_call == families);
            assert(in_call <= root[1]);
            done++;
            memset(root, 0, sizeof root), memset(children, 0, sizeof children), memset(self, 0, sizeof self);
            families = in_call = 0;
        }
        if (end == NULL) break;
        frame = at;
        unsigned long long a, b, ns, net;
        unsigned calls, below;
        char kind[16], name[128];
        if (sscanf(text, "frame %*d update %llu draw %llu", &a, &b) == 2) {
            root[0] = a, root[1] = b;
        } else if (sscanf(text, "frame %*d child %15s %127s %u %llu %llu %u", kind, name, &calls, &ns, &net, &below) == 6) {
            assert(net <= ns);
            children[strcmp(kind, "draw") == 0] += ns;
        } else if (sscanf(text, "frame %*d self %15s %llu", kind, &ns) == 2) {
            self[strcmp(kind, "draw") == 0] = ns;
        } else if (sscanf(text, "frame %*d family %15s %u %llu", name, &calls, &ns) == 3) {
            if (strcmp(name, "pattern") != 0) families += ns;
        } else if (sscanf(text, "frame %*d in-call %llu", &ns) == 1) {
            in_call = ns;
        }
        line = end + 1;
    }
    assert(done == frame_count);
}

/* Every family line ends with its cost-model units and window switches; a cls is one unit. */
static void check_family_units(const char *text, int per_frame) {
    int lines = 0;
    for (const char *line = text; *line != '\0';) {
        const char *end = strchr(line, '\n');
        assert(end != NULL);
        char row[LINE_CAP];
        size_t length = (size_t)(end - line);
        assert(length < sizeof row);
        memcpy(row, line, length);
        row[length] = '\0';
        int body = 0;
        if (per_frame) sscanf(row, "frame %*d family %n", &body);
        else if (strncmp(row, "family ", 7) == 0) body = 7;
        if (body > 0) {
            const char *units = strstr(row, " units ");
            unsigned unit_count, switches;
            int tail = 0;
            if (units == NULL || sscanf(units, " units %u switches %u%n", &unit_count, &switches, &tail) != 2 ||
                units[tail] != '\0') {
                fprintf(stderr, "test_frame_profile: no trailing units and switches on \"%s\"\n", row);
                assert(0 && "family line units");
            }
            unsigned calls = 0;
            if (sscanf(row + body, "cls %u", &calls) == 1) assert(unit_count == calls && switches == 0);
            lines++;
        }
        line = end + 1;
    }
    assert(lines > 0);
}

static void check_profile(const char *events, const char *dir) {
    char state[LINE_CAP], replay_state[LINE_CAP];
    snprintf(state, sizeof state, "mkdir -p %s", dir);
    assert(run(state) == 0);
    snprintf(state, sizeof state, "%s/state.txt", dir);
    snprintf(replay_state, sizeof replay_state, "%s/replay_state.txt", dir);
    int frame_count = replay_frames(events, replay_state);
    profile(events, dir, state);
    char *frames = profile_file(dir, "frames.txt");

    char line[LINE_CAP];
    snprintf(line, sizeof line, "profile %s frames %d hook-pair-ns ", events, frame_count);
    expect_line(frames, line);
    assert(strstr(frames, " clock monotonic-ns\n") != NULL);
    expect_line(frames, "root draw cart_draw\nroot update cart_update60\n");
    for (int i = 0; i < frame_count; i++) {
        snprintf(line, sizeof line, "frame %d update ", i);
        expect_line(frames, line);
        for (int c = 0; c < COUNT(DRAW_CHILDREN); c++) {
            snprintf(line, sizeof line, "frame %d child draw %s 1 ", i, DRAW_CHILDREN[c]);
            expect_line(frames, line);
        }
        for (int c = 0; c < COUNT(UPDATE_CHILDREN); c++) {
            snprintf(line, sizeof line, "frame %d child update %s ", i, UPDATE_CHILDREN[c]);
            expect_line(frames, line);
        }
        snprintf(line, sizeof line, "frame %d family cls 1 ", i);
        expect_line(frames, line);
        for (int f = 0; f < COUNT(FAMILIES); f++) {
            snprintf(line, sizeof line, "frame %d family %s ", i, FAMILIES[f]);
            expect_line(frames, line);
        }
        snprintf(line, sizeof line, "frame %d self draw ", i);
        expect_line(frames, line);
        snprintf(line, sizeof line, "frame %d in-call ", i);
        expect_line(frames, line);
    }
    check_times_add_up(frames, frame_count);
    check_family_units(frames, 1);

    char *summary = profile_file(dir, "summary.txt");
    snprintf(line, sizeof line, "profile %s frames %d hook-pair-ns ", events, frame_count);
    expect_line(summary, line);
    expect_line(summary, "root draw cart_draw\nroot update cart_update60\n");
    snprintf(line, sizeof line, "child draw draw_layers#2 %d ", frame_count);
    expect_line(summary, line);
    snprintf(line, sizeof line, "family cls %d ", frame_count);
    expect_line(summary, line);
    expect_line(summary, "self draw ");
    expect_line(summary, "in-call ");
    check_family_units(summary, 0);
    assert(strstr(summary, "%\n") != NULL);
    free(summary);

    char again[LINE_CAP];
    snprintf(again, sizeof again, "%s/again", dir);
    snprintf(state, sizeof state, "%s/again/state.txt", dir);
    profile(events, again, state);
    char *frames_again = profile_file(again, "frames.txt");
    char *counts = counts_only(frames);
    char *counts_again = counts_only(frames_again);
    assert(strlen(counts) > 0);
    assert(strcmp(counts, counts_again) == 0);
    free(counts), free(counts_again), free(frames_again), free(frames);

    char command[LINE_CAP * 2];
    snprintf(command, sizeof command,
             "sh tools/end_state.sh normalize %s/state.txt > %s/state.norm && "
             "sh tools/end_state.sh normalize %s > %s/replay_state.norm && "
             "cmp -s %s/state.norm %s/replay_state.norm",
             dir, dir, replay_state, dir, dir, dir);
    assert(run(command) == 0);
}

static void test_a_title_start_recording(void) {
    check_profile(TITLE_EVENTS, OUT "/title");
}

static void test_a_resume_start_recording(void) {
    write_resume_events();
    check_profile(RESUME_EVENTS, OUT "/resume");
}

static void test_a_draw_layers_root_lists_the_draw_methods_it_dispatches(void) {
    const char *dir = OUT "/draw_layers";
    profile_rooted(TITLE_EVENTS, dir, OUT "/draw_layers_state.txt", "draw_layers");
    char *frames = profile_file(dir, "frames.txt");
    expect_line(frames, "root draw draw_layers\n");
    expect_line(frames, "frame 0 child draw button_draw_method ");
    expect_line(frames, "frame 0 child update update_entities ");
    assert(!has_line_starting(frames, "frame 0 child draw draw_layers"));
    char *summary = profile_file(dir, "summary.txt");
    expect_line(summary, "root draw draw_layers\n");
    expect_line(summary, "child draw background_draw_method ");
    free(summary), free(frames);
}

static void test_a_ui_update_root_lists_the_functions_ui_update_calls(void) {
    const char *dir = OUT "/ui_update";
    profile_rooted(TITLE_EVENTS, dir, OUT "/ui_update_state.txt", "cart_draw ui_update");
    char *frames = profile_file(dir, "frames.txt");
    expect_line(frames, "root draw cart_draw\nroot update ui_update\n");
    static const char *const UI_UPDATE_CHILDREN[] = {"main_mouse_typed", "main_camera_typed", "ui_update_hover",
                                                      "ui_handle_button", "ui_main_mouse_button"};
    for (int c = 0; c < COUNT(UI_UPDATE_CHILDREN); c++) {
        char child[LINE_CAP];
        snprintf(child, sizeof child, "frame 0 child update %s 1 ", UI_UPDATE_CHILDREN[c]);
        expect_line(frames, child);
    }
    expect_line(frames, "frame 0 child draw draw_layers#2 1 ");
    assert(!has_line_starting(frames, "frame 0 child update ui_update "));
    assert(!has_line_starting(frames, "frame 0 child update update_entities "));
    char *summary = profile_file(dir, "summary.txt");
    expect_line(summary, "root draw cart_draw\nroot update ui_update\n");
    char line[LINE_CAP];
    snprintf(line, sizeof line, "child update ui_update_hover %d ", replay_frames(TITLE_EVENTS, OUT "/ui_update_replay_state.txt"));
    expect_line(summary, line);
    free(summary), free(frames);
}

static void expect_usage_error(const char *arguments, const char *message) {
    char command[LINE_CAP];
    snprintf(command, sizeof command, TOOL " %s > /dev/null 2> " STDERR_LOG, arguments);
    int status = run(command);
    char *err = read_whole_file(STDERR_LOG);
    if (status != 1 || strstr(err, message) == NULL) {
        fprintf(stderr, "test_frame_profile: '%s': expected exit 1 and \"%s\", exit %d\n%s", arguments, message, status,
                err);
        assert(0 && "a usage error exits 1");
    }
    free(err);
}

static void test_usage_errors_exit_1(void) {
    assert(run("mkdir -p " OUT "/usage") == 0);
    expect_usage_error("", "usage: frame_profile");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage", "usage: frame_profile");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage tmp/no_such_symbols.syms", "tmp/no_such_symbols.syms");
    expect_usage_error("tmp/no_such_events.json " OUT "/usage " SYMBOLS, "tmp/no_such_events.json");
    expect_usage_error(TITLE_EVENTS " " OUT "/no_such_dir/deeper " SYMBOLS, OUT "/no_such_dir/deeper");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " no_such_function", "no text symbol 'no_such_function'");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " min", "several text symbols 'min'");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " cart_update60", "the draw root cannot be cart_update60");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " cart_draw ui_update extra", "usage: frame_profile");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " draw_layers cart_draw", "the update root cannot be cart_draw");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " draw_layers draw_layers",
                       "the update root cannot be draw_layers");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " cart_draw no_such_update",
                       "no text symbol 'no_such_update'");
    expect_usage_error(TITLE_EVENTS " " OUT "/usage " SYMBOLS " cart_draw min", "several text symbols 'min'");
}

int main(void) {
    assert(run("rm -rf " OUT " && mkdir -p " OUT) == 0);
    test_usage_errors_exit_1();
    test_a_title_start_recording();
    test_a_resume_start_recording();
    test_a_draw_layers_root_lists_the_draw_methods_it_dispatches();
    test_a_ui_update_root_lists_the_functions_ui_update_calls();
    printf("test_frame_profile: ok\n");
    return 0;
}
