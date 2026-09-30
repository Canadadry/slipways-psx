/* The camera `qb` (`mo` once `jl()` has run), read and written on its
 * members through pico8_mo_qb() and driven through the real `dg`/`dc`, in games
 * built directly: no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
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
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"

#define SCRIPTED_TICKS 110
#define SCRIPTED_HASH 0xDF9F2872u
#define SHAKE_TICKS 31
#define SHAKE_HASH 0xF1E7A593u
#define GEOHARVEST_WINDOW 60

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

static int listed(const pico8_ptrs_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
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

static pico8_qb_t *mo_qb(void) {
    assert(pico8_mo_qb() != NULL);
    return pico8_mo_qb();
}

static pico8_vec_t must_vec(const pico8_vec_t *v) {
    assert(v != NULL);
    return *v;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *v) {
    if (v == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)v->x);
    return fnv(h, (uint32_t)v->y);
}

static uint32_t fold_mo(uint32_t h) {
    const pico8_qb_t *mo = mo_qb();

    h = fold_vec(h, mo->p);
    h = fold_vec(h, mo->v);
    h = fold_vec(h, mo->ra);
    h = fnv(h, (uint32_t)mo->ma);
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

static int is_vec(pico8_vec_t v, int x, int y) {
    return v.x == pico8_fixed_from_int(x) && v.y == pico8_fixed_from_int(y);
}

/* `jl()` builds `mo`: in hv only, the spec's defaults, ke's `me`/`t`, `pc`
 * in the order left, right, up, down, and `p`/`ra` two distinct vectors. */
static void check_fresh_mo(const pico8_qb_t *mo) {
    const pico8_vec_seq_t *pc;
    pico8_vec_t p = must_vec(mo->p);
    pico8_vec_t ra = must_vec(mo->ra);
    pico8_vec_t v = must_vec(mo->v);

    assert(listed(pico8_hv(), mo) && !in_any_cv(mo));
    assert(p.x == pico8_fixed_from_int(192) && p.y == pico8_fixed_from_int(192));
    assert(ra.x == pico8_fixed_from_int(192) && ra.y == pico8_fixed_from_int(192));
    assert(v.x == 0 && v.y == 0);
    assert(mo->ma == 0);
    assert(mo->ke.header.t == 0);
    assert(mo->ke.header.me != NULL && strcmp(mo->ke.header.me, "ph") == 0);
    assert(mo->p != mo->ra);

    pc = mo->pc;
    assert(pc != NULL);
    assert(pc == &pico8_qb_pc_vecs && pc->count == 4);
    assert(is_vec(pc->items[0], -1, 0));
    assert(is_vec(pc->items[1], 1, 0));
    assert(is_vec(pc->items[2], 0, -1));
    assert(is_vec(pc->items[3], 0, 1));
}

static void test_jl_builds_a_camera(void) {
    pico8_jl();
    check_fresh_mo(mo_qb());
}

static void md_to(pico8_fixed_t fy, int want_x, int want_y) {
    pico8_camera(&gfx, 7, 11);
    assert(pico8_mo()->cls == pico8_qb_struct_class());
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)pico8_mo(), fy);
    assert(gfx.camera_x == want_x && gfx.camera_y == want_y);
}

/* `md(fy)` puts the camera on `ra*fy`. The later tests of this process
 * start from the camera the last `md` leaves: 192,192. */
