#include "pico8_audio.h"

/* See pico8_audio.h's own top comment: these two are, and stay, permanent
 * no-ops -- audio is explicitly out of scope for this port, not deferred
 * (docs/plan.md's "Garde-fous"). */

static int g_ih = 0;

void pico8_sfx(int n, int channel) {
    (void)n;
    (void)channel;
}

void pico8_music(int n) {
    (void)n;
}

/* See pico8_audio.h's own comment on this declaration. */
int pico8_ih(void) {
    return g_ih;
}

void pico8_set_ih(int value) {
    g_ih = value;
}

/* function rn(no)
 * if (no and not ih)sfx(no,3)
 * end
 * See pico8_audio.h's own comment on this declaration. */
void pico8_rn(pico8_value_t no) {
    int truthy = !(no.kind == PICO8_VALUE_BOOL && no.as.boolean == 0);

    if (truthy && !pico8_ih()) {
        int n = (no.kind == PICO8_VALUE_FIXED) ? (int)pico8_fixed_to_double(no.as.fixed) : 0;
        pico8_sfx(n, 3);
    }
}
