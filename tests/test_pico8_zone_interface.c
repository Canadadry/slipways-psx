/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_zone_transitoire.h"

#define STDERR_LOG "tmp/test_pico8_zone_interface_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static int case_live_places_follow_takes(void) {
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) assert(pico8_zone_interface_vivantes((pico8_sorte_t)s) == 0);

    const pico8_zone_choisie_t *a = pico8_zone_interface_place(PICO8_SORTE_JS);
    const pico8_zone_choisie_t *b = pico8_zone_interface_place(PICO8_SORTE_JS);
    const pico8_zone_choisie_t *c = pico8_zone_interface_place(PICO8_SORTE_IV);
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_JS) == 2);
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_IV) == 1);
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_MH) == 0);

    pico8_zone_arene_rendre(a);
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_JS) == 1 && "rendre une place decompte sa sorte");
    pico8_zone_arene_rendre(b);
    pico8_zone_arene_rendre(c);
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) assert(pico8_zone_interface_vivantes((pico8_sorte_t)s) == 0);
    return 0;
}

static int case_place_alive_until_released(void) {
    const pico8_zone_choisie_t *place = pico8_zone_interface_place(PICO8_SORTE_RANGEE);
    unsigned char *text = pico8_zone_alloc(place, 64);
    memset(text, 5, 64);
    for (int i = 0; i < 64; i++) assert(text[i] == 5);
    pico8_zone_arene_rendre(place);
    return 0;
}

static int case_place_dead_after_release(void) {
    const pico8_zone_choisie_t *place = pico8_zone_interface_place(PICO8_SORTE_RANGEE);
    unsigned char *text = pico8_zone_alloc(place, 64);
    memset(text, 5, 64);
    pico8_zone_arene_rendre(place);
    sink = text[63];
    return 0;
}

static int case_js_place_full(void) {
    pico8_zone_alloc(pico8_zone_interface_place(PICO8_SORTE_JS), 5 * 1024);
    return 0;
}

static int case_rangee_place_full(void) {
    pico8_zone_alloc(pico8_zone_interface_place(PICO8_SORTE_RANGEE), 1184);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"live_places_follow_takes", case_live_places_follow_takes},
    {"place_alive_until_released", case_place_alive_until_released},
    {"place_dead_after_release", case_place_dead_after_release},
    {"js_place_full", case_js_place_full},
    {"rangee_place_full", case_rangee_place_full},
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

static void expect_freed_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "heap-use-after-free") == NULL) {
        fprintf(stderr, "%s: attendu heap-use-after-free, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "une place rendue a la transitoire puis lue doit planter sous ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "une place trop petite doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    pico8_zone_interface_adosser(&pico8_zone_transitoire);
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_interface: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("live_places_follow_takes");
    expect_clean("place_alive_until_released");
    expect_freed_crash("place_dead_after_release");
    expect_fatal("js_place_full", "zone place js pleine, limite ");
    expect_fatal("rangee_place_full", "zone place rangee pleine, limite ");
    remove(STDERR_LOG);
    printf("test_pico8_zone_interface: all tests passed\n");
    return 0;
}
