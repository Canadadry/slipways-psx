/* The RNG sync probe: is saving rnd_state enough? tests/e2e/pad_colonise.json
 * is replayed from boot to a save frame, then forked: run A (the parent)
 * goes on as is, run B (the child) first drops the state a save never holds
 * (save_check_drop_unsaved_state). Both play the same following frames and
 * rnd_state, the game stream, is compared after each one. The draws the
 * drop could disturb (particles, bursts, transit dots) are on the visual
 * stream, so run A's visual stream must move over the window. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "devtools/save_check.h"
#include "p8/assets.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"
#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/pad_panel.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/visual_random.h"

#define RECORDING "tests/e2e/pad_colonise.json"
#define WINDOW_CAP 128

typedef struct {
    const char *name;
    int save_frame;
    int last_frame;
    int harvest_p25;
} probe_case_t;

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;
static input_step_t step;

static void play_frame(int frame) {
    input_step_run(&step, &input.frames[input.current_frame]);
    cart_update60();
    cart_draw();
    if (frame < input.frame_count - 1) {
        p8_input_step(&input);
    }
}

static int is_burst(const entity_t *e) {
    return e->class == burst_class();
}

/* The first burst of the live entities, dead or not; NULL when none. */
static entity_t *first_burst(void) {
    const entity_list_t *live = live_entities();
    for (int i = 0; i < live->count; i++) {
        if (is_burst(live->items[i])) {
            return live->items[i];
        }
    }
    return NULL;
}

static int live_burst_count(void) {
    const entity_list_t *live = live_entities();
    int n = 0;
    for (int i = 0; i < live->count; i++) {
        entity_t *e = live->items[i];
        n += is_burst(e) && !e->is_dead;
    }
    return n;
}

static planet_t *planet_named(const char *id) {
    const entity_list_t *live = live_entities();
    for (int i = 0; i < live->count; i++) {
        entity_t *e = live->items[i];
        if (planet_is_family(e) && !is_burst(e)) {
            const char *own = planet_id_or_null((planet_t *)e);
            if (own != NULL && strcmp(own, id) == 0) {
                return (planet_t *)e;
            }
        }
    }
    return NULL;
}

static research_project_t *harvest_recipe(void) {
    research_project_t *found = NULL;
    for (int i = 1; i <= research_projects_count(); i++) {
        research_project_t *project = research_projects_at(i);
        if (project->class_name != NULL && strcmp(project->class_name, "burst") == 0) {
            found = project;
        }
    }
    assert(found != NULL && "the harvest is the research project whose class name is \"burst\"");
    return found;
}

static const probe_t *the_probe(void) {
    const entity_list_t *live = live_entities();
    for (int i = 0; i < live->count; i++) {
        entity_t *e = live->items[i];
        if (e->class->lineage & LINEAGE(LINEAGE_PROBE)) {
            return (const probe_t *)e;
        }
    }
    return NULL;
}

/* Plan section 2: a save is only written with no wormhole, nothing being
 * placed and no box or menu open. */
static void assert_save_frame_conditions(void) {
    const entity_list_t *live = live_entities();

    assert(entity_registry_count_tagged(TAG_WORMHOLE) == 0 && "save frame: a wormhole is live");
    for (int i = 0; i < live->count; i++) {
        assert(((entity_t *)live->items[i])->state != ENTITY_STATE_PLACING && "save frame: an entity is being placed");
    }
    assert(entity_registry_count_tagged(TAG_MESSAGE_BOX) == 0 && shown_message_box() == NULL && "save frame: a message box is open");
    assert(!pad_panel_box_open() && !pad_panel_open() && "save frame: a panel or menu is open");
    assert(!pause_menu_shown() && "save frame: the pause menu is open");
}

static void apply_harvest_to_p25(void) {
    planet_t *p25 = planet_named("p25");

    assert(p25 != NULL);
    assert(p25->header.state == ENTITY_STATE_KNOWN && "p25 is known at f98");
    assert(!planet_is_colonized(p25) && p25->slipways.count == 0);
    assert(entity_geom_is_on_screen(&p25->header));
    assert(first_burst() == NULL);
    planet_apply_project(p25, harvest_recipe());
}

static int window_length(const probe_case_t *c) {
    return c->last_frame - c->save_frame;
}

static void run_window(const probe_case_t *c, uint32_t *states, uint32_t *visual_states) {
    for (int frame = c->save_frame + 1; frame <= c->last_frame; frame++) {
        play_frame(frame);
        states[frame - c->save_frame - 1] = p8_fixed_rnd_state();
        visual_states[frame - c->save_frame - 1] = visual_random_state();
    }
}

