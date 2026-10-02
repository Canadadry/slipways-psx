/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "engine/allocator.h"
#include "game/ui_zone.h"

#define STDERR_LOG "tmp/test_ui_zone_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static int case_live_slots_follow_takes(void) {
    for (int s = 0; s < SLOT_KIND_COUNT; s++) assert(ui_zone_live((slot_kind_t)s) == 0);

    const allocator_t *a = ui_zone_slot(SLOT_KIND_BUTTON);
    const allocator_t *b = ui_zone_slot(SLOT_KIND_BUTTON);
    const allocator_t *c = ui_zone_slot(SLOT_KIND_TOAST);
    assert(ui_zone_live(SLOT_KIND_BUTTON) == 2);
    assert(ui_zone_live(SLOT_KIND_TOAST) == 1);
    assert(ui_zone_live(SLOT_KIND_MESSAGE_BOX) == 0);

    arena_release(a);
    assert(ui_zone_live(SLOT_KIND_BUTTON) == 1 && "rendre une place decompte sa sorte");
    arena_release(b);
    arena_release(c);
    for (int s = 0; s < SLOT_KIND_COUNT; s++) assert(ui_zone_live((slot_kind_t)s) == 0);
    return 0;
}

static int case_slot_alive_until_released(void) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_ROW);
    unsigned char *text = allocator_alloc(slot, 64);
    memset(text, 5, 64);
    for (int i = 0; i < 64; i++) assert(text[i] == 5);
    arena_release(slot);
    return 0;
}

static int case_slot_dead_after_release(void) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_ROW);
    unsigned char *text = allocator_alloc(slot, 64);
    memset(text, 5, 64);
    arena_release(slot);
    sink = text[63];
    return 0;
}

static int case_button_slot_full(void) {
    allocator_alloc(ui_zone_slot(SLOT_KIND_BUTTON), 6240);
    return 0;
}

static int case_modal_backdrop_zone_full(void) {
    for (int i = 0; i <= 4; i++) ui_zone_slot(SLOT_KIND_MODAL_BACKDROP);
    return 0;
}

static int case_row_slot_full(void) {
    allocator_alloc(ui_zone_slot(SLOT_KIND_ROW), 1192);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"live_slots_follow_takes", case_live_slots_follow_takes},
    {"slot_alive_until_released", case_slot_alive_until_released},
    {"slot_dead_after_release", case_slot_dead_after_release},
    {"button_slot_full", case_button_slot_full},
    {"row_slot_full", case_row_slot_full},
    {"modal_backdrop_zone_full", case_modal_backdrop_zone_full},
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
        assert(0 && "une place rendue puis lue doit planter sous ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "une place trop petite ou une zone pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_ui_zone: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("live_slots_follow_takes");
    expect_clean("slot_alive_until_released");
    expect_poison_crash("slot_dead_after_release");
    expect_fatal("button_slot_full", "zone place button full, limit ");
    expect_fatal("row_slot_full", "zone place rangee full, limit ");
    expect_fatal("modal_backdrop_zone_full", "zone interface modal_backdrop full, limit 4 slots");
    remove(STDERR_LOG);
    printf("test_ui_zone: all tests passed\n");
    return 0;
}
