/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_ents.h"
#include "../src/pico8_ptrs.h"
#include "../src/pico8_zone.h"
#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_transitoire.h"

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

static int case_transient_round_trip(void) {
    unsigned char *items = grow_counting(&pico8_zone_transitoire);
    expect_counting(items);
    pico8_zone_release(&pico8_zone_transitoire, items);
    pico8_zone_release(&pico8_zone_transitoire, pico8_zone_alloc(&pico8_zone_transitoire, 8));
    return 0;
}

#define LIST_LENGTH 12

static pico8_struct_header_t cells[LIST_LENGTH];

static pico8_ents_t grown_ents(void) {
    pico8_ents_t ents;
    pico8_ents_init(&ents, &handle);
    for (int i = 0; i < LIST_LENGTH; i++) pico8_ents_add(&ents, &cells[i]);
    return ents;
}

static int is_even_cell(pico8_struct_header_t *e, void *ctx) {
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
    pico8_struct_header_t *e;
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

static int case_ents_transient_round_trip(void) {
    pico8_ents_t ents;
    pico8_ents_init(&ents, &pico8_zone_transitoire);
    for (int i = 0; i < LIST_LENGTH; i++) pico8_ents_add(&ents, &cells[i]);
    pico8_ents_t both = pico8_ents_cat(&pico8_zone_transitoire, &ents, &ents);
    assert(both.count == 2 * LIST_LENGTH);
    pico8_ents_free(&both);
    pico8_ents_free(&ents);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"bump_grow_keeps_order", case_bump_grow_keeps_order},
    {"bump_read_before_reset", case_bump_read_before_reset},
    {"bump_stale_after_reset", case_bump_stale_after_reset},
    {"bump_grow_past_capacity", case_bump_grow_past_capacity},
    {"transient_round_trip", case_transient_round_trip},
    {"ents_grow_keeps_order", case_ents_grow_keeps_order},
    {"ptrs_grow_keeps_order", case_ptrs_grow_keeps_order},
    {"ents_read_before_reset", case_ents_read_before_reset},
    {"ents_stale_after_reset", case_ents_stale_after_reset},
    {"ents_transient_round_trip", case_ents_transient_round_trip},
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
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_choisie: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("bump_grow_keeps_order");
    expect_clean("bump_read_before_reset");
    expect_poison_crash("bump_stale_after_reset");
    expect_fatal("bump_grow_past_capacity", "zone essai-liste pleine, limite 1024 octets");
    expect_clean("transient_round_trip");
    expect_clean("ents_grow_keeps_order");
    expect_clean("ptrs_grow_keeps_order");
    expect_clean("ents_read_before_reset");
    expect_poison_crash("ents_stale_after_reset");
    expect_clean("ents_transient_round_trip");
    remove(STDERR_LOG);
    printf("test_pico8_zone_choisie: all tests passed\n");
    return 0;
}
