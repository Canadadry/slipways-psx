/* The camera `qb` (`mo` once `jl()` has run), read and written only through
 * the dual-form field API and driven through the real `dg`/`dc`. The frozen
 * values were taken on the code where `qb` was still a class instance and
 * must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdint.h>
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
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define SCRIPTED_TICKS 110
#define SCRIPTED_HASH 0xDF9F2872u
#define SHAKE_TICKS 31
#define SHAKE_HASH 0xF1E7A593u
#define TITLE_REPLAY "tests/fixtures/long_session_events.json"
#define TITLE_FRAMES 4800
#define TITLE_HASH 0xE21970E1u
#define GEOHARVEST_REPLAY "tests/fixtures/geoharvest_burst_events.json"
#define GEOHARVEST_GA_FRAME 205
#define GEOHARVEST_WINDOW 60
#define GEOHARVEST_HASH 0x38A9D79Du

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static int listed(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return 1;
        }
    }
    return 0;
}

static int in_any_cv(const void *e) {
    const char *names[] = {"kj", "ev", "hd", "hk", "hb"};
    int i;

    for (i = 0; i < 5; i++) {
        if (listed(pico8_cv(names[i]), e)) {
            return 1;
        }
    }
    return 0;
}

/* 0 when absent; asserts a non-NULL vector REF otherwise. */
static int vec_of(const void *e, const char *name, pico8_vec_t *out) {
    pico8_value_t v;

    if (!pico8_entity_get_field(e, name, &v)) {
        return 0;
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    *out = *(const pico8_vec_t *)v.as.ref;
    return 1;
}

static const void *ref_of(const void *e, const char *name) {
    pico8_value_t v;

    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* 0 when absent; asserts a number otherwise. */
static int fixed_of(const void *e, const char *name, pico8_fixed_t *out) {
    pico8_value_t v;

    if (!pico8_entity_get_field(e, name, &v)) {
        return 0;
    }
    assert(v.kind == PICO8_VALUE_FIXED);
    *out = v.as.fixed;
    return 1;
}

static pico8_vec_t must_vec(const void *e, const char *name) {
    pico8_vec_t out;
    assert(vec_of(e, name, &out));
    return out;
}

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_fixed_t out;
    assert(fixed_of(e, name, &out));
    return out;
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_vec_t v;

    if (!vec_of(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)v.x);
    return fnv(h, (uint32_t)v.y);
}

static uint32_t fold_fixed(uint32_t h, const void *e, const char *name) {
    pico8_fixed_t f;

    if (!fixed_of(e, name, &f)) {
        return fnv(h, 0xDEADu);
    }
    return fnv(h, (uint32_t)f);
}

static uint32_t fold_mo(uint32_t h) {
    const void *mo = pico8_mo();

    h = fold_vec(h, mo, "p");
    h = fold_vec(h, mo, "v");
    h = fold_vec(h, mo, "ra");
    h = fold_fixed(h, mo, "ma");
    h = fnv(h, (uint32_t)gfx.camera_x);
    return fnv(h, (uint32_t)gfx.camera_y);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static pico8_input_frame_t g_frames[SCRIPTED_TICKS];

static void script(int n) {
    input.frames = g_frames;
    input.frame_count = n;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
}

static void unscript(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
}

static void pad(int i, int left, int right, int up, int down) {
    memset(&g_frames[i], 0, sizeof g_frames[i]);
    g_frames[i].dpad[0] = left;
    g_frames[i].dpad[1] = right;
    g_frames[i].dpad[2] = up;
    g_frames[i].dpad[3] = down;
}

static int is_vec(pico8_value_t v, int x, int y) {
    const pico8_vec_t *p = v.as.ref;
    return v.kind == PICO8_VALUE_REF && p != NULL && p->x == pico8_fixed_from_int(x) && p->y == pico8_fixed_from_int(y);
}

/* `jl()` builds `mo`: in hv only, the spec's defaults, ke's `me`/`t`, `pc`
 * in the order left, right, up, down, and `p`/`ra` two distinct vectors. */
static void check_fresh_mo(const void *mo) {
    const pico8_ob_table_t *pc;
    pico8_value_t me;
    pico8_value_t d;
    pico8_vec_t p = must_vec(mo, "p");
    pico8_vec_t ra = must_vec(mo, "ra");
    pico8_vec_t v = must_vec(mo, "v");

    assert(listed(pico8_hv(), mo) && !in_any_cv(mo));
    assert(p.x == pico8_fixed_from_int(192) && p.y == pico8_fixed_from_int(192));
    assert(ra.x == pico8_fixed_from_int(192) && ra.y == pico8_fixed_from_int(192));
    assert(v.x == 0 && v.y == 0);
    assert(must_fixed(mo, "ma") == 0);
    assert(must_fixed(mo, "t") == 0);
    assert(pico8_entity_get_field(mo, "me", &me) && me.kind == PICO8_VALUE_STRING && strcmp(me.as.string, "ph") == 0);
    assert(ref_of(mo, "p") != ref_of(mo, "ra"));

    pc = ref_of(mo, "pc");
    assert(pico8_type_tag_of(pc) == PICO8_TYPE_TAG_OB_TABLE && pc->count == 4);
    assert(pico8_ob_table_get_index(pc, 1, &d) && is_vec(d, -1, 0));
    assert(pico8_ob_table_get_index(pc, 2, &d) && is_vec(d, 1, 0));
    assert(pico8_ob_table_get_index(pc, 3, &d) && is_vec(d, 0, -1));
    assert(pico8_ob_table_get_index(pc, 4, &d) && is_vec(d, 0, 1));
}

static void test_jl_builds_a_camera(void) {
    pico8_jl();
    check_fresh_mo(pico8_mo());
}

static void md_to(pico8_value_t fy, int want_x, int want_y) {
    pico8_value_t out;

    pico8_camera(&gfx, 7, 11);
    assert(pico8_mv(pico8_mo(), "md", 1, fy, pico8_value_ref(NULL), &out));
    assert(gfx.camera_x == want_x && gfx.camera_y == want_y);
}

/* `md(fy)` puts the camera on `ra*fy`, an absent `fy` counting as 1. */
static void test_md_sets_the_camera(void) {
    pico8_jl();
    md_to(pico8_value_fixed(pico8_fixed_from_int(1)), 192, 192);
    md_to(pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)), 96, 96);
    md_to(pico8_value_fixed(0), 0, 0);
    md_to(pico8_value_ref(NULL), 192, 192);
}

