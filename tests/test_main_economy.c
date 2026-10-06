/* The provisional main economy that probe_init() leaves until
 * cart_start_game() replaces it with an economy, read through
 * main_economy_placeholder() and main_economy_typed(): the cheat code typed
 * on the title screen, a new init, the init/shutdown cycle, the
 * replacement. */

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
#include "p8/vram.h"
#include "game/probe.h"
#include "game/score_panel.h"
#include "game/tech_gate.h"
#include "engine/entity_class.h"

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static void assert_at_rest(void) {
    const main_economy_placeholder_t *fm = main_economy_placeholder();

    assert(fm != NULL && (const void *)fm == (const void *)main_economy());
    assert(!fm->has_research_points && !fm->has_credits);
}

/* × × ○ ○ × × ○ ○, one press every other frame; returns how often it fired. */
static int type_cheat(void) {
    static const int presses[] = {P8_PAD_CROSS, P8_PAD_CROSS, P8_PAD_CIRCLE, P8_PAD_CIRCLE,
                                  P8_PAD_CROSS, P8_PAD_CROSS, P8_PAD_CIRCLE, P8_PAD_CIRCLE};
    int frames[16];
    cheat_t cheat;
    int fired = 0;
    int i;

    memset(frames, 0, sizeof frames);
    for (i = 0; i < 8; i++) {
        frames[2 * i] = presses[i];
    }
    cheat_init(&cheat);
    for (i = 0; i < 16; i++) {
        fired += cheat_step(&cheat, frames[i]);
    }
    return fired;
}

static void assert_cheated(void) {
    const main_economy_placeholder_t *fm = main_economy_placeholder();

    assert(fm != NULL);
    assert(fm->has_research_points && fm->research_points == p8_fixed_from_int(99));
    assert(fm->has_credits && fm->credits == p8_fixed_from_int(200));
}

static void new_game(void) {
    cart_start_game(NULL);
}

static void test_at_rest(void) {
    cart_init();
    assert_at_rest();
}

static int available_technologies_affordable(const technologies_entry_t *t) {
    (void)tech_gate_available_technologies(1);
    return tech_gate_affordable_flags(t);
}

static void test_cheat_on_title(void) {
    void *provisional;
    const technologies_entry_t *t = technologies_table;

    cart_init();
    provisional = main_economy();
    assert(main_economy_typed() == NULL && (const void *)main_economy_placeholder() == provisional);
    while (tech_gate_research_cost(t) <= p8_fixed_from_int(0) || tech_gate_research_cost(t) > p8_fixed_from_int(99)) {
        t++;
        assert(t < technologies_table + TECHNOLOGIES_COUNT);
    }
    assert(available_technologies_affordable(t) == 0);
    score_panel_refresh();
    assert(score_panel_readouts()->sub_items[2][0].is_nil && strcmp(score_panel_readouts()->sub_items[0][0].text, "0$") == 0);
    assert(type_cheat() == 1 && main_economy() == provisional);
    assert(available_technologies_affordable(t) == 1);
    score_panel_refresh();
    assert(!score_panel_readouts()->sub_items[2][0].is_nil && strcmp(score_panel_readouts()->sub_items[0][0].text, "200$") == 0);
    assert_cheated();
    assert(type_cheat() == 1 && main_economy() == provisional);
    assert_cheated();
}

static void test_new_init(void) {
    cart_init();
    assert(type_cheat() == 1);
    probe_init();
    assert_at_rest();
}

static void test_cycle(void) {
    cart_init();
    probe_shutdown();
    assert(main_economy() == NULL && main_economy_typed() == NULL);
    assert(type_cheat() == 1);
    assert(main_economy() == NULL && main_economy_typed() == NULL);
    probe_shutdown();
    assert(main_economy() == NULL);
    probe_init();
    assert_at_rest();
    probe_init();
    probe_init();
    assert_at_rest();
}

static void test_replacement(void) {
    cart_init();
    assert(type_cheat() == 1);
    new_game();
    assert(main_economy_placeholder() == NULL && (void *)main_economy_typed() == (void *)main_economy());
    assert(main_economy_typed()->research_points == p8_fixed_from_int(0));
    assert(main_economy_typed()->credits == p8_fixed_from_int(100));
    probe_init();
    assert_at_rest();
}

/* The provisional is one static struct whose descriptor has no method;
 * cart_start_game() puts an economy in its place. */
static void test_main_economy_is_a_struct(void) {
    const entity_class_t *cls = main_economy_placeholder_class();
    void *provisional;
    int j;

    cart_init();
    provisional = main_economy();
    assert(provisional != NULL);
    assert(((const entity_t *)provisional)->class == cls);
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->has_refresh == NULL && cls->left_press == NULL && cls->accept_drop == NULL);
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }

    probe_init();
    assert(main_economy() == provisional);

    new_game();
    assert(((const entity_t *)main_economy())->class == economy_class());
}

/* tech_gate_available_technologies() keeps a technology of the current tier
 * until it is invented, even when told to keep the unaffordable ones. */
static void test_available_technologies_drops_invented(void) {
    available_technologies_kept_t kept;
    const technologies_entry_t *t;
    p8_fixed_t ew;
    int i;

    cart_init();
    new_game();
    kept = tech_gate_available_technologies(1);
    assert(kept.count > 0);
    t = kept.items[0];
    ew = main_economy_typed()->era;
    economy_invent(main_economy_typed(), t);
    assert(main_economy_typed()->era == ew);
    kept = tech_gate_available_technologies(1);
    for (i = 0; i < kept.count; i++) {
        assert(kept.items[i] != t);
    }
}

/* Each scenario runs in a child forked right after cart_load(), as a fresh
 * process would start it. */
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
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_at_rest);
    in_fresh_child(test_cheat_on_title);
    in_fresh_child(test_new_init);
    in_fresh_child(test_cycle);
    in_fresh_child(test_replacement);
    in_fresh_child(test_main_economy_is_a_struct);
    in_fresh_child(test_available_technologies_drops_invented);
    printf("test_main_economy: ok\n");
    return 0;
}
