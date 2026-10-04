/* The message box, built the two ways the game builds it
 * (score_panel_new_year() from the rewritten year banner,
 * score_panel_update() from the score rows) and lived through the real
 * update_entities() and draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY). */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/entity_class.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"
#include "game/message_box.h"
#include "game/economy_score.h"
#include "game/main_economy.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"

#define YEAR_MESSAGE_TICKS 192
#define YEAR_MESSAGE_SEQUENCE_HASH 0x6C50E246u
#define YEAR_SILENT_TICKS 104
#define YEAR_SILENT_SEQUENCE_HASH 0x1AD4FEEBu
#define TWO_YEARS_TICKS 213
#define TWO_YEARS_SEQUENCE_HASH 0x7A1BEE68u
#define SCORES_TICKS 113
#define SCORES_SEQUENCE_HASH 0x49018588u

#define MAX_BOXES 4

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

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
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            h ^= (uint32_t)memory.screen[y][x];
            h *= 16777619u;
        }
    }
    return h;
}

static uint32_t draw_hash(void) {
    p8_gfx_init(&graphics);
    p8_cls(&memory);
    draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY);
    return screen_hash();
}

static int entity_list_listed(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

static int entity_list_count_of(const entity_list_t *list) {
    return list != NULL ? list->count : 0;
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, BOOL_KIND = 2 };

/* The kind words are constant by the struct's shape, kept so that the
 * sequence hashes still prove the typed reads. */
static uint32_t fold_box(uint32_t h, const message_box_t *e) {
    h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.state_time);
    if (e->is_dead) {
        h = fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)e->is_dead);
    } else {
        h = fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)(shown_message_box() == e));
    h = fnv(h, (uint32_t)entity_list_listed(live_entities(), e));
    h = fnv(h, (uint32_t)entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), e));
    return fnv(h, (uint32_t)entity_list_listed(entities_by_tag(TAG_MESSAGE_BOX), e));
}

typedef struct {
    const message_box_t *boxes[MAX_BOXES];
    int count;
    uint32_t hash;
    int ticks;
    int drew;
} run_t;

static void run_start(run_t *run) {
    run->count = 0;
    run->hash = 2166136261u;
    run->ticks = 0;
    run->drew = 0;
}

static void track(run_t *run, const message_box_t *box) {
    assert(box != NULL && run->count < MAX_BOXES);
    run->boxes[run->count++] = box;
}

static int any_listed(const run_t *run) {
    int i;
    for (i = 0; i < run->count; i++) {
        if (entity_list_listed(live_entities(), run->boxes[i])) {
            return 1;
        }
    }
    return 0;
}

/* One game tick: frame_zone_reset() and update_entities() as cart_update60()
 * does them, then draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY) on a fresh
 * framebuffer, folded with every tracked box's state. */
static void tick(run_t *run) {
    uint32_t drawn;
    int i;

    frame_zone_reset();
    update_entities();
    drawn = draw_hash();
    p8_cls(&memory);
    run->drew |= drawn != screen_hash();
    run->hash = fnv(run->hash, drawn);
    for (i = 0; i < run->count; i++) {
        run->hash = fold_box(run->hash, run->boxes[i]);
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
        assert(!entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), run->boxes[i]));
        assert(!entity_list_listed(entities_by_tag(TAG_MESSAGE_BOX), run->boxes[i]));
        assert(shown_message_box() != run->boxes[i]);
    }
    assert(run->drew);
    printf("[%s] ticks=%d sequence=0x%08Xu\n", label, run->ticks, (unsigned)run->hash);
    fflush(stdout);
    assert(run->ticks == want_ticks);
    assert(run->hash == want_sequence);
}

/* score_panel_new_year() discards the box it builds; message_box_construct()
 * has just made it the shown message box. */
static const message_box_t *year_box(int yr) {
    const message_box_t *box;
    int before = entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX));

    score_panel_new_year(p8_fixed_from_int(yr));
    box = shown_message_box();
    assert(box != NULL);
    assert(entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX)) == before + 1);
    assert(entity_list_listed(entities_by_tag(TAG_MESSAGE_BOX), box) && entity_list_listed(live_entities(), box) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), box));
    return box;
}

/* 3425 is the first countdown row, "last year!": keep_open counts down 90
 * calls. */
static void test_year_with_a_message(void) {
    run_t run;

    reset_world(NULL);
    cart_flush_exit_queue();
    run_start(&run);
    track(&run, year_box(3425));
    run.hash = fnv(run.hash, draw_hash());
    run_out(&run);
    run_check(&run, YEAR_MESSAGE_TICKS, YEAR_MESSAGE_SEQUENCE_HASH, "year with a message");
}

/* 3400 has no message: the banner's third article is dropped and the
 * previous year's keep_open, its countdown already spent, is copied again. */
