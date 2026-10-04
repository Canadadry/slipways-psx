/* make cursor-labels-test: the cursor label of interfaces built by hand,
 * entities of real classes on chosen layers, then tmp/replay with LABELS on
 * a title-screen fixture. Temporary (plan 12). */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devtools/cursor_label.h"
#include "game/background.h"
#include "game/help.h"
#include "game/menu.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/score_panel.h"
#include "game/structure_icon.h"
#include "game/ui.h"
#include "game/ui_button.h"

enum { MAX_PER_LAYER = 4 };

typedef struct {
    ui_t ui;
    entity_t *items[UI_HOVER_LAYERS][MAX_PER_LAYER];
} hand_ui_t;

static void hand_ui_init(hand_ui_t *h) {
    int layer;
    memset(h, 0, sizeof *h);
    h->ui.hovered_by_layer_own.layer_count = UI_HOVER_LAYERS;
    for (layer = 0; layer < UI_HOVER_LAYERS; layer++) {
        h->ui.hovered_by_layer_own.layers[layer].items = h->items[layer];
        h->ui.hovered_by_layer_own.layers[layer].capacity = MAX_PER_LAYER;
    }
    h->ui.hovered_by_layer = &h->ui.hovered_by_layer_own;
}

static void hand_ui_put(hand_ui_t *h, int layer, entity_t *e) {
    entity_list_t *list = &h->ui.hovered_by_layer_own.layers[layer];
    assert(list->count < MAX_PER_LAYER);
    list->items[list->count++] = e;
}

static entity_t of_class(const entity_class_t *c) {
    entity_t e;
    entity_init_header(&e, c);
    return e;
}

static button_t button_with(int layer, button_fn_t on_click) {
    button_t b;
    memset(&b, 0, sizeof b);
    entity_init_header(&b.header, button_class());
    b.layer = p8_fixed_from_int(layer);
    b.on_click = on_click;
    b.has_on_click = on_click != NULL;
    return b;
}

static void expect(const ui_t *ui, const char *label) {
    const char *got = cursor_label_read(ui);
    assert(got != NULL && strcmp(got, label) == 0);
}

typedef struct {
    const entity_class_t *(*class_of)(void);
    const char *label;
} named_class_t;

static const named_class_t NAMED[] = {
    {planet_class, "planet"},       {structure_class, "structure"},
    {trading_hub_class, "trading_hub"}, {processor_class, "processor"},
    {ascension_class, "ascension"}, {protostar_class, "protostar"},
    {synthesizer_class, "synthesizer"}, {burst_class, "burst"},
    {slipgate_class, "slipgate"},   {infraspace_gate_class, "infraspace_gate"},
    {modal_backdrop_class, "modal_backdrop"}, {help_screen_class, "help_screen"},
    {structure_icon_class, "structure_icon"}, {background_class, "carte"},
};

#define FIXTURE "tests/fixtures/title_click_reasonable.json"
#define LABELS_OUT "tmp/test_cursor_label_labels.txt"
#define CLAMP_OUT "tmp/test_cursor_label_clamp.txt"
#define LOG_OUT "tmp/test_cursor_label_log.txt"
#define DUMP_WITH "tmp/test_cursor_label_dump_with.txt"
#define DUMP_WITHOUT "tmp/test_cursor_label_dump_without.txt"

/* Allocated buffer, freed by the caller; NUL-terminated. */
static char *read_whole_file(const char *path, size_t *out_len) {
    FILE *file = fopen(path, "rb");
    long size;
    char *buf;
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    size = ftell(file);
    assert(size >= 0);
    rewind(file);
    buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    fclose(file);
    buf[size] = '\0';
    *out_len = (size_t)size;
    return buf;
}

/* Line `n` (from 0) of `text`, copied into a static buffer. */
static const char *nth_line(const char *text, int n) {
    static char line[64];
    size_t len;
    while (n-- > 0) {
        text = strchr(text, '\n');
        assert(text != NULL);
        text++;
    }
    len = strcspn(text, "\n");
    assert(len < sizeof line);
    memcpy(line, text, len);
    line[len] = '\0';
    return line;
}

