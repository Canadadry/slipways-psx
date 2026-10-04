#include "devtools/state.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "engine/entity.h"
#include "engine/entity_geom.h"
#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"
#include "engine/zone_sizes.h"
#include "game/main_economy.h"
#include "game/particle_zone.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/slipway.h"
#include "game/ui_zone.h"

static const char *const SLOT_KIND_NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "row", "ephemerals"};
static const char *const PARTICLE_NAMES[PARTICLE_SLOT_COUNT] = {"particle", "particles", "particles_help_screen", "burst", "transit_dots"};

static void write_fixed(FILE *out, p8_fixed_t x) {
    int64_t v = x;
    if (v < 0) {
        fputc('-', out);
        v = -v;
    }
    long whole = (long)(v >> 16);
    long hundredths = (long)(((v & 0xffff) * 100) >> 16);
    if (hundredths == 0) {
        fprintf(out, "%ld", whole);
    } else {
        fprintf(out, "%ld.%02ld", whole, hundredths);
    }
}

static vec_t ra_of(const entity_t *e) {
    const vec_t *pos = e != NULL && e->pos != NULL ? *e->pos : NULL;
    return pos != NULL ? *pos : vec_make(0, 0);
}

static void write_position(FILE *out, const entity_t *e, vec_t camera) {
    vec_t ra = ra_of(e);
    fputs(" world ", out);
    write_fixed(out, ra.x);
    fputc(' ', out);
    write_fixed(out, ra.y);
    fputs(" screen ", out);
    write_fixed(out, p8_fixed_sub(ra.x, camera.x));
    fputc(' ', out);
    write_fixed(out, p8_fixed_sub(ra.y, camera.y));
    fprintf(out, " visible %d", entity_geom_is_on_screen(e));
}

static const char *class_of(const entity_t *e) {
    const entity_class_t *c = e->class;
    if (c == planet_class()) return "planet";
    if (c == structure_class()) return "structure";
    if (c == trading_hub_class()) return "trading_hub";
    if (c == processor_class()) return "processor";
    if (c == ascension_class()) return "ascension";
    if (c == protostar_class()) return "protostar";
    if (c == synthesizer_class()) return "synthesizer";
    if (c == burst_class()) return "burst";
    if (c == slipgate_class()) return "slipgate";
    if (c == infraspace_gate_class()) return "infraspace_gate";
    return NULL;
}

static const char *planet_state(state_t state) {
    if (state == ENTITY_STATE_NONE) return "-";
    if (state == ENTITY_STATE_SETTLED) return "colonized";
    if (state == ENTITY_STATE_VANISHING) return "empty";
    return entity_state_name(state);
}

static const char *id_of(const entity_t *e) {
    if (e == NULL || class_of(e) == NULL) return "-";
    const char *id = ((const planet_t *)e)->planet_id;
    return id != NULL ? id : "-";
}

static int count_of(const entity_list_t *l) {
    return l != NULL ? l->count : 0;
}

static void write_clock(FILE *out) {
    const economy_t *economy = main_economy_typed();
    if (economy == NULL) {
        fputs("clock year - month -\n", out);
        return;
    }
    fputs("clock year ", out);
    write_fixed(out, economy->year);
    fputs(" month ", out);
    write_fixed(out, economy->year_ticks);
    fputc('\n', out);
}

static void write_resources(FILE *out) {
    const economy_t *economy = main_economy_typed();
    const main_economy_placeholder_t *placeholder = main_economy_placeholder();
    fputs("resources credits ", out);
    if (economy != NULL) {
        write_fixed(out, economy->credits);
    } else if (placeholder != NULL && placeholder->has_credits) {
        write_fixed(out, placeholder->credits);
    } else {
        fputc('-', out);
    }
    fputs("\nresources science ", out);
    if (economy != NULL) {
        write_fixed(out, economy->research_points);
    } else if (placeholder != NULL && placeholder->has_research_points) {
        write_fixed(out, placeholder->research_points);
    } else {
        fputc('-', out);
    }
    fputs("\nresources income ", out);
    if (economy != NULL) {
        write_fixed(out, economy->income);
    } else {
        fputc('-', out);
    }
    fputc('\n', out);
}

