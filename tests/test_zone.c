/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "engine/zone.h"

#define STDERR_LOG "tmp/test_pico8_zone_stderr.txt"

PICO8_BUMP_ZONE_STORAGE(bump_cells, 64);
PICO8_SLOT_ZONE_STORAGE(slot_cells, slot_taken, 16, 4);
PICO8_BASCULE_ZONE_STORAGE(bascule_cells, 64);

static pico8_bump_zone_t bump;
static pico8_slot_zone_t slots;
static pico8_bascule_zone_t bascule;

static void init_zones(void) {
    pico8_bump_zone_init(&bump, "essai-frame", bump_cells, 64);
    pico8_slot_zone_init(&slots, "essai-places", slot_cells, slot_taken, 16, 4);
    pico8_bascule_zone_init(&bascule, "essai-partie", bascule_cells, 64);
}

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static int case_control(void) {
    unsigned char *a = pico8_bump_zone_alloc(&bump, 24);
    memset(a, 7, 24);
    sink = a[23];
    pico8_bump_zone_reset(&bump);
    unsigned char *b = pico8_bump_zone_alloc(&bump, 64);
    memset(b, 9, 64);
    sink = b[63];

    unsigned char *s = pico8_slot_zone_alloc(&slots);
    memset(s, 5, 16);
    sink = s[15];
    pico8_slot_zone_release(&slots, s);
    for (int i = 0; i < 4; i++) {
        unsigned char *t = pico8_slot_zone_alloc(&slots);
        memset(t, 3, 16);
        sink = t[0];
    }
    return 0;
}

static int case_bump_stale_after_reset(void) {
    unsigned char *a = pico8_bump_zone_alloc(&bump, 24);
    a[0] = 1;
    pico8_bump_zone_reset(&bump);
    for (int i = 0; i < 2; i++) {
        unsigned char *b = pico8_bump_zone_alloc(&bump, 24);
        memset(b, 2, 24);
    }
    sink = a[0];
    return 0;
}

static int case_slot_stale_after_release(void) {
    unsigned char *a = pico8_slot_zone_alloc(&slots);
    a[0] = 1;
    pico8_slot_zone_release(&slots, a);
    for (int i = 0; i < 3; i++) {
        unsigned char *b = pico8_slot_zone_alloc(&slots);
        memset(b, 2, 16);
    }
    sink = a[0];
    return 0;
}

static int case_bump_overflow_past_object(void) {
    unsigned char *a = pico8_bump_zone_alloc(&bump, 5);
    memset(a, 1, 5);
    sink = a[5];
    return 0;
}

static int case_bump_full(void) {
    pico8_bump_zone_alloc(&bump, 40);
    pico8_bump_zone_alloc(&bump, 40);
    return 0;
}

static int case_slot_taken_follows(void) {
    assert(pico8_slot_zone_taken(&slots) == 0);
    unsigned char *a = pico8_slot_zone_alloc(&slots);
    unsigned char *b = pico8_slot_zone_alloc(&slots);
    assert(pico8_slot_zone_taken(&slots) == 2);
    pico8_slot_zone_release(&slots, a);
    assert(pico8_slot_zone_taken(&slots) == 1 && "rendre une place la decompte");
    pico8_slot_zone_release(&slots, b);
    assert(pico8_slot_zone_taken(&slots) == 0);
    return 0;
}

static int case_slot_full(void) {
    for (int i = 0; i < 5; i++) pico8_slot_zone_alloc(&slots);
    return 0;
}

static int case_bascule_prise_follows(void) {
    assert(pico8_bascule_zone_prise(&bascule) == 0);
    memset(pico8_bascule_zone_alloc(&bascule, 5), 1, 5);
    assert(pico8_bascule_zone_prise(&bascule) == 8 && "arrondi a la cellule");
    memset(pico8_bascule_zone_alloc(&bascule, 16), 1, 16);
    assert(pico8_bascule_zone_prise(&bascule) == 24);
    assert(pico8_bascule_zone_max(&bascule) == 24);
    return 0;
}

static void expect_filled(const unsigned char *p, size_t n, unsigned char v) {
    for (size_t i = 0; i < n; i++) assert(p[i] == v && "bloc recouvert");
}

static int case_bascule_old_generation_lives(void) {
    unsigned char *a = pico8_bascule_zone_alloc(&bascule, 24);
    memset(a, 7, 24);
    pico8_bascule_zone_basculer(&bascule);
    for (int i = 0; i < 5; i++) memset(pico8_bascule_zone_alloc(&bascule, 8), 9, 8);
    expect_filled(a, 24, 7);
    assert(pico8_bascule_zone_prise(&bascule) == 64 && "les deux generations comptent");
    return 0;
}

/* Une generation par bout : rend le premier bloc de la generation `gen`
 * (0 au bas, 1 en haut), apres que la suivante a servi trois blocs. */
static unsigned char *bascule_then_serve(int gen) {
    unsigned char *kept = NULL;
    for (int g = 0; g <= gen + 1; g++) {
        if (g > 0) {
            pico8_bascule_zone_basculer(&bascule);
            pico8_bascule_zone_rendre(&bascule);
        }
        unsigned char *a = pico8_bascule_zone_alloc(&bascule, 8);
        a[0] = 1;
        if (g == gen) kept = a;
    }
    for (int i = 0; i < 2; i++) memset(pico8_bascule_zone_alloc(&bascule, 8), 2, 8);
    return kept;
}

static int case_bascule_new_generation_after_rendre(void) {
    bascule_then_serve(1);
    return 0;
}

static int case_bascule_stale_bas_after_rendre(void) {
    unsigned char *a = bascule_then_serve(0);
    sink = a[0];
    return 0;
}

