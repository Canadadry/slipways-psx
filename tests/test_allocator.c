/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "engine/collect.h"
#include "engine/entity_list.h"
#include "engine/pointer_list.h"
#include "p8/str.h"
#include "engine/zone.h"
#include "engine/allocator.h"
#include "heap_allocator.h"

#define STDERR_LOG "tmp/test_pico8_zone_choisie_stderr.txt"
#define CAPACITY 1024

PICO8_BUMP_ZONE_STORAGE(bump_cells, CAPACITY);

static pico8_bump_zone_t bump;
static pico8_zone_choisie_t handle;

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

/* Fait grandir un tableau de 1 a 64 octets en doublant, comme le font les
 * tableaux croissants, en ecrivant i a l'indice i. */
static unsigned char *grow_counting(const pico8_zone_choisie_t *z) {
    size_t capacity = 1;
    unsigned char *items = pico8_zone_alloc(z, capacity);
    for (size_t i = 0; i < 64; i++) {
        if (i == capacity) {
            items = pico8_zone_grow(z, items, capacity, capacity * 2);
            capacity *= 2;
        }
        items[i] = (unsigned char)i;
    }
    return items;
}

static void expect_counting(const unsigned char *items) {
    for (size_t i = 0; i < 64; i++) assert(items[i] == i && "agrandir garde le contenu, dans l'ordre");
}

static int case_bump_grow_keeps_order(void) {
    unsigned char *items = grow_counting(&handle);
    expect_counting(items);
    pico8_zone_release(&handle, items);
    expect_counting(items);
    return 0;
}

static int case_bump_read_before_reset(void) {
    unsigned char *items = grow_counting(&handle);
    sink = items[63];
    pico8_bump_zone_reset(&bump);
    return 0;
}

static int case_bump_stale_after_reset(void) {
    unsigned char *items = grow_counting(&handle);
    pico8_bump_zone_reset(&bump);
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(&handle, 64), 2, 64);
    sink = items[63];
    return 0;
}

static int case_bump_grow_past_capacity(void) {
    unsigned char *items = pico8_zone_alloc(&handle, 512);
    pico8_zone_grow(&handle, items, 512, 1024);
    return 0;
}

#define LIST_LENGTH 12

static entity_t cells[LIST_LENGTH];

static pico8_ents_t grown_ents(void) {
    pico8_ents_t ents;
    pico8_ents_init(&ents, &handle);
    for (int i = 0; i < LIST_LENGTH; i++) pico8_ents_add(&ents, &cells[i]);
    return ents;
}

static int is_even_cell(entity_t *e, void *ctx) {
    (void)ctx;
    return (e - cells) % 2 == 0;
}

static int case_ents_grow_keeps_order(void) {
    pico8_ents_t ents = grown_ents();
    assert(ents.count == LIST_LENGTH);
    for (int i = 0; i < LIST_LENGTH; i++) assert(ents.items[i] == &cells[i]);

    pico8_ents_t both = pico8_ents_cat(&handle, &ents, &ents);
    assert(both.count == 2 * LIST_LENGTH);
    for (int i = 0; i < 2 * LIST_LENGTH; i++) assert(both.items[i] == &cells[i % LIST_LENGTH]);

    pico8_ents_t even = pico8_jp_ents(&handle, &ents, is_even_cell, NULL);
    assert(even.count == LIST_LENGTH / 2);
    for (int i = 0; i < LIST_LENGTH / 2; i++) assert(even.items[i] == &cells[2 * i]);

    pico8_ents_iter_t it = pico8_ents_all(&ents);
    entity_t *e;
    int seen = 0;
    while (pico8_ents_iter_next(&it, &e)) {
        assert(e == &cells[seen] && "all() voit chaque element malgre la suppression");
        seen++;
        pico8_ents_del(&ents, e);
    }
    assert(seen == LIST_LENGTH && ents.count == 0);

    pico8_ents_free(&ents);
    assert(ents.count == 0 && ents.items == NULL);
    pico8_ents_add(&ents, &cells[0]);
    assert(ents.count == 1 && ents.items[0] == &cells[0]);
    return 0;
}