/* Idle, left, right+up, down held into the 320 bound, then released. */
static void test_scripted_ticks(void) {
    uint32_t h = 2166136261u;
    int bounded = 0;
    int i;

    pico8_jl();
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        if (i < 3) {
            pad(i, 0, 0, 0, 0);
        } else if (i < 8) {
            pad(i, 1, 0, 0, 0);
        } else if (i < 16) {
            pad(i, 0, 1, 1, 0);
        } else if (i < 90) {
            pad(i, 0, 0, 0, 1);
        } else {
            pad(i, 0, 0, 0, 0);
        }
    }
    script(SCRIPTED_TICKS);
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        pico8_dg();
        h = fold_mo(h);
        h = fold_fixed(h, pico8_mo(), "t");
        if (must_vec(pico8_mo(), "p").y == pico8_fixed_from_int(320)) {
            bounded++;
        }
        if (i < SCRIPTED_TICKS - 1) {
            pico8_input_step(&input);
        }
    }
    unscript();
    printf("[scripted ticks] p.y at the 320 bound on %d ticks\n", bounded);
    assert(bounded > 0);
    check("scripted ticks", h, SCRIPTED_HASH);
}

/* `mo.ma=3`, as `ga:pq` writes it, decays by 0.1 per `ph`, shaking `ra`
 * off `p` by `qm(ma, t*0.2)`. */
static void test_shake_decays(void) {
    uint32_t h = 2166136261u;
    int ticks = 0;
    const void *mo;

    pico8_jl();
    mo = pico8_mo();
    pico8_entity_set_field(pico8_mo(), "ma", pico8_value_fixed(pico8_fixed_from_int(3)));
    assert(must_fixed(mo, "ma") == pico8_fixed_from_int(3));
    unscript();
    while (must_fixed(mo, "ma") != 0) {
        pico8_vec_t off;
        pico8_dg();
        ticks++;
        assert(ticks < 100);
        off = pico8_vec_sub(must_vec(mo, "ra"), must_vec(mo, "p"));
        h = fnv(h, (uint32_t)must_fixed(mo, "ma"));
        h = fnv(h, (uint32_t)off.x);
        h = fnv(h, (uint32_t)off.y);
    }
    printf("[shake] ma back to 0 after %d ticks\n", ticks);
    fflush(stdout);
    assert(ticks == SHAKE_TICKS);
    check("shake", h, SHAKE_HASH);
}

/* `eu()` runs `jl()` again over a live `mo`; a shutdown and a fresh `jl()`
 * after it must not crash either. */
static void test_second_jl(void) {
    const void *first;

    pico8_jl();
    first = pico8_mo();
    pico8_jl();
    assert(pico8_mo() != first);
    assert(!listed(pico8_hv(), first));
    check_fresh_mo(pico8_mo());

    pico8_bootstrap_shutdown();
    pico8_jl();
    check_fresh_mo(pico8_mo());
}

/* Before `jl()`, `mo` is the field-less placeholder: `md` dispatches
 * nothing and the camera stays put. */