static void test_md_sets_the_camera(void) {
    pico8_jl();
    md_to(pico8_fixed_from_int(1), 192, 192);
    md_to(PICO8_FIXED_LIT(0x00008000, 0.5), 96, 96);
    md_to(0, 0, 0);
    md_to(pico8_fixed_from_int(1), 192, 192);
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
        h = fnv(h, (uint32_t)mo_qb()->ke.header.t);
        if (must_vec(mo_qb()->p).y == pico8_fixed_from_int(320)) {
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
    const pico8_qb_t *mo;

    pico8_jl();
    mo = mo_qb();
    mo_qb()->ma = pico8_fixed_from_int(3);
    assert(mo->ma == pico8_fixed_from_int(3));
    unscript();
    while (mo->ma != 0) {
        pico8_vec_t off;
        pico8_dg();
        ticks++;
        assert(ticks < 100);
        off = pico8_vec_sub(must_vec(mo->ra), must_vec(mo->p));
        h = fnv(h, (uint32_t)mo->ma);
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
    check_fresh_mo(mo_qb());

    pico8_bootstrap_shutdown();
    pico8_jl();
    check_fresh_mo(mo_qb());
}

/* Before `jl()`, `mo` is the placeholder, not a qb: `md` dispatches
 * nothing and the camera stays put. */
static void test_placeholder_mo(void) {
    pico8_entity_init();
    pico8_camera(&gfx, 5, 6);
    assert(pico8_mo()->cls != pico8_qb_struct_class());
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(pico8_mo_qb() == NULL);
}

/* A game started as `eu` starts it, the right button pressed over empty
 * space then dragged: `ok` hands the press to `bg`, whose `hq` moves `mo`. */
static void test_right_drag_moves_the_camera(void) {
    pico8_vec_t before;
    pico8_vec_t after;
    int i;

    pico8__init();
    pico8_eu(NULL);
    for (i = 0; i < 20; i++) {
        memset(&g_frames[i], 0, sizeof g_frames[i]);
        g_frames[i].mouse_x = 64 + 2 * i;
        g_frames[i].mouse_y = 64 + i;
        g_frames[i].mouse_buttons = i < 18 ? 2 : 0;
    }
    script(20);
    pico8__update60();
    before = must_vec(mo_qb()->p);
    for (i = 1; i < 20; i++) {
        pico8_input_step(&input);
        pico8__update60();
    }
    unscript();
    after = must_vec(mo_qb()->p);
    assert(after.x != before.x || after.y != before.y);
}

static int ga_alive(void) {
    const pico8_ptrs_t *hv = pico8_hv();
    int i;

    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i];
        if (e != NULL &&
            ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class()) {
            return 1;
        }
    }
    return 0;
}

/* A `ga`'s `pq` shakes the camera: `mo.ma=3` as soon as it is built, then
 * the decay back to 0 within GEOHARVEST_WINDOW ticks. */
static void test_ga_shakes(void) {
    int i;

    pico8__init();
    pico8_eu(NULL);
    assert(!ga_alive());
    (void)pico8_ga_new(NULL);
    assert(ga_alive());
    assert(mo_qb()->ma == pico8_fixed_from_int(3));
    for (i = 0; i < GEOHARVEST_WINDOW; i++) {
        pico8_dg();
    }
    assert(mo_qb()->ma == 0);
}

/* The galaxy depends on the rnd state, which the tests before it consume: run
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
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_qb_struct_class();
}

/* `jl()`'s `mo` and one built through the class are qb structs in hv only,
 * with `ph` and `md` as methods and no `kj`; `ph` reads a nil `v` as zero. */
static void test_qb_is_a_struct(void) {
    pico8_qb_t *generic;

    pico8_jl();
    generic = pico8_qb_new();
    assert(is_a_qb_struct(pico8_mo()) && is_a_qb_struct(generic));
    assert(listed(pico8_hv(), generic) && !in_any_cv(generic));
    check_fresh_mo(generic);
    assert(generic->ke.header.cls->get_ra(generic) == generic->ra);
    assert(generic->ke.header.cls->get_p(generic) == generic->p);
    pico8_qb_struct_class()->states[PICO8_STATE_PH](generic, pico8_fixed_from_int(0));
    pico8_qb_md(pico8_dispatch_gfx(), generic, pico8_fixed_from_int(1));
    assert(pico8_qb_struct_class()->kj == NULL);
    {
        pico8_vec_t p_before = must_vec(generic->p);

        generic->v = NULL;
        pico8_qb_struct_class()->states[PICO8_STATE_PH](generic, pico8_fixed_from_int(0));
        assert(generic->v != NULL && generic->v->x == 0 && generic->v->y == 0);
        assert(generic->p->x == p_before.x && generic->p->y == p_before.y);
    }
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
    in_fresh_child(test_right_drag_moves_the_camera);
    in_fresh_child(test_ga_shakes);

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