static int case_ptrs_grow_keeps_order(void) {
    pico8_ptrs_t ptrs;
    pico8_ptrs_init(&ptrs, &handle);
    for (int i = 0; i < LIST_LENGTH; i++) pico8_ptrs_add(&ptrs, &cells[i]);
    assert(ptrs.count == LIST_LENGTH);
    for (int i = 0; i < LIST_LENGTH; i++) assert(ptrs.items[i] == &cells[i]);

    pico8_ptrs_iter_t it = pico8_ptrs_all(&ptrs);
    void *e;
    int seen = 0;
    while (pico8_ptrs_iter_next(&it, &e)) {
        assert(e == &cells[seen] && "all() voit chaque element malgre la suppression");
        seen++;
        pico8_ptrs_del(&ptrs, e);
    }
    assert(seen == LIST_LENGTH && ptrs.count == 0);

    pico8_ptrs_free(&ptrs);
    assert(ptrs.count == 0 && ptrs.items == NULL);
    return 0;
}

static int case_ents_read_before_reset(void) {
    pico8_ents_t ents = grown_ents();
    sink = (unsigned char)(ents.items[LIST_LENGTH - 1] == &cells[LIST_LENGTH - 1]);
    pico8_bump_zone_reset(&bump);
    return 0;
}

static int case_ents_stale_after_reset(void) {
    pico8_ents_t ents = grown_ents();
    pico8_bump_zone_reset(&bump);
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(&handle, 64), 2, 64);
    sink = (unsigned char)(ents.items[LIST_LENGTH - 1] == &cells[LIST_LENGTH - 1]);
    return 0;
}

static void expect_text(char *got, const char *expected) {
    if (strcmp(got, expected) != 0) fprintf(stderr, "attendu \"%s\", obtenu \"%s\"\n", expected, got);
    assert(strcmp(got, expected) == 0);
}

static int case_texts_in_bump(void) {
    expect_text(p8_concat2(&handle, "level ", "3"), "level 3");
    expect_text(p8_concat2(&handle, "", "p7"), "p7");
    expect_text(p8_concat2(&handle, "p7", ""), "p7");
    expect_text(p8_concat2(&handle, "", ""), "");
    expect_text(p8_sub(&handle, "hello", 2, 4), "ell");
    expect_text(p8_sub(&handle, "hello", 3, 2), "");
    expect_text(p8_tostr_fixed(&handle, p8_fixed_from_int(-5)), "-5");
    expect_text(p8_tostr_fixed(&handle, (p8_fixed_t)0x00028000u), "2.5");

    int color = -1;
    expect_text(pico8_ne(&handle, p8_fixed_from_int(12), 10, "$", &color), "+12$");
    assert(color == 10);
    expect_text(pico8_ne(&handle, p8_fixed_from_int(12), 3, NULL, &color), "+12");
    assert(color == 3);
    expect_text(pico8_ne(&handle, p8_fixed_from_int(-4), 10, "$", &color), "-4$");
    assert(color == 8);
    expect_text(pico8_ne(&handle, p8_fixed_from_int(-4), 10, NULL, &color), "-4");
    assert(color == 8);

    pico8_qz_box_t *box = pico8_qz(&handle, p8_fixed_from_int(0), p8_fixed_from_int(-1),
                                   p8_fixed_from_int(20), p8_fixed_from_int(11));
    assert(pico8_qz_call(box, (pico8_vec_t){p8_fixed_from_int(5), p8_fixed_from_int(5)}));
    assert(!pico8_qz_call(box, (pico8_vec_t){p8_fixed_from_int(21), p8_fixed_from_int(5)}));
    return 0;
}

#define ARENA_SIZE 512

static unsigned char *arena_counting(const pico8_zone_choisie_t *arena) {
    unsigned char *items = pico8_zone_alloc(arena, 64);
    for (int i = 0; i < 64; i++) items[i] = (unsigned char)i;
    return items;
}

static int case_arena_alive_until_released(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    unsigned char *items = arena_counting(arena);
    unsigned char *grown = grow_counting(arena);
    pico8_zone_release(arena, items);
    expect_counting(items);
    expect_counting(grown);
    pico8_zone_arene_rendre(arena);
    return 0;
}

static int case_arena_dead_after_release(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    unsigned char *items = arena_counting(arena);
    pico8_zone_arene_rendre(arena);
    sink = items[63];
    return 0;
}

static int case_arena_full(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    pico8_zone_alloc(arena, ARENA_SIZE);
    return 0;
}

static int case_arena_taken_counts_head_and_blocks(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    size_t head = pico8_zone_arene_prise(arena);
    assert(head > 0 && head < (size_t)ARENA_SIZE && "la tete de l'arene compte dans ce qu'elle a pris");
    pico8_zone_alloc(arena, 10);
    assert(pico8_zone_arene_prise(arena) == head + 16 && "un bloc compte en cellules entieres");
    pico8_zone_alloc(arena, ARENA_SIZE - head - 16);
    assert(pico8_zone_arene_prise(arena) == (size_t)ARENA_SIZE && "une arene pleine a tout pris");
    pico8_zone_arene_rendre(arena);
    return 0;
}

