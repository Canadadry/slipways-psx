/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/message_box.h"
#include "game/score_panel.h"
#include "game/ui.h"
#include "game/ui_button.h"

#define STDERR_LOG "tmp/test_exit_queue_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static int released;

static void release_count(entity_t *self) {
    (void)self;
    released++;
}

static void release_frees(entity_t *self) {
    free(self);
}

static const entity_class_t counted_cls = {.release = release_count};
static const entity_class_t freed_cls = {.release = release_frees};
static const entity_class_t kept_cls = {0};

/* Une boite `mh` : `kc` et `ju.ed` ne designent que des boites. `m_` vise
 * un champ a elle, pour que `ui:gw` puisse l'ecrire par `gj`. */
static pico8_mh_t *new_entity(const entity_class_t *cls) {
    pico8_mh_t *mh = calloc(1, sizeof *mh);
    assert(mh != NULL);
    mh->header.class = cls;
    mh->header.is_hovered = &mh->p_;
    return mh;
}

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
    /* Le `pz` du titre a remplace `ok.le` : la premiere frame vide la file. */
    pico8_cart_vider_file_sortie();
}

static int case_entity_unregister_alive_until_flush(void) {
    load_cart_and_start();
    pico8_mh_t *e = new_entity(&freed_cls);
    entity_register(&e->header);
    entity_unregister(&e->header);
    assert(exit_queue_count() == 1 && "he enfile une entite de hv qui a de quoi rendre");
    sink = (unsigned char)e->p_;
    pico8__update60();
    assert(exit_queue_count() == 0 && "update60 vide la file");
    return 0;
}

static int case_entity_unregister_released_at_flush(void) {
    load_cart_and_start();
    pico8_mh_t *e = new_entity(&freed_cls);
    entity_register(&e->header);
    entity_unregister(&e->header);
    pico8__update60();
    sink = (unsigned char)e->p_;
    return 0;
}

/* `cj` remplace `ok.le` ; la frame relit encore l'ancienne liste. */
static const entity_list_t *replaced_ephemerals(void) {
    load_cart_and_start();
    const entity_list_t *old = main_ui()->ephemerals;
    assert(old != ui_ephemerals_shared());
    ui_clear_ephemerals(main_ui());
    assert(exit_queue_count() == 1 && "cj enfile la liste qu'il remplace");
    return old;
}

static int case_ephemerals_alive_until_flush(void) {
    const entity_list_t *old = replaced_ephemerals();
    sink = (unsigned char)old->count;
    pico8__update60();
    assert(exit_queue_count() == 0);
    return 0;
}

static int case_ephemerals_released_at_flush(void) {
    const entity_list_t *old = replaced_ephemerals();
    pico8__update60();
    sink = (unsigned char)old->count;
    return 0;
}

static int case_entity_unregister_outside_live_entities_or_without_member(void) {
    load_cart_and_start();
    pico8_mh_t *absent = new_entity(&counted_cls);
    pico8_mh_t *kept = new_entity(&kept_cls);
    entity_unregister(&absent->header);
    entity_register(&kept->header);
    entity_unregister(&kept->header);
    assert(exit_queue_count() == 0);
    return 0;
}

static int registered_with_release(void) {
    const entity_list_t *hv = live_entities();
    int n = 0;
    for (int i = 0; i < hv->count; i++) n += hv->items[i]->class->release != NULL;
    return n;
}

static int case_reset_world_enqueues_registered(void) {
    load_cart_and_start();
    pico8_mh_t *counted = new_entity(&counted_cls);
    pico8_mh_t *kept = new_entity(&kept_cls);
    entity_register(&counted->header);
    entity_register(&kept->header);
    int expected = registered_with_release();
    reset_world();
    assert(exit_queue_count() == expected && "jl enfile chaque inscrit qui a de quoi rendre");
    entity_unregister(&counted->header);
    assert(exit_queue_count() == expected && "he apres jl : l'entite n'est plus dans hv, pas deux fois");
    assert(released == 0 && "jl ne rend rien avant la fin de la frame");
    pico8_cart_load();
    assert(released == 1 && exit_queue_count() == 0 && "cart_load vide la file");
    return 0;
}

