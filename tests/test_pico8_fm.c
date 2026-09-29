/* The provisional `fm` that pico8_probe_init() leaves until `eu()` replaces
 * it with a `hy`, read only through the dual-form field API: its names, the
 * cheat code typed on the title screen, a new init, the init/shutdown cycle,
 * the replacement. The frozen values were taken on the code where the
 * provisional was still a class-less instance and must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const others[] = {"ge", "ew", "lo", "yr", "invented", "po", "fu", "happiness", "hl", "hk", "md", "x"};
#define OTHER_COUNT ((int)(sizeof others / sizeof others[0]))

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v);
}

static pico8_fixed_t get_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void assert_others_absent(void) {
    int n;
    for (n = 0; n < OTHER_COUNT; n++) {
        assert(!has(pico8_fm(), others[n]));
    }
}

static void assert_at_rest(void) {
    assert(pico8_fm() != NULL);
    assert(!has(pico8_fm(), "jk") && !has(pico8_fm(), "nz"));
    assert_others_absent();
}

/* A A B B A A B B, one press every other frame; returns how often it fired. */
static int type_cheat(void) {
    static const int presses[] = {4, 4, 5, 5, 4, 4, 5, 5};
    pico8_input_frame_t frames[16];
    pico8_input_t pad;
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
    assert(get_fixed(pico8_fm(), "jk") == pico8_fixed_from_int(99));
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(200));
    assert_others_absent();
}

static void new_game(void) {
    pico8_eu(NULL);
}

static void test_at_rest(void) {
    pico8__init();
    assert_at_rest();
}

static int cb_affordable(const pico8_nb_entry_t *t) {
    pico8_list_t kept = pico8_cb(pico8_value_bool(1));

    pico8_list_free(&kept);
    return pico8_nb_e_(t);
}

static void test_cheat_on_title(void) {
    void *provisional;
    const pico8_nb_entry_t *t = pico8_nb_table;

    pico8__init();
    provisional = pico8_fm();
    assert(pico8_fm_hy() == NULL && (const void *)pico8_fm_placeholder() == provisional);
    while (pico8_gi_nb(t) <= pico8_fixed_from_int(0) || pico8_gi_nb(t) > pico8_fixed_from_int(99)) {
        t++;
        assert(t < pico8_nb_table + PICO8_NB_COUNT);
    }
    assert(cb_affordable(t) == 0);
    pico8_pj_hb();
    assert(pico8_pj_ou()->sub_items[2][0].is_ref && strcmp(pico8_pj_ou()->sub_items[0][0].s, "0$") == 0);
    assert(type_cheat() == 1 && pico8_fm() == provisional);
    assert(cb_affordable(t) == 1);
    pico8_pj_hb();
    assert(!pico8_pj_ou()->sub_items[2][0].is_ref && strcmp(pico8_pj_ou()->sub_items[0][0].s, "200$") == 0);
    assert_cheated();
    assert(type_cheat() == 1 && pico8_fm() == provisional);
    assert_cheated();
}

static void test_new_init(void) {
    pico8__init();
    assert(type_cheat() == 1);
    pico8_probe_init();
    assert(pico8_fm() != NULL);
    assert(!has(pico8_fm(), "jk") && !has(pico8_fm(), "nz"));
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
    assert(get_fixed(pico8_fm(), "jk") == pico8_fixed_from_int(0));
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(100));
    pico8_probe_init();
    assert(pico8_fm() != NULL);
    assert(!has(pico8_fm(), "jk") && !has(pico8_fm(), "nz"));
}

static void write_jk(void) {
    pico8_entity_set_field(pico8_fm(), "jk", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void must_abort(void (*write)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write();
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

/* The provisional is one static struct whose descriptor has no method, a
 * reader and no writer; `eu()` puts a `hy` in its place. */
static void test_fm_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_fm_placeholder_struct_class();
    void *provisional;
    int j;

    pico8__init();
    provisional = pico8_fm();
    assert(pico8_type_tag_of(provisional) == PICO8_TYPE_TAG_STRUCT);
    assert(((const pico8_struct_header_t *)provisional)->cls == cls);
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL);
    assert(cls->hq == NULL && cls->ey == NULL && cls->kf == NULL && cls->cn == NULL && cls->ol == NULL &&
           cls->mm == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    must_abort(write_jk);

    pico8_probe_init();
    assert(pico8_fm() == provisional);

    new_game();
    assert(((const pico8_struct_header_t *)pico8_fm())->cls == pico8_hy_struct_class());
}

/* The raw reader, not `lookup`: a member present but nil or 0 shows here. */
static void assert_raw_absent(const char *name) {
    const pico8_struct_class_t *cls = pico8_fm_placeholder_struct_class();
    pico8_value_t out = pico8_value_fixed(0x1234);
    assert(cls->field(pico8_fm(), name, &out) == 0);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == 0x1234);
    assert(pico8_entity_get_field(pico8_fm(), name, &out) == 0);
}

static void assert_raw_fixed(const char *name, int n) {
    const pico8_struct_class_t *cls = pico8_fm_placeholder_struct_class();
    pico8_value_t out = pico8_value_fixed(0x1234);
    assert(cls->field(pico8_fm(), name, &out) == 1);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(n));
}

static void assert_raw_others_absent(void) {
    int n;
    for (n = 0; n < OTHER_COUNT; n++) {
        assert_raw_absent(others[n]);
    }
    assert_raw_absent("");
    assert_raw_absent("x");
}

static void test_fm_members_by_name(void) {
    void *provisional;

    pico8__init();
    provisional = pico8_fm();
    assert_raw_absent("jk");
    assert_raw_absent("nz");
    assert_raw_others_absent();

    assert(type_cheat() == 1);
    assert_raw_fixed("jk", 99);
    assert_raw_fixed("nz", 200);
    assert_raw_others_absent();

    pico8_probe_init();
    assert(pico8_fm() == provisional);
    assert_raw_absent("jk");
    assert_raw_absent("nz");

    assert(type_cheat() == 1);
    assert_raw_fixed("jk", 99);
    assert_raw_fixed("nz", 200);

    pico8_probe_shutdown();
    assert(type_cheat() == 1 && pico8_fm() == NULL);
    pico8_probe_init();
    assert_raw_absent("jk");
    assert_raw_absent("nz");
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
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_at_rest);
    in_fresh_child(test_cheat_on_title);
    in_fresh_child(test_new_init);
    in_fresh_child(test_cycle);
    in_fresh_child(test_replacement);
    in_fresh_child(test_fm_is_a_struct);
    in_fresh_child(test_fm_members_by_name);
    printf("test_pico8_fm: ok\n");
    return 0;
}
