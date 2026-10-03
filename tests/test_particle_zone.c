/* Linked under ASan. Each case runs in a child process: this binary runs
 * itself again with the case name, and the parent reads its status and its
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/probe.h"
#include "engine/pointer_list.h"
#include "game/slipway_display.h"
#include "engine/allocator.h"
#include "game/particle_zone.h"

#define STDERR_LOG "tmp/test_particle_zone_stderr.txt"

/* volatile: the read through a stale pointer must not be removed by the
 * optimizer, it is what the case observes. */
static volatile unsigned char sink;

static particle_t *particle(void) {
    particle_t *p = (particle_t *)allocator_alloc(particle_zone(PARTICLE_SLOT_PARTICLE), sizeof *p);
    memset(p, 0, sizeof *p);
    return p;
}

static void release_particle(particle_t *p) {
    allocator_release(particle_zone(PARTICLE_SLOT_PARTICLE), (unsigned char *)p);
}

/* A particle list of the kind, in its slot, reserved to `capacity`. */
static pointer_list_t *list(particle_slot_kind_t slot_kind, int capacity) {
    const allocator_t *slot = particle_zone_slot(slot_kind);
    pointer_list_t *qy = (pointer_list_t *)allocator_alloc(slot, sizeof *qy);
    pointer_list_init_reserve(qy, slot, capacity);
    return qy;
}

static int case_live_slots_follow_takes(void) {
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++)
        assert(particle_zone_live((particle_slot_kind_t)s) == 0);

    particle_t *a = particle();
    particle_t *b = particle();
    const allocator_t *qy = particle_zone_slot(PARTICLE_SLOT_LIST);
    const allocator_t *fv = particle_zone_slot(PARTICLE_SLOT_HELP_LIST);
    const allocator_t *ga = particle_zone_slot(PARTICLE_SLOT_BURST);
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 2);
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 1);
    assert(particle_zone_live(PARTICLE_SLOT_HELP_LIST) == 1);
    assert(particle_zone_live(PARTICLE_SLOT_BURST) == 1);

    release_particle(a);
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 1 && "giving back a slot counts its kind down");
    release_particle(b);
    arena_release(qy);
    arena_release(fv);
    arena_release(ga);
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++)
        assert(particle_zone_live((particle_slot_kind_t)s) == 0);
    return 0;
}

static int case_particle_dead_after_release(void) {
    particle_t *p = particle();
    release_particle(p);
    sink = ((volatile unsigned char *)p)[0];
    return 0;
}

static int case_burst_zone_full(void) {
    for (int i = 0; i < 9; i++) particle_zone_slot(PARTICLE_SLOT_BURST);
    return 0;
}

static int case_particle_zone_full(void) {
    for (int i = 0; i < 100000; i++) particle();
    return 0;
}

static int case_particles_holds_its_capacity(void) {
    pointer_list_t *qy = list(PARTICLE_SLOT_LIST, PARTICLES_CAPACITY);
    for (int i = 0; i < PARTICLES_CAPACITY; i++) pointer_list_add(qy, NULL);
    arena_release(qy->zone);
    return 0;
}

static int case_particles_past_capacity(void) {
    pointer_list_t *qy = list(PARTICLE_SLOT_LIST, PARTICLES_CAPACITY);
    for (int i = 0; i <= PARTICLES_CAPACITY; i++) pointer_list_add(qy, NULL);
    return 0;
}

static int case_particles_help_screen_holds_its_capacity(void) {
    pointer_list_t *qy = list(PARTICLE_SLOT_HELP_LIST, PARTICLES_HELP_SCREEN_CAPACITY);
    for (int i = 0; i < PARTICLES_HELP_SCREEN_CAPACITY; i++) pointer_list_add(qy, NULL);
    arena_release(qy->zone);
    return 0;
}

static int case_particles_help_screen_past_capacity(void) {
    pointer_list_t *qy = list(PARTICLE_SLOT_HELP_LIST, PARTICLES_HELP_SCREEN_CAPACITY);
    for (int i = 0; i <= PARTICLES_HELP_SCREEN_CAPACITY; i++) pointer_list_add(qy, NULL);
    return 0;
}