static int replayed_frames(void) {
    size_t len;
    int frames = -1;
    char *log = read_whole_file(LOG_OUT, &len);
    assert(sscanf(log, "replay: %d frame(s)", &frames) == 1);
    free(log);
    return frames;
}

static void test_driver_writes_one_label_per_frame(void) {
    size_t labels_len, with_len, without_len, clamp_len, i;
    int lines = 0;
    char *labels, *with, *without;
    remove(LABELS_OUT);
    remove(CLAMP_OUT);
    assert(system("LABELS=" LABELS_OUT " CAMERA_CLAMP=" CLAMP_OUT " ./tmp/replay " FIXTURE " /dev/null " DUMP_WITH " > " LOG_OUT) == 0);
    assert(system("./tmp/replay " FIXTURE " /dev/null " DUMP_WITHOUT " > /dev/null") == 0);
    labels = read_whole_file(LABELS_OUT, &labels_len);
    for (i = 0; i < labels_len; i++) {
        lines += labels[i] == '\n';
    }
    assert(labels_len > 0 && labels[labels_len - 1] == '\n');
    assert(lines == replayed_frames());
    assert(strcmp(nth_line(labels, 0), "carte") == 0);
    assert(strcmp(nth_line(labels, 49), "carte") == 0);
    assert(strcmp(nth_line(labels, 50), "button:difficulte") == 0 && "frame 50 moves onto a difficulty row: its own hit test, not the previous one");
    assert(strcmp(nth_line(labels, 60), "button:difficulte") == 0 &&
           "frame 60 clicks the difficulty: the ui that dispatched it, not the one reset_world() built during it");
    assert(strcmp(nth_line(labels, 61), "carte") == 0);
    free(read_whole_file(CLAMP_OUT, &clamp_len));
    assert(clamp_len == 0 && "the title screen's camera stays at (192,192)");
    with = read_whole_file(DUMP_WITH, &with_len);
    without = read_whole_file(DUMP_WITHOUT, &without_len);
    assert(with_len == without_len && memcmp(with, without, with_len) == 0 && "LABELS must not change the replay");
    free(labels);
    free(with);
    free(without);
    remove(LABELS_OUT);
    remove(CLAMP_OUT);
    remove(LOG_OUT);
    remove(DUMP_WITH);
    remove(DUMP_WITHOUT);
}

static void expect_button(hand_ui_t *h, button_t *b, const char *label) {
    hand_ui_init(h);
    hand_ui_put(h, 8, &b->header);
    expect(&h->ui, label);
}

static void test_buttons_named_by_their_fields(hand_ui_t *h) {
    button_t b;

    b = button_with(9, NULL);
    expect_button(h, &b, "button:monde");
    b = button_with(0, help_start_game_field_value());
    expect_button(h, &b, "button:monde");

    b = button_with(11, NULL);
    b.technology = &technologies_table[0];
    expect_button(h, &b, "button:technologies");

    score_panel_init();
    b = button_with(10, NULL);
    expect_button(h, &b, "button:technologies");
    score_panel_shutdown();
    expect_button(h, &b, "button");

    b = button_with(11, help_start_game_field_value());
    expect_button(h, &b, "button:difficulte");

    b = button_with(pause_menu_tail.back_color, score_panel_init_field_value());
    expect_button(h, &b, "button:pause");
    b = button_with(pause_menu_tail.back_color, NULL);
    expect_button(h, &b, "button:pause");

    b = button_with(15, score_panel_init_field_value());
    expect_button(h, &b, "button:fin_de_partie");
}

static vec_t at(int x, int y) {
    return vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
}

