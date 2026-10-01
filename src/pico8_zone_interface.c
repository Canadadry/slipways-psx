#include "pico8_zone_interface.h"

#include <assert.h>
#include <stddef.h>

#include "engine/zone_sizes.h"

/* Taille : la plus grande place que le jeu construit, en-tete d'arene
 * compris (x86-64). Nombre : ce qui vit a la fois au pire de la frame. */
#define JS_SIZE 6240u
#define JS_COUNT 24u
#define FQ_SIZE 256u
#define FQ_COUNT 4u
#define MH_SIZE 4624u
#define MH_COUNT 6u
#define IV_SIZE 320u
#define IV_COUNT 128u
#define RANGEE_SIZE 1192u
#define RANGEE_COUNT 100u
#define LE_SIZE 216u
#define LE_COUNT 16u

typedef char pico8_zone_interface_exceeded[
    (PICO8_SLOT_ZONE_BYTES(JS_SIZE, JS_COUNT) + PICO8_SLOT_ZONE_BYTES(FQ_SIZE, FQ_COUNT) +
     PICO8_SLOT_ZONE_BYTES(MH_SIZE, MH_COUNT) + PICO8_SLOT_ZONE_BYTES(IV_SIZE, IV_COUNT) +
     PICO8_SLOT_ZONE_BYTES(RANGEE_SIZE, RANGEE_COUNT) + PICO8_SLOT_ZONE_BYTES(LE_SIZE, LE_COUNT)
     <= PICO8_ZONE_INTERFACE_BYTES) ? 1 : -1];

PICO8_SLOT_ZONE_STORAGE(js_cells, js_taken, JS_SIZE, JS_COUNT);
PICO8_SLOT_ZONE_STORAGE(fq_cells, fq_taken, FQ_SIZE, FQ_COUNT);
PICO8_SLOT_ZONE_STORAGE(mh_cells, mh_taken, MH_SIZE, MH_COUNT);
PICO8_SLOT_ZONE_STORAGE(iv_cells, iv_taken, IV_SIZE, IV_COUNT);
PICO8_SLOT_ZONE_STORAGE(rangee_cells, rangee_taken, RANGEE_SIZE, RANGEE_COUNT);
PICO8_SLOT_ZONE_STORAGE(le_cells, le_taken, LE_SIZE, LE_COUNT);

static const pico8_zone_sorte_t SORTES[PICO8_SORTE_COUNT] = {
    [PICO8_SORTE_JS] = {"interface js", "place js", js_cells, js_taken, JS_SIZE, JS_COUNT},
    [PICO8_SORTE_FQ] = {"interface fq", "place fq", fq_cells, fq_taken, FQ_SIZE, FQ_COUNT},
    [PICO8_SORTE_MH] = {"interface mh", "place mh", mh_cells, mh_taken, MH_SIZE, MH_COUNT},
    [PICO8_SORTE_IV] = {"interface iv", "place iv", iv_cells, iv_taken, IV_SIZE, IV_COUNT},
    [PICO8_SORTE_RANGEE] = {"interface rangee", "place rangee", rangee_cells, rangee_taken, RANGEE_SIZE,
                            RANGEE_COUNT},
    [PICO8_SORTE_LE] = {"interface le", "place le", le_cells, le_taken, LE_SIZE, LE_COUNT},
};

static pico8_slot_zone_t g_zones[PICO8_SORTE_COUNT];
static pico8_zone_choisie_t g_handles[PICO8_SORTE_COUNT];
static int g_ready;

/* Paresseuse : pico8_slot_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void init_once(void) {
    if (g_ready) return;
    pico8_zone_sortes_init(SORTES, PICO8_SORTE_COUNT, g_zones, g_handles);
    g_ready = 1;
}

const pico8_zone_choisie_t *pico8_zone_interface(pico8_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_SORTE_COUNT);
    init_once();
    return &g_handles[sorte];
}

const pico8_zone_choisie_t *pico8_zone_interface_place(pico8_sorte_t sorte) {
    return pico8_zone_arene_prendre(pico8_zone_interface(sorte), SORTES[sorte].place_name, SORTES[sorte].size);
}

int pico8_zone_interface_vivantes(pico8_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_SORTE_COUNT);
    init_once();
    return (int)pico8_slot_zone_taken(&g_zones[sorte]);
}
