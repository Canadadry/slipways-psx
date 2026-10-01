/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_particle.h"
#include "../src/pico8_ptrs.h"
#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_particules.h"
#include "../src/pico8_zone_transitoire.h"

#define STDERR_LOG "tmp/test_pico8_zone_particules_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static pico8_qy_t *particule(void) {
    pico8_qy_t *p = (pico8_qy_t *)pico8_zone_alloc(pico8_zone_particules(PICO8_PARTICULES_PARTICULE), sizeof *p);
    memset(p, 0, sizeof *p);
    return p;
}

static void rendre_particule(pico8_qy_t *p) {
    pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_PARTICULE), (unsigned char *)p);
}

/* Une liste qy de la sorte, dans sa place, reservee a `capacite`. */
static pico8_ptrs_t *liste(pico8_particules_sorte_t sorte, int capacite) {
    const pico8_zone_choisie_t *place = pico8_zone_particules_place(sorte);
    pico8_ptrs_t *qy = (pico8_ptrs_t *)pico8_zone_alloc(place, sizeof *qy);
    pico8_ptrs_init_reserve(qy, place, capacite);
    return qy;
}

static int case_live_places_follow_takes(void) {
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++)
        assert(pico8_zone_particules_vivantes((pico8_particules_sorte_t)s) == 0);

    pico8_qy_t *a = particule();
    pico8_qy_t *b = particule();
    const pico8_zone_choisie_t *qy = pico8_zone_particules_place(PICO8_PARTICULES_QY);
    const pico8_zone_choisie_t *fv = pico8_zone_particules_place(PICO8_PARTICULES_QY_FV);
    const pico8_zone_choisie_t *ga = pico8_zone_particules_place(PICO8_PARTICULES_GA);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 2);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 1);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV) == 1);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_GA) == 1);

    rendre_particule(a);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 1 && "rendre une place decompte sa sorte");
    rendre_particule(b);
    pico8_zone_arene_rendre(qy);
    pico8_zone_arene_rendre(fv);
    pico8_zone_arene_rendre(ga);
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++)
        assert(pico8_zone_particules_vivantes((pico8_particules_sorte_t)s) == 0);
    return 0;
}

static int case_particule_dead_after_release(void) {
    pico8_qy_t *p = particule();
    rendre_particule(p);
    sink = ((volatile unsigned char *)p)[0];
    return 0;
}

static int case_ga_zone_full(void) {
    for (int i = 0; i < 9; i++) pico8_zone_particules_place(PICO8_PARTICULES_GA);
    return 0;
}

static int case_particule_zone_full(void) {
    for (int i = 0; i < 2049; i++) particule();
    return 0;
}

static int case_qy_holds_its_capacity(void) {
    pico8_ptrs_t *qy = liste(PICO8_PARTICULES_QY, PICO8_QY_CAPACITE);
    for (int i = 0; i < PICO8_QY_CAPACITE; i++) pico8_ptrs_add(qy, NULL);
    pico8_zone_arene_rendre(qy->zone);
    return 0;
}

static int case_qy_past_capacity(void) {
    pico8_ptrs_t *qy = liste(PICO8_PARTICULES_QY, PICO8_QY_CAPACITE);
    for (int i = 0; i <= PICO8_QY_CAPACITE; i++) pico8_ptrs_add(qy, NULL);
    return 0;
}

static int case_qy_fv_holds_its_capacity(void) {
    pico8_ptrs_t *qy = liste(PICO8_PARTICULES_QY_FV, PICO8_QY_FV_CAPACITE);
    for (int i = 0; i < PICO8_QY_FV_CAPACITE; i++) pico8_ptrs_add(qy, NULL);
    pico8_zone_arene_rendre(qy->zone);
    return 0;
}

static int case_qy_fv_past_capacity(void) {
    pico8_ptrs_t *qy = liste(PICO8_PARTICULES_QY_FV, PICO8_QY_FV_CAPACITE);
    for (int i = 0; i <= PICO8_QY_FV_CAPACITE; i++) pico8_ptrs_add(qy, NULL);
    return 0;
}

