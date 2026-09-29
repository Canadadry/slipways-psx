/* The message box `mh`, built the two ways the game builds it (`pj:hk` from
 * the mutated `pj.ea`, `pj:ph` from `bz`) and lived through the real
 * `dg`/`dc("ev")`. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_score_panel_display.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define YEAR_MESSAGE_TICKS 192
#define YEAR_MESSAGE_SEQUENCE_HASH 0xEF726FD2u
#define YEAR_SILENT_TICKS 104
#define YEAR_SILENT_SEQUENCE_HASH 0xA1327C17u
#define TWO_YEARS_TICKS 213
#define TWO_YEARS_SEQUENCE_HASH 0xFB1D13D2u
#define SCORES_TICKS 113
#define SCORES_SEQUENCE_HASH 0xE45D2CF8u

#define MAX_BOXES 4

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

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static uint32_t draw_hash(void) {
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_dc("ev");
    return screen_hash();
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

static int count_of(const pico8_list_t *list) {
    return list != NULL ? list->count : 0;
}

/* The kind words are constant by the struct's shape, kept so that the
 * sequence hashes still prove the typed reads. */
static uint32_t fold_box(uint32_t h, const pico8_mh_t *e) {
    h = fnv(fnv(h, (uint32_t)PICO8_VALUE_FIXED), (uint32_t)e->ke.t);
    if (e->has_p_) {
        h = fnv(fnv(h, (uint32_t)PICO8_VALUE_BOOL), (uint32_t)e->p_);
    } else {
        h = fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)(pico8_kc() == e));
    h = fnv(h, (uint32_t)listed(pico8_hv(), e));
    h = fnv(h, (uint32_t)listed(pico8_cv("ev"), e));
    return fnv(h, (uint32_t)listed(pico8_cg("mh"), e));
}

typedef struct {
    const pico8_mh_t *boxes[MAX_BOXES];
    int count;
    uint32_t h;
    int ticks;
    int drew;
} run_t;

static void run_start(run_t *run) {
    run->count = 0;
    run->h = 2166136261u;
    run->ticks = 0;
    run->drew = 0;
}

static void track(run_t *run, const pico8_mh_t *box) {
    assert(box != NULL && run->count < MAX_BOXES);
    run->boxes[run->count++] = box;
}

static int any_listed(const run_t *run) {
    int i;
    for (i = 0; i < run->count; i++) {
        if (listed(pico8_hv(), run->boxes[i])) {
            return 1;
        }
    }
    return 0;
}

/* One game tick: `dg`, then `dc("ev")` on a fresh framebuffer, folded with
 * every tracked box's state. */
static void tick(run_t *run) {
    uint32_t drawn;
    int i;

    pico8_dg();
    drawn = draw_hash();
    pico8_cls(&mem);
    run->drew |= drawn != screen_hash();
    run->h = fnv(run->h, drawn);
    for (i = 0; i < run->count; i++) {
        run->h = fold_box(run->h, run->boxes[i]);
    }
    run->ticks++;
    assert(run->ticks < 5000);
}

static void run_out(run_t *run) {
    while (any_listed(run)) {
        tick(run);
    }
}

static void run_check(const run_t *run, int want_ticks, uint32_t want_sequence, const char *label) {
    int i;

    for (i = 0; i < run->count; i++) {
        assert(!listed(pico8_cv("ev"), run->boxes[i]));
        assert(!listed(pico8_cg("mh"), run->boxes[i]));
        assert(pico8_kc() != run->boxes[i]);
    }
    assert(run->drew);
    printf("[%s] ticks=%d sequence=0x%08Xu\n", label, run->ticks, (unsigned)run->h);
    fflush(stdout);
    assert(run->ticks == want_ticks);
    assert(run->h == want_sequence);
}

/* `pj:hk` discards the box it builds; `mh:pq` has just made it `kc`. */
static const pico8_mh_t *year_box(int yr) {
    const pico8_mh_t *box;
    int before = count_of(pico8_cg("mh"));

    pico8_pj_hk(pico8_fixed_from_int(yr));
    box = pico8_kc();
    assert(box != NULL);
    assert(count_of(pico8_cg("mh")) == before + 1);
    assert(listed(pico8_cg("mh"), box) && listed(pico8_hv(), box) && listed(pico8_cv("ev"), box));
    return box;
}

/* 3425 is `pj.gs[1]`, "last year!": `lh` counts down 90 calls. */
static void test_year_with_a_message(void) {
    run_t run;

    pico8_jl();
    run_start(&run);
    track(&run, year_box(3425));
    run.h = fnv(run.h, draw_hash());
    run_out(&run);
    run_check(&run, YEAR_MESSAGE_TICKS, YEAR_MESSAGE_SEQUENCE_HASH, "year with a message");
}

/* 3400 has no message: `pj.ea[3]` is removed and the previous year's `lh`,
 * its counter already spent, is copied again. */
static void test_year_without_a_message(void) {
    run_t run;

    pico8_jl();
    run_start(&run);
    track(&run, year_box(3400));
    run.h = fnv(run.h, draw_hash());
    run_out(&run);
    run_check(&run, YEAR_SILENT_TICKS, YEAR_SILENT_SEQUENCE_HASH, "year without a message");
}

/* The second `pj:hk` marks the first box `p_` through `kc`, and rewrites
 * `pj.ea[2][1]`, a table the first box still shares, before it is drawn
 * one last time. */