static int case_arena_vider_back_to_head(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    size_t head = pico8_zone_arene_prise(arena);
    unsigned char *before = arena_counting(arena);
    pico8_zone_arene_vider(arena);
    assert(pico8_zone_arene_prise(arena) == head && "une arene videe n'a plus que sa tete");
    unsigned char *after = arena_counting(arena);
    /* L'angle mort du recyclage : la meme adresse revient, un pointeur
     * perime y lirait le nouveau bloc sans planter. */
    assert(after == before && "une arene videe reprend depuis le debut de son bloc");
    expect_counting(after);
    pico8_zone_arene_rendre(arena);
    return 0;
}

static int case_arena_vider_kills_old_block(void) {
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", ARENA_SIZE);
    unsigned char *items = arena_counting(arena);
    pico8_zone_arene_vider(arena);
    sink = items[63];
    return 0;
}

#define RESERVED 40

/* Une arene qui tient juste la liste et ses RESERVED elements : un seul
 * agrandissement la ferait deborder. */
static const pico8_zone_choisie_t *reserved_list(pico8_ptrs_t **out) {
    const pico8_zone_choisie_t *probe = pico8_zone_arene_prendre(&zone_tas, "sonde", ARENA_SIZE);
    size_t head = pico8_zone_arene_prise(probe);
    pico8_zone_arene_rendre(probe);
    size_t size = head + PICO8_ZONE_CELLS(sizeof(pico8_ptrs_t)) * sizeof(pico8_zone_cell_t) +
                  RESERVED * sizeof(void *);
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&zone_tas, "place essai", size);
    pico8_ptrs_t *ptrs = (pico8_ptrs_t *)pico8_zone_alloc(arena, sizeof *ptrs);
    pico8_ptrs_init_reserve(ptrs, arena, RESERVED);
    assert(pico8_zone_arene_prise(arena) == size);
    *out = ptrs;
    return arena;
}

static int case_ptrs_reserved_fills_without_growing(void) {
    pico8_ptrs_t *ptrs;
    const pico8_zone_choisie_t *arena = reserved_list(&ptrs);
    assert(ptrs->count == 0);
    for (int i = 0; i < RESERVED; i++) pico8_ptrs_add(ptrs, &cells[i % LIST_LENGTH]);
    assert(ptrs->count == RESERVED);
    for (int i = 0; i < RESERVED; i++) assert(ptrs->items[i] == &cells[i % LIST_LENGTH]);
    pico8_zone_arene_rendre(arena);
    return 0;
}

static int case_ptrs_reserved_overflow(void) {
    pico8_ptrs_t *ptrs;
    reserved_list(&ptrs);
    for (int i = 0; i <= RESERVED; i++) pico8_ptrs_add(ptrs, &cells[i % LIST_LENGTH]);
    return 0;
}

/* Les places voisines d'une arene, hors du bloc qu'elle a pris : sous ASan,
 * l'arene ne doit pas les empoisonner. */
static pico8_zone_cell_t neighbours[2 * PICO8_ZONE_CELLS(ARENA_SIZE)];

static unsigned char *first_neighbour(const pico8_zone_choisie_t *z, size_t n) {
    (void)z;
    assert(n == ARENA_SIZE);
    return (unsigned char *)neighbours;
}

static void no_grow_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    (void)z;
    (void)p;
}

static int case_arena_spares_neighbour(void) {
    static const pico8_zone_choisie_t places = {first_neighbour, NULL, no_grow_release, NULL, NULL};
    unsigned char *next = (unsigned char *)neighbours + ARENA_SIZE;
    memset(next, 7, ARENA_SIZE);
    const pico8_zone_choisie_t *arena = pico8_zone_arene_prendre(&places, "place essai", ARENA_SIZE);
    for (int i = 0; i < ARENA_SIZE; i++) assert(next[i] == 7);
    sink = arena_counting(arena)[63];
    pico8_zone_arene_rendre(arena);
    return 0;
}

#define SLOT_SIZE 64

PICO8_SLOT_ZONE_STORAGE(slot_cells, slot_taken, SLOT_SIZE, 2);

static pico8_slot_zone_t slots;
static pico8_zone_choisie_t slot_handle;

