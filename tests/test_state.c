/* make state: replay with STATE set writes the text state of the last frame. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "devtools/state.h"
#include "engine/zone_sizes.h"
#include "p8/fatal.h"

#define STATE "tmp/test_state.txt"
#define REPLAY(events) "STATE=" STATE " ./tmp/replay " events " tmp/test_state.bin tmp/test_state_dump.txt > /dev/null"

static char text[1 << 16];

static void replay(const char *command) {
    remove(STATE);
    assert(system(command) == 0);
    FILE *f = fopen(STATE, "r");
    assert(f != NULL && "state file missing");
    size_t n = fread(text, 1, sizeof text - 1, f);
    text[n] = '\0';
    fclose(f);
}

static int has(const char *line) {
    char pattern[128];
    snprintf(pattern, sizeof pattern, "\n%s\n", line);
    return strstr(text, pattern) != NULL;
}

static const char *const MEMORY_LINES[] = {
    "zone frame", "zone ui", "zone particles", "zone game", "zone load",
    "cap button", "cap modal_backdrop", "cap message_box", "cap toast", "cap row", "cap ephemerals",
    "cap bytes_button", "cap bytes_modal_backdrop", "cap bytes_message_box", "cap bytes_toast", "cap bytes_row",
    "cap bytes_ephemerals", "cap particle", "cap particles", "cap particles_help_screen", "cap burst",
    "cap transit_dots", "cap exit_queue", "cap elements_particles", "cap elements_particles_help_screen",
    "cap elements_transit_dots", "cap elements_ephemerals", "cap bytes_particles", "cap bytes_particles_help_screen", "cap bytes_burst",
};

/* "-" reads as -1. */
static long memory_value(const char *line, int column) {
    char prefix[16], name[64], v[3][32];
    assert(sscanf(line, "%15s %63s %31s %31s %31s", prefix, name, v[0], v[1], v[2]) == 5);
    return strcmp(v[column], "-") == 0 ? -1 : atol(v[column]);
}

/* The line of that name, without its newline, in a buffer the next call
 * overwrites; NULL if absent. */
static const char *memory_line(const char *name) {
    static char line[160];
    char pattern[96];
    snprintf(pattern, sizeof pattern, "\n%s ", name);
    const char *at = strstr(text, pattern);
    if (at == NULL) return NULL;
    at++;
    snprintf(line, sizeof line, "%.*s", (int)strcspn(at, "\n"), at);
    return line;
}

/* Every zone and cap line: "<zone|cap> <name> <current> <peak> <limit>". */
static void memory_lines_have_peak_and_limit(void) {
    for (size_t i = 0; i < sizeof MEMORY_LINES / sizeof MEMORY_LINES[0]; i++) {
        const char *line = memory_line(MEMORY_LINES[i]);
        assert(line != NULL && "a zone or cap line is missing");
        char copy[160];
        snprintf(copy, sizeof copy, "%s", line);
        char extra[8];
        char prefix[16], name[64], v[3][32];
        assert(sscanf(copy, "%15s %63s %31s %31s %31s %7s", prefix, name, v[0], v[1], v[2], extra) == 5 &&
               "five columns, prefix included");
        long current = memory_value(copy, 0), peak = memory_value(copy, 1), limit = memory_value(copy, 2);
        assert(peak >= 0 && limit > 0);
        assert(peak >= current && "the peak is never under the current");
        assert(peak <= limit && "a clean replay stays within its limits");
    }
    const char *frame = memory_line("zone frame");
    assert(memory_value(frame, 1) > 0);
    assert(memory_value(frame, 2) == (long)FRAME_ZONE_BYTES);
    assert(memory_value(memory_line("cap message_box"), 1) >= 1 && "the replay opens a message box");
    assert(memory_value(memory_line("cap bytes_message_box"), 1) > 0);
}

static void trade_rich_mine(void) {
    replay(REPLAY("tests/e2e/trade_rich_mine.json"));
    assert(has("count planets 93"));
    assert(has("count slipways 7"));
    assert(has("count colonies 7") && "p1, p27, p48, p71 and the three labs");
    assert(has("count structures 0"));
    assert(has("planet p1 colonized planet e world 259.10 206.16 screen 67.10 14.16 visible 1"));
    assert(has("planet p2 unknown planet - world 60.38 236.90 screen -131.61 44.90 visible 0"));
    assert(has("slipway p27 p1 normal"));
    assert(has("slipway p1 s1 normal"));
    assert(has("techs space folding"));
    assert(has("techs geneseeds"));
    assert(strstr(text, "stopped") == NULL && "a replay that runs to the end writes no stop line");
    memory_lines_have_peak_and_limit();
}

/* The cheat replayed as is: 99 science, $200 at the last frame. */
static void cheat_code(void) {
    replay(REPLAY("tests/e2e/cheat_code.json"));
    assert(strstr(text, "\nresources credits 200\n") != NULL);
    assert(strstr(text, "\nresources science 99\n") != NULL);
    memory_lines_have_peak_and_limit();
}

/* A p8_fatal still writes the state, with the stop frame first. */
static void fatal_writes_the_stop_frame(void) {
    remove(STATE);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        static const int frame = 120;
        assert(freopen("/dev/null", "w", stderr) != NULL);
        state_write_on_exit(STATE, &frame);
        p8_fatal("test zone full");
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 3 && "p8_fatal exits with 3");
    FILE *f = fopen(STATE, "r");
    assert(f != NULL);
    size_t n = fread(text, 1, sizeof text - 1, f);
    text[n] = '\0';
    fclose(f);
    assert(strncmp(text, "stopped p8_fatal frame 120\n", 27) == 0);
    assert(memory_line("zone frame") != NULL && memory_line("cap exit_queue") != NULL &&
           "the memory section follows the stop line");
}

int main(void) {
    trade_rich_mine();
    cheat_code();
    fatal_writes_the_stop_frame();
    remove(STATE);
    remove("tmp/test_state.bin");
    remove("tmp/test_state_dump.txt");
    printf("test_state: all tests passed\n");
    return 0;
}