static int case_shutdown_drains(void) {
    load_cart_and_start();
    pico8_mh_t *e = new_entity(&counted_cls);
    entity_register(&e->header);
    entity_unregister(&e->header);
    pico8_cart_shutdown();
    assert(released == 1 && exit_queue_count() == 0 &&
           "cart_shutdown vide la file : aucune entree ne survit jusqu'au cart_load suivant");
    return 0;
}

static pico8_mh_t *pointed_at_everywhere(void) {
    load_cart_and_start();
    pico8_eu(NULL);
    pico8_mh_t *e = new_entity(&freed_cls);
    entity_register(&e->header);
    main_ui()->hovered = &e->header;
    main_ui()->previous_hovered = &e->header;
    pico8_msgbox_set_kc(e);
    pico8_ju()->ed = e;
    entity_unregister(&e->header);
    return e;
}

static int case_flush_forgets_references(void) {
    (void)pointed_at_everywhere();
    pico8_cart_vider_file_sortie();
    assert(main_ui()->hovered == NULL && main_ui()->previous_hovered == NULL);
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

/* `eu()` drops the title, and the difficulty button with it; `dk` still
 * reads it for the whole game. */
static int case_difficulty_outlives_its_button(void) {
    load_cart_and_start();
    const entity_list_t *hd = entities_by_message("hit_test");
    button_t *nj = NULL;
    for (int i = 0; i < hd->count && nj == NULL; i++)
        if (hd->items[i]->class == button_class() && ((button_t *)hd->items[i])->has_type_bias)
            nj = (button_t *)hd->items[i];
    assert(nj != NULL);
    const char *px = nj->name;
    p8_fixed_t trade = nj->trade;
    p8_fixed_t en = nj->type_bias;
    pico8_eu(nj);
    pico8__update60();
    pico8__draw();
    const button_t *dk = economy_score_difficulty();
    assert(dk != NULL && dk->name == px && dk->has_trade && dk->trade == trade && dk->has_type_bias && dk->type_bias == en);
    return 0;
}

static int case_full(void) {
    pico8_mh_t *e = new_entity(&counted_cls);
    for (int i = 0; i <= EXIT_QUEUE_CAP; i++) exit_queue_put(&e->header);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"entity_unregister_alive_until_flush", case_entity_unregister_alive_until_flush},
    {"entity_unregister_released_at_flush", case_entity_unregister_released_at_flush},
    {"ephemerals_alive_until_flush", case_ephemerals_alive_until_flush},
    {"ephemerals_released_at_flush", case_ephemerals_released_at_flush},
    {"he_outside_hv_or_without_member", case_entity_unregister_outside_live_entities_or_without_member},
    {"reset_world_enqueues_registered", case_reset_world_enqueues_registered},
    {"shutdown_drains", case_shutdown_drains},
    {"flush_forgets_references", case_flush_forgets_references},
    {"update60_after_forgetting", case_update60_after_forgetting},
    {"difficulty_outlives_its_button", case_difficulty_outlives_its_button},
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

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: attendu use-after-poison, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "une liste lue apres le vidage de la file doit planter sous ASan");
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
        fprintf(stderr, "test_exit_queue: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("entity_unregister_alive_until_flush");
    expect_freed_crash("entity_unregister_released_at_flush");
    expect_clean("ephemerals_alive_until_flush");
    expect_poison_crash("ephemerals_released_at_flush");
    expect_clean("he_outside_hv_or_without_member");
    expect_clean("reset_world_enqueues_registered");
    expect_clean("shutdown_drains");
    expect_clean("flush_forgets_references");
    expect_clean("update60_after_forgetting");
    expect_clean("difficulty_outlives_its_button");
    expect_fatal("full", "file de sortie pleine, limite 256");
    remove(STDERR_LOG);
    printf("test_exit_queue: all tests passed\n");
    return 0;
}
