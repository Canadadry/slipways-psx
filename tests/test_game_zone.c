/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "p8/assets.h"
#include "game/background.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_list.h"
#include "game/main_economy.h"
#include "game/gen.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "game/mouse.h"
#include "game/planet_family.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"
#include "engine/zone_sizes.h"

#define STDERR_LOG "tmp/test_game_zone_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static void load_cart_and_start(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);
    pico8_cart_load();
    pico8__init();
}

static unsigned char *counting_block(void) {
    unsigned char *block = allocator_alloc(game_zone(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static void allocate_again(void) {
    for (int i = 0; i < 4; i++) memset(allocator_alloc(game_zone(), 64), 2, 64);
}

static int case_alive_through_start_game(void) {
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
static size_t populate_galaxy_states(void) {
    return (2 * ZONE_CELLS(sizeof(random_walk_t)) + ZONE_CELLS(sizeof(weighted_picker_t))) *
           sizeof(zone_cell_t);
}

/* Octets qu'une liste a pris dans sa generation depuis un jl() : les
 * capacites 4, 8, ... jusqu'a la sienne, car chaque agrandissement copie et
 * laisse l'ancien bloc mort jusqu'au rendu. NULL (un `cg` jamais vu) : rien. */
static size_t registry_bytes(const entity_list_t *list) {
    size_t bytes = 0;
    for (int k = 4; list != NULL && k <= list->capacity; k *= 2) {
        bytes += ZONE_CELLS((size_t)k * sizeof(entity_t *)) * sizeof(zone_cell_t);
    }
    return bytes;
}

static size_t registries_bytes(void) {
    static const char *const CV[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    static const char *const CG[] = {"structure", "ascension", "slipgate", "probe", "message_box", "slipway", "wormhole", "planet", "node", "targeting"};
    size_t r = registry_bytes(live_entities());
    for (size_t i = 0; i < sizeof CV / sizeof CV[0]; i++) r += registry_bytes(entities_by_message(CV[i]));
    for (size_t i = 0; i < sizeof CG / sizeof CG[0]; i++) r += registry_bytes(entities_by_tag(CG[i]));
    return r;
}

static size_t block_bytes(size_t size) {
    return ZONE_CELLS(size) * sizeof(zone_cell_t);
}

/* Octets d'une liste de planete faite dans la generation : son conteneur,
 * puis ses blocs comme un registre. */
static size_t planet_list_bytes(const entity_list_t *list) {
    if (list == NULL) return 0;
    return block_bytes(sizeof *list) + registry_bytes(list);
}

/* Empreinte `P` des planetes de `cg.jo` : structure, `p`, `ra`, identifiant
 * et listes `oe`, `by`, `cc`. Sans `jy` ni copies de besoins, que les
 * planetes de `bw` n'ont pas avant une action. */
static size_t planets_bytes(void) {
    const entity_list_t *jo = entities_by_tag("planet");
    size_t bytes = 0;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        assert(p->needs == NULL && "une planete a des besoins, que P ne compte pas");
        bytes += block_bytes(sizeof *p) + block_bytes(sizeof *p->map_pos) + block_bytes(sizeof *p->pos) +
                  block_bytes(strlen(p->planet_id) + 1) + planet_list_bytes(p->slipways) +
                  planet_list_bytes(p->outgoing_routes) + planet_list_bytes(p->incoming_routes);
    }
    return bytes;
}

/* Octets de l'economie `fm` faite par `eu` : sa structure et le conteneur
 * `invented`, sans elements tant que rien n'est invente. */
static size_t economy_bytes(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    assert(fm != NULL && fm->invented != NULL && "eu a fait l'economie de la partie");
    assert(fm->invented->count == 0 && fm->invented->items == NULL && "une invention, que l'economie ne compte pas");
    return block_bytes(sizeof *fm) + block_bytes(sizeof *fm->invented);
}

/* Octets des sept icones `ie` de `hv`, dans l'ordre de `dr` : structure,
 * bulle `jb` refaite dans une arene de la zone frame, et la boite de `ie.gy`
 * de chaque icone revelee. */
static size_t icons_bytes(void) {
    const entity_list_t *hv = live_entities();
    const allocator_t *mesure = arena_take(frame_zone(), "mesure des bulles", 16384);
    const size_t head = arena_used(mesure);
    size_t bytes = 0;
    int k = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const pico8_ie_t *ie = (const pico8_ie_t *)hv->items[i];
        if (ie->header.class != pico8_ie_struct_class()) continue;
        assert(k < ICON_TOOLTIPS_COUNT && "plus d'icones que de bulles dans dr");
        (void)tooltip_new(mesure, &icon_tooltips_table[k]);
        bytes += block_bytes(sizeof *ie);
        if (ie->has_kh && ie->kh) bytes += block_bytes(sizeof(vec_t));
        k++;
    }
    assert(k == ICON_TOOLTIPS_COUNT && "eu fait une icone par bulle de dr");
    return bytes + arena_used(mesure) - head;
}

/* Terme « recherche et economie » de la generation : ce que `eu` fait pour
 * `fm` et les icones. */
static size_t research_and_economy_bytes(void) {
    return economy_bytes() + icons_bytes();
}

/* Terme « singletons » : `mo`, `je`, `ok` du jl() de `eu`, son `bg`, seul
 * dans `hv`, et `ju`. */
static size_t singletons_bytes(void) {
    const entity_list_t *hv = live_entities();
    int bg = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) bg += hv->items[i]->class == pico8_bg_struct_class();
    assert(bg == 1 && "eu fait un seul bg");
    assert(pico8_mo_qb() != NULL && pico8_je_nf() != NULL && pico8_ok() != NULL && pico8_ju() != NULL);
    return block_bytes(sizeof(pico8_qb_t)) + block_bytes(sizeof(pico8_nf_t)) +
           block_bytes(sizeof(pico8_ui_t)) + block_bytes(sizeof(pico8_bg_t)) +
           block_bytes(sizeof(pico8_pj_t));
}

static int case_grab_arena_and_max(void) {
    const size_t g = populate_galaxy_states();
    load_cart_and_start();
    const size_t title = game_zone_used();
    (void)counting_block();
    assert(game_zone_used() == title + 64);
    pico8_eu(NULL);
    const size_t game_zone_used_before = game_zone_used();
    assert(game_zone_used_before >= title + 64 + g && "les deux generations comptent jusqu'au rendu");
    (void)counting_block();
    assert(game_zone_used() == game_zone_used_before + 64);
    pico8__update60();
    assert(game_zone_used() == g + registries_bytes() + planets_bytes() + research_and_economy_bytes() +
                                         singletons_bytes() + 64);
    assert(game_zone_max() == game_zone_used_before + 64);
    pico8_cart_shutdown();
    assert(game_zone_used() == 0);
    assert(game_zone_max() == game_zone_used_before + 64 && "le maximum ne se remet qu'avec le processus");
    return 0;
}

static unsigned char *counting_block_load_zone(void) {
    unsigned char *block = allocator_alloc(game_zone_load_zone(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static int case_load_zone_survives_start_game(void) {
    load_cart_and_start();
    unsigned char *block = counting_block_load_zone();
    pico8_eu(NULL);
    pico8__update60();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "la sous-zone chargement ne suit pas les generations");
    return 0;
}

static int case_load_zone_dead_after_cart_load(void) {
    load_cart_and_start();
    unsigned char *block = counting_block_load_zone();
    pico8_cart_load();
    for (int i = 0; i < 4; i++) memset(allocator_alloc(game_zone_load_zone(), 64), 2, 64);
    sink = block[63];
    return 0;
}

static int case_load_zone_grab_arena_and_max(void) {
    load_cart_and_start();
    const size_t start = load_zone_used();
    (void)counting_block_load_zone();
    assert(load_zone_used() == start + 64);
    pico8_eu(NULL);
    pico8__update60();
    assert(load_zone_used() == start + 64 && "ni eu ni le rendu ne touchent la sous-zone");
    assert(game_zone_load_zone_max() == start + 64);
    pico8_cart_shutdown();
    assert(load_zone_used() == 0);
    assert(game_zone_load_zone_max() == start + 64 && "le maximum ne se remet qu'avec le processus");
    return 0;
}

static int case_load_zone_full(void) {
    load_cart_and_start();
    (void)allocator_alloc(game_zone_load_zone(), LOAD_ZONE_BYTES);
    (void)allocator_alloc(game_zone_load_zone(), 1);
    return 0;
}

static size_t four_pointers_bytes(void) {
    return ZONE_CELLS(4 * sizeof(entity_t *)) * sizeof(zone_cell_t);
}

static int case_static_lists_in_load_zone(void) {
    static entity_t element;
    load_cart_and_start();
    const size_t start = load_zone_used();
    entity_list_t *le = pico8_ui_le_shared();
    entity_list_add(le, &element);
    assert(load_zone_used() == start + four_pointers_bytes() &&
           "la liste de depart de ok.le prend sa capacite dans la sous-zone chargement");
    entity_list_t *by = pico8_ga_new(NULL)->outgoing_routes;
    entity_list_add(by, &element);
    assert(load_zone_used() == start + 2 * four_pointers_bytes() &&
           "le by partage des ga prend sa capacite dans la sous-zone chargement");
    pico8_cart_shutdown();
    assert(le->items == NULL && le->count == 0 && le->capacity == 0 && "ui_shutdown oublie la liste de depart");
    assert(by->items == NULL && by->count == 0 && by->capacity == 0 && "le shutdown oublie le by partage");
    return 0;
}

static int case_double_start_game_without_flush(void) {
    load_cart_and_start();
    pico8_eu(NULL);
    pico8_eu(NULL);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"alive_through_start_game", case_alive_through_start_game},
    {"dead_after_next_update", case_dead_after_next_update},
    {"init_keeps_generation", case_init_keeps_generation},
    {"dead_after_cart_load", case_dead_after_cart_load},
    {"prise_and_max", case_grab_arena_and_max},
    {"double_start_game_without_flush", case_double_start_game_without_flush},
    {"load_zone_survives_start_game", case_load_zone_survives_start_game},
    {"chargement_dead_after_cart_load", case_load_zone_dead_after_cart_load},
    {"chargement_prise_and_max", case_load_zone_grab_arena_and_max},
    {"chargement_full", case_load_zone_full},
    {"listes_statiques_dans_chargement", case_static_lists_in_load_zone},
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
        fprintf(stderr, "test_game_zone: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("alive_through_start_game");
    expect_poison_crash("dead_after_next_update");
    expect_clean("init_keeps_generation");
    expect_poison_crash("dead_after_cart_load");
    expect_clean("prise_and_max");
    expect_assert("double_start_game_without_flush", "vider la file de sortie entre deux eu");
    expect_clean("load_zone_survives_start_game");
    expect_poison_crash("chargement_dead_after_cart_load");
    expect_clean("chargement_prise_and_max");
    expect_fatal("chargement_full", "zone chargement pleine, limite 65536 octets");
    expect_clean("listes_statiques_dans_chargement");
    remove(STDERR_LOG);
    printf("test_game_zone: all tests passed\n");
    return 0;
}