static void test_year_without_a_message(void) {
    run_t run;

    reset_world(NULL);
    cart_flush_exit_queue();
    run_start(&run);
    track(&run, year_box(3400));
    run.hash = fnv(run.hash, draw_hash());
    run_out(&run);
    run_check(&run, YEAR_SILENT_TICKS, YEAR_SILENT_SEQUENCE_HASH, "year without a message");
}

/* The second score_panel_new_year() marks the first box dead through the
 * shown message box, and rewrites the banner's year text, a row the first
 * box still shares, before it is drawn one last time. */
static void test_two_years_in_a_row(void) {
    run_t run;
    const message_box_t *first;
    const message_box_t *second;
    int i;

    reset_world(NULL);
    cart_flush_exit_queue();
    run_start(&run);
    first = year_box(3424);
    track(&run, first);
    for (i = 0; i < 20; i++) {
        tick(&run);
    }
    assert(!first->is_dead);
    second = year_box(3421);
    track(&run, second);
    assert(first->is_dead);
    run.hash = fnv(run.hash, draw_hash());
    tick(&run);
    assert(!entity_list_listed(live_entities(), first));
    run_out(&run);
    run_check(&run, TWO_YEARS_TICKS, TWO_YEARS_SEQUENCE_HASH, "two years in a row");
}

static void set_is_shown(score_panel_t *ju, int on) {
    ju->is_shown = on;
    ju->has_is_shown = 1;
}

/* A game started by cart_start_game() with no difficulty picked (a NULL
 * difficulty reads as difficulty 0), then reset_world() again: the main
 * economy and the main score panel stay, the live entities hold only the
 * main camera, mouse and user interface. */
static score_panel_t *game_main_score_panel(void) {
    cart_start_game(NULL);
    reset_world(NULL);
    cart_flush_exit_queue();
    assert(main_score_panel() != NULL);
    return main_score_panel();
}

/* score_panel_update() on the game's one panel, its curtain already set: the
 * box of duration scale 3 stays open while is_shown, and is not rebuilt
 * while it is the panel's last box. */
static void test_scores_box_follows_is_shown(void) {
    run_t run;
    score_panel_t *ju = game_main_score_panel();
    const message_box_t *box;
    int built;
    int i;

    ju->curtain = p8_fixed_from_int(0);
    ju->has_curtain = 1;
    set_is_shown(ju, 1);
    run_start(&run);

    score_panel_update(ju);
    box = shown_message_box();
    assert(box != NULL);
    assert(ju->last_box == box);
    built = entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX));
    track(&run, box);
    run.hash = fnv(run.hash, draw_hash());

    for (i = 0; i < 80; i++) {
        score_panel_update(ju);
        assert(entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX)) == built);
        tick(&run);
        assert(entity_list_listed(live_entities(), box));
    }
    set_is_shown(ju, 0);
    while (any_listed(&run)) {
        score_panel_update(ju);
        assert(entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX)) == built);
        tick(&run);
    }
    run_check(&run, SCORES_TICKS, SCORES_SEQUENCE_HASH, "scores box follows is_shown");
}

/* The box's layer and duration scale are the class's generated defaults. */
static void test_defaults_match_the_parsed_spec(void) {
    const message_box_t *box;

    reset_world(NULL);
    cart_flush_exit_queue();
    box = year_box(3400);
    assert(box->archetype == &message_box_archetype);
    assert(box->archetype->layer == p8_fixed_from_int(14));
    assert(box->duration_scale == box->archetype->duration_scale);
    assert(box->duration_scale == p8_fixed_from_int(5));
    assert(box->header.class->tags == TAG(TAG_MESSAGE_BOX));
    assert(box->header.state == ENTITY_STATE_NORMAL);
    assert(message_box_ready());
    message_box_shutdown();
    assert(!message_box_ready());
    message_box_init();
}

static int g_keep_open_calls = 0;

static int counting_keep_open(message_box_t *self) {
    (void)self;
    g_keep_open_calls++;
    return 1;
}

/* Built by the score panel or directly, a box is a struct that lands in the
 * live entities, the draw_overlay message list and the message_box tag only;
 * its keep_open is a field the overlay draw calls, and it has no update
 * state for update_entities() to call. */
