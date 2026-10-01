/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_gfx.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_ptrs.h"
#include "../src/pico8_slipway_display.h"
#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_particules.h"

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
    for (int i = 0; i < 100000; i++) particule();
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

/* Un hote d'emetteur reduit a ce que lisent `f_` et `ei` : `fa` 10, donc
 * dessine sans `gd`, et `fc` 0, donc sans naissance. */
static const pico8_emitter_def_t g_sans_naissance;
static const pico8_struct_class_t g_hote_cls;
static const pico8_fixed_t g_fa_dix = 10 << 16;
static const pico8_vec_t *const g_ra_nil = NULL;
static pico8_emitter_t g_emitter;
static pico8_struct_header_t g_hote;
static pico8_gfx_t g_gfx;
static pico8_mem_t g_mem;

static pico8_struct_header_t *hote(void) {
    pico8_gfx_init(&g_gfx);
    pico8_mem_init(&g_mem);
    pico8_struct_header_init(&g_hote, &g_hote_cls);
    memset(&g_emitter, 0, sizeof g_emitter);
    g_emitter.def = &g_sans_naissance;
    g_hote.fa = &g_fa_dix;
    g_hote.ra = &g_ra_nil;
    g_hote.emitter = &g_emitter;
    pico8_f_(&g_hote);
    return &g_hote;
}

/* Une particule qui meurt au prochain `ei` : `l` 1.1, moins 0.14. */
static pico8_qy_t *mourante(pico8_struct_header_t *h) {
    pico8_qy_t *p = pico8_particule_prendre();
    memset(p, 0, sizeof *p);
    p->l = (1 << 16) + 6554;
    pico8_ptrs_add(h->emitter->qy, p);
    return p;
}

static int case_f__takes_a_qy_list(void) {
    pico8_struct_header_t *h = hote();
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 1);
    assert(h->emitter->qy != NULL && h->emitter->qy->count == 0);
    pico8_qy_rendre(h->emitter);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 0);
    return 0;
}

static int case_f__fv_takes_a_qy_fv_list(void) {
    pico8_struct_header_t *h = hote();
    pico8_qy_rendre(h->emitter);
    pico8_f__dans(h, PICO8_PARTICULES_QY_FV);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV) == 1);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 0);
    return 0;
}

static int case_second_f__gives_back_the_old_list(void) {
    pico8_struct_header_t *h = hote();
    mourante(h);
    mourante(h);
    pico8_f_(h);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY) == 1 && "l'ancienne liste est rendue");
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 0 && "et ses particules avec elle");
    assert(h->emitter->qy->count == 0);
    return 0;
}

static int case_ei_gives_back_a_dying_particle(void) {
    pico8_struct_header_t *h = hote();
    mourante(h);
    pico8_qy_t *survivante = mourante(h);
    survivante->l = 5 << 16;
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 2);
    pico8_ei(&g_gfx, &g_mem, h);
    assert(h->emitter->qy->count == 1);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 1 && "ei rend la particule qu'il retire");
    return 0;
}

static int case_particle_dead_after_ei(void) {
    pico8_struct_header_t *h = hote();
    pico8_qy_t *p = mourante(h);
    pico8_ei(&g_gfx, &g_mem, h);
    sink = ((volatile unsigned char *)p)[0];
    return 0;
}

static int case_ez_notes_the_list_length(void) {
    static const pico8_struct_class_t cls = {.mm = pico8_g__mm};
    static const pico8_emitter_def_t def = {.ki = 9 << 16, .fc = 1 << 16};
    pico8_struct_header_t *h = hote();
    g_hote.cls = &cls;
    g_emitter.def = &def;
    g_emitter.pn = 1 << 16;
    pico8_ez(h, 3 << 16);
    assert(h->emitter->qy->count == 3);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE) == 3 && "g_:mm prend a la poignee particule");
    assert(pico8_zone_particules_max_elements(PICO8_PARTICULES_QY) == 3);
    return 0;
}

static pico8_ts_t *trainee(void) {
    return (pico8_ts_t *)pico8_zone_alloc(pico8_zone_particules(PICO8_PARTICULES_TS), sizeof(pico8_ts_t));
}

static void rendre_trainee(pico8_ts_t *t) {
    pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_TS), (unsigned char *)t);
}

static int case_ts_takes_and_gives_back(void) {
    pico8_ts_t *a = trainee();
    pico8_ts_t *b = trainee();
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_TS) == 2);
    rendre_trainee(a);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_TS) == 1 && "rendre une trainee decompte sa sorte");
    rendre_trainee(b);
    assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_TS) == 0);
    assert(pico8_zone_particules_max_vivantes(PICO8_PARTICULES_TS) == 2);
    assert(pico8_zone_particules_max_octets(PICO8_PARTICULES_TS) == 0 && "une trainee est une place nue");
    pico8_zone_particules_noter_elements(PICO8_PARTICULES_TS, 5);
    pico8_zone_particules_noter_elements(PICO8_PARTICULES_TS, 2);
    assert(pico8_zone_particules_max_elements(PICO8_PARTICULES_TS) == 5);
    return 0;
}

static int case_ts_dead_after_release(void) {
    pico8_ts_t *t = trainee();
    rendre_trainee(t);
    sink = ((volatile unsigned char *)t)[0];
    return 0;
}

static int case_ts_zone_full(void) {
    for (int i = 0; i < 681; i++) trainee();
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
    {"f__takes_a_qy_list", case_f__takes_a_qy_list},
    {"f__fv_takes_a_qy_fv_list", case_f__fv_takes_a_qy_fv_list},
    {"second_f__gives_back_the_old_list", case_second_f__gives_back_the_old_list},
    {"ei_gives_back_a_dying_particle", case_ei_gives_back_a_dying_particle},
    {"particle_dead_after_ei", case_particle_dead_after_ei},
    {"ez_notes_the_list_length", case_ez_notes_the_list_length},
    {"ts_takes_and_gives_back", case_ts_takes_and_gives_back},
    {"ts_dead_after_release", case_ts_dead_after_release},
    {"ts_zone_full", case_ts_zone_full},
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
        assert(0 && "une place rendue a sa zone puis lue doit planter sous ASan");
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
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_particules: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("live_places_follow_takes");
    expect_poison_crash("particule_dead_after_release");
    expect_fatal("ga_zone_full", "zone particules ga pleine, limite 8 places");
    expect_fatal("particule_zone_full", "zone particules particule pleine, limite 2085 places");
    expect_clean("qy_holds_its_capacity");
    expect_fatal("qy_past_capacity", "zone place qy pleine, limite ");
    expect_clean("qy_fv_holds_its_capacity");
    expect_fatal("qy_fv_past_capacity", "zone place qy_fv pleine, limite ");
    expect_clean("maxima_keep_the_peak");
    expect_clean("qy_rendre_gives_back_particles_then_list");
    expect_poison_crash("qy_rendre_releases_the_arena");
    expect_clean("f__takes_a_qy_list");
    expect_clean("f__fv_takes_a_qy_fv_list");
    expect_clean("second_f__gives_back_the_old_list");
    expect_clean("ei_gives_back_a_dying_particle");
    expect_poison_crash("particle_dead_after_ei");
    expect_clean("ez_notes_the_list_length");
    expect_clean("ts_takes_and_gives_back");
    expect_poison_crash("ts_dead_after_release");
    expect_fatal("ts_zone_full", "zone particules ts pleine, limite 680 places");
    remove(STDERR_LOG);
    printf("test_pico8_zone_particules: all tests passed\n");
    return 0;
}
