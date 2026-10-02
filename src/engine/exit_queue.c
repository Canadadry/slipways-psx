#include "engine/exit_queue.h"

#include <assert.h>
#include <stdio.h>

#include "p8/fatal.h"

typedef struct {
    entity_t *entite;
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

void pico8_file_sortie_mettre(entity_t *e) {
    entree_t entree = {e, NULL};
    assert(e->class->release != NULL && "file de sortie : la classe n'a rien a rendre");
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

void pico8_file_sortie_vider(void (*oublier)(const entity_t *e)) {
    for (int i = 0; i < g_compte; i++) {
        entity_t *e = g_file[i].entite;
        if (e == NULL) {
            pico8_zone_arene_rendre(g_file[i].arene);
            continue;
        }
        oublier(e);
        e->class->release(e);
    }
    g_compte = 0;
}
