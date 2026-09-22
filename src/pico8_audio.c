#include "pico8_audio.h"

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

void pico8_rn(pico8_value_t no) {
    int truthy = !(no.kind == PICO8_VALUE_BOOL && no.as.boolean == 0);

    if (truthy && !pico8_ih()) {
        int n = (no.kind == PICO8_VALUE_FIXED) ? (int)pico8_fixed_to_double(no.as.fixed) : 0;
        pico8_sfx(n, 3);
    }
}
