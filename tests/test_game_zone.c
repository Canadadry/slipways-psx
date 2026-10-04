/* Linked under ASan. Each case runs in a child process: this binary reruns
 * itself with the case name, and the parent reads its exit status and its
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
#include "p8/vram.h"
#include "game/mouse.h"
#include "game/planet.h"
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

/* volatile: the read through a stale pointer must not be dropped by the
 * optimiser, it is what the case observes. */
static volatile unsigned char sink;

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static void load_cart_and_start(void) {
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);
    cart_load();
    cart_init();
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
    cart_start_game(NULL);
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "the generation left behind lives until the flush after cart_start_game");
    return 0;
}

static int case_dead_after_next_update(void) {
    load_cart_and_start();
    unsigned char *block = counting_block();
    cart_start_game(NULL);
    cart_update60();
    allocate_again();
    sink = block[63];
    return 0;
}

static int case_dead_after_init(void) {
    load_cart_and_start();
    cart_start_game(NULL);
    cart_update60();
    unsigned char *block = counting_block();
    cart_init();
    cart_update60();
    sink = block[63];
    return 0;
}

static int case_dead_after_cart_load(void) {
    load_cart_and_start();
    unsigned char *block = counting_block();
    cart_load();
    sink = block[63];
    return 0;
}

/* Bytes galaxy_populate takes in the generation of each game: the type
 * picker, its random source and the vanish roll. */
static size_t populate_galaxy_states(void) {
    return (2 * ZONE_CELLS(sizeof(random_walk_t)) + ZONE_CELLS(sizeof(weighted_picker_t))) *
           sizeof(zone_cell_t);
}

/* Bytes a list has taken in its generation since a reset_world(): the
 * capacities 4, 8, ... up to its own, since each growth copies and leaves
 * the old block dead until the release. NULL (a tag never seen): none. */
static size_t registry_bytes(const entity_list_t *list) {
    size_t bytes = 0;
    for (int k = 4; list != NULL && k <= list->capacity; k *= 2) {
        bytes += ZONE_CELLS((size_t)k * sizeof(entity_t *)) * sizeof(zone_cell_t);
    }
    return bytes;
}

static size_t registries_bytes(void) {
    static const entity_message_t CV[] = {ENTITY_MESSAGE_DRAW, ENTITY_MESSAGE_DRAW_OVERLAY, ENTITY_MESSAGE_HIT_TEST, ENTITY_MESSAGE_NEW_YEAR, ENTITY_MESSAGE_REFRESH};
    static const tag_t CG[] = {TAG_STRUCTURE, TAG_ASCENSION, TAG_SLIPGATE, TAG_PROBE, TAG_MESSAGE_BOX, TAG_SLIPWAY, TAG_WORMHOLE, TAG_PLANET, TAG_NODE, TAG_TARGETING};
    size_t r = registry_bytes(live_entities());
    for (size_t i = 0; i < sizeof CV / sizeof CV[0]; i++) r += registry_bytes(entities_by_message(CV[i]));
    for (size_t i = 0; i < sizeof CG / sizeof CG[0]; i++) r += registry_bytes(entities_by_tag(CG[i]));
    return r;
}

static size_t block_bytes(size_t size) {
    return ZONE_CELLS(size) * sizeof(zone_cell_t);
}

/* Bytes of a planet list made in the generation: its container, then its
 * blocks as for a registry. */
static size_t planet_list_bytes(const entity_list_t *list) {
    if (list == NULL) return 0;
    return block_bytes(sizeof *list) + registry_bytes(list);
}

/* Footprint of the tagged planets: struct, map position, position,
 * identifier and the slipways, outgoing and incoming route lists. No needs
 * nor copies of needs, which populated planets lack before an action. */
static size_t planets_bytes(void) {
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    size_t bytes = 0;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        assert(p->needs == NULL && "a planet has needs, which planets_bytes does not count");
        bytes += block_bytes(sizeof *p) + block_bytes(sizeof *p->map_pos) + block_bytes(sizeof *p->pos) +
                  block_bytes(strlen(p->planet_id) + 1) + planet_list_bytes(p->slipways) +
                  planet_list_bytes(p->outgoing_routes) + planet_list_bytes(p->incoming_routes);
    }
    return bytes;
}

/* Bytes of the economy cart_start_game makes: its struct and the
 * `invented` container, with no element while nothing is invented. */
static size_t economy_bytes(void) {
    const economy_t *fm = main_economy_typed();
    assert(fm != NULL && fm->invented != NULL && "cart_start_game built the economy of the game");
    assert(fm->invented->count == 0 && fm->invented->items == NULL && "an invention, which economy_bytes does not count");
    return block_bytes(sizeof *fm) + block_bytes(sizeof *fm->invented);
}

