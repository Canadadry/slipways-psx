#include "pico8_file_sortie.h"

#include <assert.h>
#include <stdio.h>

#include "pico8_fatal.h"

static pico8_struct_header_t *g_file[PICO8_FILE_SORTIE_CAP];
static int g_compte;

void pico8_file_sortie_mettre(pico8_struct_header_t *e) {
    assert(e->cls->rendre != NULL && "file de sortie : la classe n'a rien a rendre");
    if (g_compte == PICO8_FILE_SORTIE_CAP) {
        char message[64];
        snprintf(message, sizeof message, "file de sortie pleine, limite %d", PICO8_FILE_SORTIE_CAP);
        pico8_fatal(message);
    }
    g_file[g_compte++] = e;
}

int pico8_file_sortie_compte(void) {
    return g_compte;
}

void pico8_file_sortie_vider(void (*oublier)(const pico8_struct_header_t *e)) {
    for (int i = 0; i < g_compte; i++) {
        pico8_struct_header_t *e = g_file[i];
        oublier(e);
        e->cls->rendre(e);
    }
    g_compte = 0;
}