static int case_maxima_keep_the_peak(void) {
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++) {
        assert(pico8_zone_particules_max_vivantes((pico8_particules_sorte_t)s) == 0);
        assert(pico8_zone_particules_max_octets((pico8_particules_sorte_t)s) == 0);
    }
    assert(pico8_zone_particules_max_elements(PICO8_PARTICULES_QY) == 0);

    pico8_qy_t *p[3] = {particule(), particule(), particule()};
    for (int i = 0; i < 3; i++) rendre_particule(p[i]);
    pico8_qy_t *q = particule();
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 1);
    assert(pico8_zone_particules_max_vivantes(PICO8_PARTICULES_PARTICULE) == 3 &&
           "le maximum se releve a chaque prise, pas a la lecture");
    rendre_particule(q);

    const pico8_zone_choisie_t *rendue = pico8_zone_particules_place(PICO8_PARTICULES_GA);
    pico8_zone_alloc(rendue, 200);
    size_t prise_rendue = pico8_zone_arene_prise(rendue);
    pico8_zone_arene_rendre(rendue);
    assert(pico8_zone_particules_max_octets(PICO8_PARTICULES_GA) == prise_rendue &&
           "une place rendue garde ce qu'elle a pris");

    const pico8_zone_choisie_t *vivante = pico8_zone_particules_place(PICO8_PARTICULES_GA);
    pico8_zone_alloc(vivante, 400);
    assert(pico8_zone_particules_max_octets(PICO8_PARTICULES_GA) == pico8_zone_arene_prise(vivante) &&
           "une place encore vivante compte a la lecture");
    assert(pico8_zone_particules_max_octets(PICO8_PARTICULES_QY) == 0);

    pico8_zone_particules_noter_elements(PICO8_PARTICULES_QY, 7);
    pico8_zone_particules_noter_elements(PICO8_PARTICULES_QY, 3);
    pico8_zone_particules_noter_elements(PICO8_PARTICULES_QY_FV, 12);
    assert(pico8_zone_particules_max_elements(PICO8_PARTICULES_QY) == 7);
    assert(pico8_zone_particules_max_elements(PICO8_PARTICULES_QY_FV) == 12);
    return 0;
}

static int case_qy_rendre_gives_back_particles_then_list(void) {
    pico8_emitter_t emitter;
    memset(&emitter, 0, sizeof emitter);
    emitter.qy = liste(PICO8_PARTICULES_QY, PICO8_QY_CAPACITE);
    pico8_ptrs_add(emitter.qy, particule());
    pico8_ptrs_add(emitter.qy, NULL);
    pico8_ptrs_add(emitter.qy, particule());
    pico8_qy_t *gardee = particule();
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 3);

    pico8_qy_rendre(&emitter);

    assert(emitter.qy == NULL && "l'hote ne vise plus la liste rendue");
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 1 &&
           "les particules de la liste sont rendues, pas les autres");
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 0);
    rendre_particule(gardee);

    pico8_qy_rendre(&emitter);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 0 && "un hote sans liste n'a rien a rendre");
    return 0;
}

static int case_qy_rendre_releases_the_arena(void) {
    pico8_emitter_t emitter;
    memset(&emitter, 0, sizeof emitter);
    emitter.qy = liste(PICO8_PARTICULES_QY, PICO8_QY_CAPACITE);
    const pico8_ptrs_t *qy = emitter.qy;
    pico8_qy_rendre(&emitter);
    sink = ((const volatile unsigned char *)qy)[0];
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"live_places_follow_takes", case_live_places_follow_takes},
    {"particule_dead_after_release", case_particule_dead_after_release},
    {"ga_zone_full", case_ga_zone_full},
    {"particule_zone_full", case_particule_zone_full},
    {"qy_holds_its_capacity", case_qy_holds_its_capacity},
    {"qy_past_capacity", case_qy_past_capacity},
    {"qy_fv_holds_its_capacity", case_qy_fv_holds_its_capacity},
    {"qy_fv_past_capacity", case_qy_fv_past_capacity},
    {"maxima_keep_the_peak", case_maxima_keep_the_peak},
    {"qy_rendre_gives_back_particles_then_list", case_qy_rendre_gives_back_particles_then_list},
    {"qy_rendre_releases_the_arena", case_qy_rendre_releases_the_arena},
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
        assert(0 && "une zone pleine ou une place trop petite doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    pico8_zone_particules_adosser(&pico8_zone_transitoire);
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_particules: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("live_places_follow_takes");
    expect_freed_crash("particule_dead_after_release");
    expect_fatal("ga_zone_full", "zone particules ga pleine, limite 8 places");
    expect_fatal("particule_zone_full", "zone particules particule pleine, limite 2048 places");
    expect_clean("qy_holds_its_capacity");
    expect_fatal("qy_past_capacity", "zone place qy pleine, limite ");
    expect_clean("qy_fv_holds_its_capacity");
    expect_fatal("qy_fv_past_capacity", "zone place qy_fv pleine, limite ");
    expect_clean("maxima_keep_the_peak");
    expect_clean("qy_rendre_gives_back_particles_then_list");
    expect_freed_crash("qy_rendre_releases_the_arena");
    remove(STDERR_LOG);
    printf("test_pico8_zone_particules: all tests passed\n");
    return 0;
}