static int case_slot_handle_round_trip(void) {
    unsigned char *a = pico8_zone_alloc(&slot_handle, SLOT_SIZE);
    unsigned char *b = pico8_zone_alloc(&slot_handle, 1);
    memset(a, 4, SLOT_SIZE);
    *b = 5;
    assert(pico8_slot_zone_taken(&slots) == 2);
    pico8_zone_release(&slot_handle, a);
    assert(pico8_slot_zone_taken(&slots) == 1 && "rendre par la poignee rend la place");
    assert(*b == 5);
    pico8_zone_release(&slot_handle, NULL);
    assert(pico8_slot_zone_taken(&slots) == 1);
    return 0;
}

static int case_slot_handle_too_big(void) {
    pico8_zone_alloc(&slot_handle, SLOT_SIZE + 1);
    return 0;
}

static int case_slot_handle_no_grow(void) {
    pico8_zone_grow(&slot_handle, pico8_zone_alloc(&slot_handle, 8), 8, 16);
    return 0;
}

static int case_slot_handle_stale_after_release(void) {
    unsigned char *a = pico8_zone_alloc(&slot_handle, SLOT_SIZE);
    memset(a, 4, SLOT_SIZE);
    pico8_zone_release(&slot_handle, a);
    sink = a[0];
    return 0;
}

static int case_slot_handle_full(void) {
    for (int i = 0; i < 3; i++) pico8_zone_alloc(&slot_handle, 8);
    return 0;
}

PICO8_BASCULE_ZONE_STORAGE(bascule_cells, CAPACITY);

static pico8_bascule_zone_t bascule;
static pico8_zone_choisie_bascule_t bascule_storage;
static const pico8_zone_choisie_t *bascule_handle;

/* Un bloc de 8 octets a 6, agrandi a 16 : l'ancien bloc et sa copie. */
static unsigned char *bascule_grown(unsigned char **copy) {
    unsigned char *old = pico8_zone_alloc(bascule_handle, 8);
    memset(old, 6, 8);
    *copy = pico8_zone_grow(bascule_handle, old, 8, 16);
    return old;
}

static int case_bascule_handle_round_trip(void) {
    unsigned char *items = grow_counting(bascule_handle);
    expect_counting(items);
    size_t prise = pico8_bascule_zone_prise(&bascule);
    pico8_zone_release(bascule_handle, items);
    pico8_zone_release(bascule_handle, NULL);
    assert(pico8_bascule_zone_prise(&bascule) == prise && "rendre par la poignee ne fait rien");
    expect_counting(items);
    unsigned char *copy;
    unsigned char *old = bascule_grown(&copy);
    for (size_t i = 0; i < 8; i++) assert(copy[i] == 6 && "agrandir copie");
    memset(copy + 8, 7, 8);
    pico8_bascule_zone_basculer(&bascule);
    memset(pico8_zone_alloc(bascule_handle, 64), 2, 64);
    for (size_t i = 0; i < 8; i++)
        assert(old[i] == 6 && copy[i] == 6 && "l'ancienne generation vit jusqu'a son rendu");
    expect_counting(items);
    return 0;
}

static int case_bascule_handle_stale_after_rendre(void) {
    unsigned char *copy;
    unsigned char *old = bascule_grown(&copy);
    pico8_bascule_zone_basculer(&bascule);
    pico8_bascule_zone_rendre(&bascule);
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(bascule_handle, 64), 2, 64);
    sink = old[0];
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"slot_handle_round_trip", case_slot_handle_round_trip},
    {"slot_handle_too_big", case_slot_handle_too_big},
    {"slot_handle_no_grow", case_slot_handle_no_grow},
    {"slot_handle_stale_after_release", case_slot_handle_stale_after_release},
    {"slot_handle_full", case_slot_handle_full},
    {"bascule_handle_round_trip", case_bascule_handle_round_trip},
    {"bascule_handle_stale_after_rendre", case_bascule_handle_stale_after_rendre},
    {"arena_alive_until_released", case_arena_alive_until_released},
    {"arena_dead_after_release", case_arena_dead_after_release},
    {"arena_full", case_arena_full},
    {"arena_spares_neighbour", case_arena_spares_neighbour},
    {"arena_taken_counts_head_and_blocks", case_arena_taken_counts_head_and_blocks},
    {"arena_vider_back_to_head", case_arena_vider_back_to_head},
    {"arena_vider_kills_old_block", case_arena_vider_kills_old_block},
    {"ptrs_reserved_fills_without_growing", case_ptrs_reserved_fills_without_growing},
    {"ptrs_reserved_overflow", case_ptrs_reserved_overflow},
    {"bump_grow_keeps_order", case_bump_grow_keeps_order},
    {"bump_read_before_reset", case_bump_read_before_reset},
    {"bump_stale_after_reset", case_bump_stale_after_reset},
    {"bump_grow_past_capacity", case_bump_grow_past_capacity},
    {"ents_grow_keeps_order", case_ents_grow_keeps_order},
    {"ptrs_grow_keeps_order", case_ptrs_grow_keeps_order},
    {"ents_read_before_reset", case_ents_read_before_reset},
    {"ents_stale_after_reset", case_ents_stale_after_reset},
    {"texts_in_bump", case_texts_in_bump},
};