/* Bytes of the seven structure icons among the live entities, in table
 * order: struct, tooltip remade in a frame-zone arena, and the vector box
 * of each revealed icon. */
static size_t icons_bytes(void) {
    const entity_list_t *hv = live_entities();
    const allocator_t *measure_arena = arena_take(frame_zone(), "tooltip-measure", 16384);
    const size_t head = arena_used(measure_arena);
    size_t bytes = 0;
    int k = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const structure_icon_t *ie = (const structure_icon_t *)hv->items[i];
        if (ie->header.class != structure_icon_class()) continue;
        assert(k < ICON_TOOLTIPS_COUNT && "more icons than tooltips in icon_tooltips_table");
        (void)tooltip_new(measure_arena, &icon_tooltips_table[k]);
        bytes += block_bytes(sizeof *ie);
        if (ie->has_is_revealed && ie->is_revealed) bytes += block_bytes(sizeof(vec_t));
        k++;
    }
    assert(k == ICON_TOOLTIPS_COUNT && "cart_start_game makes one icon per tooltip of icon_tooltips_table");
    return bytes + arena_used(measure_arena) - head;
}

/* The "research and economy" term of the generation: what cart_start_game
 * makes for the economy and the icons. */
static size_t research_and_economy_bytes(void) {
    return economy_bytes() + icons_bytes();
}

/* The "singletons" term: main camera, mouse and user interface of the
 * reset_world() of cart_start_game, its background, alone among the live
 * entities, and the score panel. */
static size_t singletons_bytes(void) {
    const entity_list_t *hv = live_entities();
    int bg = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) bg += hv->items[i]->class == background_class();
    assert(bg == 1 && "cart_start_game makes a single background");
    assert(main_camera_typed() != NULL && main_mouse_typed() != NULL && main_ui() != NULL && main_score_panel() != NULL);
    return block_bytes(sizeof(camera_t)) + block_bytes(sizeof(mouse_t)) +
           block_bytes(sizeof(ui_t)) + block_bytes(sizeof(background_t)) +
           block_bytes(sizeof(score_panel_t));
}

static int case_grab_arena_and_max(void) {
    const size_t g = populate_galaxy_states();
    load_cart_and_start();
    const size_t title = game_zone_used();
    (void)counting_block();
    assert(game_zone_used() == title + 64);
    cart_start_game(NULL);
    const size_t game_zone_used_before = game_zone_used();
    assert(game_zone_used_before >= title + 64 + g && "both generations count until the release");
    (void)counting_block();
    assert(game_zone_used() == game_zone_used_before + 64);
    cart_update60();
    assert(game_zone_used() == g + registries_bytes() + planets_bytes() + research_and_economy_bytes() +
                                         singletons_bytes() + 64);
    assert(game_zone_max() == game_zone_used_before + 64);
    cart_shutdown();
    assert(game_zone_used() == 0);
    assert(game_zone_max() == game_zone_used_before + 64 && "the maximum is only reset with the process");
    return 0;
}

static const allocator_t *grab(size_t bytes) {
    planet_grab_arena_open(bytes);
    const allocator_t *arena = planet_zone_construction();
    planet_grab_arena_close();
    return arena;
}

/* Wormholes leave arenas of a growing size on the free grabs: when they are
 * full, the smallest is dropped, never a p8_fatal. */
static int case_free_grabs_keep_the_largest(void) {
    load_cart_and_start();
    const allocator_t *arenas[PLANET_GRAB_ARENAS_FREE_MAX + 1];
    for (int k = 0; k <= PLANET_GRAB_ARENAS_FREE_MAX; k++) arenas[k] = grab((size_t)(k + 1) * 64);
    for (int k = 0; k <= PLANET_GRAB_ARENAS_FREE_MAX; k++) planet_grab_arena_arena_released(arenas[k]);
    const size_t used = game_zone_used();
    assert(grab(64) == arenas[1] && "the smallest free grab that holds it; the smallest of all was dropped");
    assert(grab((PLANET_GRAB_ARENAS_FREE_MAX + 1) * 64) == arenas[PLANET_GRAB_ARENAS_FREE_MAX]);
    assert(game_zone_used() == used && "a reused grab takes nothing");
    (void)grab((PLANET_GRAB_ARENAS_FREE_MAX + 2) * 64);
    assert(game_zone_used() == used + arena_size_for((PLANET_GRAB_ARENAS_FREE_MAX + 2) * 64) &&
           "no free grab holds it: a new arena of its size");
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
    cart_start_game(NULL);
    cart_update60();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "the load sub-zone does not follow the generations");
    return 0;
}

