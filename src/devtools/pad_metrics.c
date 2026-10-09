#include "devtools/pad_metrics.h"

#include "engine/allocator.h"
#include "engine/entity_id.h"
#include "engine/entity_list.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/pad_nav.h"
#include "game/pad_panel.h"
#include "game/pad_target.h"
#include "game/scroll.h"

#include <assert.h>
#include <string.h>

static const char *const BUTTON_NAMES[PAD_METRICS_BUTTON_COUNT] = {
    "up", "down", "left", "right", "cross", "circle", "square", "triangle", "l1", "r1", "start", "select",
};

static const char *const MODE_NAMES[PAD_METRICS_MODE_COUNT] = {
    "title", "pause", "placement", "bar", "tech", "score", "menu", "jump", "aim", "slipway",
};

void pad_metrics_init(pad_metrics_t *metrics, bump_zone_t *scratch) {
    assert(metrics != NULL && scratch != NULL);
    memset(metrics, 0, sizeof *metrics);
    metrics->scratch = scratch;
}

pad_metrics_flags_t pad_metrics_flags_read(const input_step_t *step) {
    const pad_nav_t *nav = &step->pad_nav;
    pad_metrics_flags_t flags;
    flags.has_economy = main_economy_typed() != NULL;
    flags.pause_shown = pause_menu_shown();
    flags.placing = pad_nav_placing(nav) != NULL;
    flags.build_bar_open = pad_nav_build_bar_open(nav);
    flags.tech_open = pad_nav_tech_open(nav);
    flags.score_open = pad_nav_score_open(nav);
    flags.panel_open = pad_nav_panel_open();
    flags.aim_mode = pad_nav_aim_mode(nav);
    flags.slipway_started = pad_nav_slipway_start(nav) != NULL;
    return flags;
}

pad_metrics_mode_t pad_metrics_classify(const pad_metrics_flags_t *flags) {
    if (!flags->has_economy) return PAD_METRICS_MODE_TITLE;
    if (flags->pause_shown) return PAD_METRICS_MODE_PAUSE;
    if (flags->placing) return PAD_METRICS_MODE_PLACEMENT;
    if (flags->build_bar_open) return PAD_METRICS_MODE_BAR;
    if (flags->tech_open) return PAD_METRICS_MODE_TECH;
    if (flags->score_open) return PAD_METRICS_MODE_SCORE;
    if (flags->panel_open) return PAD_METRICS_MODE_MENU;
    if (flags->aim_mode) return PAD_METRICS_MODE_AIM;
    if (flags->slipway_started) return PAD_METRICS_MODE_SLIPWAY;
    return PAD_METRICS_MODE_JUMP;
}

pad_metrics_mode_t pad_metrics_mode(const input_step_t *step) {
    pad_metrics_flags_t flags = pad_metrics_flags_read(step);
    return pad_metrics_classify(&flags);
}

const char *pad_metrics_mode_name(pad_metrics_mode_t mode) {
    assert(mode >= 0 && mode < PAD_METRICS_MODE_COUNT);
    return MODE_NAMES[mode];
}

const char *pad_metrics_button_name(int index) {
    assert(index >= 0 && index < PAD_METRICS_BUTTON_COUNT);
    return BUTTON_NAMES[index];
}

static void camera_point(int *x, int *y) {
    const camera_t *camera = main_camera_typed();
    if (camera == NULL) {
        *x = 0;
        *y = 0;
        return;
    }
    vec_t at = camera_pos_before_shake(camera);
    *x = p8_fixed_flr_int(at.x);
    *y = p8_fixed_flr_int(at.y);
}

static int count_of(const entity_list_t *list) {
    return list != NULL ? list->count : 0;
}

static entity_id_t id_or_none(const entity_t *e) {
    return e != NULL ? entity_id_of(e) : ENTITY_ID_NONE;
}

pad_metrics_view_t pad_metrics_view_read(const input_step_t *step) {
    const pad_nav_t *nav = &step->pad_nav;
    const economy_t *economy = main_economy_typed();
    pad_metrics_view_t view;
    memset(&view, 0, sizeof view);
    view.mode = pad_metrics_mode(step);
    view.hovered = id_or_none(pad_nav_hovered(nav));
    view.slipway_start = id_or_none(pad_nav_slipway_start(nav));
    view.slipway_target = id_or_none(pad_nav_slipway_target(nav));
    view.build_icon = id_or_none(pad_nav_build_icon(nav));
    view.placing = id_or_none(pad_nav_placing(nav));
    view.focus = id_or_none(pad_nav_focus(nav));
    camera_point(&view.camera_x, &view.camera_y);
    view.panel_open = pad_nav_panel_open();
    view.box_open = economy != NULL && pad_panel_box_open();
    view.message_box = shown_message_box() != NULL;
    if (economy != NULL) {
        view.credits = economy->credits;
        view.year = economy->year;
        view.year_ticks = economy->year_ticks;
        view.techs = economy->invented.count;
    }
    view.nodes = count_of(entities_by_tag(TAG_NODE));
    view.probes = count_of(entities_by_tag(TAG_PROBE));
    return view;
}

