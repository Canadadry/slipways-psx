/* Live slots of the UI and particle zones, and the bytes of the game zone,
 * on replays of tests/e2e/ and tests/fixtures/ driven as src/cmd/game.c
 * drives a frame (cheat_step, cart_update60, cart_draw): after a session each
 * slot kind returns to its starting level, and none rises during an idle
 * window. Each count is read after a flush of the exit queue. One replay per
 * child process: the zone maxima and module state last as long as the
 * process. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/background.h"
#include "game/cart.h"
#include "game/cheat.h"
#include "engine/entity_id.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "engine/zone_sizes.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/vram.h"
#include "game/mouse.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_trade.h"
#include "game/probe.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "game/ui.h"
#include "game/wormhole.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"
#include "game/particle_zone.h"
#include "engine/game_zone.h"

#define BEFORE_F0 (-1)
#define MAX_READINGS 4

static const char *const NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "row", "ephemerals"};

typedef struct {
    int frame;
    int live[SLOT_KIND_COUNT];
    int particle;
    int particles;
    int particles_help_screen;
    int burst;
    int transit_dots;
    size_t game_zone;
} reading_t;

static void take_reading(reading_t *r, int frame) {
    cart_flush_exit_queue();
    r->frame = frame;
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        r->live[s] = ui_zone_live((slot_kind_t)s);
    }
    r->particle = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    r->particles = particle_zone_live(PARTICLE_SLOT_LIST);
    r->particles_help_screen = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    r->burst = particle_zone_live(PARTICLE_SLOT_BURST);
    r->transit_dots = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    r->game_zone = game_zone_used();
}

/* Particles and lists held by the emitters among the live entities,
 * registered bursts, and transit dots of the slipways under the slipway tag:
 * what the particle zone counts must say. */
typedef struct {
    int particle;
    int lists;
    int bursts;
    int trails;
} registered_t;

static registered_t count_registered(void) {
    registered_t n = {0, 0, 0, 0};
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const entity_t *e = hv->items[i];
        if (e->emitter != NULL && e->emitter->particles != NULL) {
            n.lists++;
            n.particle += e->emitter->particles->count;
        }
        if (e->class == burst_class()) n.bursts++;
    }
    const entity_list_t *jc = entities_by_tag(TAG_SLIPWAY);
    for (int i = 0; jc != NULL && i < jc->count; i++) {
        const slipway_t *s = (const slipway_t *)jc->items[i];
        if (s->transit_dots != NULL) n.trails += s->transit_dots->count;
    }
    return n;
}

/* Read after a flush: a lost particle, list or burst breaks it on the very
 * frame it is lost. */
static void check_invariant(const char *e2e, int frame) {
    reading_t r;
    take_reading(&r, frame);
    registered_t n = count_registered();
    if (r.particle != n.particle || r.particles + r.particles_help_screen != n.lists || r.burst != n.bursts || r.transit_dots != n.trails) {
        fprintf(stderr, "  %s f%d: particle=%d (registered %d) particles+particles_help_screen=%d (lists %d) burst=%d (bursts %d) transit_dots=%d (trails %d)\n",
                e2e, frame, r.particle, n.particle, r.particles + r.particles_help_screen, n.lists, r.burst, n.bursts, r.transit_dots, n.trails);
        assert(0 && "a particle, particle list, burst or transit dot lives outside the registered emitters and the slipway tag");
    }
}

static void print_reading(const char *e2e, const reading_t *r) {
    printf("  %s f%d:", e2e, r->frame);
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        printf(" %s=%d", NAMES[s], r->live[s]);
    }
    printf(" particle=%d particles=%d particles_help_screen=%d burst=%d game=%lu\n", r->particle, r->particles, r->particles_help_screen, r->burst,
           (unsigned long)r->game_zone);
}

/* Runs cart_load and cart_init to replay the named e2e (NULL: no input; a
 * path when the name holds a '/') and returns its input, which is static. */
static p8_input_t *load_e2e(const char *e2e) {
    static p8_vram_t mem;
    static p8_graphics_t gfx;
    static p8_input_t input;
    char path[256];

    p8_vram_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    p8_input_init(&input);
    if (e2e != NULL) {
        snprintf(path, sizeof path, strchr(e2e, '/') != NULL ? "%s" : "tests/e2e/%s.json", e2e);
        assert(p8_input_load(&input, path));
    }
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);
    cart_load();
    cart_init();
    return &input;
}

/* Takes and prints a reading after each listed frame (BEFORE_F0: after
 * cart_init, before frame 0); every listed frame must lie in the replay. */
static void replay_e2e(const char *e2e, const int *frames, int n, reading_t *out) {
    p8_input_t *input = load_e2e(e2e);
    int k = 0;

    if (k < n && frames[k] == BEFORE_F0) {
        take_reading(&out[k], BEFORE_F0);
        print_reading(e2e, &out[k]);
        k++;
    }
    cheat_t cheat;
    cheat_init(&cheat);
    for (int i = 0; i < input->frame_count && k < n; i++) {
        dispatch_context_set_input(input);
        cheat_step(&cheat, input);
        cart_update60();
        cart_draw();
        if (frames[k] == i) {
            take_reading(&out[k], i);
            print_reading(e2e, &out[k]);
            k++;
        }
        if (i < input->frame_count - 1) p8_input_step(input);
    }
    assert(k == n && "a reading frame lies past the end of the replay");
}

/* Replays the whole e2e, checking the invariant after every frame. */
static void replay_under_invariant(const char *e2e) {
    p8_input_t *input = load_e2e(e2e);
    check_invariant(e2e, BEFORE_F0);
    cheat_t cheat;
    cheat_init(&cheat);
    for (int i = 0; i < input->frame_count; i++) {
        dispatch_context_set_input(input);
        cheat_step(&cheat, input);
        cart_update60();
        cart_draw();
        check_invariant(e2e, i);
        if (i < input->frame_count - 1) p8_input_step(input);
    }
}

/* Everything but the particle count, which an active emitter moves without
 * losing anything: the invariant follows it frame by frame. */
static int same_counts(const reading_t *a, const reading_t *b) {
    return memcmp(a->live, b->live, sizeof a->live) == 0 && a->particles == b->particles && a->particles_help_screen == b->particles_help_screen &&
           a->burst == b->burst;
}

static int none_rises(const reading_t *a, const reading_t *b) {
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        if (b->live[s] > a->live[s]) return 0;
    }
    return 1;
}

static void in_child(void (*test)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0 && "a live slots reading failed");
}

#define ASAN_LOG "tmp/test_live_slots_asan.txt"

static char *read_file(const char *path, size_t *len);

/* The child must die under ASan, reading back a released block. */
static void in_child_dying_by_asan(void (*test)(void)) {
    fflush(stdout);
    remove(ASAN_LOG);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        assert(freopen(ASAN_LOG, "w", stderr) != NULL);
        test();
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    size_t len;
    char *err = read_file(ASAN_LOG, &len);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        fprintf(stderr, "%s", err);
        assert(0 && "the released block reads back: its generation was not released");
    }
    if (strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s", err);
        assert(0 && "expected use-after-poison on the released block");
    }
    printf("  reading the released block crashes (use-after-poison)\n");
    free(err);
    remove(ASAN_LOG);
}

static char g_e2e[128];

static void invariant_on_current_e2e(void) {
    replay_under_invariant(g_e2e);
}

/* Every frame of every e2e of tests/e2e/; returns how many e2e ran. */
static int invariant_on_all_e2e(void) {
    DIR *dir = opendir("tests/e2e");
    struct dirent *ent;
    int n = 0;
    assert(dir != NULL);
    while ((ent = readdir(dir)) != NULL) {
        size_t len = strlen(ent->d_name);
        if (len <= 5 || strcmp(ent->d_name + len - 5, ".json") != 0) continue;
        assert(len - 5 < sizeof g_e2e);
        memcpy(g_e2e, ent->d_name, len - 5);
        g_e2e[len - 5] = '\0';
        in_child(invariant_on_current_e2e);
        n++;
    }
    closedir(dir);
    return n;
}

/* f7: 'restart'; f11: title screen, before the f12 click. */
static void pause_restart_returns_to_title(void) {
    const int frames[] = {BEFORE_F0, 11};
    reading_t r[2];
    replay_e2e("pause_restart", frames, 2, r);
    assert(same_counts(&r[0], &r[1]) && "reset_world must give back every slot it drops");
    assert(r[0].particles_help_screen == 1 && r[0].particles == 0 && r[0].burst == 0);
    /* No help screen particle dies before 115 draws: 5 per draw, no draw
     * before f0, and the draws of f7 to f11 for the help screen of 'restart'. */
    assert(r[0].particle == 0 && r[1].particle == 5 * 5 && "reset_world must give back the help screen particles");
}

