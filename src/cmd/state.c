/* The make state driver: replays argv[1] from the title screen like
 * replay, cheat included, then writes the state of the last frame to
 * argv[2] (default tmp/state.txt). A p8_fatal during the replay still
 * writes the state there, stop line first, and exits with 3. With no
 * argument, or on an argument starting with '-': usage and exit code 2. */

#include "devtools/replay_loop.h"
#include "devtools/state.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: state <events.json> [<state.txt>]\n");
        return 2;
    }
    const char *events_path = argv[1];
    const char *state_path = (argc > 2) ? argv[2] : "tmp/state.txt";

    static replay_loop_t loop;
    replay_loop_open(&loop, events_path);
    int frame_count = replay_loop_frames(&loop);
    static int frame;
    state_write_on_exit(state_path, &frame);
    for (frame = 0; frame < frame_count; frame++) replay_loop_play_frame(&loop, frame);
    state_release();

    FILE *out = fopen(state_path, "w");
    if (out == NULL) {
        fprintf(stderr, "state: cannot open '%s'\n", state_path);
        return 1;
    }
    state_write(out);
    if (fclose(out) != 0) {
        fprintf(stderr, "state: failed to write '%s'\n", state_path);
        return 1;
    }
    replay_loop_close(&loop);
    printf("state: %d frame(s) from %s; state written to %s\n", frame_count, events_path, state_path);
    return 0;
}