static int write_techs(FILE *out) {
    const economy_t *economy = main_economy_typed();
    if (economy == NULL || economy->invented == NULL) return 0;
    for (int i = 0; i < economy->invented->count; i++) {
        fprintf(out, "techs %s\n", (const char *)economy->invented->items[i]);
    }
    return economy->invented->count;
}

static vec_t write_camera(FILE *out) {
    vec_t camera = ra_of(main_camera());
    fputs("camera world ", out);
    write_fixed(out, camera.x);
    fputc(' ', out);
    write_fixed(out, camera.y);
    fputc('\n', out);
    return camera;
}

static void write_planets(FILE *out, vec_t camera) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; i < count_of(planets); i++) {
        const planet_t *p = (const planet_t *)planets->items[i];
        const char *cls = class_of(&p->header);
        fprintf(out, "planet %s %s %s %s", id_of(&p->header), planet_state(p->header.state),
                cls != NULL ? cls : "-", p->planet_type != NULL && p->planet_type->name != NULL ? p->planet_type->name : "-");
        write_position(out, &p->header, camera);
        fputc('\n', out);
    }
}

static int slipway_placed(const entity_t *e) {
    return e->state != ENTITY_STATE_PLACING;
}

static int write_slipways(FILE *out) {
    const entity_list_t *jc = entities_by_tag(TAG_SLIPWAY);
    int n = 0;
    for (int i = 0; i < count_of(jc); i++) {
        const slipway_t *s = (const slipway_t *)jc->items[i];
        if (!slipway_placed(&s->header)) continue;
        fprintf(out, "slipway %s %s %s\n", id_of(s->start), id_of(s->end),
                s->has_infraspace && s->infraspace != 0 ? "infragate" : "normal");
        n++;
    }
    return n;
}

static void write_structures(FILE *out, vec_t camera) {
    const entity_list_t *nr = entities_by_tag(TAG_SLIPGATE);
    for (int i = 0; i < count_of(nr); i++) {
        const planet_t *g = (const planet_t *)nr->items[i];
        fprintf(out, "structure %s %s", id_of(&g->header),
                g->header.class == infraspace_gate_class() ? "infragate" : "slipgate");
        write_position(out, &g->header, camera);
        fprintf(out, " links %d\n", count_of(g->slipways));
    }
}

/* Not planet_is_colonized(): it skips labs, whose archetype has no is_planet. */
static int count_colonies(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    int n = 0;
    for (int i = 0; i < count_of(planets); i++) {
        n += planets->items[i]->state == ENTITY_STATE_SETTLED;
    }
    return n;
}

static void write_zone(FILE *out, const char *name, size_t current, size_t peak, size_t limit) {
    fprintf(out, "zone %s %lu %lu %lu\n", name, (unsigned long)current, (unsigned long)peak, (unsigned long)limit);
}

static void write_zones(FILE *out) {
    write_zone(out, "game", game_zone_used(), game_zone_max(), GENERATION_ZONE_BYTES);
    write_zone(out, "load", load_zone_used(), game_zone_load_zone_max(), LOAD_ZONE_BYTES);
    write_zone(out, "frame", frame_zone_used(), frame_zone_max(), FRAME_ZONE_BYTES);
    write_zone(out, "ui", ui_zone_used(), ui_zone_max_used(), ui_zone_reserved());
    write_zone(out, "particles", particle_zone_used(), particle_zone_max_used(), particle_zone_reserved());
    fprintf(out, "budget zones %lu %lu\n", (unsigned long)ZONE_BUDGET_USED_BYTES, (unsigned long)ZONE_BUDGET_BYTES);
}

static void write_peak(FILE *out, const char *name, size_t peak, size_t limit) {
    fprintf(out, "cap %s - %lu %lu\n", name, (unsigned long)peak, (unsigned long)limit);
}