int pad_metrics_navigation_changed(const pad_metrics_view_t *a, const pad_metrics_view_t *b) {
    return a->mode != b->mode || !entity_id_equal(a->hovered, b->hovered) ||
           !entity_id_equal(a->slipway_start, b->slipway_start) ||
           !entity_id_equal(a->slipway_target, b->slipway_target) || !entity_id_equal(a->build_icon, b->build_icon) ||
           !entity_id_equal(a->placing, b->placing) || !entity_id_equal(a->focus, b->focus) ||
           a->camera_x != b->camera_x || a->camera_y != b->camera_y || a->panel_open != b->panel_open ||
           a->box_open != b->box_open || a->message_box != b->message_box;
}

int pad_metrics_view_changed(const pad_metrics_view_t *a, const pad_metrics_view_t *b) {
    return pad_metrics_navigation_changed(a, b) || a->credits != b->credits || a->year != b->year ||
           a->year_ticks != b->year_ticks || a->nodes != b->nodes || a->probes != b->probes || a->techs != b->techs;
}

int pad_metrics_hold_step(pad_metrics_hold_t *hold, pad_metrics_mode_t mode, int buttons) {
    int repeats = 0;
    for (int i = 0; i < PAD_JUMP_DIRECTIONS; i++) {
        int direction = pad_jump_direction(i);
        hold->held[i] = (buttons & direction) ? hold->held[i] + 1 : 0;
        int past = hold->held[i] - PAD_JUMP_REPEAT_DELAY;
        if ((mode == PAD_METRICS_MODE_JUMP || mode == PAD_METRICS_MODE_SLIPWAY) && past > 0 &&
            (past - 1) % PAD_JUMP_REPEAT_PERIOD == 0) {
            repeats |= direction;
        }
    }
    return repeats;
}

int pad_metrics_cone_empty(const pad_metrics_t *metrics, const entity_t *from, int direction) {
    bump_zone_reset(metrics->scratch);
    allocator_t zone = allocator_from_bump_zone(metrics->scratch);
    entity_list_t candidates = pad_jump_candidates(&zone, entities_by_tag(TAG_NODE));
    return from == NULL || pad_target_pick(from, direction, &candidates) == NULL;
}

void pad_metrics_observe(pad_metrics_t *metrics, const input_step_t *step) {
    metrics->view = pad_metrics_view_read(step);
}

const pad_metrics_view_t *pad_metrics_view(const pad_metrics_t *metrics) {
    return &metrics->view;
}

int pad_metrics_pressed(const pad_metrics_t *metrics, int buttons) {
    return buttons & ~metrics->previous_buttons;
}

static int bit_count(int bits) {
    int n = 0;
    for (; bits != 0; bits &= bits - 1) n++;
    return n;
}

void pad_metrics_record(pad_metrics_t *metrics, const input_step_t *step, int buttons, int cheat) {
    pad_metrics_mode_t mode = metrics->view.mode;
    int pressed = pad_metrics_pressed(metrics, buttons);
    metrics->frames++;
    metrics->cheat_activations += cheat;
    metrics->mode_frames[mode]++;
    for (int b = 0; b < PAD_METRICS_BUTTON_COUNT; b++) {
        if (pressed & (1 << b)) metrics->presses[mode][b]++;
    }
    metrics->hold_repeats += bit_count(pad_metrics_hold_step(&metrics->hold, mode, buttons));
    metrics->previous_buttons = buttons;
    metrics->view = pad_metrics_view_read(step);
}

void pad_metrics_count_required_noop(pad_metrics_t *metrics) {
    metrics->required_noops++;
}

int pad_metrics_mode_frames(const pad_metrics_t *metrics, pad_metrics_mode_t mode) {
    assert(mode >= 0 && mode < PAD_METRICS_MODE_COUNT);
    return metrics->mode_frames[mode];
}

int pad_metrics_cheat_activations(const pad_metrics_t *metrics) {
    return metrics->cheat_activations;
}

void pad_metrics_write_totals(const pad_metrics_t *metrics, FILE *out, const char *prefix) {
    int frames = metrics->frames;
    int aim = metrics->mode_frames[PAD_METRICS_MODE_AIM];
    int per_mille = frames > 0 ? (1000 * aim + frames / 2) / frames : 0;
    fprintf(out, "%sframes %d\n", prefix, frames);
    for (int m = 0; m < PAD_METRICS_MODE_COUNT; m++) {
        if (metrics->mode_frames[m] > 0) {
            fprintf(out, "%smode %s frames %d\n", prefix, MODE_NAMES[m], metrics->mode_frames[m]);
        }
    }
    fprintf(out, "%saim share %d.%d%% (%d of %d frames)\n", prefix, per_mille / 10, per_mille % 10, aim, frames);
    for (int m = 0; m < PAD_METRICS_MODE_COUNT; m++) {
        for (int b = 0; b < PAD_METRICS_BUTTON_COUNT; b++) {
            if (metrics->presses[m][b] > 0) {
                fprintf(out, "%spresses %s %s %d\n", prefix, MODE_NAMES[m], BUTTON_NAMES[b], metrics->presses[m][b]);
            }
        }
    }
    fprintf(out, "%shold repeats %d\n", prefix, metrics->hold_repeats);
    fprintf(out, "%srequired no-ops %d\n", prefix, metrics->required_noops);
    fprintf(out, "%scheat activations %d\n", prefix, metrics->cheat_activations);
}
