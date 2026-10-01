#include "pico8_file_sortie.h"

#include <assert.h>
#include <stdio.h>

#include "pico8_fatal.h"

typedef struct {
    pico8_struct_header_t *entite;
    const pico8_zone_choisie_t *arene;
} entree_t;

static entree_t g_file[PICO8_FILE_SORTIE_CAP];
static int g_compte;

static void mettre(entree_t entree) {
    if (g_compte == PICO8_FILE_SORTIE_CAP) {
        char message[64];
        snprintf(message, sizeof message, "file de sortie pleine, limite %d", PICO8_FILE_SORTIE_CAP);
        pico8_fatal(message);
    }
    g_file[g_compte++] = entree;
}

void pico8_file_sortie_mettre(pico8_struct_header_t *e) {
    entree_t entree = {e, NULL};
    assert(e->cls->rendre != NULL && "file de sortie : la classe n'a rien a rendre");
    mettre(entree);
}

void pico8_file_sortie_mettre_arene(const pico8_zone_choisie_t *arene) {
    entree_t entree = {NULL, arene};
    assert(arene != NULL);
    mettre(entree);
}

int pico8_file_sortie_compte(void) {
    return g_compte;
}

void pico8_file_sortie_vider(void (*oublier)(const pico8_struct_header_t *e)) {
    for (int i = 0; i < g_compte; i++) {
        pico8_struct_header_t *e = g_file[i].entite;
        if (e == NULL) {
            pico8_zone_arene_rendre(g_file[i].arene);
            continue;
        }
        oublier(e);
        e->cls->rendre(e);
    }
    g_compte = 0;
}