/* volatile: the optimizer must not drop the read of a released sentinel,
 * the very read the case observes. */
static volatile unsigned char g_spend;

static p8_input_t *g_input;
static cheat_t g_cheat;
static int g_frame;

/* Without take_reading: its flush of the exit queue would release the
 * generation itself, instead of the cart_update60 of the frame after the
 * click. */
static void play_until(int last) {
    for (; g_frame <= last && g_frame < g_input->frame_count; g_frame++) {
        dispatch_context_set_input(g_input);
        cheat_step(&g_cheat, g_input);
        cart_update60();
        cart_draw();
        if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    }
    assert(g_frame == last + 1 && "a sentinel frame lies past the end of the replay");
}

static void start_e2e(const char *e2e) {
    g_input = load_e2e(e2e);
    cheat_init(&g_cheat);
    g_frame = 0;
}

static unsigned char *sentinel(void) {
    unsigned char *s = allocator_alloc(game_zone(), 64);
    for (int i = 0; i < 64; i++) s[i] = (unsigned char)i;
    return s;
}

static void check_sentinel(const unsigned char *s) {
    for (int i = 0; i < 64; i++) assert(s[i] == i && "the game zone sentinel changed");
}

/* f12: 'challenging' click, so cart_start_game: the zone flips, and the
 * generation of the f7 title screen lives until the end of the frame. */
static void pause_restart_keeps_title_during_click(void) {
    start_e2e("pause_restart");
    play_until(11);
    assert(!game_zone_pending());
    unsigned char *s = sentinel();
    size_t used_before = game_zone_used();
    play_until(12);
    assert(game_zone_pending() && "the f12 click flips the game zone");
    check_sentinel(s);
    assert(game_zone_used() >= used_before && "the used bytes count the quit game until it is released");
    printf("  pause_restart f12: game=%lu, sentinel read\n", (unsigned long)game_zone_used());
}

/* f13: cart_update60 flushes the exit queue, then releases the title
 * generation. */
static void pause_restart_releases_title_next_frame(void) {
    start_e2e("pause_restart");
    play_until(11);
    unsigned char *s = sentinel();
    play_until(13);
    g_spend = s[63];
}

/* f7: 'restart' (cart_init) flips the zone, so the title screen has its
 * own generation; the quit game lives until the end of the click frame. */
static void pause_restart_keeps_game_during_restart_click(void) {
    start_e2e("pause_restart");
    play_until(6);
    assert(!game_zone_pending());
    unsigned char *s = sentinel();
    play_until(7);
    assert(game_zone_pending() && "'restart' flips the game zone");
    check_sentinel(s);
}

/* f8: cart_update60 releases the generation of the game quit at f7. */
static void pause_restart_releases_game_after_restart(void) {
    start_e2e("pause_restart");
    play_until(6);
    unsigned char *s = sentinel();
    play_until(8);
    assert(!game_zone_pending());
    g_spend = s[63];
}

/* f8: the quit game's economy and score panel died at the release; the
 * title screen must not keep them as main economy and main score panel,
 * which the exit-queue flush, the cheat and make state read. */
static void pause_restart_title_drops_quit_economy_and_panel(void) {
    start_e2e("pause_restart");
    play_until(8);
    assert(main_economy() == NULL && "the title after 'restart' keeps the released economy");
    assert(main_score_panel() == NULL && "the title after 'restart' keeps the released score panel");
}

static const planet_t *planet_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(6);
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    assert(jo != NULL && jo->count > 0 && "the first game has planets");
    return (const planet_t *)jo->items[0];
}

/* f7: the 'restart' click flips the zone; the planets of the quit game
 * still live. */
static void pause_restart_keeps_quit_planets_during_click(void) {
    const planet_t *p = planet_of_first_game();
    play_until(7);
    assert(game_zone_pending());
    g_spend = (unsigned char)p->has_production_bonus;
}

/* f8: they die with their generation. */
static void pause_restart_releases_quit_planets(void) {
    const planet_t *p = planet_of_first_game();
    play_until(8);
    g_spend = (unsigned char)p->has_production_bonus;
}

static const economy_t *economy_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(6);
    const economy_t *fm = main_economy_typed();
    assert(fm != NULL && "the first game has its main economy");
    return fm;
}

/* f7: its generation lives until the end of the frame. */
static void pause_restart_keeps_quit_economy_during_click(void) {
    const economy_t *fm = economy_of_first_game();
    play_until(7);
    assert(game_zone_pending());
    g_spend = (unsigned char)fm->credits;
}

/* f8: it dies with its generation. */
static void pause_restart_releases_quit_economy(void) {
    const economy_t *fm = economy_of_first_game();
    play_until(8);
    g_spend = (unsigned char)fm->credits;
}

/* f6: the live entities hold the structure icons of the first game; the
 * reset_world of cart_init removes them at f7. */
static const structure_icon_t *icon_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(6);
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class == structure_icon_class()) return (const structure_icon_t *)hv->items[i];
    }
    assert(0 && "the first game has structure icons among the live entities");
    return NULL;
}

static void pause_restart_keeps_quit_icons_during_click(void) {
    const structure_icon_t *ie = icon_of_first_game();
    play_until(7);
    assert(game_zone_pending());
    g_spend = (unsigned char)ie->is_revealed;
}

static void pause_restart_releases_quit_icons(void) {
    const structure_icon_t *ie = icon_of_first_game();
    play_until(8);
    g_spend = (unsigned char)ie->is_revealed;
}

/* The help screen among the live entities, made by cart_init. */
static const probe_t *help_screen_in_live_entities(void) {
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class == help_screen_class()) return (const probe_t *)hv->items[i];
    }
    assert(0 && "the title screen has its help screen among the live entities");
    return NULL;
}

static const background_t *background_in_live_entities(void);

/* The first five live in the generation of the first game, read at f6 and
 * released at f8; the last four in that of the f7 title screen, read at
 * f11 and released at f13. */
enum {
    SINGLETON_GAME_CAMERA, SINGLETON_GAME_MOUSE, SINGLETON_GAME_UI, SINGLETON_BACKGROUND, SINGLETON_MAIN_SCORE_PANEL,
    SINGLETON_TITLE_CAMERA, SINGLETON_TITLE_MOUSE, SINGLETON_TITLE_UI, SINGLETON_HELP_SCREEN, SINGLETON_COUNT
};
static const char *const NAMES_OF_SINGLETONS[SINGLETON_COUNT] = {
    "game main_camera", "game main_mouse", "game main_ui", "background", "main_score_panel",
    "title main_camera", "title main_mouse", "title main_ui", "help_screen"};

static void singletons_of_quit_game(const void *out[SINGLETON_COUNT]) {
    start_e2e("pause_restart");
    play_until(6);
    out[SINGLETON_GAME_CAMERA] = main_camera_typed();
    out[SINGLETON_GAME_MOUSE] = main_mouse_typed();
    out[SINGLETON_GAME_UI] = main_ui();
    out[SINGLETON_BACKGROUND] = background_in_live_entities();
    out[SINGLETON_MAIN_SCORE_PANEL] = main_score_panel();
    for (int k = 0; k < SINGLETON_TITLE_CAMERA; k++) assert(out[k] != NULL && "a singleton is missing at f6");
}

static void singletons_of_title(const void *out[SINGLETON_COUNT]) {
    singletons_of_quit_game(out);
    play_until(11);
    out[SINGLETON_TITLE_CAMERA] = main_camera_typed();
    out[SINGLETON_TITLE_MOUSE] = main_mouse_typed();
    out[SINGLETON_TITLE_UI] = main_ui();
    out[SINGLETON_HELP_SCREEN] = help_screen_in_live_entities();
    for (int k = SINGLETON_TITLE_CAMERA; k < SINGLETON_COUNT; k++) assert(out[k] != NULL && "a singleton is missing at f11");
}

/* f7: the quit main_ui finishes its update after the reset_world of
 * cart_init. */
