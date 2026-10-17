#include "devtools/situation.h"

#include "engine/entity_geom.h"
#include "engine/entity_id.h"
#include "engine/entity_list.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "game/pad_jump.h"
#include "game/pad_panel.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/scroll.h"

#include <assert.h>
#include <string.h>

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

int situation_layout_moved(const situation_layout_t *before, const situation_layout_t *after) {
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

static entity_id_t id_or_none(const entity_t *e) {
    return e != NULL ? entity_id_of(e) : ENTITY_ID_NONE;
}

situation_layout_t situation_layout_read(const pad_nav_t *nav) {
    situation_layout_t layout;
    memset(&layout, 0, sizeof layout);
    const entity_t *items[SITUATION_LAYOUT_MAX];
    const entity_t *focus = pad_nav_focus(nav);
    if (focus == NULL) return layout;
    items[layout.count++] = focus;
    for (int head = 0; head < layout.count; head++) {
        layout.items[head] = entity_id_of(items[head]);
        pad_panel_focus_point(items[head], &layout.x[head], &layout.y[head]);
        for (int d = 0; d < PAD_JUMP_DIRECTIONS && layout.count < SITUATION_LAYOUT_MAX; d++) {
            const entity_t *next = pad_nav_neighbor(items[head], pad_jump_direction(d));
            int seen = next == NULL;
            for (int i = 0; i < layout.count && !seen; i++) seen = items[i] == next;
            if (!seen) items[layout.count++] = next;
        }
    }
    return layout;
}

static int count_of(const entity_list_t *list) {
    return list != NULL ? list->count : 0;
}

static int colonised_planets(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    int count = 0;
    for (int i = 0; i < count_of(planets); i++) count += planet_is_colonized((const planet_t *)planets->items[i]);
    return count;
}

/* A protostar is a collapsed planet, not a build; a structure still in placement is not built yet. */
static int built_structures(void) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    int count = 0;
    for (int i = 0; i < count_of(nodes); i++) {
        const entity_t *e = nodes->items[i];
        count += (e->class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && e->class != protostar_class() &&
                 e->state != ENTITY_STATE_PLACING;
    }
    return count;
}

static int placed_slipways(void) {
    const entity_list_t *slipways = entities_by_tag(TAG_SLIPWAY);
    int count = 0;
    for (int i = 0; i < count_of(slipways); i++) count += slipways->items[i]->state != ENTITY_STATE_PLACING;
    return count;
}

/* Off screen, planet_draw skips the planet and its shake never runs down. */
static int any_planet_shaking(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; i < count_of(planets); i++) {
        if (planets->items[i]->shake > 0 && entity_geom_is_on_screen(planets->items[i])) return 1;
    }
    return 0;
}

situation_snapshot_t situation_snapshot_read(const input_step_t *step) {
    assert(step != NULL);
    const pad_nav_t *nav = &step->pad_nav;
    const economy_t *economy = main_economy_typed();
    const camera_t *camera = main_camera_typed();
    situation_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof snapshot);
    snapshot.mode = pad_metrics_mode(step);
    snapshot.attract = input_step_attract_phase(step) == ATTRACT_PHASE_PLAYING;
    snapshot.has_economy = economy != NULL;
    if (economy != NULL) {
        snapshot.game_ended = economy->has_end_status && economy->end_status != NULL;
        snapshot.year = economy->year;
        snapshot.techs = economy->invented.count;
    }
    snapshot.colonised = colonised_planets();
    snapshot.slipways = placed_slipways();
    snapshot.structures = built_structures();
    snapshot.probes = count_of(entities_by_tag(TAG_PROBE));
    if (camera != NULL) {
        vec_t at = camera_pos_before_shake(camera);
        snapshot.camera_x = p8_fixed_flr_int(at.x);
        snapshot.camera_y = p8_fixed_flr_int(at.y);
    }
    snapshot.hovered = id_or_none(pad_nav_hovered(nav));
    snapshot.selected = id_or_none(pad_nav_focus(nav));
    snapshot.layout = situation_layout_read(nav);
    snapshot.shaking = (camera != NULL && camera->header.shake > 0) || any_planet_shaking();
    return snapshot;
}

situation_t situation_start(const situation_snapshot_t *start) {
    return steady(start, start);
}

situation_t situation_classify(const situation_snapshot_t *before, const situation_snapshot_t *after,
                               situation_t previous) {
    assert(previous >= 0 && previous < SITUATION_COUNT);
    situation_t happened = event(before, after);
    if (happened != SITUATION_COUNT) return happened;
    int settling = situation_layout_moved(&before->layout, &after->layout) || after->shaking;
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
