/* make state: the text state of a replay's last frame, read from the
 * outside as an agent would read it. In-process replays, made for
 * comparison, each run in a child: the cart loads only once per process. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "devtools/replay_loop.h"
#include "devtools/state.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"
#include "game/ui_zone.h"
#include "p8/fatal.h"

#define TOOL "./tmp/state"
#define STATE_TRM "tmp/test_state_trade_rich_mine.txt"
#define STATE_CHEAT "tmp/test_state_cheat_code.txt"
#define REPLAY "./tmp/replay"
#define FILL "tmp/test_state_fill.txt"
#define CAPTURE "tmp/test_state_capture.bin"
#define DUMP "tmp/test_state_dump.txt"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL && "state file missing");
    assert(fseek(f, 0, SEEK_END) == 0);
    long n = ftell(f);
    assert(n >= 0);
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)n, f) == (size_t)n);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

/* Lines starting with `prefix`. */
static int count_lines(const char *text, const char *prefix) {
    int n = 0;
    size_t len = strlen(prefix);
    for (const char *l = text; *l != '\0';) {
        if (strncmp(l, prefix, len) == 0) n++;
        const char *end = strchr(l, '\n');
        if (end == NULL) break;
        l = end + 1;
    }
    return n;
}

/* The first integer after `key` at the start of a line. */
static long value(const char *text, const char *key) {
    char pattern[96];
    long v;
    snprintf(pattern, sizeof pattern, "\n%s ", key);
    const char *l = strstr(text, pattern);
    if (l == NULL) {
        fprintf(stderr, "missing key: %s\n", key);
        assert(0 && "key missing from the state");
    }
    assert(sscanf(l + strlen(pattern), "%ld", &v) == 1);
    return v;
}

/* The n integers after `key`. */
static void values(const char *text, const char *key, long *out, int n) {
    char pattern[96];
    snprintf(pattern, sizeof pattern, "\n%s ", key);
    const char *l = strstr(text, pattern);
    if (l == NULL) {
        fprintf(stderr, "missing key: %s\n", key);
        assert(0 && "key missing from the state");
    }
    l += strlen(pattern);
    for (int i = 0; i < n; i++) {
        char *end;
        out[i] = strtol(l, &end, 10);
        assert(end != l && "missing value");
        l = end;
    }
}

/* The number of entities under tag `tag` at the end of an in-process replay
 * of `events`, counted in a child. */
static int count_after_replay(const char *events, const char *tag) {
    int pipe_fd[2];
    assert(pipe(pipe_fd) == 0);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        static replay_loop_t r;
        close(pipe_fd[0]);
        replay_loop_open(&r, events);
        for (int i = 0; i < replay_loop_frames(&r); i++) replay_loop_play_frame(&r, i);
        const entity_list_t *l = entities_by_tag(tag);
        int n = l != NULL ? l->count : 0;
        assert(write(pipe_fd[1], &n, sizeof n) == (ssize_t)sizeof n);
        _exit(0);
    }
    close(pipe_fd[1]);
    int n = -1;
    assert(read(pipe_fd[0], &n, sizeof n) == (ssize_t)sizeof n);
    close(pipe_fd[0]);
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return n;
}

static void one_planet_line_per_game_planet(void) {
    remove(STATE_TRM);
    assert(system(TOOL " tests/e2e/trade_rich_mine.json " STATE_TRM " > /dev/null") == 0);
    char *state = read_file(STATE_TRM);
    int planets = count_after_replay("tests/e2e/trade_rich_mine.json", "planet");
    printf("  trade_rich_mine: %d planets\n", planets);
    assert(planets > 0);
    assert(count_lines(state, "planet ") == planets && "one planet line per planet of the planet tag");
    assert(value(state, "count planets") == planets);
    free(state);
}

/* Six slipways from p27 (three suppliers, three labs), then p1 to lab s1;
 * none goes through an infragate. */
static void the_built_network_shows(void) {
    char *state = read_file(STATE_TRM);
    static const char *const expected[] = {
        "\nslipway p27 p1 normal\n", "\nslipway p27 p71 normal\n", "\nslipway p27 p48 normal\n",
        "\nslipway p27 s1 normal\n", "\nslipway p27 s2 normal\n", "\nslipway p27 s3 normal\n",
        "\nslipway p1 s1 normal\n",
    };
    for (size_t i = 0; i < sizeof expected / sizeof expected[0]; i++) {
        if (strstr(state, expected[i]) == NULL) {
            fprintf(stderr, "missing:%s", expected[i]);
            assert(0 && "expected slipway missing from the state");
        }
    }
    assert(count_lines(state, "slipway ") == 7);
    assert(value(state, "count slipways") == 7);
    assert(strstr(state, "\nplanet p27 colonized ") != NULL);
    assert(value(state, "count colonies") == 7 && "p1, p27, p48, p71 and the three labs");
    assert(count_lines(state, "structure ") == 0);
    assert(value(state, "count structures") == 0);
    free(state);
}

/* The cheat replayed as is: 99 science, $200 at the last frame (f35). */
static void resources_reflect_the_cheat(void) {
    remove(STATE_CHEAT);
    assert(system(TOOL " tests/e2e/cheat_code.json " STATE_CHEAT " > /dev/null") == 0);
    char *state = read_file(STATE_CHEAT);
    assert(value(state, "resources credits") == 200);
    assert(value(state, "resources science") == 99);
    free(state);
    remove(STATE_CHEAT);
}

static void invented_techs_are_named(void) {
    char *state = read_file(STATE_TRM);
    assert(strstr(state, "\ntechs space folding\n") != NULL);
    assert(strstr(state, "\ntechs geneseeds\n") != NULL);
    assert(count_lines(state, "techs ") == 2);
    assert(value(state, "count techs") == 2);
    free(state);
}

