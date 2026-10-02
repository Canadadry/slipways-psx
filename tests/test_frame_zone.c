/* Lie sous ASan. Chaque cas tourne dans un processus enfant : ce binaire se
 * relance lui-meme avec le nom du cas, et le parent lit son statut et son
 * stderr. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "p8/assets.h"
#include "game/cart.h"
#include "engine/collect.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_list.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/score_panel.h"
#include "p8/str.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "engine/zone_sizes.h"
#include "heap_allocator.h"

#define STDERR_LOG "tmp/test_frame_zone_stderr.txt"

/* volatile : la lecture d'un pointeur perime ne doit pas etre retiree par
 * l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char sink;

static unsigned char *counting_block(void) {
    unsigned char *block = allocator_alloc(frame_zone(), 64);
    for (int i = 0; i < 64; i++) block[i] = (unsigned char)i;
    return block;
}

static void allocate_again(void) {
    for (int i = 0; i < 4; i++) memset(allocator_alloc(frame_zone(), 64), 2, 64);
}

static int case_lazy_init(void) {
    unsigned char *block = counting_block();
    assert(block[63] == 63);
    return 0;
}

static int case_reset_before_first_use(void) {
    frame_zone_reset();
    unsigned char *block = counting_block();
    assert(block[63] == 63);
    return 0;
}

static int case_alive_within_frame(void) {
    unsigned char *block = counting_block();
    allocate_again();
    for (int i = 0; i < 64; i++) assert(block[i] == i && "un bloc de frame vit jusqu'a la remise a zero");
    return 0;
}

static int case_dead_next_frame(void) {
    unsigned char *block = counting_block();
    frame_zone_reset();
    allocate_again();
    sink = block[63];
    return 0;
}

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static void load_cart_and_start(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);
    pico8_cart_load();
    pico8__init();
}

static int case_update60_alive_until_next(void) {
    load_cart_and_start();
    pico8__update60();
    unsigned char *block = counting_block();
    pico8__draw();
    sink = block[63];
    return 0;
}

static int case_update60_resets(void) {
    load_cart_and_start();
    pico8__update60();
    unsigned char *block = counting_block();
    pico8__draw();
    pico8__update60();
    allocate_again();
    sink = block[63];
    return 0;
}

static void start_game(void) {
    load_cart_and_start();
    pico8_eu(NULL);
}

/* Les textes seulement : une case ROW est empruntee jusqu'a la mise en
 * page (pico8_layout.h), un tuple de frame y est mort apres la remise a
 * zero. */
static size_t read_cells(const layout_row_t *lq) {
    size_t sum = 0;
    for (int i = 0; i < lq->items_count; i++) {
        if (lq->items[i].kind == LAYOUT_ROW_CELL_STRING) sum += strlen(lq->items[i].as.text);
    }
    return sum;
}

/* Remet la zone frame a zero apres la construction du panneau, puis relit
 * et dessine chaque bouton, bulle `jb` comprise : rend le nombre de
 * bulles dessinees. */
static int reset_then_draw_panel(void) {
    const entity_list_t *le = pico8_ok()->le;
    int bubbles = 0;

    frame_zone_reset();
    allocate_again();
    for (int i = 0; i < le->count; i++) {
        pico8_js_t *js = (pico8_js_t *)le->items[i];
        vec_t p = js->ra != NULL ? *js->ra : vec_make(0, 0);

        sink = (unsigned char)read_cells(js->lq);
        if (js->h_ != NULL) sink = (unsigned char)read_cells(js->h_);
        if (js->jb != NULL) {
            for (int k = 0; k < js->jb->rows_count; k++) sink = (unsigned char)read_cells(js->jb->rows[k]);
            bubbles++;
        }
        js->m_ = 1;
        pico8_js_kj(&gfx, &mem, js, p);
        pico8_js_ev(&gfx, &mem, js);
    }
    return bubbles;
}

static int case_inspection_panel_rows_survive_reset(void) {
    static production_t kw = {.has_resource = 1};
    planet_t *planet;

    start_game();
    planet = (planet_t *)entities_by_tag("planet")->items[0];
    kw.resource = p8_fixed_from_int(99);
    kw.amount = p8_fixed_from_int(1);
    planet->production = &kw;
    planet_inspection_panel(planet);
    assert(pico8_ok()->le->count >= 2 && "jo:gh: l'icone et la rangee exports");
    reset_then_draw_panel();
    return 0;
}

