#include "p8/audio.h"

static int g_ih = 0;

/* TROU-10 */
void pico8_sfx(int n, int channel) {
    (void)n;
    (void)channel;
}

/* TROU-10 */
void pico8_music(int n) {
    (void)n;
}

int pico8_ih(void) {
    return g_ih;
}

void pico8_set_ih(int value) {
    g_ih = value;
}

void pico8_rn(pico8_fixed_t no) {
    if (!pico8_ih()) {
        int n = pico8_fixed_flr_int(no);
        pico8_sfx(n, 3);
    }
}