/* The game zone peak is the one replay writes to REPLAY_FILL for the same
 * replay. */
static void game_peak_matches_the_fill_report(void) {
    remove(FILL);
    assert(system("REPLAY_FILL=" FILL " " REPLAY " tests/e2e/trade_rich_mine.json " CAPTURE " " DUMP
                  " > /dev/null") == 0);
    char *fill = read_file(FILL);
    char *state = read_file(STATE_TRM);
    long game[3];
    values(state, "zone game", game, 3);
    assert(game[1] == value(fill, "max_bytes game"));
    assert(game[2] == 704L * 1024L && "the game generation, load zone excluded");
    free(fill);
    free(state);
    remove(FILL);
    remove(CAPTURE);
    remove(DUMP);
}

/* "zone|cap <name> <current> <peak> <capacity>": the peak covers the
 * current value and stays under the capacity. */
static void check_peak(const char *state, const char *key, long *peak) {
    long v[3];
    values(state, key, v, 3);
    printf("  %s: current %ld, peak %ld, capacity %ld\n", key, v[0], v[1], v[2]);
    assert(v[1] >= v[0] && "a peak below the current value");
    assert(v[1] <= v[2] && "a peak beyond the capacity");
    if (peak != NULL) *peak = v[1];
}

static void every_zone_and_cap_has_its_peak(void) {
    char *state = read_file(STATE_TRM);
    long peak;
    check_peak(state, "zone frame", &peak);
    assert(peak > 0 && "the frame zone is used every frame");
    static const char *const zones[] = {
        "zone ui button", "zone ui modal_backdrop", "zone ui message_box", "zone ui toast",
        "zone ui row", "zone ui ephemerals", "zone particles particle", "zone particles particles",
        "zone particles particles_help_screen", "zone particles burst", "zone particles transit_dots",
        "zone game", "zone load",
    };
    for (size_t i = 0; i < sizeof zones / sizeof zones[0]; i++) check_peak(state, zones[i], NULL);
    static const char *const caps[] = {
        "cap button", "cap modal_backdrop", "cap message_box", "cap toast", "cap row", "cap ephemerals",
        "cap particle", "cap particles", "cap particles_help_screen", "cap burst", "cap transit_dots",
        "cap exit_queue",
    };
    for (size_t i = 0; i < sizeof caps / sizeof caps[0]; i++) check_peak(state, caps[i], NULL);
    check_peak(state, "cap row", &peak);
    assert(peak > 0 && "open panels take rows");
    check_peak(state, "cap exit_queue", &peak);
    assert(peak > 0 && "entities leave the live list during the replay");
    free(state);
}

typedef struct {
    size_t game;
    size_t load;
    size_t frame;
    int exit_queue;
    int live[SLOT_KIND_COUNT];
} memory_t;

static void measure(memory_t *m) {
    memset(m, 0, sizeof *m);
    m->game = game_zone_used();
    m->load = load_zone_used();
    m->frame = frame_zone_used();
    m->exit_queue = exit_queue_count();
    for (int s = 0; s < SLOT_KIND_COUNT; s++) m->live[s] = ui_zone_live((slot_kind_t)s);
}

/* In a child: writing the state after the replay takes and releases nothing. */
static void write_without_touching_memory(void) {
    static replay_loop_t r;
    replay_loop_open(&r, "tests/e2e/trade_rich_mine.json");
    for (int i = 0; i < replay_loop_frames(&r); i++) replay_loop_play_frame(&r, i);
    memory_t before, after;
    measure(&before);
    FILE *out = fopen("/dev/null", "w");
    assert(out != NULL);
    state_write(out);
    fclose(out);
    measure(&after);
    assert(memcmp(&before, &after, sizeof before) == 0 && "writing the state touched the game zones");
    _exit(0);
}

static void writing_the_state_leaves_zones_alone(void) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) write_without_touching_memory();
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

#define STATE_FATAL "tmp/test_state_fatal.txt"

/* A p8_fatal during a replay is itself a measurement: the state is
 * still written, with the stop frame first. */
static void a_fatal_writes_the_state_up_to_the_stop(void) {
    remove(STATE_FATAL);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        static replay_loop_t r;
        static int frame;
        assert(freopen("/dev/null", "w", stderr) != NULL);
        replay_loop_open(&r, "tests/e2e/trade_rich_mine.json");
        state_write_on_exit(STATE_FATAL, &frame);
        for (frame = 0; frame < 120; frame++) replay_loop_play_frame(&r, frame);
        p8_fatal("test zone full");
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 3 && "p8_fatal exits with 3");
    char *state = read_file(STATE_FATAL);
    assert(strncmp(state, "stopped p8_fatal frame 120\n", 27) == 0);
    assert(value(state, "count planets") > 0);
    free(state);
    remove(STATE_FATAL);
}

/* A replay that runs to the end writes no stop line. */
static void a_full_replay_has_no_stop_line(void) {
    char *state = read_file(STATE_TRM);
    assert(strstr(state, "stopped") == NULL);
    free(state);
}

int main(void) {
    one_planet_line_per_game_planet();
    the_built_network_shows();
    invented_techs_are_named();
    resources_reflect_the_cheat();
    game_peak_matches_the_fill_report();
    every_zone_and_cap_has_its_peak();
    writing_the_state_leaves_zones_alone();
    a_fatal_writes_the_state_up_to_the_stop();
    a_full_replay_has_no_stop_line();
    printf("test_state: all tests passed\n");
    return 0;
}
