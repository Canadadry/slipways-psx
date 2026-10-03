#ifndef GAME_GAME_DATA_H
#define GAME_GAME_DATA_H

#include "p8/fixed.h"
#include "engine/geom.h"
#include "game/map_tables.h"
#include "engine/vec.h"

enum {
    UPKEEP_BY_TIER_COUNT = 4,
    PARTNER_BONUS_BY_TIER_COUNT = 4,
    DEVELOPMENT_SCORE_BY_TIER_COUNT = 4,
    HAPPINESS_BY_TIER_COUNT = 4,
    TIER_THRESHOLDS_COUNT = 4,
    LAYER_FADES_COUNT = 15,
    PERIOD_NAMES_COUNT = 15,
    PLANET_TYPES_COUNT = 11,
    GALAXY_MENU_COUNT = 4,
    PAUSE_MENU_COUNT = 4,
    DIFFICULTY_OPTIONS_COUNT = 4,
    SCORE_ROWS_COUNT = 12,
    PLANET_MOOD_ROWS_COUNT = 4
};

/* items is NULL when count is 0. */
typedef struct {
    int count;
    const int *items;
} int_seq_t;

typedef struct str_seq {
    int count;
    const char *const *items;
} str_seq_t;

typedef struct {
    int count;
    const vec_t *items;
} vec_seq_t;

/* A kind's required_tech, sprite, state and cost as its chain of defaults
 * resolves them: a NULL string, or has_* 0, when absent. The strings are the ones its
 * archetype points at. */
typedef struct kind_record {
    const char *required_tech;
    int has_sprite;
    p8_fixed_t sprite;
    const char *state;
    int has_cost;
    p8_fixed_t cost;
} kind_record_t;

/* `resource` is only meaningful when has_resource is set. */
typedef struct {
    int has_resource;
    p8_fixed_t resource;
    p8_fixed_t amount;
} production_t;

/* A p8_fixed_t below holds the 16.16 bits PICO-8 reads for the decimal
 * literal it was written as, 0xffff.fffe mask included: -0x5540 is -0.333. */

/* description is NULL when absent (the vanishing entry), state NULL when
 * absent. */
typedef struct {
    const char *name;
    int sprite;
    p8_fixed_t weight;
    const char *description;
    const char *state;
} planet_types_entry_t;

typedef struct {
    int pos_x;
    int pos_y;
    const data_row_t *layout_row;
} galaxy_menu_entry_t;

typedef struct {
    const data_row_t *layout_row;
} pause_menu_entry_t;

/* The pause menu's named entries, which follow its rows. */
typedef struct {
    int fixed_pos_x;
    int fixed_pos_y;
    int row_step_x;
    int row_step_y;
    int back_color;
} pause_menu_tail_t;

typedef struct {
    const data_row_t *layout_row;
    const char *name;
    int trade;
    p8_fixed_t type_bias;
} difficulty_options_entry_t;

/* The row field holds the positionals, its color coming last; then draw_function
 * (NULL when absent), the score and the position, each only meaningful under
 * its has_ flag. */
typedef struct {
    data_row_t row;
    const char *draw_function;
    int has_score;
    int score;
    int has_pos;
    int pos_x;
    int pos_y;
} score_rows_entry_t;

/* A class's archetype: its defaults resolved along its init chain, the
 * entity's state and state time first. A default that is the address of a
 * module's static object (the shared ephemerals of game/ui.c, the mouse's
 * position, the wormhole's too-far target) is not here: the module's init
 * still sets it. */
typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    p8_fixed_t hit_test;
    const int_seq_t *tile_choices;
} background_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t cursor_sprite;
    p8_fixed_t layer;
    box_t panel_box;
} ui_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
} button_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    p8_fixed_t hit_test;
} modal_backdrop_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    vec_t velocity;
    p8_fixed_t shake;
    vec_t pos_before_shake;
    vec_t pos;
    const vec_seq_t *dpad_directions;
} camera_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
} mouse_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    p8_fixed_t duration_scale;
} message_box_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    p8_fixed_t fade;
    vec_t velocity;
} toast_archetype_t;

typedef struct {
    const data_row_t *row_tail;
} research_project_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    box_t hit_box;
    p8_fixed_t layer;
    vec_t pos;
    vec_t rest_cursor;
} structure_icon_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    const data_row_t *countdown_rows;
    const data_row_t *frame_rows;
} score_panel_archetype_t;

/* A NULL confirmed_state is absent: the confirmed probe then goes to
 * "normal". clock_ticks is what the confirmation hands to
 * economy_advance_clock(), an absent value held as 0. */
typedef struct {
    p8_fixed_t attach_distance;
    p8_fixed_t cost;
    int has_cost;
    const char *confirmed_state;
    p8_fixed_t clock_ticks;
} targeting_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t spawn_snd;
    p8_fixed_t radius;
    p8_fixed_t layer;
    const int_seq_t *color_ramp;
    targeting_archetype_t targeting;
} wormhole_archetype_t;

typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t layer;
    const int_seq_t *trail_colors;
    p8_fixed_t fade_in_step;
    targeting_archetype_t targeting;
} slipway_archetype_t;

/* quarter_turn is 1 or false, false held as 0: only its truth is read. */
typedef struct {
    p8_fixed_t angle_slices;
    p8_fixed_t base_radius;
    p8_fixed_t speed_factor;
    p8_fixed_t lifetime;
    p8_fixed_t quarter_turn;
    p8_fixed_t spawn_rate;
    const int_seq_t *trail_colors;
    p8_fixed_t matrix_xx;
    p8_fixed_t matrix_xy;
    p8_fixed_t matrix_yx;
    p8_fixed_t matrix_yy;
} emitter_archetype_t;

/* The probe and the help screen share this type; the help screen's own
 * members are zero in the probe's. */
typedef struct {
    const char *state;
    p8_fixed_t state_time;
    p8_fixed_t fold_anchor;
    p8_fixed_t layer;
    vec_t pos;
    int has_pos; /* 0: the instance's `pos` is NULL */
    vec_t fixed_pos;
    p8_fixed_t back_color;
    vec_t row_step;
    p8_fixed_t clear_ephemerals_after;
    box_t hit_box;
    p8_fixed_t cursor_sprite;
    const data_row_t *rows;
    emitter_archetype_t emitter;
    targeting_archetype_t targeting;
} probe_archetype_t;

/* The ten planet kinds share this type; a kind's value leaves at zero the
 * members of the kinds it does not descend from. `production`, and the
 * burst's `outgoing_routes`, point at objects their module owns, and
 * `production_own` only holds what planet_install_project() writes: not
 * here. */
typedef struct {
    const char *state;
    p8_fixed_t state_time;
    int is_planet;
    p8_fixed_t trade_multiplier;
    p8_fixed_t layer;
    box_t hit_box;
    p8_fixed_t shake;
    const int_seq_t *ring_colors;
    const data_row_t *mood_rows; /* PLANET_MOOD_ROWS_COUNT rows */
    const int_seq_t *made_resources;
    vec_t row_step;
    p8_fixed_t back_color;
    const data_row_t *spacer;
    p8_fixed_t development_tier;
    int has_development_tier;
    p8_fixed_t project_index;
    const data_row_t *name;
    int is_structure;
    p8_fixed_t sprite;
    const int_seq_t *accepted_resources;
    const char *required_tech;
    p8_fixed_t is_trading_hub;
    p8_fixed_t palette_block;
    vec_t spawn_offset;
    const char *class_name;
    p8_fixed_t acceleration_factor;
    p8_fixed_t speed;
    const int_seq_t *link_sprites;
    p8_fixed_t infraspace;
    const int_seq_t *synthesizable_resources;
    targeting_archetype_t targeting;
    emitter_archetype_t emitter;
} planet_archetype_t;

/* The keys a protostar passes to the burst it spawns, besides `pos`. */
typedef struct {
    p8_fixed_t matrix_xx;
    p8_fixed_t matrix_xy;
    p8_fixed_t matrix_yx;
    p8_fixed_t matrix_yy;
    p8_fixed_t acceleration_factor;
    p8_fixed_t speed_factor;
    p8_fixed_t speed;
    const int_seq_t *trail_colors;
} burst_construct_keys_t;

extern const int upkeep_by_tier_table[UPKEEP_BY_TIER_COUNT];
extern const p8_fixed_t partner_bonus_by_tier_table[PARTNER_BONUS_BY_TIER_COUNT];
extern const int development_score_by_tier_table[DEVELOPMENT_SCORE_BY_TIER_COUNT];
extern const int happiness_by_tier_table[HAPPINESS_BY_TIER_COUNT];
extern const int tier_thresholds_table[TIER_THRESHOLDS_COUNT];
extern const p8_fixed_t layer_fades_table[LAYER_FADES_COUNT];
extern const char *const period_names_table[PERIOD_NAMES_COUNT];
extern const planet_types_entry_t planet_types_table[PLANET_TYPES_COUNT];
extern const galaxy_menu_entry_t galaxy_menu_table[GALAXY_MENU_COUNT];
extern const pause_menu_entry_t pause_menu_table[PAUSE_MENU_COUNT];
extern const pause_menu_tail_t pause_menu_tail;
extern const difficulty_options_entry_t difficulty_options_table[DIFFICULTY_OPTIONS_COUNT];
extern const score_rows_entry_t score_rows_table[SCORE_ROWS_COUNT];