#define CASE_COUNT (sizeof CASES / sizeof CASES[0])

/* Tampon alloue, termine par un nul, que l'appelant libere. */
static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size >= 0);
    rewind(file);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    buf[size] = '\0';
    fclose(file);
    return buf;
}

static const char *self_path;

/* Rend le statut brut de system() ; *err recoit le stderr de l'enfant, que
 * l'appelant libere. */
static int run_child(const char *name, char **err) {
    char cmd[512];
    remove(STDERR_LOG);
    snprintf(cmd, sizeof cmd, "%s %s > /dev/null 2> " STDERR_LOG, self_path, name);
    int status = system(cmd);
    assert(status != -1 && WIFEXITED(status));
    *err = read_whole_file(STDERR_LOG);
    return status;
}

static void expect_clean(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 0 || err[0] != '\0') {
        fprintf(stderr, "%s: attendu propre, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "le cas ne doit ni planter ni ecrire sur stderr");
    }
    free(err);
}

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: attendu use-after-poison, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "l'acces hors duree de vie doit planter sous ASan");
    }
    free(err);
}

static void expect_freed_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "heap-use-after-free") == NULL) {
        fprintf(stderr, "%s: attendu heap-use-after-free, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "un bloc d'arene lu apres le rendu de l'arene doit planter sous ASan");
    }
    free(err);
}

static void expect_assert(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu un assert \"%s\", sortie %d\n%s", name, message, WEXITSTATUS(status), err);
        assert(0 && "la poignee d'une zone a places refuse ce qu'une place ne tient pas");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "une croissance qui remplit la zone doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    pico8_bump_zone_init(&bump, "essai-liste", bump_cells, CAPACITY);
    handle = pico8_zone_choisie_bump(&bump);
    pico8_slot_zone_init(&slots, "essai-places", slot_cells, slot_taken, SLOT_SIZE, 2);
    slot_handle = pico8_zone_choisie_slot(&slots);
    pico8_bascule_zone_init(&bascule, "essai-partie", bascule_cells, CAPACITY);
    bascule_handle = pico8_zone_choisie_bascule(&bascule_storage, &bascule);
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_choisie: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("bump_grow_keeps_order");
    expect_clean("slot_handle_round_trip");
    expect_assert("slot_handle_too_big", "plus grand qu'une place");
    expect_assert("slot_handle_no_grow", "une place ne grandit pas");
    expect_poison_crash("slot_handle_stale_after_release");
    expect_fatal("slot_handle_full", "zone essai-places pleine, limite 2 places");
    expect_clean("bascule_handle_round_trip");
    expect_poison_crash("bascule_handle_stale_after_rendre");
    expect_clean("bump_read_before_reset");
    expect_poison_crash("bump_stale_after_reset");
    expect_fatal("bump_grow_past_capacity", "zone essai-liste pleine, limite 1024 octets");
    expect_clean("ents_grow_keeps_order");
    expect_clean("ptrs_grow_keeps_order");
    expect_clean("ents_read_before_reset");
    expect_poison_crash("ents_stale_after_reset");
    expect_clean("texts_in_bump");
    expect_clean("arena_alive_until_released");
    expect_freed_crash("arena_dead_after_release");
    expect_fatal("arena_full", "zone place essai pleine, limite ");
    expect_clean("arena_spares_neighbour");
    expect_clean("arena_taken_counts_head_and_blocks");
    expect_clean("arena_vider_back_to_head");
    expect_poison_crash("arena_vider_kills_old_block");
    expect_clean("ptrs_reserved_fills_without_growing");
    expect_fatal("ptrs_reserved_overflow", "zone place essai pleine, limite ");
    remove(STDERR_LOG);
    printf("test_pico8_zone_choisie: all tests passed\n");
    return 0;
}
