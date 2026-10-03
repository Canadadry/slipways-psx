/* make state: replay with STATE set writes the text state of the last frame. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "devtools/state.h"
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
}

/* The cheat replayed as is: 99 science, $200 at the last frame. */
static void cheat_code(void) {
    replay(REPLAY("tests/e2e/cheat_code.json"));
    assert(strstr(text, "\nresources credits 200\n") != NULL);
    assert(strstr(text, "\nresources science 99\n") != NULL);
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
    char first[64];
    assert(fgets(first, sizeof first, f) != NULL);
    fclose(f);
    assert(strcmp(first, "stopped p8_fatal frame 120\n") == 0);
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
