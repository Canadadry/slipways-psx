#ifndef ENGINE_ALLOCATOR_H
#define ENGINE_ALLOCATOR_H

#include <stddef.h>

#include "engine/zone.h"

/* Zone designee : la poignee qu'un proprietaire ou un appelant passe aux
 * fonctions partagees qui allouent pour lui. Un bloc n'est agrandi ou rendu
 * que par la poignee qui l'a alloue. */
typedef struct allocator {
    unsigned char *(*alloc)(const struct allocator *z, size_t n);
    unsigned char *(*grow)(const struct allocator *z, unsigned char *p,
                           size_t old_n, size_t new_n);
    void (*release)(const struct allocator *z, unsigned char *p);
    bump_zone_t *bump; /* NULL hors zone a pointeur montant */
    slot_zone_t *slot; /* NULL hors zone a places */
} allocator_t;

/* Poignee sur une zone deja initialisee, qui doit lui survivre. Rendre n'y
 * fait rien : la memoire repart au reset de la zone. */
allocator_t allocator_from_bump_zone(bump_zone_t *bump);

/* Poignee sur une zone a places deja initialisee, qui doit lui survivre :
 * chaque bloc est une place entiere. alloc(n) exige n <= taille de place,
 * grow est refuse, rendre rend la place. */
allocator_t allocator_from_slot_zone(slot_zone_t *slot);

/* Le grow d'une poignee sur places : assert, rend NULL. */
unsigned char *allocator_grow_refused(const allocator_t *z, unsigned char *p,
                                       size_t old_n, size_t new_n);

/* Une sorte de places ; place_name NULL quand la place n'est pas une arene. */
typedef struct {
    const char *zone_name;
    const char *slot_name;
    zone_cell_t *cells;
    unsigned char *taken;
    size_t size;
    size_t count;
} slot_kind_zone_t;

/* Initialise zones[i] sur s[i] et range dans handles[i] sa poignee, pour i < n. */
void allocator_init_slot_kinds(const slot_kind_zone_t *s, int n, slot_zone_t *zones,
                            allocator_t *handles);

/* Poignee sur la generation courante d'une zone a deux bouts, rangee dans
 * un stockage a part : un champ de plus dans pico8_zone_choisie_t grossirait
 * la tete de chaque arene, et les places sont taillees au plus juste. */
typedef struct {
    allocator_t handle; /* en premier : son adresse est celle du tout */
    flip_zone_t *zone;
} flip_zone_allocator_t;

/* Range dans `storage` une poignee sur `zone`, deja initialisee, et la rend ;
 * storage et zone doivent survivre a la poignee. Rendre n'y fait rien et
 * agrandir copie : la memoire repart au rendu de sa generation. */
const allocator_t *allocator_from_flip_zone(flip_zone_allocator_t *storage,
                                                       flip_zone_t *zone);

/* Memoire non initialisee ; pico8_fatal si la zone est pleine. */
unsigned char *allocator_alloc(const allocator_t *z, size_t n);
/* Rend un bloc de new_n octets qui commence par les old_n octets de p ; p,
 * peut-etre NULL avec old_n nul, ne doit plus servir. */
unsigned char *allocator_grow(const allocator_t *z, unsigned char *p,
                               size_t old_n, size_t new_n);
/* p peut etre NULL. */
void allocator_release(const allocator_t *z, unsigned char *p);

/* Arene : un bloc de `size` octets pris a `from`, qui range dans sa tete
 * une zone montante privee et la poignee rendue, laquelle vit autant que le
 * bloc. Ce qu'on y alloue vit jusqu'a pico8_zone_arene_rendre ; y rendre un
 * bloc ne fait rien, y agrandir copie et laisse l'ancien bloc mort. `name`
 * doit vivre autant que l'arene : il compose « zone <name> pleine ». */
const allocator_t *arena_take(const allocator_t *from,
                                                     const char *name, size_t size);
/* Octets pris dans le bloc, tete comprise. */
size_t arena_used(const allocator_t *arena);
/* Remet l'arene a sa seule tete et empoisonne le reste de son bloc ; le
 * bloc reste pris a sa zone. Angle mort ASan : les allocations suivantes
 * reprennent les memes adresses, un pointeur garde d'avant le vidage y lit
 * le nouveau bloc sans planter. */
void arena_flush(const allocator_t *arena);
/* Rend le bloc entier a la poignee qui l'a donne. */
void arena_release(const allocator_t *arena);

#endif