static void pause_restart_keeps_quit_singletons_during_click(void) {
    const void *s[SINGLETON_COUNT];
    singletons_of_quit_game(s);
    play_until(7);
    assert(game_zone_pending());
    assert(main_camera_typed() != s[SINGLETON_GAME_CAMERA] && main_ui() != s[SINGLETON_GAME_UI] &&
           "the f7 click makes new singletons");
    for (int k = 0; k < SINGLETON_TITLE_CAMERA; k++) g_spend = *(const unsigned char *)s[k];
}

/* f12: the title main_ui finishes its update after the reset_world of
 * cart_start_game. */
static void pause_restart_keeps_title_singletons_during_click(void) {
    const void *s[SINGLETON_COUNT];
    singletons_of_title(s);
    play_until(12);
    assert(game_zone_pending());
    assert(main_camera_typed() != s[SINGLETON_TITLE_CAMERA] && main_ui() != s[SINGLETON_TITLE_UI] &&
           "the f12 click makes new singletons");
    for (int k = SINGLETON_TITLE_CAMERA; k < SINGLETON_COUNT; k++) g_spend = *(const unsigned char *)s[k];
}

static int g_singleton;

/* Each one dies with its generation. */
static void pause_restart_releases_singleton(void) {
    const void *s[SINGLETON_COUNT];
    if (g_singleton < SINGLETON_TITLE_CAMERA) {
        singletons_of_quit_game(s);
        play_until(8);
    } else {
        singletons_of_title(s);
        play_until(13);
    }
    g_spend = *(const unsigned char *)s[g_singleton];
}

/* Bytes that galaxy_populate takes in the generation of each
 * cart_start_game: the type picker, its random walk and the vanish roll. */
static size_t populate_galaxy_states(void) {
    return (2 * ZONE_CELLS(sizeof(random_walk_t)) + ZONE_CELLS(sizeof(weighted_picker_t))) *
           sizeof(zone_cell_t);
}

/* Bytes a list has taken in its generation: its first block, of `first`
 * entries, then each doubling up to its capacity, as each growth copies and
 * leaves the old block dead until the release. A NULL list (a tag nothing
 * registered under yet) still holds its first block. */
static size_t list_bytes_from(const entity_list_t *list, int first) {
    size_t bytes = 0;
    const int capacity = list != NULL ? list->capacity : first;
    for (int k = first; k > 0 && k <= capacity; k *= 2) {
        bytes += ZONE_CELLS((size_t)k * sizeof(entity_t *)) * sizeof(zone_cell_t);
    }
    return bytes;
}

/* The registries of a game, each taken at its reserve by cart_start_game. */
static size_t registries_bytes(void) {
    const entity_registry_reserve_t *rv = &cart_game_registry_reserve;
    size_t r = ENTITY_TABLE_BYTES + list_bytes_from(live_entities(), rv->live);
    for (int m = 0; m < ENTITY_MESSAGE_COUNT; m++) {
        r += list_bytes_from(entities_by_message((entity_message_t)m), rv->by_message[m]);
    }
    for (int t = 0; t < TAG_COUNT; t++) r += list_bytes_from(entities_by_tag((tag_t)t), rv->by_tag[t]);
    return r;
}

static size_t block_bytes(size_t size) {
    return ZONE_CELLS(size) * sizeof(zone_cell_t);
}

/* Bytes of a planet list made in the generation: its container, then its
 * blocks as for a registry. */
static size_t id_list_bytes(const entity_id_list_t *list) {
    size_t bytes = 0;
    for (int k = 4; list != NULL && k <= list->capacity; k *= 2) bytes += block_bytes((size_t)k * sizeof(entity_id_t));
    return bytes;
}

static size_t planet_list_bytes(const entity_id_list_t *list) {
    if (list == NULL) return 0;
    return block_bytes(sizeof *list) + id_list_bytes(list);
}

static size_t route_list_bytes(const trade_route_list_t *list) {
    if (list == NULL) return 0;
    size_t bytes = block_bytes(sizeof *list);
    for (int k = 4; k <= list->values.capacity; k *= 2) bytes += block_bytes((size_t)k * sizeof(trade_route_t));
    return bytes;
}

/* Footprint of the planets under the planet tag: struct, map_pos and pos
 * boxes, planet_id, and the slipways, outgoing_routes and incoming_routes
 * lists. No needs list nor copies of needs, which the planets of
 * galaxy_populate lack before any action. */
static size_t planets_bytes(void) {
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    size_t bytes = 0;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        assert(p->needs == NULL && "a planet has needs, which planets_bytes does not count");
        bytes += block_bytes(sizeof *p) + block_bytes(sizeof *p->map_pos) + block_bytes(sizeof *p->pos) +
                  block_bytes(strlen(p->planet_id) + 1) + planet_list_bytes(p->slipways) +
                  route_list_bytes(p->outgoing_routes) + route_list_bytes(p->incoming_routes);
    }
    return bytes;
}

/* Bytes of the main economy made by cart_start_game: its struct and the
 * invented container, with no items while nothing is invented. */
static size_t economy_bytes(void) {
    const economy_t *fm = main_economy_typed();
    assert(fm != NULL && fm->invented != NULL && "cart_start_game made the game economy");
    assert(fm->invented->count == 0 && fm->invented->items == NULL && "an invention, which economy_bytes does not count");
    return block_bytes(sizeof *fm) + block_bytes(sizeof *fm->invented);
}

/* Bytes of the seven structure icons among the live entities, in the order
 * of icon_tooltips_table: struct, tooltip (rebuilt in a frame zone arena to
 * measure it), and for each revealed icon the box of the shared icon cursor
 * it moved. */
static size_t icons_bytes(void) {
    const entity_list_t *hv = live_entities();
    const allocator_t *measure_arena = arena_take(frame_zone(), "tooltip-measure", 16384);
    const size_t head = arena_used(measure_arena);
    size_t bytes = 0;
    int k = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const structure_icon_t *ie = (const structure_icon_t *)hv->items[i];
        if (ie->header.class != structure_icon_class()) continue;
        assert(k < ICON_TOOLTIPS_COUNT && "more structure icons than icon tooltips");
        (void)tooltip_new(measure_arena, &icon_tooltips_table[k]);
        bytes += block_bytes(sizeof *ie);
        if (ie->has_is_revealed && ie->is_revealed) bytes += block_bytes(sizeof(vec_t));
        k++;
    }
    assert(k == ICON_TOOLTIPS_COUNT && "cart_start_game makes one structure icon per icon tooltip");
    return bytes + arena_used(measure_arena) - head;
}

/* The 'research and economy' term of the generation: what cart_start_game
 * makes for the main economy and the structure icons. */
static size_t research_and_economy_bytes(void) {
    return economy_bytes() + icons_bytes();
}

/* The background among the live entities, which only cart_start_game
 * makes. */
static const background_t *background_in_live_entities(void) {
    const entity_list_t *hv = live_entities();
    const background_t *bg = NULL;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class != background_class()) continue;
        assert(bg == NULL && "the live entities hold two backgrounds");
        bg = (const background_t *)hv->items[i];
    }
    assert(bg != NULL && "cart_start_game made the game background");
    return bg;
}

/* The 'singletons' term: the camera, mouse and main_ui of the reset_world of
 * cart_start_game, its background and its score panel. */
static size_t singletons_bytes(void) {
    assert(main_camera_typed() != NULL && main_mouse_typed() != NULL && main_ui() != NULL && main_score_panel() != NULL);
    (void)background_in_live_entities();
    return block_bytes(sizeof(camera_t)) + block_bytes(sizeof(mouse_t)) +
           block_bytes(sizeof(ui_t)) + block_bytes(sizeof(background_t)) +
           block_bytes(sizeof(score_panel_t));
}

/* After the f13 release, the generation holds only what the f12
 * cart_start_game made and what has grown since: the galaxy_populate states,
 * the registries at their reserves, its planets, its economy and
 * structure icons, and its singletons. */
static void pause_restart_bounds_dead_registry_blocks(void) {
    const size_t g = populate_galaxy_states();
    start_e2e("pause_restart");
    play_until(13);
    const size_t r = registries_bytes();
    const size_t p = planets_bytes();
    printf("  pause_restart f13: game=%lu, galaxy_populate states=%lu, registries=%lu, planets=%lu\n",
           (unsigned long)game_zone_used(), (unsigned long)g, (unsigned long)r, (unsigned long)p);
    assert(game_zone_used() == g + r + p + research_and_economy_bytes() + singletons_bytes() &&
           "the registries take their reserves and leave no dead block, and nothing else gets in");
}

