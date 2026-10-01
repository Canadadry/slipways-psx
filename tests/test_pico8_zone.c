/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_zone.h"

#define STDERR_LOG "tmp/test_pico8_zone_stderr.txt"

PICO8_BUMP_ZONE_STORAGE(bump_cells, 64);
PICO8_SLOT_ZONE_STORAGE(slot_cells, slot_taken, 16, 4);

static pico8_bump_zone_t bump;
static pico8_slot_zone_t slots;

static void init_zones(void) {
    pico8_bump_zone_init(&bump, "essai-frame", bump_cells, 64);
    pico8_slot_zone_init(&slots, "essai-places", slot_cells, slot_taken, 16, 4);
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

static int case_slot_full(void) {
    for (int i = 0; i < 5; i++) pico8_slot_zone_alloc(&slots);
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
    {"slot_full", case_slot_full},
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
    expect_poison_crash("bump_stale_after_reset");
    expect_poison_crash("slot_stale_after_release");
    expect_poison_crash("bump_overflow_past_object");
    expect_fatal("bump_full", "essai-frame", " 64 ");
    expect_fatal("slot_full", "essai-places", " 4 ");
    remove(STDERR_LOG);
    printf("test_pico8_zone: all tests passed\n");
    return 0;
}