static int case_maxima_keep_the_peak(void) {
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        assert(particle_zone_max_live((particle_slot_kind_t)s) == 0);
        assert(particle_zone_max_bytes((particle_slot_kind_t)s) == 0);
    }
    assert(particle_zone_max_elements(PARTICLE_SLOT_LIST) == 0);

    particle_t *p[3] = {particle(), particle(), particle()};
    for (int i = 0; i < 3; i++) release_particle(p[i]);
    particle_t *q = particle();
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 1);
    assert(particle_zone_max_live(PARTICLE_SLOT_PARTICLE) == 3 &&
           "the maximum is taken at every take, not at reading");
    release_particle(q);

    const allocator_t *released = particle_zone_slot(PARTICLE_SLOT_BURST);
    allocator_alloc(released, 200);
    size_t released_used = arena_used(released);
    arena_release(released);
    assert(particle_zone_max_bytes(PARTICLE_SLOT_BURST) == released_used &&
           "a given-back slot keeps what it took");

    const allocator_t *live = particle_zone_slot(PARTICLE_SLOT_BURST);
    allocator_alloc(live, 400);
    assert(particle_zone_max_bytes(PARTICLE_SLOT_BURST) == arena_used(live) &&
           "a slot still live counts at reading");
    assert(particle_zone_max_bytes(PARTICLE_SLOT_LIST) == 0);

    particle_zone_record_elements(PARTICLE_SLOT_LIST, 7);
    particle_zone_record_elements(PARTICLE_SLOT_LIST, 3);
    particle_zone_record_elements(PARTICLE_SLOT_HELP_LIST, 12);
    assert(particle_zone_max_elements(PARTICLE_SLOT_LIST) == 7);
    assert(particle_zone_max_elements(PARTICLE_SLOT_HELP_LIST) == 12);
    return 0;
}

static int case_particles_release_gives_back_particles_then_list(void) {
    emitter_t emitter;
    memset(&emitter, 0, sizeof emitter);
    emitter.particles = list(PARTICLE_SLOT_LIST, PARTICLES_CAPACITY);
    pointer_list_add(emitter.particles, particle());
    pointer_list_add(emitter.particles, NULL);
    pointer_list_add(emitter.particles, particle());
    particle_t *kept = particle();
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 3);

    emitter_release_particles(&emitter);

    assert(emitter.particles == NULL && "the host no longer points at the given-back list");
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 1 &&
           "the list's particles are given back, not the others");
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 0);
    release_particle(kept);

    emitter_release_particles(&emitter);
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 0 && "a host without a list has nothing to give back");
    return 0;
}

static int case_particles_release_releases_the_arena(void) {
    emitter_t emitter;
    memset(&emitter, 0, sizeof emitter);
    emitter.particles = list(PARTICLE_SLOT_LIST, PARTICLES_CAPACITY);
    const pointer_list_t *qy = emitter.particles;
    emitter_release_particles(&emitter);
    sink = ((const volatile unsigned char *)qy)[0];
    return 0;
}

/* An emitter host cut down to what emitter_init and emitter_update read:
 * layer 10, so drawn without the on-screen test, and spawn_rate 0, so
 * nothing is born. */
static const emitter_archetype_t g_without_birth;
static const entity_class_t g_host_cls;
static const p8_fixed_t g_layer_ten = 10 << 16;
static const vec_t *const g_pos_nil = NULL;
static emitter_t g_emitter;
static entity_t g_host;
static p8_graphics_t g_gfx;
static p8_memory_t g_mem;

static entity_t *host(void) {
    p8_gfx_init(&g_gfx);
    p8_mem_init(&g_mem);
    entity_init_header(&g_host, &g_host_cls);
    memset(&g_emitter, 0, sizeof g_emitter);
    g_emitter.archetype = &g_without_birth;
    g_host.layer = &g_layer_ten;
    g_host.pos = &g_pos_nil;
    g_host.emitter = &g_emitter;
    emitter_init(&g_host);
    return &g_host;
}