/* f11: the title screen alone, built by cart_init with empty registries;
 * f13: the game the f12 cart_start_game started, its registries taken at
 * their reserves, none of which the pause_restart game exceeds. */
static void pause_restart_reserves_registries_at_game_start_only(void) {
    const entity_registry_reserve_t *rv = &cart_game_registry_reserve;
    start_e2e("pause_restart");
    play_until(11);
    assert(live_entities()->capacity < rv->live && "the title screen takes the reserves of a game");
    play_until(13);
    assert(live_entities()->capacity == rv->live);
    for (int m = 0; m < ENTITY_MESSAGE_COUNT; m++) {
        assert(entities_by_message((entity_message_t)m)->capacity == rv->by_message[m]);
    }
    for (int t = 0; t < TAG_COUNT; t++) {
        const entity_list_t *list = entities_by_tag((tag_t)t);
        assert((list == NULL || list->capacity == rv->by_tag[t]) && "a registry of the game grew past its reserve");
    }
    printf("  pause_restart f13: live %d/%d\n", live_entities()->count, live_entities()->capacity);
}

/* Only cart_load fills the load zone: two games add nothing to it. */
static void pause_restart_leaves_load_zone_alone(void) {
    start_e2e("pause_restart");
    play_until(1);
    const size_t f1 = load_zone_used();
    assert(f1 > 0 && "the research projects live in the load zone");
    play_until(g_input->frame_count - 1);
    printf("  pause_restart: load=%lu at f1 and at the end of the replay\n", (unsigned long)f1);
    assert(load_zone_used() == f1 && "a game takes from the load zone");
    assert(game_zone_load_zone_max() == f1);
}

/* f11: the title screen alone, the quit game released at f8; f12: the
 * galaxy_populate of the new game takes its states beside the title
 * generation, released at f13. */
static void pause_restart_counts_populate_galaxy_states(void) {
    const size_t g = populate_galaxy_states();
    start_e2e("pause_restart");
    play_until(11);
    const size_t f11 = game_zone_used();
    play_until(12);
    const size_t f12 = game_zone_used();
    assert(f12 >= f11 + g && "the title generation lives beside the new game until the release");
    play_until(13);
    assert(game_zone_used() == g + registries_bytes() + planets_bytes() + research_and_economy_bytes() +
                                             singletons_bytes() &&
           "the f13 release leaves the states, registries, planets, economy and singletons of the new game");
    assert(game_zone_max() >= f12);
    printf("  pause_restart f11/f12/f13: game=%lu/%lu/%lu, max=%lu\n", (unsigned long)f11, (unsigned long)f12,
           (unsigned long)game_zone_used(), (unsigned long)game_zone_max());
}

/* Bytes of the generation outside the registries, which grow whenever an
 * entity registers. */
static size_t game_without_registries(void) {
    return game_zone_used() - registries_bytes();
}

/* f3-51: 25 year jumps, so as many economy_tiers_rebuild_incoming_routes
 * calls, which must not grow the generation: each incoming_routes list is
 * emptied in place. */
static void game_end_final_score_rebuild_does_not_grow_game(void) {
    start_e2e("game_end_final_score");
    play_until(2);
    const size_t f2 = game_without_registries();
    play_until(51);
    const size_t f51 = game_without_registries();
    printf("  game_end_final_score f2/f51: game - registries = %lu/%lu\n", (unsigned long)f2, (unsigned long)f51);
    assert(f2 == f51 && "an incoming routes rebuild grows the generation");
}

/* f140-151: the f140 year jump over seven placed slipways, so
 * planet_rebuild_trades, planet_offer_trade and slipgate_offer_trade, which
 * offer known routes again. The f139 rebuild keeps the route of the slipway
 * placed at f136. The refused slipway of the f142 click takes an arena, which
 * the f148 one reuses. */
static void trade_rich_mine_route_rebuild_does_not_grow_game(void) {
    start_e2e("trade_rich_mine");
    play_until(139);
    const size_t f139 = game_without_registries();
    play_until(151);
    const size_t f151 = game_without_registries();
    printf("  trade_rich_mine f139/f151: game - registries = %lu/%lu\n", (unsigned long)f139,
           (unsigned long)f151);
    assert(f151 == f139 + arena_size_for(SLIPWAY_CONSTRUCTION_BYTES) &&
           "a rebuild over routes grows the generation, or the f148 refusal does not reuse the f142 arena");
}

static const planet_t *planet_with_id(const char *id) {
    const entity_list_t *np = entities_by_tag(TAG_NODE);
    for (int i = 0; np != NULL && i < np->count; i++) {
        const planet_t *p = (const planet_t *)np->items[i];
        if (p->planet_id != NULL && strcmp(p->planet_id, id) == 0) return p;
    }
    assert(0 && "planet not found under the node tag");
    return NULL;
}

/* A route of outgoing_routes kept by the f139 rebuild reads back at f151,
 * after twelve resets of the frame zone where its candidates are made. */
static void trade_rich_mine_kept_route_survives_frame(void) {
    entity_id_t lp[16];
    entity_id_t rl[16];
    start_e2e("trade_rich_mine");
    play_until(139);
    const trade_route_list_t *by = planet_with_id("p27")->outgoing_routes;
    assert(by != NULL && trade_route_list_count(by) > 0 && "f139: p27 keeps a route");
    const trade_route_t *t = trade_route_list_at(by, trade_route_list_count(by) - 1);
    const int nlp = t->planets->count;
    const int nrl = t->slipways->count;
    assert(nlp <= 16 && nrl <= 16);
    memcpy(lp, t->planets->items, (size_t)nlp * sizeof *lp);
    memcpy(rl, t->slipways->items, (size_t)nrl * sizeof *rl);
    play_until(151);
    assert(t->planets->count == nlp && memcmp(lp, t->planets->items, (size_t)nlp * sizeof *lp) == 0);
    assert(t->slipways->count == nrl && memcmp(rl, t->slipways->items, (size_t)nrl * sizeof *rl) == 0);
    printf("  trade_rich_mine: p27 route (%d planets, %d slipways) read back at f151\n", nlp, nrl);
}

/* At f139, a rebuild of p27 over its kept routes gives back to the frame
 * zone every candidate offer it made, and the routes it hands back still
 * read the same. */
static void trade_rich_mine_rebuild_gives_back_frame(void) {
    entity_id_t lp[16];
    start_e2e("trade_rich_mine");
    play_until(139);
    planet_t *p27 = (planet_t *)planet_with_id("p27");
    assert(p27->slipways != NULL && p27->slipways->count > 0 && "f139: p27 is linked");
    const trade_route_list_t *by = p27->outgoing_routes;
    assert(by != NULL && trade_route_list_count(by) > 0 && "f139: p27 keeps a route");
    const int n = trade_route_list_count(by);
    const trade_route_t *t = trade_route_list_at(by, n - 1);
    const int nlp = t->planets->count;
    assert(nlp <= 16);
    memcpy(lp, t->planets->items, (size_t)nlp * sizeof *lp);
    const size_t before = frame_zone_used();
    trade_route_list_t *out = planet_rebuild_trades(p27);
    const size_t after = frame_zone_used();
    printf("  trade_rich_mine f139: frame %lu -> %lu across a rebuild of p27\n", (unsigned long)before,
           (unsigned long)after);
    assert(after == before && "planet_rebuild_trades keeps its candidate offers in the frame zone");
    assert(out == by && trade_route_list_count(out) == n && "the rebuild hands back the kept routes");
    assert(t->planets->count == nlp && memcmp(lp, t->planets->items, (size_t)nlp * sizeof *lp) == 0);
}

/* The transit dots still alive at the end of the game go back to the
 * particle zone through the release method of the slipway class, on the next
 * cart_load. */
static void trade_rich_mine_releases_trails_on_reload(void) {
    start_e2e("trade_rich_mine");
    play_until(g_input->frame_count - 1);
    const int end = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    cart_load();
    const int after = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    printf("  trade_rich_mine: transit_dots %d at the end of the replay, %d after reload (max %d, list max %d)\n", end, after,
           particle_zone_max_live(PARTICLE_SLOT_TRANSIT_DOT),
           particle_zone_max_elements(PARTICLE_SLOT_TRANSIT_DOT));
    assert(end > 0 && "trade_rich_mine ends with live transit dots");
    assert(after == 0 && "releasing a slipway gives back its transit dots");
}

#define GRABS "tests/fixtures/grabs_refused.json"