static int case_bascule_stale_haut_after_rendre(void) {
    unsigned char *a = bascule_then_serve(1);
    sink = a[0];
    return 0;
}

static int case_bascule_twice_without_rendre(void) {
    pico8_bascule_zone_basculer(&bascule);
    pico8_bascule_zone_basculer(&bascule);
    return 0;
}

static int case_bascule_ends_meet(void) {
    memset(pico8_bascule_zone_alloc(&bascule, 40), 1, 40);
    pico8_bascule_zone_basculer(&bascule);
    memset(pico8_bascule_zone_alloc(&bascule, 24), 2, 24);
    fprintf(stderr, "remplie\n");
    pico8_bascule_zone_alloc(&bascule, 8);
    return 0;
}

static int case_bascule_max_keeps_peak(void) {
    pico8_bascule_zone_alloc(&bascule, 24);
    pico8_bascule_zone_basculer(&bascule);
    pico8_bascule_zone_alloc(&bascule, 16);
    pico8_bascule_zone_rendre(&bascule);
    assert(pico8_bascule_zone_prise(&bascule) == 16 && "le rendu decompte l'ancienne generation");
    pico8_bascule_zone_alloc(&bascule, 8);
    assert(pico8_bascule_zone_prise(&bascule) == 24);
    assert(pico8_bascule_zone_max(&bascule) == 40 && "le maximum garde la frame de bascule");
    return 0;
}

/* Deux generations vivantes, puis la remise complete. */
static unsigned char *reset_both_ends(unsigned char **haut) {
    unsigned char *bas = pico8_bascule_zone_alloc(&bascule, 24);
    bas[0] = 1;
    pico8_bascule_zone_basculer(&bascule);
    *haut = pico8_bascule_zone_alloc(&bascule, 16);
    (*haut)[0] = 1;
    pico8_bascule_zone_reset(&bascule);
    return bas;
}

static int case_bascule_reset_empties(void) {
    unsigned char *haut;
    reset_both_ends(&haut);
    assert(pico8_bascule_zone_prise(&bascule) == 0);
    assert(pico8_bascule_zone_max(&bascule) == 40 && "la remise ne touche pas au maximum de session");
    pico8_bascule_zone_basculer(&bascule);
    memset(pico8_bascule_zone_alloc(&bascule, 64), 3, 64);
    return 0;
}

static int case_bascule_stale_bas_after_reset(void) {
    unsigned char *haut;
    unsigned char *bas = reset_both_ends(&haut);
    sink = bas[0];
    return 0;
}

static int case_bascule_stale_haut_after_reset(void) {
    unsigned char *haut;
    reset_both_ends(&haut);
    for (int i = 0; i < 3; i++) memset(pico8_bascule_zone_alloc(&bascule, 8), 2, 8);
    sink = haut[0];
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"control", case_control},
    {"bump_stale_after_reset", case_bump_stale_after_reset},
    {"slot_stale_after_release", case_slot_stale_after_release},
    {"bump_overflow_past_object", case_bump_overflow_past_object},
    {"bump_full", case_bump_full},
    {"slot_taken_follows", case_slot_taken_follows},
    {"slot_full", case_slot_full},
    {"bascule_prise_follows", case_bascule_prise_follows},
    {"bascule_old_generation_lives", case_bascule_old_generation_lives},
    {"bascule_new_generation_after_rendre", case_bascule_new_generation_after_rendre},
    {"bascule_stale_bas_after_rendre", case_bascule_stale_bas_after_rendre},
    {"bascule_stale_haut_after_rendre", case_bascule_stale_haut_after_rendre},
    {"bascule_twice_without_rendre", case_bascule_twice_without_rendre},
    {"bascule_ends_meet", case_bascule_ends_meet},
    {"bascule_max_keeps_peak", case_bascule_max_keeps_peak},
    {"bascule_reset_empties", case_bascule_reset_empties},
    {"bascule_stale_bas_after_reset", case_bascule_stale_bas_after_reset},
    {"bascule_stale_haut_after_reset", case_bascule_stale_haut_after_reset},
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
        assert(0 && "le cas de controle ne doit pas planter");
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

static void expect_assert(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu un assert \"%s\", sortie %d\n%s", name, message, WEXITSTATUS(status), err);
        assert(0 && "le cas doit echouer par cet assert");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *zone, const char *limit) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, zone) == NULL || strstr(err, limit) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 nommant %s et %s, sortie %d\n%s",
                name, zone, limit, WEXITSTATUS(status), err);
        assert(0 && "une zone pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    init_zones();
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("control");
    expect_clean("slot_taken_follows");
    expect_clean("bascule_prise_follows");
    expect_clean("bascule_old_generation_lives");
    expect_clean("bascule_new_generation_after_rendre");
    expect_clean("bascule_max_keeps_peak");
    expect_clean("bascule_reset_empties");
    expect_poison_crash("bump_stale_after_reset");
    expect_poison_crash("slot_stale_after_release");
    expect_poison_crash("bump_overflow_past_object");
    expect_poison_crash("bascule_stale_bas_after_rendre");
    expect_poison_crash("bascule_stale_haut_after_rendre");
    expect_poison_crash("bascule_stale_bas_after_reset");
    expect_poison_crash("bascule_stale_haut_after_reset");
    expect_assert("bascule_twice_without_rendre", "vider la file de sortie entre deux eu");
    expect_fatal("bump_full", "essai-frame", " 64 ");
    expect_fatal("slot_full", "essai-places", " 4 ");
    expect_fatal("bascule_ends_meet", "remplie\npico8_fatal: zone essai-partie pleine", " 64 ");
    remove(STDERR_LOG);
    printf("test_pico8_zone: all tests passed\n");
    return 0;
}