/* A particle that dies at the next emitter_update: life 1.1, minus 0.14. */
static particle_t *dying(entity_t *h) {
    particle_t *p = particle_take();
    memset(p, 0, sizeof *p);
    p->life = (1 << 16) + 6554;
    pointer_list_add(h->emitter->particles, p);
    return p;
}

static int case_emitter_init_takes_a_particles_list(void) {
    entity_t *h = host();
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 1);
    assert(h->emitter->particles != NULL && h->emitter->particles->count == 0);
    emitter_release_particles(h->emitter);
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 0);
    return 0;
}

static int case_emitter_init_help_screen_takes_a_help_list(void) {
    entity_t *h = host();
    emitter_release_particles(h->emitter);
    emitter_init_in(h, PARTICLE_SLOT_HELP_LIST);
    assert(particle_zone_live(PARTICLE_SLOT_HELP_LIST) == 1);
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 0);
    return 0;
}

static int case_second_emitter_init_gives_back_the_old_list(void) {
    entity_t *h = host();
    dying(h);
    dying(h);
    emitter_init(h);
    assert(particle_zone_live(PARTICLE_SLOT_LIST) == 1 && "the old list is given back");
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 0 && "and its particles with it");
    assert(h->emitter->particles->count == 0);
    return 0;
}

static int case_emitter_update_gives_back_a_dying_particle(void) {
    entity_t *h = host();
    dying(h);
    particle_t *surviving = dying(h);
    surviving->life = 5 << 16;
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 2);
    emitter_update(&g_gfx, &g_mem, h);
    assert(h->emitter->particles->count == 1);
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 1 && "emitter_update gives back the particle it removes");
    return 0;
}

static int case_particle_dead_after_emitter_update(void) {
    entity_t *h = host();
    particle_t *p = dying(h);
    emitter_update(&g_gfx, &g_mem, h);
    sink = ((volatile unsigned char *)p)[0];
    return 0;
}

static int case_emitter_spawn_notes_the_list_length(void) {
    static const entity_class_t cls = {.spawn_particle = probe_spawn_particle};
    static const emitter_archetype_t archetype = {.lifetime = 9 << 16, .spawn_rate = 1 << 16};
    entity_t *h = host();
    g_host.class = &cls;
    g_emitter.archetype = &archetype;
    g_emitter.angle_slices = 1 << 16;
    emitter_spawn(h, 3 << 16);
    assert(h->emitter->particles->count == 3);
    assert(particle_zone_live(PARTICLE_SLOT_PARTICLE) == 3 && "probe_spawn_particle takes from the particle handle");
    assert(particle_zone_max_elements(PARTICLE_SLOT_LIST) == 3);
    return 0;
}

static transit_dot_t *trail(void) {
    return (transit_dot_t *)allocator_alloc(particle_zone(PARTICLE_SLOT_TRANSIT_DOT), sizeof(transit_dot_t));
}

static void release_trail(transit_dot_t *t) {
    allocator_release(particle_zone(PARTICLE_SLOT_TRANSIT_DOT), (unsigned char *)t);
}

static int case_transit_dots_takes_and_gives_back(void) {
    transit_dot_t *a = trail();
    transit_dot_t *b = trail();
    assert(particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT) == 2);
    release_trail(a);
    assert(particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT) == 1 && "giving back a transit dot counts its kind down");
    release_trail(b);
    assert(particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT) == 0);
    assert(particle_zone_max_live(PARTICLE_SLOT_TRANSIT_DOT) == 2);
    assert(particle_zone_max_bytes(PARTICLE_SLOT_TRANSIT_DOT) == 0 && "a transit dot is a bare slot");
    particle_zone_record_elements(PARTICLE_SLOT_TRANSIT_DOT, 5);
    particle_zone_record_elements(PARTICLE_SLOT_TRANSIT_DOT, 2);
    assert(particle_zone_max_elements(PARTICLE_SLOT_TRANSIT_DOT) == 5);
    return 0;
}

