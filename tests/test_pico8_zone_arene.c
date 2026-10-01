/* Lie sans ASan, expres : la garde qui interdit de remettre une arene a zero
 * doit mordre dans le build ou PICO8_BUMP_ZONE_HALVES vaut 1. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "engine/zone.h"
#include "engine/allocator.h"
#include "zone_tas.h"

#define ARENA_SIZE 512

static void reset_arena(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    pico8_bump_zone_reset(arena->bump);
}

int main(void) {
    fflush(stdout);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        reset_arena();
        _exit(0);
    }
    int status;
    assert(waitpid(child, &status, 0) == child);
    if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGABRT) {
        fprintf(stderr, "test_pico8_zone_arene: remettre une arene a zero doit s'arreter sur un assert\n");
        return 1;
    }
    printf("test_pico8_zone_arene: all tests passed\n");
    return 0;
}
