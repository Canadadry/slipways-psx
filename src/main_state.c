/* The make state driver: replays argv[1] from the title screen like
 * pico8_replay, cheat included, then writes the state of the last frame to
 * argv[2] (default tmp/state.txt). A pico8_fatal during the replay still
 * writes the state there, stop line first, and exits with 3. With no
 * argument, or on an argument starting with '-': usage and exit code 2. */

#include "pico8_replay_loop.h"
#include "pico8_state.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: pico8_state <events.json> [<state.txt>]\n");
        return 2;
    }
    const char *events_path = argv[1];
    const char *state_path = (argc > 2) ? argv[2] : "tmp/state.txt";

    static pico8_replay_loop_t loop;
    pico8_replay_loop_open(&loop, events_path);
    int frame_count = pico8_replay_loop_frames(&loop);
    static int frame;
    pico8_state_write_on_exit(state_path, &frame);
    for (frame = 0; frame < frame_count; frame++) pico8_replay_loop_play_frame(&loop, frame);
    pico8_state_release();

    FILE *out = fopen(state_path, "w");
    if (out == NULL) {
        fprintf(stderr, "pico8 state: cannot open '%s'\n", state_path);
        return 1;
    }
    pico8_state_write(out);
    if (fclose(out) != 0) {
        fprintf(stderr, "pico8 state: failed to write '%s'\n", state_path);
        return 1;
    }
    pico8_replay_loop_close(&loop);
    printf("pico8 state: %d frame(s) from %s; state written to %s\n", frame_count, events_path, state_path);
    return 0;
}