static void test_message_box_is_a_struct_registered_under_draw_overlay(void) {
    const entity_message_t others[] = {ENTITY_MESSAGE_DRAW, ENTITY_MESSAGE_HIT_TEST, ENTITY_MESSAGE_NEW_YEAR, ENTITY_MESSAGE_REFRESH};
    message_box_keys_t keys = {0};
    const message_box_t *from_score_panel;
    void *generic;
    int out;
    int i;

    reset_world(NULL);
    cart_flush_exit_queue();
    from_score_panel = year_box(3400);
    keys.keep_open = counting_keep_open;
    generic = message_box_new(&keys);

    assert(from_score_panel != NULL);
    assert(generic != NULL);
    assert(shown_message_box() == generic);
    assert(entity_list_listed(live_entities(), generic) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), generic) && entity_list_listed(entities_by_tag(TAG_MESSAGE_BOX), generic));
    for (i = 0; i < 4; i++) {
        assert(!entity_list_listed(entities_by_message(others[i]), from_score_panel) && !entity_list_listed(entities_by_message(others[i]), generic));
    }
    assert(((message_box_t *)generic)->keep_open != NULL);
    out = ((message_box_t *)generic)->keep_open(generic);
    assert(g_keep_open_calls == 1 && out);
    assert(((const entity_t *)generic)->class->states[ENTITY_STATE_NORMAL] == NULL);
}

/* A NULL keep_open key leaves the box without keep_open, as a box built
 * with no key. */
static void test_message_box_nil_keep_open_key_reads_absent(void) {
    message_box_keys_t keys = {0};
    message_box_t *box;

    reset_world(NULL);
    cart_flush_exit_queue();
    keys.keep_open = NULL;
    box = message_box_new(&keys);
    assert(box->keep_open == NULL);
}


/* A box takes a slot; once update_entities() takes it out at the end of its
 * run, emptying the exit queue gives the slot back. */
static void test_box_gives_its_slot_back(void) {
    run_t run;
    int base;

    reset_world(NULL);
    cart_flush_exit_queue();
    base = ui_zone_live(SLOT_KIND_MESSAGE_BOX);
    run_start(&run);
    track(&run, year_box(3400));
    assert(ui_zone_live(SLOT_KIND_MESSAGE_BOX) == base + 1);
    run_out(&run);
    assert(ui_zone_live(SLOT_KIND_MESSAGE_BOX) == base + 1 && "out of the live entities, still read this frame");
    cart_flush_exit_queue();
    assert(ui_zone_live(SLOT_KIND_MESSAGE_BOX) == base);
}

/* Fills the frame zone with garbage after a reset. */
static void scribble_frame_zone(void) {
    unsigned char *reuse;

    frame_zone_reset();
    reuse = allocator_alloc(frame_zone(), 4096);
    memset(reuse, 'X', 4096);
}

/* score_panel_update() makes its texts, rows and their draw_row wrappers in
 * the frame zone: the box draws them on later frames, after the zone is
 * reset and reused. */
static void test_scores_box_keeps_its_articles(void) {
    score_panel_t *ju = game_main_score_panel();
    uint32_t want;

    ju->curtain = p8_fixed_from_int(0);
    ju->has_curtain = 1;
    set_is_shown(ju, 1);
    score_panel_update(ju);
    assert(shown_message_box() != NULL && ju->last_box == shown_message_box());
    want = draw_hash();
    scribble_frame_zone();
    assert(draw_hash() == want);
}

/* score_panel_new_year() makes its year label in the frame zone: the year
 * banner, which every year box shares, keeps its own copy. */
static void test_year_box_keeps_its_label(void) {
    uint32_t want;

    reset_world(NULL);
    cart_flush_exit_queue();
    (void)year_box(3424);
    want = draw_hash();
    scribble_frame_zone();
    assert(draw_hash() == want);
}

/* The largest box the game builds: score_panel_update()'s, with the longest
 * label ("current score" and the longest difficulty) and the longest
 * happiness text, a negative fraction. Its slot holds it without stopping. */
static void test_largest_scores_box_fits_its_slot(void) {
    static button_t challenging;
    score_panel_t *ju = game_main_score_panel();
    const button_t *dk = economy_score_difficulty();

    challenging.name = "challenging";
    economy_score_set_difficulty(&challenging);
    main_economy_typed()->end_status = NULL;
    main_economy_typed()->has_end_status = 0;
    main_economy_typed()->happiness = P8_FIXED_LIT(0x80000007, -32767.9999);
    main_economy_typed()->has_happiness = 1;
    ju->curtain = p8_fixed_from_int(0);
    ju->has_curtain = 1;
    set_is_shown(ju, 1);
    score_panel_update(ju);
    assert(shown_message_box() != NULL && ju->last_box == shown_message_box());
    economy_score_set_difficulty(dk);
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
    cart_init();

    test_year_with_a_message();
    test_year_without_a_message();
    test_two_years_in_a_row();
    test_scores_box_follows_is_shown();
    test_defaults_match_the_parsed_spec();
    test_message_box_is_a_struct_registered_under_draw_overlay();
    test_message_box_nil_keep_open_key_reads_absent();
    test_scores_box_keeps_its_articles();
    test_box_gives_its_slot_back();
    test_year_box_keeps_its_label();
    test_largest_scores_box_fits_its_slot();

    printf("test_message_box: all tests passed\n");
    return 0;
}
