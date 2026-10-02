#include "p8/audio.h"

static int g_sfx_muted = 0;

/* TROU-10 */
void p8_sfx(int n, int channel) {
    (void)n;
    (void)channel;
}

/* TROU-10 */
void p8_music(int n) {
    (void)n;
}

int p8_sfx_muted(void) {
    return g_sfx_muted;
}

void p8_set_sfx_muted(int value) {
    g_sfx_muted = value;
}

void p8_play_sfx(p8_fixed_t no) {
    if (!p8_sfx_muted()) {
        int n = p8_fixed_flr_int(no);
        p8_sfx(n, 3);
    }
}
