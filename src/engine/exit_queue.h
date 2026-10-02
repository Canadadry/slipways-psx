#ifndef PICO8_FILE_SORTIE_H
#define PICO8_FILE_SORTIE_H

#include "engine/entity_class.h"
#include "engine/allocator.h"

#define PICO8_FILE_SORTIE_CAP 256

/* Une entite qui a quitte hv, et dont la classe a `rendre` : la frame en
 * cours la relit encore, on ne la rend qu'au vidage. pico8_fatal si la file
 * est pleine. */
void pico8_file_sortie_mettre(entity_t *e);

/* Une arene que plus rien ne designe hors de la frame en cours, rendue
 * entiere au vidage. Meme limite que les entites. */
void pico8_file_sortie_mettre_arene(const pico8_zone_choisie_t *arene);

int pico8_file_sortie_compte(void);

/* Pour chaque entree, dans l'ordre : une entite passe par `oublier(e)`, qui
 * remet a nil ce qui la designe encore, puis par `rendre` ; une arene est
 * rendue. La file est vide au retour. */
void pico8_file_sortie_vider(void (*oublier)(const entity_t *e));

#endif