static void read_all(int fd, void *buffer, size_t size) {
    size_t done = 0;
    while (done < size) {
        ssize_t got = read(fd, (char *)buffer + done, size - done);
        assert(got > 0 && "run B stopped before sending every state");
        done += (size_t)got;
    }
}

static void write_all(int fd, const void *buffer, size_t size) {
    size_t done = 0;
    while (done < size) {
        ssize_t put = write(fd, (const char *)buffer + done, size - done);
        assert(put > 0);
        done += (size_t)put;
    }
}

/* Returns 0 when run A and run B agree after every window frame. */
static int run_case(const probe_case_t *c) {
    uint32_t states_a[WINDOW_CAP];
    uint32_t states_b[WINDOW_CAP];
    uint32_t visual_a[WINDOW_CAP];
    uint32_t visual_b[WINDOW_CAP];
    int n = window_length(c);
    uint32_t at_save;
    uint32_t visual_at_save;
    int fds[2];
    pid_t run_b;
    int status;

    assert(n > 0 && n <= WINDOW_CAP && c->last_frame < input.frame_count);
    cart_init();
    input_step_init(&step);
    for (int frame = 0; frame <= c->save_frame; frame++) {
        play_frame(frame);
    }
    if (c->harvest_p25) {
        apply_harvest_to_p25();
        assert(first_burst() != NULL && !first_burst()->is_dead && "burst case: a live burst at the save frame");
        assert(entity_geom_is_on_screen(first_burst()) && "burst case: the burst is on screen at the save frame");
    } else {
        assert(first_burst() == NULL && "quiet case: no burst at the save frame");
        assert(entity_registry_count_tagged(TAG_SLIPWAY) >= 1 && "quiet case: a slipway at the save frame");
        assert(the_probe() != NULL && (the_probe()->emitter.has & EMITTER_HAS_PARTICLES) &&
               the_probe()->emitter.particles.count > 0 && "quiet case: the probe's particles are live");
    }
    assert_save_frame_conditions();
    at_save = p8_fixed_rnd_state();
    visual_at_save = visual_random_state();

    assert(pipe(fds) == 0);
    fflush(stdout);
    run_b = fork();
    assert(run_b >= 0);
    if (run_b == 0) {
        close(fds[0]);
        save_check_drop_unsaved_state();
        run_window(c, states_b, visual_b);
        write_all(fds[1], states_b, (size_t)n * sizeof states_b[0]);
        close(fds[1]);
        _exit(0);
    }
    close(fds[1]);
    run_window(c, states_a, visual_a);
    read_all(fds[0], states_b, (size_t)n * sizeof states_b[0]);
    close(fds[0]);
    assert(waitpid(run_b, &status, 0) == run_b && WIFEXITED(status) && WEXITSTATUS(status) == 0);

    assert(visual_a[n - 1] != visual_at_save && "run A's visual stream never moved over the window");
    if (c->harvest_p25) {
        assert(live_burst_count() == 0 && "burst case: the burst outlives the window");
    }

    for (int i = 0; i < n; i++) {
        if (states_a[i] != states_b[i]) {
            printf("test_rng_sync: %s case diverges at f%d: run A rnd_state 0x%08X, run B 0x%08X (save frame f%d, 0x%08X)\n",
                   c->name, c->save_frame + 1 + i, (unsigned)states_a[i], (unsigned)states_b[i], c->save_frame, (unsigned)at_save);
            return 1;
        }
    }
    printf("test_rng_sync: %s case in sync over f%d-f%d (final rnd_state 0x%08X)\n", c->name, c->save_frame + 1, c->last_frame,
           (unsigned)states_a[n - 1]);
    return 0;
}

/* Each case in its own child forked right after cart_load(): both start
 * from the boot rnd_state. Returns the child's exit status. */
static int in_fresh_child(const probe_case_t *c) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        int result = run_case(c);
        fflush(stdout);
        _exit(result);
    }
    assert(waitpid(child, &status, 0) == child);
    if (!WIFEXITED(status)) {
        printf("test_rng_sync: %s case stopped by signal %d before any comparison\n", c->name, WIFSIGNALED(status) ? WTERMSIG(status) : -1);
        return 2;
    }
    return WEXITSTATUS(status);
}

int main(void) {
    const probe_case_t burst_case = {"burst", 98, 198, 1};
    const probe_case_t quiet_case = {"quiet", 288, 314, 0};
    int failed = 0;

    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    assert(p8_input_load(&input, RECORDING));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    failed |= in_fresh_child(&burst_case) != 0;
    failed |= in_fresh_child(&quiet_case) != 0;

    p8_input_free(&input);
    if (failed) {
        printf("test_rng_sync: FAILED, saving rnd_state is not enough\n");
        return 1;
    }
    printf("test_rng_sync: all tests passed\n");
    return 0;
}
