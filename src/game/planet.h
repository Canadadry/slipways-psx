#ifndef GAME_PLANET_H
#define GAME_PLANET_H

#include "engine/entity_id_list.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "game/layout.h"
#include "game/particle.h"
#include "game/targeting.h"
#include "engine/pointer_list.h"
#include "game/research.h"
#include "game/trade.h"
#include "engine/entity_class.h"

typedef struct {
    entity_t header;
    const planet_archetype_t *archetype;
    const char *planet_id;
    entity_id_list_t slipways;
    research_project_id_t project;
    const char *description;
    pointer_list_t needs;
    const planet_types_entry_t *planet_type;
    layout_row_t *layout_row;
    const allocator_t *layout_row_slot;
    entity_id_t targeted_by;
    p8_fixed_t production_bonus;
    int has_production_bonus;
    p8_fixed_t net_income;
    int has_net_income;
    trade_route_list_t *incoming_routes;
    trade_route_list_t *outgoing_routes;
    p8_fixed_t development_tier;
    int has_development_tier;
    production_t production;
    p8_fixed_t sprite;
    p8_fixed_t palette_block;
    p8_fixed_t acceleration_factor, speed;
    const int_seq_t *synthesizable_resources;
    uint16_t has;
    targeting_t targeting;
    emitter_t emitter;
} planet_t;

#define PLANET_HAS_TARGETING (1u << 0)
#define PLANET_HAS_EMITTER (1u << 1)
#define PLANET_HAS_PRODUCTION (1u << 2)
#define PLANET_HAS_NEEDS (1u << 3)

/* GAP-8 */
void planet_init(void);

void planet_shutdown(void);

int planet_ready(void);

const entity_class_t *planet_class(void);

const data_row_t *planet_mood_rows(void);
const int_seq_t *planet_made_resources(void);
const int_seq_t *planet_ring_colors(void);
const data_row_t *planet_spacer(void);

production_t *planet_shared_production(void);

/* Without PLANET_HAS_PRODUCTION, the result is the shared production: a write through it shows on every such planet. Valid for the call only. */
production_t *planet_production(planet_t *self);
const production_t *planet_production_read(const planet_t *self);

planet_t *planet_new(const planet_keys_t *keys);

void planet_init_instance(planet_t *self, const entity_class_t *instance_class, const planet_archetype_t *archetype);

size_t planet_kind_construction_bytes(const entity_kind_t *kind);

void planet_grab_arena_released(const planet_t *self);

void planet_construct(planet_t *self);
void planet_construct_in(planet_t *self, const allocator_t *zone);

int planet_hit_test_method(entity_t *self, const vec_t *mouse_world, const vec_t *mouse_screen);
void planet_refresh_method(entity_t *self);
void planet_release_row(entity_t *self);
void planet_release_method(entity_t *self);
down_t planet_accept_drop_method(entity_t *self, const entity_t *s);
void planet_new_year_method(entity_t *self, p8_fixed_t year);
void planet_draw_method(entity_t *self, const vec_t *screen_pos);
down_t planet_left_press_method(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);
void planet_attach_slipway_method(entity_t *self, entity_t *slipway);

int planet_is_family(const entity_t *e);
void planet_offer_trade_method(entity_t *self, struct trade_offer *offer);
void planet_update_vanishing_method(entity_t *self, p8_fixed_t t);

void planet_install_project(planet_t *self, research_project_t *research_project_instance);

int planet_is_colonized(const planet_t *self);

void planet_restart_shake(planet_t *self);

void planet_update_vanishing(planet_t *self, p8_fixed_t t);

void planet_roll_type(planet_t *self);

void planet_new_year(const planet_t *self);

down_t planet_left_press(planet_t *self);

void planet_set_type_picker(weighted_picker_t *picker);
void planet_set_vanish_roll(random_walk_t *ir);

void planet_bob(entity_t *self, p8_fixed_t t);

const entity_kind_t *planet_kind_named(const char *fz);

#endif