static void write_caps(FILE *out) {
    for (int i = 0; i < SLOT_KIND_COUNT; i++) {
        slot_kind_t k = (slot_kind_t)i;
        fprintf(out, "cap %s %d %d %d\n", SLOT_KIND_NAMES[i], ui_zone_live(k), ui_zone_max_live(k), ui_zone_count(k));
    }
    for (int i = 0; i < PARTICLE_SLOT_COUNT; i++) {
        particle_slot_kind_t k = (particle_slot_kind_t)i;
        fprintf(out, "cap %s %d %d %d\n", PARTICLE_NAMES[i], particle_zone_live(k), particle_zone_max_live(k),
                particle_zone_count(k));
    }
    fprintf(out, "cap exit_queue %d %d %d\n", exit_queue_count(), exit_queue_max(), EXIT_QUEUE_CAP);
    fprintf(out, "cap entity_table %d %d %d\n", entity_id_live(), entity_id_max(), ENTITY_TABLE_CAP);
    write_peak(out, "elements_particles", (size_t)particle_zone_max_elements(PARTICLE_SLOT_LIST), PARTICLES_CAPACITY);
    write_peak(out, "elements_particles_help_screen", (size_t)particle_zone_max_elements(PARTICLE_SLOT_HELP_LIST),
               PARTICLES_HELP_SCREEN_CAPACITY);
    write_peak(out, "elements_transit_dots", (size_t)particle_zone_max_elements(PARTICLE_SLOT_TRANSIT_DOT),
               SLIPWAY_TRANSIT_DOTS_RESERVE);
    write_peak(out, "elements_ephemerals", (size_t)ui_zone_max_ephemerals(), UI_EPHEMERALS_RESERVE);
    char name[64];
    for (int i = PARTICLE_SLOT_LIST; i <= PARTICLE_SLOT_BURST; i++) {
        particle_slot_kind_t k = (particle_slot_kind_t)i;
        snprintf(name, sizeof name, "bytes_%s", PARTICLE_NAMES[i]);
        write_peak(out, name, particle_zone_max_bytes(k), particle_zone_slot_size(k));
    }
    for (int i = 0; i < SLOT_KIND_COUNT; i++) {
        slot_kind_t k = (slot_kind_t)i;
        snprintf(name, sizeof name, "bytes_%s", SLOT_KIND_NAMES[i]);
        write_peak(out, name, ui_zone_max_bytes(k), ui_zone_slot_size(k));
    }
}

void state_write(FILE *out) {
    write_clock(out);
    write_resources(out);
    int techs = write_techs(out);
    vec_t camera = write_camera(out);
    write_planets(out, camera);
    int slipways = write_slipways(out);
    write_structures(out, camera);
    fprintf(out, "count planets %d\n", count_of(entities_by_tag(TAG_PLANET)));
    fprintf(out, "count colonies %d\n", count_colonies());
    fprintf(out, "count slipways %d\n", slipways);
    fprintf(out, "count structures %d\n", count_of(entities_by_tag(TAG_SLIPGATE)));
    fprintf(out, "count techs %d\n", techs);
    write_zones(out);
    write_caps(out);
}

static const char *g_exit_path;
static const int *g_exit_frame;
static int g_exit_registered;

static void write_on_exit(void) {
    if (g_exit_path == NULL) return;
    FILE *out = fopen(g_exit_path, "w");
    if (out == NULL) return;
    fprintf(out, "stopped p8_fatal frame %d\n", *g_exit_frame);
    state_write(out);
    fclose(out);
    fprintf(stderr, "state: stopped at frame %d, state written to %s\n", *g_exit_frame, g_exit_path);
}

void state_write_on_exit(const char *path, const int *frame) {
    assert(path != NULL && frame != NULL);
    g_exit_path = path;
    g_exit_frame = frame;
    if (!g_exit_registered) {
        int registered = atexit(write_on_exit) == 0;
        assert(registered);
        (void)registered;
        g_exit_registered = 1;
    }
}

void state_release(void) {
    g_exit_path = NULL;
}
