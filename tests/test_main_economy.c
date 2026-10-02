/* The provisional `fm` that pico8_probe_init() leaves until `eu()` replaces
 * it with a `hy`, read through pico8_fm_placeholder() and pico8_fm_hy(): the
 * cheat code typed on the title screen, a new init, the init/shutdown cycle,
 * the replacement. The frozen values were taken on the code where the
 * provisional was still a class-less instance and must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/cart.h"
#include "game/cheat.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/probe.h"
#include "game/score_panel.h"
#include "game/tech_gate.h"
#include "engine/entity_class.h"

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static void assert_at_rest(void) {
    const pico8_fm_placeholder_t *fm = pico8_fm_placeholder();

    assert(fm != NULL && (const void *)fm == (const void *)pico8_fm());
    assert(!fm->has_jk && !fm->has_nz);
}

/* A A B B A A B B, one press every other frame; returns how often it fired. */
static int type_cheat(void) {
    static const int presses[] = {4, 4, 5, 5, 4, 4, 5, 5};
    p8_input_frame_t frames[16];
    p8_input_t pad;
    pico8_cheat_t cheat;
    int fired = 0;
    int i;

    memset(frames, 0, sizeof frames);
    for (i = 0; i < 8; i++) {
        frames[2 * i].dpad[presses[i]] = 1;
    }
    pad.frames = frames;
    pad.frame_count = 16;
    pico8_cheat_init(&cheat);
    for (pad.current_frame = 0; pad.current_frame < 16; pad.current_frame++) {
        fired += pico8_cheat_step(&cheat, &pad);
    }
    return fired;
}

static void assert_cheated(void) {
    const pico8_fm_placeholder_t *fm = pico8_fm_placeholder();

    assert(fm != NULL);
    assert(fm->has_jk && fm->jk == p8_fixed_from_int(99));
    assert(fm->has_nz && fm->nz == p8_fixed_from_int(200));
}

static void new_game(void) {
    pico8_eu(NULL);
}

static void test_at_rest(void) {
    pico8__init();
    assert_at_rest();
}

static int cb_affordable(const technologies_entry_t *t) {
    (void)pico8_cb(1);
    return pico8_nb_e_(t);
}

static void test_cheat_on_title(void) {
    void *provisional;
    const technologies_entry_t *t = technologies_table;

    pico8__init();
    provisional = pico8_fm();
    assert(pico8_fm_hy() == NULL && (const void *)pico8_fm_placeholder() == provisional);
    while (pico8_gi_nb(t) <= p8_fixed_from_int(0) || pico8_gi_nb(t) > p8_fixed_from_int(99)) {
        t++;
        assert(t < technologies_table + TECHNOLOGIES_COUNT);
    }
    assert(cb_affordable(t) == 0);
    pico8_pj_hb();
    assert(pico8_pj_ou()->sub_items[2][0].is_ref && strcmp(pico8_pj_ou()->sub_items[0][0].text, "0$") == 0);
    assert(type_cheat() == 1 && pico8_fm() == provisional);
    assert(cb_affordable(t) == 1);
    pico8_pj_hb();
    assert(!pico8_pj_ou()->sub_items[2][0].is_ref && strcmp(pico8_pj_ou()->sub_items[0][0].text, "200$") == 0);
    assert_cheated();
    assert(type_cheat() == 1 && pico8_fm() == provisional);
    assert_cheated();
}

static void test_new_init(void) {
    pico8__init();
    assert(type_cheat() == 1);
    pico8_probe_init();
    assert_at_rest();
}

static void test_cycle(void) {
    pico8__init();
    pico8_probe_shutdown();
    assert(pico8_fm() == NULL && pico8_fm_hy() == NULL);
    assert(type_cheat() == 1);
    assert(pico8_fm() == NULL && pico8_fm_hy() == NULL);
    pico8_probe_shutdown();
    assert(pico8_fm() == NULL);
    pico8_probe_init();
    assert_at_rest();
    pico8_probe_init();
    pico8_probe_init();
    assert_at_rest();
}

static void test_replacement(void) {
    pico8__init();
    assert(type_cheat() == 1);
    new_game();
    assert(pico8_fm_placeholder() == NULL && (void *)pico8_fm_hy() == (void *)pico8_fm());
    assert(pico8_fm_hy()->jk == p8_fixed_from_int(0));
    assert(pico8_fm_hy()->nz == p8_fixed_from_int(100));
    pico8_probe_init();
    assert_at_rest();
}

/* The provisional is one static struct whose descriptor has no method;
 * `eu()` puts a `hy` in its place. */
static void test_fm_is_a_struct(void) {
    const entity_class_t *cls = pico8_fm_placeholder_struct_class();
    void *provisional;
    int j;

    pico8__init();
    provisional = pico8_fm();
    assert(provisional != NULL);
    assert(((const entity_t *)provisional)->class == cls);
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL);
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }

    pico8_probe_init();
    assert(pico8_fm() == provisional);

    new_game();
    assert(((const entity_t *)pico8_fm())->class == pico8_hy_struct_class());
}

/* `cb` keeps a tech of the current tier until it is invented, even with `bi`. */
static void test_cb_drops_invented(void) {
    pico8_cb_kept_t kept;
    const technologies_entry_t *t;
    p8_fixed_t ew;
    int i;

    pico8__init();
    new_game();
    kept = pico8_cb(1);
    assert(kept.count > 0);
    t = kept.items[0];
    ew = pico8_fm_hy()->ew;
    pico8_hy_kt(pico8_fm_hy(), t);
    assert(pico8_fm_hy()->ew == ew);
    kept = pico8_cb(1);
    for (i = 0; i < kept.count; i++) {
        assert(kept.items[i] != t);
    }
}

/* Each scenario runs in a child forked right after pico8_cart_load(), as a
 * fresh process would start it. */
static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_at_rest);
    in_fresh_child(test_cheat_on_title);
    in_fresh_child(test_new_init);
    in_fresh_child(test_cycle);
    in_fresh_child(test_replacement);
    in_fresh_child(test_fm_is_a_struct);
    in_fresh_child(test_cb_drops_invented);
    printf("test_pico8_fm: ok\n");
    return 0;
}
