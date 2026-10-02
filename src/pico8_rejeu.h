#ifndef PICO8_REJEU_H
#define PICO8_REJEU_H

#include "pico8_cheat.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_mem.h"

/* Le moteur garde des pointeurs vers mem, gfx et input : `r` doit vivre
 * autant que le rejeu, sans jamais etre copie. */
typedef struct {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_cheat_t cheat;
} pico8_rejeu_t;

/* Charge gfx et map, les entrees de `events_path`, puis boote le cart sur
 * l'ecran titre ; un fichier illisible tue le processus par assert. */
void pico8_rejeu_ouvrir(pico8_rejeu_t *r, const char *events_path);
int pico8_rejeu_frames(const pico8_rejeu_t *r);
/* Joue la frame `i`, cheat compris, et avance les entrees sauf apres la
 * derniere. Les frames se jouent dans l'ordre, de 0 a frames-1. */
void pico8_rejeu_jouer_frame(pico8_rejeu_t *r, int i);
void pico8_rejeu_fermer(pico8_rejeu_t *r);

#endif
