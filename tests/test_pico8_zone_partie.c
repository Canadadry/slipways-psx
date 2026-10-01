/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_ents.h"
#include "../src/pico8_gen.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_partie.h"
#include "../src/pico8_zone_sizes.h"

#define STDERR_LOG "tmp/test_pico8_zone_partie_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static void load_cart_and_start(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);
    pico8_cart_load();
    pico8__init();
}

static unsigned char *counting_block(void) {
    unsigned char *block = pico8_zone_alloc(pico8_zone_partie(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static void allocate_again(void) {
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(pico8_zone_partie(), 64), 2, 64);
}

static int case_alive_through_eu(void) {
    load_cart_and_start();
    unsigned char *block = counting_block();
    pico8_eu(NULL);
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "la generation quittee vit jusqu'au vidage qui suit eu");
    return 0;
}

static int case_dead_after_next_update(void) {
    load_cart_and_start();
    unsigned char *block = counting_block();
    pico8_eu(NULL);
    pico8__update60();
    allocate_again();
    sink = block[63];
    return 0;
}

static int case_init_keeps_generation(void) {
    load_cart_and_start();
    pico8_eu(NULL);
    pico8__update60();
    unsigned char *block = counting_block();
    pico8__init();
    pico8__update60();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "_init ne fait pas basculer la zone partie");
    return 0;
}

static int case_dead_after_cart_load(void) {
    load_cart_and_start();
    unsigned char *block = counting_block();
    pico8_cart_load();
    sink = block[63];
    return 0;
}

/* Octets que le bw de chaque eu prend dans sa generation : jo.oc, sa
 * source et jo.ir. */
static size_t etats_de_bw(void) {
    return (2 * PICO8_ZONE_CELLS(sizeof(pico8_jf_state_t)) + PICO8_ZONE_CELLS(sizeof(pico8_gn_state_t))) *
           sizeof(pico8_zone_cell_t);
}

/* Octets qu'une liste a pris dans sa generation depuis un jl() : les
 * capacites 4, 8, ... jusqu'a la sienne, car chaque agrandissement copie et
 * laisse l'ancien bloc mort jusqu'au rendu. NULL (un `cg` jamais vu) : rien. */
static size_t octets_d_un_registre(const pico8_ents_t *liste) {
    size_t octets = 0;
    for (int k = 4; liste != NULL && k <= liste->capacity; k *= 2) {
        octets += PICO8_ZONE_CELLS((size_t)k * sizeof(pico8_struct_header_t *)) * sizeof(pico8_zone_cell_t);
    }
    return octets;
}

static size_t octets_des_registres(void) {
    static const char *const CV[] = {"kj", "ev", "hd", "hk", "hb"};
    static const char *const CG[] = {"qh", "hj", "nr", "g_", "mh", "jc", "mz", "jo", "np", "prober"};
    size_t r = octets_d_un_registre(pico8_hv());
    for (size_t i = 0; i < sizeof CV / sizeof CV[0]; i++) r += octets_d_un_registre(pico8_cv(CV[i]));
    for (size_t i = 0; i < sizeof CG / sizeof CG[0]; i++) r += octets_d_un_registre(pico8_cg(CG[i]));
    return r;
}

static int case_prise_and_max(void) {
    const size_t g = etats_de_bw();
    load_cart_and_start();
    const size_t titre = pico8_zone_partie_prise();
    (void)counting_block();
    assert(pico8_zone_partie_prise() == titre + 64);
    pico8_eu(NULL);
    const size_t partie = pico8_zone_partie_prise();
    assert(partie >= titre + 64 + g && "les deux generations comptent jusqu'au rendu");
    (void)counting_block();
    assert(pico8_zone_partie_prise() == partie + 64);
    pico8__update60();
    assert(pico8_zone_partie_prise() == g + octets_des_registres() + 64);
    assert(pico8_zone_partie_max() == partie + 64);
    pico8_cart_shutdown();
    assert(pico8_zone_partie_prise() == 0);
    assert(pico8_zone_partie_max() == partie + 64 && "le maximum ne se remet qu'avec le processus");
    return 0;
}

