/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_file_sortie.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_ui.h"

#define STDERR_LOG "tmp/test_pico8_file_sortie_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static int rendus;

static void rendre_compte(pico8_struct_header_t *self) {
    (void)self;
    rendus++;
}

static void rendre_libere(pico8_struct_header_t *self) {
    free(self);
}

static const pico8_struct_class_t counted_cls = {.rendre = rendre_compte};
static const pico8_struct_class_t freed_cls = {.rendre = rendre_libere};
static const pico8_struct_class_t kept_cls = {0};

/* Une boite `mh` : `kc` et `ju.ed` ne designent que des boites. `m_` vise
 * un champ a elle, pour que `ui:gw` puisse l'ecrire par `gj`. */
static pico8_mh_t *new_entity(const pico8_struct_class_t *cls) {
    pico8_mh_t *mh = calloc(1, sizeof *mh);
    assert(mh != NULL);
    mh->header.cls = cls;
    mh->header.m_ = &mh->p_;
    return mh;
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

static int case_he_alive_until_flush(void) {
    load_cart_and_start();
    pico8_mh_t *e = new_entity(&freed_cls);
    pico8_lt(&e->header);
    pico8_he(&e->header);
    assert(pico8_file_sortie_compte() == 1 && "he enfile une entite de hv qui a de quoi rendre");
    sink = (unsigned char)e->p_;
    pico8__update60();
    assert(pico8_file_sortie_compte() == 0 && "update60 vide la file");
    return 0;
}

static int case_he_released_at_flush(void) {
    load_cart_and_start();
    pico8_mh_t *e = new_entity(&freed_cls);
    pico8_lt(&e->header);
    pico8_he(&e->header);
    pico8__update60();
    sink = (unsigned char)e->p_;
    return 0;
}

static int case_he_outside_hv_or_without_member(void) {
    load_cart_and_start();
    pico8_mh_t *absent = new_entity(&counted_cls);
    pico8_mh_t *kept = new_entity(&kept_cls);
    pico8_he(&absent->header);
    pico8_lt(&kept->header);
    pico8_he(&kept->header);
    assert(pico8_file_sortie_compte() == 0);
    return 0;
}

static int case_jl_enqueues_registered(void) {
    load_cart_and_start();
    pico8_mh_t *counted = new_entity(&counted_cls);
    pico8_mh_t *kept = new_entity(&kept_cls);
    pico8_lt(&counted->header);
    pico8_lt(&kept->header);
    pico8_jl();
    assert(pico8_file_sortie_compte() == 1 && "jl enfile chaque inscrit qui a de quoi rendre");
    pico8_he(&counted->header);
    assert(pico8_file_sortie_compte() == 1 && "he apres jl : l'entite n'est plus dans hv, pas deux fois");
    assert(rendus == 0 && "jl ne rend rien avant la fin de la frame");
    pico8_cart_load();
    assert(rendus == 1 && pico8_file_sortie_compte() == 0 && "cart_load vide la file");
    return 0;
}

static pico8_mh_t *pointed_at_everywhere(void) {
    load_cart_and_start();
    pico8_eu(NULL);
    pico8_mh_t *e = new_entity(&freed_cls);
    pico8_lt(&e->header);
    pico8_ok()->o_ = &e->header;
    pico8_ok()->gj = &e->header;
    pico8_msgbox_set_kc(e);
    pico8_ju()->ed = e;
    pico8_he(&e->header);
    return e;
}

static int case_flush_forgets_references(void) {
    (void)pointed_at_everywhere();
    pico8_cart_vider_file_sortie();
    assert(pico8_ok()->o_ == NULL && pico8_ok()->gj == NULL);
    assert(pico8_kc() == NULL && pico8_ju()->ed == NULL);
    return 0;
}

static int case_update60_after_forgetting(void) {
    (void)pointed_at_everywhere();
    pico8__update60();
    pico8__draw();
    assert(pico8_kc() == NULL && pico8_ju()->ed == NULL);
    return 0;
}

static int case_full(void) {
    pico8_mh_t *e = new_entity(&counted_cls);
    for (int i = 0; i <= PICO8_FILE_SORTIE_CAP; i++) pico8_file_sortie_mettre(&e->header);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"he_alive_until_flush", case_he_alive_until_flush},
    {"he_released_at_flush", case_he_released_at_flush},
    {"he_outside_hv_or_without_member", case_he_outside_hv_or_without_member},
    {"jl_enqueues_registered", case_jl_enqueues_registered},
    {"flush_forgets_references", case_flush_forgets_references},
    {"update60_after_forgetting", case_update60_after_forgetting},
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

static void expect_freed_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "heap-use-after-free") == NULL) {
        fprintf(stderr, "%s: attendu heap-use-after-free, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "une entite lue apres le vidage de la file doit planter sous ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "une file de sortie pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_pico8_file_sortie: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("he_alive_until_flush");
    expect_freed_crash("he_released_at_flush");
    expect_clean("he_outside_hv_or_without_member");
    expect_clean("jl_enqueues_registered");
    expect_clean("flush_forgets_references");
    expect_clean("update60_after_forgetting");
    expect_fatal("full", "file de sortie pleine, limite 256");
    remove(STDERR_LOG);
    printf("test_pico8_file_sortie: all tests passed\n");
    return 0;
}