static void test_camera_clamp_zone(void) {
    assert(!cursor_label_camera_clamped(at(192, 192)));
    assert(!cursor_label_camera_clamped(at(-47, 315)));
    assert(!cursor_label_camera_clamped(vec_make(p8_fixed_sub(p8_fixed_from_int(-48), -1), p8_fixed_sub(p8_fixed_from_int(316), 1))));
    assert(cursor_label_camera_clamped(at(-48, 0)));
    assert(cursor_label_camera_clamped(at(-64, 192)));
    assert(cursor_label_camera_clamped(at(0, 316)));
    assert(cursor_label_camera_clamped(at(320, 320)));
}

static void test_driver_lists_the_clamped_camera_frames(void) {
    size_t len;
    char *clamp;
    remove(CLAMP_OUT);
    assert(system("LABELS=" LABELS_OUT " CAMERA_CLAMP=" CLAMP_OUT " ./tmp/replay tests/e2e/camera_drag_dpad.json /dev/null " DUMP_WITH
                  " > " LOG_OUT) == 0);
    clamp = read_whole_file(CLAMP_OUT, &len);
    assert(strcmp(clamp, "15\n16\n17\n18\n") == 0 && "the d-pad drives the camera to x <= -48 on these frames only");
    free(clamp);
    remove(CLAMP_OUT);
    remove(LABELS_OUT);
    remove(LOG_OUT);
    remove(DUMP_WITH);
}

int main(void) {
    hand_ui_t h;
    entity_t background = of_class(background_class());
    entity_t planet = of_class(planet_class());

    expect(NULL, "carte");

    hand_ui_init(&h);
    h.ui.hovered_by_layer = NULL;
    expect(&h.ui, "carte");

    hand_ui_init(&h);
    expect(&h.ui, "carte");

    hand_ui_put(&h, 0, &background);
    expect(&h.ui, "carte");

    hand_ui_put(&h, 3, &planet);
    expect(&h.ui, "planet");

    button_t plain = button_with(11, NULL);
    hand_ui_put(&h, 5, &plain.header);
    expect(&h.ui, "button");

    hand_ui_init(&h);
    hand_ui_put(&h, 7, &planet);
    hand_ui_put(&h, 7, &plain.header);
    expect(&h.ui, "planet");

    {
        static const char *const galaxy[] = {"button:galaxy_menu_1", "button:galaxy_menu_2", "button:galaxy_menu_3",
                                             "button:galaxy_menu_4"};
        int i;
        for (i = 1; i <= 4; i++) {
            button_t menu = button_with(11, galaxy_menu_on_click(i));
            hand_ui_init(&h);
            hand_ui_put(&h, 2, &planet);
            hand_ui_put(&h, 9, &menu.header);
            expect(&h.ui, galaxy[i - 1]);
        }
    }

    test_buttons_named_by_their_fields(&h);

    {
        size_t i;
        for (i = 0; i < sizeof NAMED / sizeof NAMED[0]; i++) {
            entity_t e = of_class(NAMED[i].class_of());
            hand_ui_init(&h);
            hand_ui_put(&h, 1, &background);
            hand_ui_put(&h, 4, &e);
            expect(&h.ui, NAMED[i].label);
        }
    }

    {
        hand_ui_t before;
        button_t menu = button_with(11, galaxy_menu_on_click(2));
        hand_ui_init(&h);
        hand_ui_put(&h, 0, &background);
        hand_ui_put(&h, 6, &planet);
        hand_ui_put(&h, 6, &menu.header);
        button_t menu_before;
        entity_t planet_before;
        memcpy(&before, &h, sizeof h);
        memcpy(&menu_before, &menu, sizeof menu);
        memcpy(&planet_before, &planet, sizeof planet);
        expect(&h.ui, "planet");
        assert(memcmp(&before, &h, sizeof h) == 0);
        assert(memcmp(&menu_before, &menu, sizeof menu) == 0);
        assert(memcmp(&planet_before, &planet, sizeof planet) == 0);
    }

    test_camera_clamp_zone();
    test_driver_writes_one_label_per_frame();
    test_driver_lists_the_clamped_camera_frames();
    return 0;
}
