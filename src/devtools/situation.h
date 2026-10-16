#ifndef DEVTOOLS_SITUATION_H
#define DEVTOOLS_SITUATION_H

#include "devtools/pad_metrics.h"
#include "engine/entity_id_type.h"
#include "p8/fixed.h"

enum { SITUATION_LAYOUT_MAX = 64 };

typedef enum {
    SITUATION_TITLE,
    SITUATION_ATTRACT,
    SITUATION_MAP_REST,
    SITUATION_MAP_MOVE,
    SITUATION_SLIPWAY_AIM,
    SITUATION_BUILD,
    SITUATION_MENU,
    SITUATION_TECH_PANEL,
    SITUATION_SCORE_PANEL,
    SITUATION_PAUSE,
    SITUATION_GALAXY_GENERATION,
    SITUATION_RETURN_TO_TITLE,
    SITUATION_GAME_END,
    SITUATION_NEW_YEAR,
    SITUATION_RESEARCH,
    SITUATION_COLONISE,
    SITUATION_STRUCTURE_BUILT,
    SITUATION_SLIPWAY_BUILT,
    SITUATION_PROBE,
    SITUATION_SCREEN_CHANGE,
    SITUATION_COUNT
} situation_t;

typedef enum { SITUATION_STEADY, SITUATION_ONE_OFF } situation_family_t;

typedef struct {
    int count;
    entity_id_t items[SITUATION_LAYOUT_MAX];
    int x[SITUATION_LAYOUT_MAX];
    int y[SITUATION_LAYOUT_MAX];
} situation_layout_t;

typedef struct {
    pad_metrics_mode_t mode;
    int attract;
    int has_economy;
    int game_ended;
    p8_fixed_t year;
    int colonised;
    int slipways;
    int structures;
    int techs;
    int probes;
    int camera_x;
    int camera_y;
    entity_id_t hovered;
    entity_id_t selected;
    situation_layout_t layout;
    int shaking;
} situation_snapshot_t;

/* The label of the state a replay starts from: frame 0's `previous`. */
situation_t situation_start(const situation_snapshot_t *start);
situation_t situation_classify(const situation_snapshot_t *before, const situation_snapshot_t *after,
                               situation_t previous);
const char *situation_name(situation_t situation);
situation_family_t situation_family(situation_t situation);
const char *situation_family_name(situation_family_t family);

#endif