static int case_invented_lines_survive_reset(void) {
    start_game();
    pointer_list_add(main_economy_typed()->invented, "slipgates");
    pico8_cr();
    assert(reset_then_draw_panel() >= 1 && "cr: la rangee n invented porte sa bulle");
    return 0;
}

static int case_research_project_tooltip_survives_reset(void) {
    planet_t *planet;

    start_game();
    planet = (planet_t *)entities_by_tag("planet")->items[0];
    planet_roll_type(planet);
    (void)planet_left_press(planet);
    assert(reset_then_draw_panel() >= 1 && "jo:fe: un batiment porte la bulle de hp:jb");
    return 0;
}

static int happiness_cells(const pico8_mh_t *mh) {
    int found = 0;
    for (int k = 0; k < mh->items_count; k++) {
        const data_row_t *ld = mh->items[k];
        if (ld->draw_function == NULL || strcmp(ld->draw_function, "draw_row") != 0 || !ld->items[0].is_ref) continue;
        const layout_row_t *lq = ld->items[0].ref;
        for (int i = 0; i < lq->items_count; i++)
            found += lq->items[i].kind == LAYOUT_ROW_CELL_STRING && strcmp(lq->items[i].as.text, "1%") == 0;
    }
    return found;
}

static int case_final_score_happiness_survives_reset(void) {
    economy_t *fm;
    pico8_pj_t *ju;

    start_game();
    fm = main_economy_typed();
    fm->happiness = p8_fixed_from_int(1);
    fm->has_happiness = 1;
    fm->end_status = "final score";
    fm->has_end_status = 1;
    ju = pico8_ju();
    ju->header.class->states[ENTITY_STATE_NORMAL](&ju->header, 0);
    assert(pico8_kc() != NULL && "pj:ph: une partie finie ouvre la boite du score");
    frame_zone_reset();
    allocate_again();
    assert(happiness_cells(pico8_kc()) == 1 && "pj:ph: la case du bonheur garde sa copie de ky");
    return 0;
}

#define LIST_LENGTH 12

static entity_t cells[LIST_LENGTH];

static int is_even_cell(entity_t *e, void *ctx) {
    (void)ctx;
    return (e - cells) % 2 == 0;
}

static entity_list_t even_cells(void) {
    entity_list_t all;
    entity_list_init(&all, frame_zone());
    for (int i = 0; i < LIST_LENGTH; i++) entity_list_add(&all, &cells[i]);
    return entity_list_filter(frame_zone(), &all, is_even_cell, NULL);
}

static int case_list_alive_within_frame(void) {
    entity_list_t even = even_cells();
    assert(even.count == LIST_LENGTH / 2);
    for (int i = 0; i < LIST_LENGTH / 2; i++) assert(even.items[i] == &cells[2 * i]);
    return 0;
}

static int case_list_dead_next_frame(void) {
    entity_list_t even = even_cells();
    frame_zone_reset();
    allocate_again();
    sink = (unsigned char)(even.items[0] == &cells[0]);
    return 0;
}

static char *frame_text(const char *s) {
    return p8_concat2(frame_zone(), s, "");
}

static layout_row_t *row_of_frame_text(const allocator_t *zone, const char *s) {
    layout_row_cell_t cell = layout_row_cell_string(frame_text(s));
    layout_row_keys_t keys = {.items = &cell, .count = 1};
    return layout_row_new(zone, &keys);
}

/* Les textes d'une case viennent de la zone frame : apres la remise a zero,
 * le panneau dessine encore sa copie. */
static int case_panel_texts_survive_reset(void) {
    data_item_t line = {.text = frame_text("bubble")};
    data_row_t bubble_row = {.count = 1, .items = &line};

    load_cart_and_start();
    layout_row_t *row = row_of_frame_text(&heap_allocator, "row");
    layout_row_add(row, layout_row_cell_string(frame_text("added")));
    layout_row_t *copy = layout_row_copy(&heap_allocator, row_of_frame_text(frame_zone(), "copy"));
    tooltip_t *bubble = tooltip_new(&heap_allocator, &bubble_row);
    data_item_t tuple_items[] = {{.text = frame_text("tuple")}, {.number = 5}};
    data_row_t *tuple = (data_row_t *)allocator_alloc(frame_zone(), sizeof *tuple);
    *tuple = (data_row_t){.count = 2, .items = tuple_items};
    layout_row_cell_t tuple_cell = layout_row_cell_row(tuple);
    layout_row_keys_t tuple_keys = {.items = &tuple_cell, .count = 1};
    layout_row_t *of_tuple = layout_row_new(&heap_allocator, &tuple_keys);
    frame_zone_reset();
    allocate_again();
    sink = (unsigned char)(read_cells(row) + read_cells(copy) + read_cells(bubble->rows[0]));
    layout_row_draw_at_default(&gfx, &mem, row, vec_make(0, 0));
    layout_row_draw_at_default(&gfx, &mem, copy, vec_make(0, 0));
    tooltip_draw_at(&gfx, &mem, bubble, 0, 0);
    layout_row_draw_at_default(&gfx, &mem, of_tuple, vec_make(0, 0));
    return 0;
}