/* GRABS: f0-20 of empty_planet_fade; then 7-frame grabs (press on the
 * icon at (154,116), drag, release, three frames of waiting): twenty refused
 * over p34 (f21-160), one placed (f161-167), one refused (f168-174); 'restart'
 * and a new game (click at f185); one grab (f189-195). */
static void refused_grabs_do_not_grow_game(void) {
    start_e2e(GRABS);
    play_until(27);
    const size_t a = game_without_registries();
    play_until(160);
    const size_t twenty = game_without_registries();
    printf("  refused grabs 1/20: game - registries = %lu/%lu\n", (unsigned long)a, (unsigned long)twenty);
    assert(a == twenty && "a refused grab leaves its structure in the generation");
}

/* The structure placed at f161-164, the only planet under the planet tag
 * whose identifier starts with an 's'. */
static const planet_t *placed_grab(void) {
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    const planet_t *placed = NULL;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        if (p->planet_id[0] != 's') continue;
        assert(placed == NULL && "a single grab was placed");
        placed = p;
    }
    assert(placed != NULL && placed->header.state == ENTITY_STATE_SETTLED && "the f161-164 grab was not placed");
    return placed;
}

/* The f4 wormhole's arena, free since f48, is too small for a structure:
 * the grabs take one new arena of the structure's size, full, and the
 * rebuild after the placement makes the outgoing_routes and incoming_routes
 * lists in the generation, outside the arena. */
static void placed_grab_does_not_fit_dead_wormhole_arena(void) {
    start_e2e(GRABS);
    play_until(160);
    const size_t before = game_without_registries();
    play_until(174);
    const size_t after = game_without_registries();
    const planet_t *placed = placed_grab();
    const allocator_t *arena = placed->slipways->zone;
    const size_t lists = route_list_bytes(placed->outgoing_routes) + route_list_bytes(placed->incoming_routes);
    printf("  placed grab: game - registries = %lu -> %lu (arena %lu, outgoing_routes/incoming_routes %lu)\n",
           (unsigned long)before, (unsigned long)after, (unsigned long)arena_size(arena), (unsigned long)lists);
    assert(arena_used(arena) == arena_size(arena) && "the placed structure fills its arena");
    assert(after == before + arena_size(arena) + lists &&
           "one new arena for the grabs past the f4 wormhole's, too small, and the placed grab's route lists");
}

/* One replay frame; when draw is 0, no draw method clears targeted_by. */
static void play_one_frame(int draw) {
    dispatch_context_set_input(g_input);
    cheat_step(&g_cheat, g_input);
    cart_update60();
    if (draw) cart_draw();
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
}

static planet_t *planet_targeted_by(const entity_t *held) {
    const entity_list_t *np = entities_by_tag(TAG_NODE);
    for (int i = 0; np != NULL && i < np->count; i++) {
        planet_t *p = (planet_t *)np->items[i];
        if (entity_get(p->targeted_by) == held) return p;
    }
    return NULL;
}

/* f23: the structure held over p34 sets its targeted_by; p34 moves off
 * screen before the draw, and its draw returns without clearing targeted_by.
 * Refused at f24, the structure is freed by the f26 flush, and the f28 press
 * reuses its arena: targeted_by must not point at the new structure. */
static void offscreen_planet_forgets_refused_grab(void) {
    static vec_t far;
    start_e2e(GRABS);
    play_until(22);
    dispatch_context_set_input(g_input);
    cheat_step(&g_cheat, g_input);
    cart_update60();
    const entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && "f23: a structure is held");
    planet_t *p34 = planet_targeted_by(held);
    assert(p34 != NULL && "f23: the held structure sets a planet's targeted_by");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p34->pos = &far;
    assert(!entity_geom_is_on_screen(&p34->header) && "p34 is off screen");
    cart_draw();
    assert(entity_get(p34->targeted_by) == held && "off screen, the draw leaves targeted_by set");
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    while (g_frame <= 27) play_one_frame(1);
    play_one_frame(0);
    assert(ui_left_held(main_ui()) == held && "f28: the next grab reuses the arena, at the same address");
    assert(entity_get(p34->targeted_by) == NULL && "an off-screen planet's targeted_by points at a refused, recycled grab");
}

static void refused_grab_crosses_new_game(void) {
    start_e2e(GRABS);
    play_until(195);
}

#define SLIPWAYS "tests/fixtures/slipways_refused.json"

/* SLIPWAYS: f0-34 of slipway_build (p27 and p48 settled); then 15-frame
 * cycles: press on p27 at (104,33), ten frames held, release, three frames of
 * waiting. Twenty refusals held at (56,96) (f35-334), one drag placed from
 * p27 to p48 (f335-338, waiting f339-341), one refusal held at (89,87),
 * beyond p48 (f342-356); 'restart' and a new game (click at f367); one icon
 * grab (f371-377). A refusal is released on the 12th frame of its cycle and
 * freed by the flush of the 14th. */
static const slipway_t *held_slipway(void) {
    const entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == slipway_class() && "a slipway is held");
    return (const slipway_t *)held;
}

static void held_slipway_does_not_grow_game(void) {
    start_e2e(SLIPWAYS);
    play_until(36);
    const slipway_t *held = held_slipway();
    const size_t f36 = game_zone_used();
    play_until(45);
    assert(held_slipway() == held && slipway_end(held) == &held->point.header && "f45: held over empty space");
    printf("  held slipway f36/f45: game = %lu/%lu\n", (unsigned long)f36, (unsigned long)game_zone_used());
    assert(game_zone_used() == f36 && "a held slipway allocates every frame");
}

static void refused_slipways_do_not_grow_game(void) {
    start_e2e(SLIPWAYS);
    play_until(49);
    const size_t first_refusal = game_without_registries();
    play_until(334);
    const size_t twenty = game_without_registries();
    printf("  refused slipways 1/20: game - registries = %lu/%lu\n", (unsigned long)first_refusal, (unsigned long)twenty);
    assert(first_refusal == twenty && "a refused slipway leaves its arena in the generation");
}

static size_t slipways_of_p27_and_p48_bytes(void) {
    return id_list_bytes(planet_with_id("p27")->slipways) + id_list_bytes(planet_with_id("p48")->slipways);
}

/* The f4 wormhole's arena, free since f48, is too small for a slipway: the
 * placed slipway reuses the arena of the last refused one, the next refused
 * one takes a new arena of a slipway's size. The planet_attach_slipway of
 * the placement grows the slipways lists at both ends, in the generation,
 * outside the arena. */
static void placed_slipway_does_not_fit_dead_wormhole_arena(void) {
    start_e2e(SLIPWAYS);
    play_until(334);
    const size_t before = game_without_registries();
    const size_t slipways_before = slipways_of_p27_and_p48_bytes();
    play_until(356);
    const size_t after = game_without_registries();
    const size_t oe = slipways_of_p27_and_p48_bytes() - slipways_before;
    assert(entities_by_tag(TAG_SLIPWAY) != NULL && entities_by_tag(TAG_SLIPWAY)->count == 1 && "f338: one placed slipway");
    printf("  slipway placed then refused: game - registries = %lu -> %lu (slipways %lu)\n", (unsigned long)before,
           (unsigned long)after, (unsigned long)oe);
    assert(after == before + arena_size_for(SLIPWAY_CONSTRUCTION_BYTES) + oe &&
           "one new slipway arena past the f4 wormhole's, too small, and the slipways lists of the placement");
}

/* f345: the slipway held from p27 towards (89,87) sets the targeted_by of
 * p48, on its way, which is pushed off screen: its draw no longer clears
 * targeted_by. Refused at f353, the slipway is freed by the f355 flush. */
static void offscreen_planet_forgets_refused_slipway(void) {
    static vec_t far;
    start_e2e(SLIPWAYS);
    play_until(344);
    dispatch_context_set_input(g_input);
    cheat_step(&g_cheat, g_input);
    cart_update60();
    const slipway_t *held = held_slipway();
    planet_t *p = planet_targeted_by(&held->header);
    assert(p != NULL && "f345: the held slipway sets a planet's targeted_by");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p->pos = &far;
    assert(!entity_geom_is_on_screen(&p->header) && "the planet is off screen");
    cart_draw();
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    play_until(354);
    assert(entity_get(p->targeted_by) == &held->header && "off screen, the draw leaves targeted_by set");
    play_until(355);
    printf("  refused slipway: targeted_by of %s cleared by the f355 flush\n", p->planet_id);
    assert(entity_get(p->targeted_by) == NULL && "an off-screen planet's targeted_by points at a refused, released slipway");
}