static void test_two_years_in_a_row(void) {
    run_t run;
    const pico8_mh_t *first;
    const pico8_mh_t *second;
    int i;

    pico8_jl();
    run_start(&run);
    first = year_box(3424);
    track(&run, first);
    for (i = 0; i < 20; i++) {
        tick(&run);
    }
    assert(!first->has_p_);
    second = year_box(3421);
    track(&run, second);
    assert(first->has_p_ && first->p_);
    run.h = fnv(run.h, draw_hash());
    tick(&run);
    assert(!listed(pico8_hv(), first));
    run_out(&run);
    run_check(&run, TWO_YEARS_TICKS, TWO_YEARS_SEQUENCE_HASH, "two years in a row");
}

static void set_c_(pico8_pj_t *ju, int on) {
    ju->c_ = on;
    ju->has_c_ = 1;
}

/* A game started by `eu()` with no difficulty picked (`dk` NULL reads as
 * difficulty 0), then `jl()` again: `fm` and `ju` stay, `hv` holds only
 * `mo`, `je` and `ok`. */
static pico8_pj_t *game_ju(void) {
    pico8_eu(NULL);
    pico8_jl();
    assert(pico8_ju() != NULL);
    return pico8_ju();
}

/* `pj:ph` on the game's one `pj`, its panel already up (`gz` set): the box
 * of `rh`=3 stays open while `c_`, and is not rebuilt while it is `ed`. */
static void test_scores_box_follows_c_(void) {
    run_t run;
    pico8_pj_t *ju = game_ju();
    const pico8_mh_t *box;
    int built;
    int i;

    ju->gz = pico8_fixed_from_int(0);
    ju->has_gz = 1;
    set_c_(ju, 1);
    run_start(&run);

    pico8_pj_ph(ju);
    box = pico8_kc();
    assert(box != NULL);
    assert(ju->ed == box);
    built = count_of(pico8_cg("mh"));
    track(&run, box);
    run.h = fnv(run.h, draw_hash());

    for (i = 0; i < 80; i++) {
        pico8_pj_ph(ju);
        assert(count_of(pico8_cg("mh")) == built);
        tick(&run);
        assert(listed(pico8_hv(), box));
    }
    set_c_(ju, 0);
    while (any_listed(&run)) {
        pico8_pj_ph(ju);
        assert(count_of(pico8_cg("mh")) == built);
        tick(&run);
    }
    run_check(&run, SCORES_TICKS, SCORES_SEQUENCE_HASH, "scores box follows c_");
}

/* The box's `fa`/`rh`/`nu` are the class's generated defaults. */
static void test_defaults_match_the_parsed_spec(void) {
    const pico8_mh_t *box;
    const pico8_str_seq_t *nu;

    pico8_jl();
    box = year_box(3400);
    assert(box->fa == pico8_mh_start.fa);
    assert(box->fa == pico8_fixed_from_int(14));
    assert(box->rh == pico8_mh_start.rh);
    assert(box->rh == pico8_fixed_from_int(5));
    assert(box->nu != NULL);
    nu = box->nu;
    assert(nu == pico8_mh_start.nu && nu == &pico8_mh_nu_tags && nu->count == 1);
    assert(strcmp(nu->items[0], "mh") == 0);
    assert(year_box(3400)->nu == nu);
    assert(box->ke.me != NULL && strcmp(box->ke.me, "ph") == 0);
    assert(pico8_mh_ready());
    pico8_mh_shutdown();
    assert(!pico8_mh_ready());
    pico8_mh_init();
}

static int g_lh_calls = 0;

static int counting_lh(pico8_mh_t *self) {
    (void)self;
    g_lh_calls++;
    return 1;
}

/* Built by `pj` or through its class, a box is a struct that lands in hv,
 * cv.ev and cg("mh") only; its `lh` is a field `mv` calls, and it has no
 * `ph` for `dg` to call. */
static void test_mh_is_a_struct_registered_under_ev(void) {
    const char *others[] = {"kj", "hd", "hk", "hb"};
    pico8_mh_keys_t keys = {0};
    const pico8_mh_t *from_pj;
    void *generic;
    int out;
    int i;

    pico8_jl();
    from_pj = year_box(3400);
    keys.lh = counting_lh;
    generic = pico8_mh_new(&keys);

    assert(pico8_type_tag_of(from_pj) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_kc() == generic);
    assert(listed(pico8_hv(), generic) && listed(pico8_cv("ev"), generic) && listed(pico8_cg("mh"), generic));
    for (i = 0; i < 4; i++) {
        assert(!listed(pico8_cv(others[i]), from_pj) && !listed(pico8_cv(others[i]), generic));
    }
    assert(((pico8_mh_t *)generic)->lh != NULL);
    out = ((pico8_mh_t *)generic)->lh(generic);
    assert(g_lh_calls == 1 && out);
    assert(((const pico8_struct_header_t *)generic)->cls->states[PICO8_STATE_PH] == NULL);
}

/* A nil `lh` key (NULL) leaves the box without `lh`, as a box built with no
 * key. */
static void test_mh_nil_lh_key_reads_absent(void) {
    pico8_mh_keys_t keys = {0};
    pico8_mh_t *box;

    pico8_jl();
    keys.lh = NULL;
    box = pico8_mh_new(&keys);
    assert(box->lh == NULL);
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
    pico8__init();

    test_year_with_a_message();
    test_year_without_a_message();
    test_two_years_in_a_row();
    test_scores_box_follows_c_();
    test_defaults_match_the_parsed_spec();
    test_mh_is_a_struct_registered_under_ev();
    test_mh_nil_lh_key_reads_absent();

    printf("test_pico8_mh: all tests passed\n");
    return 0;
}