static int case_panel_struct_in_zone(void) {
    load_cart_and_start();
    layout_row_t *row = row_of_frame_text(frame_zone(), "row");
    frame_zone_reset();
    allocate_again();
    sink = (unsigned char)row->items_count;
    return 0;
}

static int case_bubble_struct_in_zone(void) {
    load_cart_and_start();
    tooltip_t *bubble = tooltip_new(frame_zone(), NULL);
    frame_zone_reset();
    allocate_again();
    sink = (unsigned char)bubble->rows_count;
    return 0;
}

static int case_full(void) {
    allocator_alloc(frame_zone(), FRAME_ZONE_BYTES / 2);
    allocator_alloc(frame_zone(), FRAME_ZONE_BYTES / 2 + 1);
    return 0;
}

static const struct {
    const char *name;
    int (*run)(void);
} CASES[] = {
    {"lazy_init", case_lazy_init},
    {"reset_before_first_use", case_reset_before_first_use},
    {"alive_within_frame", case_alive_within_frame},
    {"dead_next_frame", case_dead_next_frame},
    {"update60_alive_until_next", case_update60_alive_until_next},
    {"update60_resets", case_update60_resets},
    {"list_alive_within_frame", case_list_alive_within_frame},
    {"list_dead_next_frame", case_list_dead_next_frame},
    {"full", case_full},
    {"inspection_panel_rows_survive_reset", case_inspection_panel_rows_survive_reset},
    {"invented_lines_survive_reset", case_invented_lines_survive_reset},
    {"research_project_tooltip_survives_reset", case_research_project_tooltip_survives_reset},
    {"ky_happiness_survives_reset", case_final_score_happiness_survives_reset},
    {"panel_texts_survive_reset", case_panel_texts_survive_reset},
    {"panel_struct_in_zone", case_panel_struct_in_zone},
    {"bubble_struct_in_zone", case_bubble_struct_in_zone},
};

#define CASE_COUNT (sizeof CASES / sizeof CASES[0])

/* Tampon alloue, termine par un nul, que l'appelant libere. */
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

/* Rend le statut brut de system() ; *err recoit le stderr de l'enfant, que
 * l'appelant libere. */
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
        fprintf(stderr, "%s: attendu propre, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "le cas ne doit ni planter ni ecrire sur stderr");
    }
    free(err);
}

static void expect_poison_crash(const char *name) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) == 0 || strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s: attendu use-after-poison, sortie %d\n%s", name, WEXITSTATUS(status), err);
        assert(0 && "un bloc de frame lu a la frame suivante doit planter sous ASan");
    }
    free(err);
}

static void expect_fatal(const char *name, const char *message) {
    char *err;
    int status = run_child(name, &err);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: attendu sortie 3 et \"%s\", sortie %d\n%s",
                name, message, WEXITSTATUS(status), err);
        assert(0 && "la zone frame pleine doit appeler pico8_fatal");
    }
    free(err);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < CASE_COUNT; i++)
            if (strcmp(argv[1], CASES[i].name) == 0) return CASES[i].run();
        fprintf(stderr, "test_frame_zone: cas inconnu %s\n", argv[1]);
        return 2;
    }
    self_path = argv[0];
    expect_clean("lazy_init");
    expect_clean("reset_before_first_use");
    expect_clean("alive_within_frame");
    expect_poison_crash("dead_next_frame");
    expect_clean("update60_alive_until_next");
    expect_poison_crash("update60_resets");
    expect_clean("list_alive_within_frame");
    expect_poison_crash("list_dead_next_frame");
    expect_fatal("full", "zone frame pleine, limite 262144 octets");
    expect_clean("inspection_panel_rows_survive_reset");
    expect_clean("invented_lines_survive_reset");
    expect_clean("research_project_tooltip_survives_reset");
    expect_clean("ky_happiness_survives_reset");
    expect_clean("panel_texts_survive_reset");
    expect_poison_crash("panel_struct_in_zone");
    expect_poison_crash("bubble_struct_in_zone");
    remove(STDERR_LOG);
    printf("test_frame_zone: all tests passed\n");
    return 0;
}