static unsigned char *counting_block_chargement(void) {
    unsigned char *block = pico8_zone_alloc(pico8_zone_chargement(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static int case_chargement_survives_eu(void) {
    load_cart_and_start();
    unsigned char *block = counting_block_chargement();
    pico8_eu(NULL);
    pico8__update60();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "la sous-zone chargement ne suit pas les generations");
    return 0;
}

static int case_chargement_dead_after_cart_load(void) {
    load_cart_and_start();
    unsigned char *block = counting_block_chargement();
    pico8_cart_load();
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(pico8_zone_chargement(), 64), 2, 64);
    sink = block[63];
    return 0;
}

static int case_chargement_prise_and_max(void) {
    load_cart_and_start();
    const size_t depart = pico8_zone_chargement_prise();
    (void)counting_block_chargement();
    assert(pico8_zone_chargement_prise() == depart + 64);
    pico8_eu(NULL);
    pico8__update60();
    assert(pico8_zone_chargement_prise() == depart + 64 && "ni eu ni le rendu ne touchent la sous-zone");
    assert(pico8_zone_chargement_max() == depart + 64);
    pico8_cart_shutdown();
    assert(pico8_zone_chargement_prise() == 0);
    assert(pico8_zone_chargement_max() == depart + 64 && "le maximum ne se remet qu'avec le processus");
    return 0;
}

static int case_chargement_full(void) {
    load_cart_and_start();
    (void)pico8_zone_alloc(pico8_zone_chargement(), PICO8_ZONE_CHARGEMENT_BYTES);
    (void)pico8_zone_alloc(pico8_zone_chargement(), 1);
    return 0;
}

static size_t octets_de_quatre_pointeurs(void) {
    return PICO8_ZONE_CELLS(4 * sizeof(pico8_struct_header_t *)) * sizeof(pico8_zone_cell_t);
}

static int case_listes_statiques_dans_chargement(void) {
    static pico8_struct_header_t element;
    load_cart_and_start();
    const size_t depart = pico8_zone_chargement_prise();
    pico8_ents_t *le = pico8_ui_le_shared();
    pico8_ents_add(le, &element);
    assert(pico8_zone_chargement_prise() == depart + octets_de_quatre_pointeurs() &&
           "la liste de depart de ok.le prend sa capacite dans la sous-zone chargement");
    pico8_ents_t *by = pico8_ga_new(NULL)->by;
    pico8_ents_add(by, &element);
    assert(pico8_zone_chargement_prise() == depart + 2 * octets_de_quatre_pointeurs() &&
           "le by partage des ga prend sa capacite dans la sous-zone chargement");
    pico8_cart_shutdown();
    assert(le->items == NULL && le->count == 0 && le->capacity == 0 && "ui_shutdown oublie la liste de depart");
    assert(by->items == NULL && by->count == 0 && by->capacity == 0 && "le shutdown oublie le by partage");
    return 0;
}

static int case_double_eu_without_flush(void) {
    load_cart_and_start();
    pico8_eu(NULL);
    pico8_eu(NULL);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"alive_through_eu", case_alive_through_eu},
    {"dead_after_next_update", case_dead_after_next_update},
    {"init_keeps_generation", case_init_keeps_generation},
    {"dead_after_cart_load", case_dead_after_cart_load},
    {"prise_and_max", case_prise_and_max},
    {"double_eu_without_flush", case_double_eu_without_flush},
    {"chargement_survives_eu", case_chargement_survives_eu},
    {"chargement_dead_after_cart_load", case_chargement_dead_after_cart_load},
    {"chargement_prise_and_max", case_chargement_prise_and_max},
    {"chargement_full", case_chargement_full},
    {"listes_statiques_dans_chargement", case_listes_statiques_dans_chargement},
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
    assert(status != -1);
    *err = read_whole_file(STDERR_LOG);
    return status;
}

static void expect_clean(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || err[0] != '\0') {
        fprintf(stderr, "%s: attendu propre, statut %d\n%s", name, status, err);
        assert(0 && "le cas ne doit ni planter ni ecrire sur stderr");
    }
    free(err);
}

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (status == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: attendu use-after-poison, statut %d\n%s", name, status, err);
        assert(0 && "un bloc d'une generation rendue doit planter sous ASan");
    }
    free(err);
}

static void expect_assert(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (status == 0 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu l'assert \"%s\", statut %d\n%s", name, message, status, err);
        assert(0 && "deux eu sans vidage doivent echouer par l'assert de la bascule");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", statut %d\n%s", name, message, status, err);
        assert(0 && "une sous-zone pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_partie: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("alive_through_eu");
    expect_poison_crash("dead_after_next_update");
    expect_clean("init_keeps_generation");
    expect_poison_crash("dead_after_cart_load");
    expect_clean("prise_and_max");
    expect_assert("double_eu_without_flush", "vider la file de sortie entre deux eu");
    expect_clean("chargement_survives_eu");
    expect_poison_crash("chargement_dead_after_cart_load");
    expect_clean("chargement_prise_and_max");
    expect_fatal("chargement_full", "zone chargement pleine, limite 65536 octets");
    expect_clean("listes_statiques_dans_chargement");
    remove(STDERR_LOG);
    printf("test_pico8_zone_partie: all tests passed\n");
    return 0;
}
