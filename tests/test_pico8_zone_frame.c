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
#include "../src/pico8_collect.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_ents.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_zone_choisie.h"
#include "../src/pico8_zone_frame.h"
#include "../src/pico8_zone_sizes.h"

#define STDERR_LOG "tmp/test_pico8_zone_frame_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static unsigned char *counting_block(void) {
    unsigned char *block = pico8_zone_alloc(pico8_zone_frame(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static void allocate_again(void) {
    for (int i = 0; i < 4; i++) memset(pico8_zone_alloc(pico8_zone_frame(), 64), 2, 64);
}

static int case_lazy_init(void) {
    unsigned char *block = counting_block();
    assert(block[63] == 63);
    return 0;
}

static int case_reset_before_first_use(void) {
    pico8_zone_frame_reset();
    unsigned char *block = counting_block();
    assert(block[63] == 63);
    return 0;
}

static int case_alive_within_frame(void) {
    unsigned char *block = counting_block();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "un bloc de frame vit jusqu'a la remise a zero");
    return 0;
}

static int case_dead_next_frame(void) {
    unsigned char *block = counting_block();
    pico8_zone_frame_reset();
    allocate_again();
    sink = block[63];
    return 0;
}

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

static int case_update60_alive_until_next(void) {
    load_cart_and_start();
    pico8__update60();
    unsigned char *block = counting_block();
    pico8__draw();
    sink = block[63];
    return 0;
}

static int case_update60_resets(void) {
    load_cart_and_start();
    pico8__update60();
    unsigned char *block = counting_block();
    pico8__draw();
    pico8__update60();
    allocate_again();
    sink = block[63];
    return 0;
}

#define LIST_LENGTH 12

static pico8_struct_header_t cells[LIST_LENGTH];

static int is_even_cell(pico8_struct_header_t *e, void *ctx) {
    (void)ctx;
    return (e - cells) % 2 == 0;
}

static pico8_ents_t even_cells(void) {
    pico8_ents_t all;
    pico8_ents_init(&all, pico8_zone_frame());
    for (int i = 0; i < LIST_LENGTH; i++) pico8_ents_add(&all, &cells[i]);
    return pico8_jp_ents(pico8_zone_frame(), &all, is_even_cell, NULL);
}

static int case_list_alive_within_frame(void) {
    pico8_ents_t even = even_cells();
    assert(even.count == LIST_LENGTH / 2);
    for (int i = 0; i < LIST_LENGTH / 2; i++) assert(even.items[i] == &cells[2 * i]);
    return 0;
}

static int case_list_dead_next_frame(void) {
    pico8_ents_t even = even_cells();
    pico8_zone_frame_reset();
    allocate_again();
    sink = (unsigned char)(even.items[0] == &cells[0]);
    return 0;
}

static int case_full(void) {
    pico8_zone_alloc(pico8_zone_frame(), PICO8_ZONE_FRAME_BYTES / 2);
    pico8_zone_alloc(pico8_zone_frame(), PICO8_ZONE_FRAME_BYTES / 2 + 1);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"lazy_init", case_lazy_init},
    {"reset_before_first_use", case_reset_before_first_use},
    {"alive_within_frame", case_alive_within_frame},
    {"dead_next_frame", case_dead_next_frame},
    {"update60_alive_until_next", case_update60_alive_until_next},
    {"update60_resets", case_update60_resets},
    {"list_alive_within_frame", case_list_alive_within_frame},
    {"list_dead_next_frame", case_list_dead_next_frame},
    {"full", case_full},
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
        assert(0 && "un bloc de frame lu a la frame suivante doit planter sous ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "la zone frame pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_zone_frame: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("lazy_init");
    expect_clean("reset_before_first_use");
    expect_clean("alive_within_frame");
    expect_poison_crash("dead_next_frame");
    expect_clean("update60_alive_until_next");
    expect_poison_crash("update60_resets");
    expect_clean("list_alive_within_frame");
    expect_poison_crash("list_dead_next_frame");
    expect_fatal("full", "zone frame pleine, limite 262144 octets");
    remove(STDERR_LOG);
    printf("test_pico8_zone_frame: all tests passed\n");
    return 0;
}