static int case_transit_dots_dead_after_release(void) {
    transit_dot_t *t = trail();
    release_trail(t);
    sink = ((volatile unsigned char *)t)[0];
    return 0;
}

static int case_transit_dots_zone_full(void) {
    for (int i = 0; i < 681; i++) trail();
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"live_slots_follow_takes", case_live_slots_follow_takes},
    {"particle_dead_after_release", case_particle_dead_after_release},
    {"burst_zone_full", case_burst_zone_full},
    {"particle_zone_full", case_particle_zone_full},
    {"particles_holds_its_capacity", case_particles_holds_its_capacity},
    {"particles_past_capacity", case_particles_past_capacity},
    {"particles_help_screen_holds_its_capacity", case_particles_help_screen_holds_its_capacity},
    {"particles_help_screen_past_capacity", case_particles_help_screen_past_capacity},
    {"maxima_keep_the_peak", case_maxima_keep_the_peak},
    {"particles_release_gives_back_particles_then_list", case_particles_release_gives_back_particles_then_list},
    {"particles_release_releases_the_arena", case_particles_release_releases_the_arena},
    {"emitter_init_takes_a_particles_list", case_emitter_init_takes_a_particles_list},
    {"emitter_init_help_screen_takes_a_help_list", case_emitter_init_help_screen_takes_a_help_list},
    {"second_emitter_init_gives_back_the_old_list", case_second_emitter_init_gives_back_the_old_list},
    {"emitter_update_gives_back_a_dying_particle", case_emitter_update_gives_back_a_dying_particle},
    {"particle_dead_after_emitter_update", case_particle_dead_after_emitter_update},
    {"emitter_spawn_notes_the_list_length", case_emitter_spawn_notes_the_list_length},
    {"transit_dots_takes_and_gives_back", case_transit_dots_takes_and_gives_back},
    {"transit_dots_dead_after_release", case_transit_dots_dead_after_release},
    {"transit_dots_zone_full", case_transit_dots_zone_full},
};

#define CASE_COUNT (sizeof CASES / sizeof CASES[0])

/* An allocated buffer, ending with a nul, which the caller frees. */
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
 * which the caller frees. */
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
        fprintf(stderr, "%s: expected clean, exit %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "the case must neither crash nor write to stderr");
    }
    free(err);
}

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: expected use-after-poison, exit %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "a slot given back to its zone then read must crash under ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: expected exit 3 and \"%s\", exit %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "a full zone or a too small slot must call p8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_particle_zone: unknown case %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("live_slots_follow_takes");
    expect_poison_crash("particle_dead_after_release");
    expect_fatal("burst_zone_full", "zone particles burst full, limit 8 slots");
    expect_fatal("particle_zone_full", "zone particles particle full, limit 2085 slots");
    expect_clean("particles_holds_its_capacity");
    expect_fatal("particles_past_capacity", "zone slot particles full, limit ");
    expect_clean("particles_help_screen_holds_its_capacity");
    expect_fatal("particles_help_screen_past_capacity", "zone slot particles_help_screen full, limit ");
    expect_clean("maxima_keep_the_peak");
    expect_clean("particles_release_gives_back_particles_then_list");
    expect_poison_crash("particles_release_releases_the_arena");
    expect_clean("emitter_init_takes_a_particles_list");
    expect_clean("emitter_init_help_screen_takes_a_help_list");
    expect_clean("second_emitter_init_gives_back_the_old_list");
    expect_clean("emitter_update_gives_back_a_dying_particle");
    expect_poison_crash("particle_dead_after_emitter_update");
    expect_clean("emitter_spawn_notes_the_list_length");
    expect_clean("transit_dots_takes_and_gives_back");
    expect_poison_crash("transit_dots_dead_after_release");
    expect_fatal("transit_dots_zone_full", "zone particles transit_dots full, limit 680 slots");
    remove(STDERR_LOG);
    printf("test_particle_zone: all tests passed\n");
    return 0;
}