static void refused_slipway_crosses_new_game(void) {
    start_e2e(SLIPWAYS);
    play_until(356);
    assert(entities_by_tag(TAG_SLIPWAY) != NULL && entities_by_tag(TAG_SLIPWAY)->count == 1);
    play_until(371);
    assert(entities_by_tag(TAG_SLIPWAY) == NULL || entities_by_tag(TAG_SLIPWAY)->count == 0);
    assert(ui_left_held(main_ui()) != NULL && "f371: a grab in the new game");
    play_until(377);
}

#define WORMHOLES "tests/fixtures/wormholes.json"

/* WORMHOLES: f0-3 of slipway_build (probe at f2); then 7-frame cycles at
 * (96,51): four frames held from the press, three released. Twenty refused
 * wormholes (f4-143), one held sixteen frames (f144-159) and opened by the
 * f160 release, freed at f188; one refused press (f193-199); 'restart' and a
 * new game (click at f210); one probe at (71,46) (f216), one refused wormhole
 * (f218-224). A refusal is released on the 5th frame of its cycle and freed
 * by the flush of the 7th. */
static const wormhole_t *held_wormhole(void) {
    const entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == wormhole_class() && "a wormhole is held");
    return (const wormhole_t *)held;
}

static void held_wormhole_does_not_grow_game(void) {
    start_e2e(WORMHOLES);
    play_until(4);
    const wormhole_t *held = held_wormhole();
    const size_t f4 = game_zone_used();
    play_until(7);
    assert(held_wormhole() == held);
    printf("  held wormhole f4/f7: game = %lu/%lu\n", (unsigned long)f4, (unsigned long)game_zone_used());
    assert(game_zone_used() == f4 && "a held wormhole allocates every frame");
}

static void refused_wormholes_do_not_grow_game(void) {
    start_e2e(WORMHOLES);
    play_until(10);
    const size_t first_refusal = game_without_registries();
    play_until(143);
    const size_t twenty = game_without_registries();
    printf("  refused wormholes 1/20: game - registries = %lu/%lu\n", (unsigned long)first_refusal, (unsigned long)twenty);
    assert(first_refusal == twenty && "a refused wormhole leaves its arena in the generation");
}

static void open_wormhole_releases_arena_on_death(void) {
    start_e2e(WORMHOLES);
    play_until(143);
    const size_t before = game_without_registries();
    play_until(160);
    const entity_list_t *mz = entities_by_tag(TAG_WORMHOLE);
    assert(mz != NULL && mz->count == 1 && ((const wormhole_t *)mz->items[0])->header.state == ENTITY_STATE_UNFOLDING &&
           "f160: the wormhole is open");
    play_until(193);
    assert(entities_by_tag(TAG_WORMHOLE)->count == 1 && "f193: only the pressed wormhole lives");
    const size_t after = game_without_registries();
    printf("  open wormhole: game - registries = %lu -> %lu\n", (unsigned long)before, (unsigned long)after);
    assert(after == before && "an open wormhole keeps its arena after its death");
}

static void refused_wormhole_crosses_new_game(void) {
    start_e2e(WORMHOLES);
    play_until(218);
    held_wormhole();
    play_until(224);
}

#define PROBES "tests/fixtures/probes_refused.json"

/* PROBES: 'reasonable' click (f0-1); then 7-frame cycles over empty space,
 * outside the UI panel (ui_panel_box), at (21,96): four frames held from the
 * press, three released. Ten refused probes (f2-71); one pressed at (21,96)
 * at f72, dragged and released over p34 at (47,80) (f73-78); one refusal
 * (f79-85); one probe placed at (71,46) (f86-87); one wormhole held
 * (f88-94); 'restart' and a new game (click at f105); one refusal
 * (f109-115). A refusal is released on the 5th frame of its cycle and freed
 * by the flush of the 7th. */
static const probe_t *held_probe(void) {
    const entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == probe_class() && "a probe is held");
    return (const probe_t *)held;
}

static void refused_probes_do_not_grow_game(void) {
    start_e2e(PROBES);
    play_until(8);
    const size_t a = game_without_registries();
    play_until(71);
    const size_t ten = game_without_registries();
    printf("  refused probes 1/10: game - registries = %lu/%lu\n", (unsigned long)a, (unsigned long)ten);
    assert(a == ten && "a refused probe leaves its arena in the generation");
}

static void each_refused_probe_reuses_previous_arena(void) {
    start_e2e(PROBES);
    play_until(2);
    const probe_t *first = held_probe();
    for (int k = 1; k < 10; k++) {
        play_until(2 + 7 * k);
        assert(held_probe() == first && "the next probe reuses the refused one's arena, at the same address");
    }
    printf("  refused probes: ten probes at the same address\n");
}

/* f74: the probe held over p34 sets its targeted_by; p34 moves off screen
 * before the draw. Refused at f76, the probe is freed by the f78 flush, and
 * the f79 press reuses its arena. */
static void offscreen_planet_forgets_refused_probe(void) {
    static vec_t far;
    start_e2e(PROBES);
    play_until(73);
    dispatch_context_set_input(g_input);
    cheat_step(&g_cheat, g_input);
    cart_update60();
    const entity_t *held = &held_probe()->header;
    planet_t *p34 = planet_targeted_by(held);
    assert(p34 != NULL && "f74: the held probe sets a planet's targeted_by");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p34->pos = &far;
    assert(!entity_geom_is_on_screen(&p34->header) && "p34 is off screen");
    cart_draw();
    assert(entity_get(p34->targeted_by) == held && "off screen, the draw leaves targeted_by set");
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    while (g_frame <= 78) play_one_frame(1);
    play_one_frame(0);
    assert(ui_left_held(main_ui()) == held && "f79: the next probe reuses the arena, at the same address");
    assert(entity_get(p34->targeted_by) == NULL && "an off-screen planet's targeted_by points at a refused, recycled probe");
}

/* f90: the anchors of the held wormhole keep only placed probes; the placed
 * probe keeps its arena, and the wormhole takes another one. */
static void wormhole_targets_only_placed_probe(void) {
    start_e2e(PROBES);
    play_until(90);
    const wormhole_t *mz = held_wormhole();
    const probe_t *placed = NULL;
    for (int i = 0; i < mz->anchors->count; i++) {
        const entity_t *e = entity_id_list_at(mz->anchors, i);
        if (e->class != probe_class()) continue;
        assert(placed == NULL && e->state == ENTITY_STATE_SETTLED && "the anchors keep a single probe, a placed one");
        placed = (const probe_t *)e;
    }
    assert(placed != NULL && "f86: the placed probe is an anchor of the wormhole");
    assert(placed->grab_arena != NULL && placed->grab_arena != mz->anchors->zone && "the placed probe keeps its arena");
}

static void refused_probe_crosses_new_game(void) {
    start_e2e(PROBES);
    play_until(109);
    held_probe();
    play_until(115);
}

#define WORMHOLE_ABANDONED "tests/fixtures/wormhole_abandoned.json"

/* WORMHOLE_ABANDONED: f0-20 of slipway_build (wormhole opened at f20), then
 * f2-17 of pause_restart: 'restart' at f26 abandons the wormhole and flips
 * the game zone. */
static const wormhole_t *abandoned_wormhole(void) {
    start_e2e(WORMHOLE_ABANDONED);
    play_until(22);
    const entity_list_t *mz = entities_by_tag(TAG_WORMHOLE);
    assert(mz != NULL && mz->count == 1 && "f22: a wormhole lives");
    return (const wormhole_t *)mz->items[0];
}

static void abandoned_wormhole_rereads_during_click(void) {
    const wormhole_t *mz = abandoned_wormhole();
    play_until(26);
    assert(game_zone_pending() && "the f26 'restart' flips the game zone");
    g_spend = (unsigned char)mz->header.is_dead;
}

static void abandoned_wormhole_dies_with_its_game(void) {
    const wormhole_t *mz = abandoned_wormhole();
    play_until(27);
    g_spend = (unsigned char)mz->header.is_dead;
}

/* f11: title after 'restart'. Reloading gives back the slots of the live
 * entities, and the reset_world of cart_init does not read back the
 * registries cleared by cart_load. */
static void pause_restart_reloads_after_title(void) {
    reading_t fresh, title, reloads;
    start_e2e("pause_restart");
    take_reading(&fresh, BEFORE_F0);
    play_until(11);
    take_reading(&title, 11);
    print_reading("pause_restart", &title);
    cart_shutdown();
    cart_load();
    cart_init();
    cart_update60();
    cart_draw();
    take_reading(&reloads, 0);
    print_reading("pause_restart reload", &reloads);
    assert(same_counts(&fresh, &reloads) && "reloading must give back the slots of the live entities");
}

