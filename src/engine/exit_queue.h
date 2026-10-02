#ifndef ENGINE_EXIT_QUEUE_H
#define ENGINE_EXIT_QUEUE_H

#include "engine/entity_class.h"
#include "engine/allocator.h"

#define EXIT_QUEUE_CAP 256

/* Une entite qui a quitte hv, et dont la classe a `rendre` : la frame en
 * cours la relit encore, on ne la rend qu'au vidage. pico8_fatal si la file
 * est pleine. */
void exit_queue_put(entity_t *e);

/* Une arene que plus rien ne designe hors de la frame en cours, rendue
 * entiere au vidage. Meme limite que les entites. */
void exit_queue_put_arena(const allocator_t *arena);

int exit_queue_count(void);

/* Pour chaque entree, dans l'ordre : une entite passe par `oublier(e)`, qui
 * remet a nil ce qui la designe encore, puis par `rendre` ; une arene est
 * rendue. La file est vide au retour. */
void exit_queue_flush(void (*forget)(const entity_t *e));

#endif