static void test_placeholder_mo(void) {
    pico8_value_t out;
    pico8_value_t v;

    pico8_entity_init();
    pico8_camera(&gfx, 5, 6);
    assert(!pico8_mv(pico8_mo(), "md", 1, pico8_value_fixed(pico8_fixed_from_int(1)), pico8_value_ref(NULL), &out));
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(!pico8_entity_get_field(pico8_mo(), "ra", &v));
    assert(!pico8_entity_get_field(pico8_mo(), "p", &v));
    assert(!pico8_entity_get_field(pico8_mo(), "ma", &v));
}

/* A recording from the title screen, through D-pad scrolling (first at
 * frame 1002) and a right-button drag, `bg:hq` (first at frame 4741). */
static void test_replay_from_title(void) {
    uint32_t h = 2166136261u;
    pico8_cheat_t cheat;
    pico8_vec_t before_pad = {0, 0};
    pico8_vec_t before_drag = {0, 0};
    pico8_vec_t after_pad;
    pico8_vec_t after_drag;
    int i;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, TITLE_REPLAY));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    for (i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        h = fold_mo(h);
        if (i == 1001) {
            before_pad = must_vec(pico8_mo(), "p");
        } else if (i == 1100) {
            after_pad = must_vec(pico8_mo(), "p");
            assert(after_pad.x != before_pad.x || after_pad.y != before_pad.y);
        } else if (i == 4740) {
            before_drag = must_vec(pico8_mo(), "p");
        } else if (i == TITLE_FRAMES - 1) {
            after_drag = must_vec(pico8_mo(), "p");
            assert(after_drag.x != before_drag.x || after_drag.y != before_drag.y);
        }
        if (i + 1 == TITLE_FRAMES || i + 1 == input.frame_count) {
            break;
        }
        pico8_input_step(&input);
    }
    pico8_input_free(&input);
    unscript();
    assert(i + 1 == TITLE_FRAMES);
    check("title replay", h, TITLE_HASH);
}

static int ga_alive(void) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i].as.ref;
        if (pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
            ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class()) {
            return 1;
        }
    }
    return 0;
}

/* The harvest builds a `ga`, whose `pq` shakes the camera: `mo.ma=3` on
 * the very frame it appears, then the decay. */
static void test_geoharvest_shakes(void) {
    uint32_t h = 2166136261u;
    pico8_cheat_t cheat;
    pico8_value_t difficulty;
    int first_ga = -1;
    int i;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, GEOHARVEST_REPLAY));
    pico8_dispatch_set_input(&input);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    pico8_cheat_init(&cheat);
    for (i = 0; i < GEOHARVEST_GA_FRAME + GEOHARVEST_WINDOW; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (first_ga < 0 && ga_alive()) {
            first_ga = i;
            assert(must_fixed(pico8_mo(), "ma") == pico8_fixed_from_int(3));
        }
        if (i >= GEOHARVEST_GA_FRAME) {
            h = fold_fixed(h, pico8_mo(), "ma");
            h = fold_vec(h, pico8_mo(), "ra");
        }
        pico8_input_step(&input);
    }
    pico8_input_free(&input);
    unscript();
    printf("[geoharvest] ga first alive at frame %d\n", first_ga);
    fflush(stdout);
    assert(first_ga == GEOHARVEST_GA_FRAME);
    assert(must_fixed(pico8_mo(), "ma") == 0);
    check("geoharvest", h, GEOHARVEST_HASH);
}

/* A replay depends on the rnd state, which the tests before it consume: run
 * it in a child forked right after pico8_cart_load(), as a fresh process
 * would start it. */
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

static int is_a_qb_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_qb_struct_class();
}

/* `jl()`'s `mo` and one built through the class are qb structs in hv only,
 * with `ph` and `md` as methods and no `kj`; a generic write of a name
 * other than `ma`/`v` stops the run instead of growing a member. */
static void test_qb_is_a_struct(void) {
    void *generic;
    pico8_value_t out;
    pid_t child;
    int status;

    pico8_jl();
    generic = pico8_qb_new(NULL);
    assert(is_a_qb_struct(pico8_mo()) && is_a_qb_struct(generic));
    assert(listed(pico8_hv(), generic) && !in_any_cv(generic));
    check_fresh_mo(generic);
    assert(pico8_mv(generic, "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(pico8_mv(generic, "md", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv(generic, "kj", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(pico8_mo(), "p", pico8_value_ref(NULL));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
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
    in_fresh_child(test_replay_from_title);
    in_fresh_child(test_geoharvest_shakes);

    test_jl_builds_a_camera();
    test_md_sets_the_camera();
    test_scripted_ticks();
    test_shake_decays();
    test_second_jl();
    test_placeholder_mo();
    test_qb_is_a_struct();

    printf("test_pico8_qb: all tests passed\n");
    return 0;
}