/* The array members of the planet kinds, one object per class that sets it. */
extern const int_seq_t planet_ring_colors_seq;
extern const data_row_t planet_mood_rows_rows[PLANET_MOOD_ROWS_COUNT];
extern const int_seq_t planet_made_resources_seq;
extern const data_row_t planet_spacer_row;
extern const data_row_t structure_name_row;
extern const data_row_t trading_hub_name_row;
extern const data_row_t processor_name_row;
extern const data_row_t ascension_name_row;
extern const data_row_t protostar_name_row;
extern const data_row_t synthesizer_name_row;
extern const int_seq_t structure_accepted_resources_seq;
extern const int_seq_t trading_hub_accepted_resources_seq;
extern const int_seq_t ascension_trail_colors_seq;
extern const int_seq_t protostar_trail_colors_seq;
extern const int_seq_t synthesizer_trail_colors_seq;
extern const int_seq_t burst_trail_colors_seq;
/* The trail_colors a protostar passes to the burst it spawns. */
extern const int_seq_t protostar_burst_trail_colors_seq;
extern const int_seq_t slipgate_link_sprites_seq;
extern const int_seq_t infraspace_gate_link_sprites_seq;
extern const int_seq_t synthesizer_synthesizable_resources_seq;

/* The array members of the other classes, one object per class that sets it. */
extern const int_seq_t probe_trail_colors_seq;
extern const int_seq_t help_screen_trail_colors_seq;
extern const int_seq_t slipway_trail_colors_seq;
extern const int_seq_t wormhole_color_ramp_seq;
extern const int_seq_t background_tile_choices_seq;
extern const vec_seq_t camera_dpad_directions_vecs;
extern const data_row_t research_project_row_tail_row;
/* Draw rows: each item a row of positionals and its draw_function; the
 * countdown rows hold two false items. */
extern const data_row_t help_screen_rows_row;
extern const data_row_t score_panel_countdown_rows_row;
extern const data_row_t score_panel_frame_rows_row;
/* The year banner and the readouts, which score_panel_init() copies into
 * its own rows. */
extern const data_row_t score_panel_year_banner_row;
extern const data_row_t score_panel_readouts_row;

/* The layout row, the tooltip, the economy and the trade route have no
 * defaults, hence no archetype; the shared ephemerals of game/ui.c and the
 * mouse's left and right buttons start empty, held by their module. */
extern const burst_construct_keys_t protostar_burst_construct_keys;
extern const background_archetype_t background_archetype;
extern const ui_archetype_t ui_archetype;
extern const button_archetype_t button_archetype;
extern const modal_backdrop_archetype_t modal_backdrop_archetype;
extern const camera_archetype_t camera_archetype;
extern const mouse_archetype_t mouse_archetype;
extern const message_box_archetype_t message_box_archetype;
extern const toast_archetype_t toast_archetype;
extern const research_project_archetype_t research_project_archetype;
extern const structure_icon_archetype_t structure_icon_archetype;
extern const score_panel_archetype_t score_panel_archetype;
extern const wormhole_archetype_t wormhole_archetype;
extern const slipway_archetype_t slipway_archetype;
/* The targeting a hovered structure icon builds to show its build cost,
 * which has no class: every field 0. */
extern const targeting_archetype_t targeting_archetype_none;
extern const probe_archetype_t probe_archetype;
extern const probe_archetype_t help_screen_archetype;
extern const planet_archetype_t planet_archetype;
extern const planet_archetype_t structure_archetype;
extern const planet_archetype_t trading_hub_archetype;
extern const planet_archetype_t processor_archetype;
extern const planet_archetype_t ascension_archetype;
extern const planet_archetype_t protostar_archetype;
extern const planet_archetype_t synthesizer_archetype;
extern const planet_archetype_t burst_archetype;
extern const planet_archetype_t slipgate_archetype;
extern const planet_archetype_t infraspace_gate_archetype;
extern const kind_record_t structure_kind_record;
extern const kind_record_t trading_hub_kind_record;
extern const kind_record_t processor_kind_record;
extern const kind_record_t ascension_kind_record;
extern const kind_record_t protostar_kind_record;
extern const kind_record_t burst_kind_record;
extern const kind_record_t slipgate_kind_record;
extern const kind_record_t infraspace_gate_kind_record;
extern const kind_record_t synthesizer_kind_record;
/* The status_text of the wormhole's too-far target. */
extern const char wormhole_too_far_target_status_text[];

/* The element at a 1-based index: returns 1 and writes *out when the index
 * is in 1..count, 0 and leaves *out untouched otherwise. */
int int_seq_at(const int *seq, int count, int i, int *out);
int game_data_fixed_seq_at(const p8_fixed_t *seq, int count, int i, p8_fixed_t *out);

/* 1 when the value is p8_fixed_from_int() of an element. */
int int_seq_has_fixed(const int_seq_t *seq, p8_fixed_t v);

/* The planet_types_table entry with that name, NULL when none or when the
 * name is NULL. */
const planet_types_entry_t *planet_types_lookup(const char *px);

/* Two bare constants of the route formulas, not fields of any table. */
p8_fixed_t game_data_route_cost_factor(void);
p8_fixed_t game_data_route_length_factor(void);

#endif