/* After the f13 release, the vanish roll and the type picker must point
 * into the new generation, or ASan stops at the first read. */
static void pause_restart_reveals_on_new_game_states(void) {
    start_e2e("pause_restart");
    play_until(13);
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    const planet_types_entry_t *ll = planet_types_lookup("vanishing");
    int revealed = 0;
    int from_type_picker = 0;
    for (int i = 0; jo != NULL && i < jo->count && revealed < 5; i++) {
        planet_t *p = (planet_t *)jo->items[i];
        if (p->header.state != ENTITY_STATE_UNKNOWN) continue;
        planet_roll_type(p);
        revealed++;
        from_type_picker += p->planet_type != NULL && p->planet_type != ll;
    }
    assert(revealed > 0 && "the new game has unknown planets");
    assert(from_type_picker > 0 && "no reveal read the type picker");
    printf("  pause_restart f13: %d planets revealed, %d through the type picker\n", revealed, from_type_picker);
}

/* f42-90: the year's message box kept open by its keep_open callback, with
 * no click. */
static void message_box_hold_idle_does_not_grow(void) {
    const int frames[] = {45, 90};
    reading_t r[2];
    replay_e2e("message_box_hold", frames, 2, r);
    assert(same_counts(&r[0], &r[1]) && r[0].particle == r[1].particle && "a slot kind takes a slot per tick");
    assert(r[0].game_zone == r[1].game_zone && "the game zone takes bytes every tick");
}

/* f44-45: the year's message box kept open, with no action. The boxes
 * remade every tick are rewritten in place: they keep their address. */
static void message_box_hold_rewrites_boxes_in_place(void) {
    start_e2e("message_box_hold");
    play_until(44);
    const camera_t *mo = main_camera_typed();
    const mouse_t *je = main_mouse_typed();
    const ui_t *ok = main_ui();
    assert(mo != NULL && je != NULL && ok != NULL);
    const void *before[5] = {mo->pos_before_shake, mo->velocity, mo->pos, je->pos, ok->mouse_world};
    size_t used_before = game_zone_used();
    play_until(45);
    assert(main_camera_typed() == mo && main_mouse_typed() == je && main_ui() == ok);
    const void *after[5] = {mo->pos_before_shake, mo->velocity, mo->pos, je->pos, ok->mouse_world};
    int stables = 0;
    for (int i = 0; i < 5; i++) stables += before[i] == after[i];
    printf("  message_box_hold f44-45: %d / 5 per-tick boxes at the same address\n", stables);
    fflush(stdout);
    assert(stables == 5 && "the camera's pos_before_shake, velocity and pos, the mouse's pos and main_ui's mouse_world must be rewritten in place");
    assert(game_zone_used() == used_before && "the game zone takes bytes every tick");
}

static int no_button(const p8_input_frame_t *f) {
    if (f->mouse_buttons != 0) return 0;
    for (int b = 0; b < 6; b++) {
        if (f->dpad[b]) return 0;
    }
    return 1;
}

static int idle_frame(const p8_input_t *input, int i) {
    const p8_input_frame_t *f = &input->frames[i];
    const p8_input_frame_t *before = &input->frames[i - (i > 0)];
    return i > 0 && no_button(f) && no_button(before) && f->mouse_x == before->mouse_x &&
           f->mouse_y == before->mouse_y;
}

/* A window: at least two frames with no button nor D-pad and a still
 * mouse, the frame before included. The generation at its second frame (the
 * first may still flush what the action before left) is the one at its last.
 * Without take_reading, whose flush would release the generation early. */
static int idle_windows_of_current_e2e(void) {
    start_e2e(g_e2e);
    int start = -1;
    size_t second = 0;
    int windows = 0;
    for (int i = 0; i < g_input->frame_count; i++) {
        int idle = idle_frame(g_input, i);
        play_until(i);
        size_t used = game_zone_used();
        if (idle && start < 0) start = i;
        if (idle && i == start + 1) second = used;
        int end = idle && (i + 1 == g_input->frame_count || !idle_frame(g_input, i + 1));
        if (end && i > start) {
            if (used != second) {
                fprintf(stderr, "  %s f%d-%d: game %lu -> %lu\n", g_e2e, start, i, (unsigned long)second,
                        (unsigned long)used);
                assert(0 && "the generation grows during an idle window");
            }
            windows++;
        }
        if (!idle || end) start = -1;
    }
    return windows;
}

/* Returns what the case returns, run in a child (below 255). */
static int count_in_child(int (*test)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        int n = test();
        fflush(stdout);
        _exit(n < 255 ? n : 255);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) < 255 && "an idle windows reading failed");
    return WEXITSTATUS(status);
}

/* Returns the number of idle windows of the e2e of tests/e2e/. */
static int idle_windows_on_all_e2e(void) {
    DIR *dir = opendir("tests/e2e");
    struct dirent *ent;
    int total = 0;
    assert(dir != NULL);
    while ((ent = readdir(dir)) != NULL) {
        size_t len = strlen(ent->d_name);
        if (len <= 5 || strcmp(ent->d_name + len - 5, ".json") != 0) continue;
        assert(len - 5 < sizeof g_e2e);
        memcpy(g_e2e, ent->d_name, len - 5);
        g_e2e[len - 5] = '\0';
        total += count_in_child(idle_windows_of_current_e2e);
    }
    closedir(dir);
    return total;
}

/* f25-94: p40 fades; its released row may lower the row slot count. */
static void empty_planet_fade_does_not_grow(void) {
    const int frames[] = {25, 94};
    reading_t r[2];
    replay_e2e("empty_planet_fade", frames, 2, r);
    assert(none_rises(&r[0], &r[1]) && "a slot kind rises while p40 fades");
}

/* f56: 'harvest', p27 becomes a burst. At f139 the burst still lives: the
 * replay goes on with no action (the mouse stays at (116,58), no click)
 * until it burns out. */
static void tech_geoharvesting_burst_releases_burst(void) {
    const int frames[] = {55, 56, 139};
    reading_t r[3];
    reading_t end;
    int frame;
    replay_e2e("tech_geoharvesting_burst", frames, 3, r);
    assert(r[0].burst == 0 && r[1].burst == 1 && "'harvest' makes a burst");
    assert(r[1].particle - r[0].particle >= 140 && "a burst is born with 140 particles");
    end = r[2];
    for (frame = 140; end.burst != 0 && frame < 160; frame++) {
        cart_update60();
        cart_draw();
        take_reading(&end, frame);
    }
    print_reading("tech_geoharvesting_burst", &end);
    assert(end.burst == 0 && "a burnt-out burst must give back its slot");
    assert(end.particles == r[0].particles && "a burnt-out burst must give back its list");
}

/* With no input, the help screen emits 5 particles per draw and each lives
 * 115 draws: past that, the count levels off. */
static void idle_title_screen_settles(void) {
    int ceiling[4] = {0, 0, 0, 0};
    load_e2e(NULL);
    for (int i = 0; i < 400; i++) {
        cart_update60();
        cart_draw();
        check_invariant("title", i);
        assert(particle_zone_live(PARTICLE_SLOT_HELP_LIST) == 1 && "the help screen keeps a single list");
        int p = particle_zone_live(PARTICLE_SLOT_PARTICLE);
        if (p > ceiling[i / 100]) ceiling[i / 100] = p;
    }
    printf("  title without input, particle max per 100-frame window: %d %d %d %d\n", ceiling[0], ceiling[1],
           ceiling[2], ceiling[3]);
    assert(ceiling[3] <= ceiling[2] && "the help screen particle count still rises past their lifetime");
}

/* Allocated buffer, NUL-terminated, that the caller frees. */
static char *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL);
    assert(fseek(f, 0, SEEK_END) == 0);
    long size = ftell(f);
    assert(size >= 0);
    rewind(f);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, f) == (size_t)size);
    buf[size] = '\0';
    fclose(f);
    *len = (size_t)size;
    return buf;
}

static int same_bytes(const char *a_path, const char *b_path) {
    size_t a_len, b_len;
    char *a = read_file(a_path, &a_len);
    char *b = read_file(b_path, &b_len);
    int same = a_len == b_len && memcmp(a, b, a_len) == 0;
    free(a);
    free(b);
    return same;
}