static int case_load_zone_dead_after_cart_load(void) {
    load_cart_and_start();
    unsigned char *block = counting_block_load_zone();
    cart_load();
    for (int i = 0; i < 4; i++) memset(allocator_alloc(game_zone_load_zone(), 64), 2, 64);
    sink = block[63];
    return 0;
}

static int case_load_zone_grab_arena_and_max(void) {
    load_cart_and_start();
    const size_t start = load_zone_used();
    (void)counting_block_load_zone();
    assert(load_zone_used() == start + 64);
    cart_start_game(NULL);
    cart_update60();
    assert(load_zone_used() == start + 64 && "neither cart_start_game nor the release touches the sub-zone");
    assert(game_zone_load_zone_max() == start + 64);
    cart_shutdown();
    assert(load_zone_used() == 0);
    assert(game_zone_load_zone_max() == start + 64 && "the maximum is only reset with the process");
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
    entity_list_t *le = ui_ephemerals_shared();
    entity_list_add(le, &element);
    assert(load_zone_used() == start + four_pointers_bytes() &&
           "the starting ui_ephemerals_shared list takes its capacity in the load sub-zone");
    entity_list_t *by = burst_new(NULL)->outgoing_routes;
    entity_list_add(by, &element);
    assert(load_zone_used() == start + 2 * four_pointers_bytes() &&
           "the outgoing_routes list bursts share takes its capacity in the load sub-zone");
    cart_shutdown();
    assert(le->items == NULL && le->count == 0 && le->capacity == 0 && "ui_shutdown forgets the starting list");
    assert(by->items == NULL && by->count == 0 && by->capacity == 0 && "the shutdown forgets the shared outgoing_routes list");
    return 0;
}

static int case_double_start_game_without_flush(void) {
    load_cart_and_start();
    cart_start_game(NULL);
    cart_start_game(NULL);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"alive_through_start_game", case_alive_through_start_game},
    {"dead_after_next_update", case_dead_after_next_update},
    {"dead_after_init", case_dead_after_init},
    {"dead_after_cart_load", case_dead_after_cart_load},
    {"grab_arena_and_max", case_grab_arena_and_max},
    {"free_grabs_keep_the_largest", case_free_grabs_keep_the_largest},
    {"double_start_game_without_flush", case_double_start_game_without_flush},
    {"load_zone_survives_start_game", case_load_zone_survives_start_game},
    {"load_zone_dead_after_cart_load", case_load_zone_dead_after_cart_load},
    {"load_zone_grab_arena_and_max", case_load_zone_grab_arena_and_max},
    {"load_zone_full", case_load_zone_full},
    {"static_lists_in_load_zone", case_static_lists_in_load_zone},
};

#define CASE_COUNT (sizeof CASES / sizeof CASES[0])

/* Allocated NUL-terminated buffer, freed by the caller. */
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

/* Returns the raw status of system(); *err receives the child's stderr,
 * freed by the caller. */
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
        fprintf(stderr, "%s: expected clean, status %d\n%s", name, status, err);
        assert(0 && "the case must neither crash nor write to stderr");
    }
    free(err);
}

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (status == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: expected use-after-poison, status %d\n%s", name, status, err);
        assert(0 && "a block of a released generation must crash under ASan");
    }
    free(err);
}

static void expect_assert(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (status == 0 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: expected an assert \"%s\", status %d\n%s", name, message, status, err);
        assert(0 && "two cart_start_game calls without a flush must fail on the flip assert");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: expected exit 3 and \"%s\", status %d\n%s", name, message, status, err);
        assert(0 && "a full sub-zone must call p8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_game_zone: unknown case %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("alive_through_start_game");
    expect_poison_crash("dead_after_next_update");
    expect_poison_crash("dead_after_init");
    expect_poison_crash("dead_after_cart_load");
    expect_clean("grab_arena_and_max");
    expect_clean("free_grabs_keep_the_largest");
    expect_assert("double_start_game_without_flush", "flush the exit queue between two game starts");
    expect_clean("load_zone_survives_start_game");
    expect_poison_crash("load_zone_dead_after_cart_load");
    expect_clean("load_zone_grab_arena_and_max");
    expect_fatal("load_zone_full", "zone load full, limit 65536 bytes");
    expect_clean("static_lists_in_load_zone");
    remove(STDERR_LOG);
    printf("test_game_zone: all tests passed\n");
    return 0;
}
