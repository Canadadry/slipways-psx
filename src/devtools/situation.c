#include "devtools/situation.h"

#include <assert.h>

static const char *const NAMES[SITUATION_COUNT] = {
    [SITUATION_TITLE] = "title",
    [SITUATION_ATTRACT] = "attract",
    [SITUATION_MAP_REST] = "map-rest",
    [SITUATION_MAP_MOVE] = "map-move",
    [SITUATION_SLIPWAY_AIM] = "slipway-aim",
    [SITUATION_BUILD] = "build",
    [SITUATION_MENU] = "menu",
    [SITUATION_TECH_PANEL] = "tech-panel",
    [SITUATION_SCORE_PANEL] = "score-panel",
    [SITUATION_PAUSE] = "pause",
    [SITUATION_GALAXY_GENERATION] = "galaxy-generation",
    [SITUATION_RETURN_TO_TITLE] = "return-to-title",
    [SITUATION_GAME_END] = "game-end",
    [SITUATION_NEW_YEAR] = "new-year",
    [SITUATION_RESEARCH] = "research",
    [SITUATION_COLONISE] = "colonise",
    [SITUATION_STRUCTURE_BUILT] = "structure-built",
    [SITUATION_SLIPWAY_BUILT] = "slipway-built",
    [SITUATION_PROBE] = "probe",
    [SITUATION_SCREEN_CHANGE] = "screen-change",
};

static int map_moved(const situation_snapshot_t *before, const situation_snapshot_t *after) {
    return before->camera_x != after->camera_x || before->camera_y != after->camera_y ||
           !entity_id_equal(before->hovered, after->hovered) || !entity_id_equal(before->selected, after->selected);
}

static situation_t steady(const situation_snapshot_t *before, const situation_snapshot_t *after) {
    if (after->attract) return SITUATION_ATTRACT;
    switch (after->mode) {
    case PAD_METRICS_MODE_TITLE: return SITUATION_TITLE;
    case PAD_METRICS_MODE_PAUSE: return SITUATION_PAUSE;
    case PAD_METRICS_MODE_PLACEMENT:
    case PAD_METRICS_MODE_BAR: return SITUATION_BUILD;
    case PAD_METRICS_MODE_TECH: return SITUATION_TECH_PANEL;
    case PAD_METRICS_MODE_SCORE: return SITUATION_SCORE_PANEL;
    case PAD_METRICS_MODE_MENU: return SITUATION_MENU;
    case PAD_METRICS_MODE_SLIPWAY: return SITUATION_SLIPWAY_AIM;
    case PAD_METRICS_MODE_JUMP:
    case PAD_METRICS_MODE_AIM: return map_moved(before, after) ? SITUATION_MAP_MOVE : SITUATION_MAP_REST;
    case PAD_METRICS_MODE_COUNT: break;
    }
    assert(!"situation: snapshot mode out of range");
    return SITUATION_TITLE;
}

static int layout_moved(const situation_layout_t *before, const situation_layout_t *after) {
    assert(before->count >= 0 && before->count <= SITUATION_LAYOUT_MAX);
    assert(after->count >= 0 && after->count <= SITUATION_LAYOUT_MAX);
    if (before->count != after->count) return 1;
    for (int i = 0; i < after->count; i++) {
        if (!entity_id_equal(before->items[i], after->items[i]) || before->x[i] != after->x[i] ||
            before->y[i] != after->y[i]) {
            return 1;
        }
    }
    return 0;
}

/* Returns SITUATION_COUNT when the frame has no event. */
static situation_t event(const situation_snapshot_t *before, const situation_snapshot_t *after) {
    if (!before->has_economy && after->has_economy) return SITUATION_GALAXY_GENERATION;
    if (before->has_economy && !after->has_economy) return SITUATION_RETURN_TO_TITLE;
    if (!before->game_ended && after->game_ended) return SITUATION_GAME_END;
    if (after->year > before->year) return SITUATION_NEW_YEAR;
    if (after->techs > before->techs) return SITUATION_RESEARCH;
    if (after->colonised > before->colonised) return SITUATION_COLONISE;
    if (after->structures > before->structures) return SITUATION_STRUCTURE_BUILT;
    if (after->slipways > before->slipways) return SITUATION_SLIPWAY_BUILT;
    if (after->probes != before->probes) return SITUATION_PROBE;
    if (after->mode != before->mode || after->attract != before->attract) return SITUATION_SCREEN_CHANGE;
    return SITUATION_COUNT;
}

situation_t situation_start(const situation_snapshot_t *start) {
    return steady(start, start);
}

situation_t situation_classify(const situation_snapshot_t *before, const situation_snapshot_t *after,
                               situation_t previous) {
    assert(previous >= 0 && previous < SITUATION_COUNT);
    situation_t happened = event(before, after);
    if (happened != SITUATION_COUNT) return happened;
    int settling = layout_moved(&before->layout, &after->layout) || after->shaking;
    if (situation_family(previous) == SITUATION_ONE_OFF && settling) return previous;
    return steady(before, after);
}

const char *situation_name(situation_t situation) {
    assert(situation >= 0 && situation < SITUATION_COUNT);
    return NAMES[situation];
}

situation_family_t situation_family(situation_t situation) {
    assert(situation >= 0 && situation < SITUATION_COUNT);
    return situation >= SITUATION_GALAXY_GENERATION ? SITUATION_ONE_OFF : SITUATION_STEADY;
}

const char *situation_family_name(situation_family_t family) {
    return family == SITUATION_ONE_OFF ? "one-off" : "steady";
}