#define DRIVER "./tmp/replay"
#define FILL "tmp/test_live_slots_fill.txt"
#define CAPTURE_A "tmp/test_live_slots_a.bin"
#define CAPTURE_B "tmp/test_live_slots_b.bin"
#define DUMP_A "tmp/test_live_slots_a.txt"
#define DUMP_B "tmp/test_live_slots_b.txt"

/* REPLAY_FILL=<file>: at the end of the session, the replay writes there a
 * '<name> <live count>' line per UI slot kind then per particle slot kind,
 * the session maxima of the particle slot kinds (live count, bytes,
 * elements), then the used bytes of the game zone and their maximum, then
 * those of the load zone, leaving the capture, the dump and the exit status
 * alone. */
static void replay_writes_fill_at_session_end(void) {
    const int frames[] = {17};
    reading_t reading[1];
    replay_e2e("pause_restart", frames, 1, reading);

    remove(FILL);
    assert(system(DRIVER " tests/e2e/pause_restart.json " CAPTURE_A " " DUMP_A " > /dev/null") == 0);
    assert(system("REPLAY_FILL=" FILL " " DRIVER " tests/e2e/pause_restart.json " CAPTURE_B " " DUMP_B
                  " > /dev/null") == 0);
    assert(same_bytes(CAPTURE_A, CAPTURE_B) && "the fill file does not change the capture");
    assert(same_bytes(DUMP_A, DUMP_B) && "the fill file does not change the dump");

    char expected[1024] = "";
    char line[48];
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        snprintf(line, sizeof line, "%s %d\n", NAMES[s], reading[0].live[s]);
        strcat(expected, line);
    }
    static const char *const PARTICLE[PARTICLE_SLOT_COUNT] = {"particle", "particles", "particles_help_screen", "burst", "transit_dots"};
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "%s %d\n", PARTICLE[s],
                 particle_zone_live((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "max_live %s %d\n", PARTICLE[s],
                 particle_zone_max_live((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "max_bytes %s %lu\n", PARTICLE[s],
                 (unsigned long)particle_zone_max_bytes((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        if (s == PARTICLE_SLOT_BURST) continue;
        snprintf(line, sizeof line, "max_elements %s %d\n", PARTICLE[s],
                 particle_zone_max_elements((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    snprintf(line, sizeof line, "game %lu\nmax_bytes game %lu\n", (unsigned long)game_zone_used(),
             (unsigned long)game_zone_max());
    strcat(expected, line);
    snprintf(line, sizeof line, "load %lu\n", (unsigned long)load_zone_used());
    strcat(expected, line);
    snprintf(line, sizeof line, "max_bytes load %lu\n", (unsigned long)game_zone_load_zone_max());
    strcat(expected, line);
    size_t len;
    char *writes = read_file(FILL, &len);
    printf("  fill file written by the replay:\n%s", writes);
    assert(strcmp(writes, expected) == 0 && "the replay writes the counts of the last frame, after a flush");
    free(writes);
    remove(FILL);
    remove(CAPTURE_A);
    remove(CAPTURE_B);
    remove(DUMP_A);
    remove(DUMP_B);
}

/* The value of the '<key> <count>' line of the fill file. */
static long fill_value(const char *fill, const char *key) {
    char pattern[64];
    long n;
    snprintf(pattern, sizeof pattern, "\n%s ", key);
    const char *l = strstr(fill, pattern);
    assert(l != NULL && "key missing from the fill file");
    assert(sscanf(l + strlen(pattern), "%ld", &n) == 1);
    return n;
}

/* The f56 burst dies at f141, after the end of the replay (f139), but its
 * 140 particles are born at once and most are dead by f139: a maximum read
 * only at the end of the session would stay under 140. */
static void replay_writes_session_peak(void) {
    remove(FILL);
    assert(system("REPLAY_FILL=" FILL " " DRIVER " tests/e2e/tech_geoharvesting_burst.json " CAPTURE_A
                  " " DUMP_A " > /dev/null") == 0);
    size_t len;
    char *writes = read_file(FILL, &len);
    long particle = fill_value(writes, "particle");
    long max_particle = fill_value(writes, "max_live particle");
    printf("  tech_geoharvesting_burst: particle %ld at the end of the session, max %ld; burst %ld, max %ld\n", particle,
           max_particle, fill_value(writes, "burst"), fill_value(writes, "max_live burst"));
    assert(fill_value(writes, "max_live burst") >= 1 && "'harvest' makes a burst");
    assert(max_particle >= 140 && particle < 140 && "the maximum is read at every take, not at the end of the session");
    free(writes);
    remove(FILL);
    remove(CAPTURE_A);
    remove(DUMP_A);
}

int main(void) {
    int e2e_count = invariant_on_all_e2e();
    printf("  invariant particle = sum of the registered #particles, transit_dots = sum of the #transit_dots of entities_by_tag(slipway): %d e2e, every frame\n", e2e_count);
    assert(e2e_count >= 26 && "tests/e2e/ lost some of its 26 mouse replays");
    in_child(pause_restart_returns_to_title);
    in_child(pause_restart_keeps_title_during_click);
    in_child_dying_by_asan(pause_restart_releases_title_next_frame);
    in_child(pause_restart_keeps_game_during_restart_click);
    in_child_dying_by_asan(pause_restart_releases_game_after_restart);
    in_child(pause_restart_title_drops_quit_economy_and_panel);
    in_child(pause_restart_keeps_quit_planets_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_planets);
    in_child(pause_restart_keeps_quit_economy_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_economy);
    in_child(pause_restart_keeps_quit_icons_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_icons);
    in_child(pause_restart_keeps_quit_singletons_during_click);
    in_child(pause_restart_keeps_title_singletons_during_click);
    for (g_singleton = 0; g_singleton < SINGLETON_COUNT; g_singleton++) {
        printf("  singleton %s:", NAMES_OF_SINGLETONS[g_singleton]);
        in_child_dying_by_asan(pause_restart_releases_singleton);
    }
    in_child(pause_restart_counts_populate_galaxy_states);
    in_child(pause_restart_leaves_load_zone_alone);
    in_child(pause_restart_reserves_registries_at_game_start_only);
    in_child(pause_restart_bounds_dead_registry_blocks);
    in_child(pause_restart_reloads_after_title);
    in_child(pause_restart_reveals_on_new_game_states);
    in_child(game_end_final_score_rebuild_does_not_grow_game);
    in_child(trade_rich_mine_route_rebuild_does_not_grow_game);
    in_child(trade_rich_mine_kept_route_survives_frame);
    in_child(trade_rich_mine_rebuild_gives_back_frame);
    in_child(trade_rich_mine_releases_trails_on_reload);
    in_child(refused_grabs_do_not_grow_game);
    in_child(placed_grab_does_not_fit_dead_wormhole_arena);
    in_child(refused_grab_crosses_new_game);
    in_child(offscreen_planet_forgets_refused_grab);
    in_child(held_slipway_does_not_grow_game);
    in_child(refused_slipways_do_not_grow_game);
    in_child(placed_slipway_does_not_fit_dead_wormhole_arena);
    in_child(refused_slipway_crosses_new_game);
    in_child(offscreen_planet_forgets_refused_slipway);
    in_child(held_wormhole_does_not_grow_game);
    in_child(refused_wormholes_do_not_grow_game);
    in_child(open_wormhole_releases_arena_on_death);
    in_child(refused_wormhole_crosses_new_game);
    in_child(refused_probes_do_not_grow_game);
    in_child(each_refused_probe_reuses_previous_arena);
    in_child(offscreen_planet_forgets_refused_probe);
    in_child(wormhole_targets_only_placed_probe);
    in_child(refused_probe_crosses_new_game);
    in_child(abandoned_wormhole_rereads_during_click);
    in_child_dying_by_asan(abandoned_wormhole_dies_with_its_game);
    in_child(message_box_hold_idle_does_not_grow);
    int windows = idle_windows_on_all_e2e();
    printf("  generation constant over %d idle windows of the 26 e2e\n", windows);
    assert(windows > 0);
    in_child(message_box_hold_rewrites_boxes_in_place);
    in_child(empty_planet_fade_does_not_grow);
    in_child(tech_geoharvesting_burst_releases_burst);
    in_child(idle_title_screen_settles);
    in_child(replay_writes_fill_at_session_end);
    in_child(replay_writes_session_peak);
    printf("test_live_slots: all tests passed\n");
    return 0;
}
