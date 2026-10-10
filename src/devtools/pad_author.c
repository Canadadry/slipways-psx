#include "devtools/pad_author.h"

#include "devtools/pad_metrics.h"

#include "engine/allocator.h"
#include "engine/dispatch_context.h"
#include "engine/entity_id.h"
#include "engine/entity_list.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/game_data.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/pad_jump.h"
#include "game/pad_nav.h"
#include "game/pad_panel.h"
#include "game/pad_select.h"
#include "game/pad_slipway.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/structure_icon.h"
#include "game/ui_button.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_CHARS 256
#define TOKENS_MAX 8
#define PATH_MAX_CHARS 256
#define INCLUDE_DEPTH_MAX 4
#define FOLD_HOLD_FRAMES 16
#define AIM_FRAME_WEIGHT 6
#define CIRCLE_EFFECT_FRAMES 2
#define TITLE_ROWS_FRAMES 7
#define PAN_ROUNDS_MAX 32
#define PATH_STEPS_MAX 64
#define YEARS_TO_END_MAX 200
#define REVEAL_FRAMES_MAX 30
#define MENU_DWELL_MAX 60

typedef struct {
    char text[LINE_MAX_CHARS];
    char *tokens[TOKENS_MAX];
    int token_count;
    int number;
} script_line_t;

typedef struct {
    const char *path;
    script_line_t *lines;
    int line_count;
    const script_line_t *current;
} script_t;

typedef struct {
    const char *name;
    const kind_record_t *record;
    const entity_class_t *(*class_of)(void);
} structure_kind_t;

static const structure_kind_t STRUCTURE_KINDS[] = {
    {"structure", &structure_kind_record, structure_class},
    {"processor", &processor_kind_record, processor_class},
    {"trading_hub", &trading_hub_kind_record, trading_hub_class},
    {"ascension", &ascension_kind_record, ascension_class},
    {"slipgate", &slipgate_kind_record, slipgate_class},
    {"infraspace_gate", &infraspace_gate_kind_record, infraspace_gate_class},
    {"synthesizer", &synthesizer_kind_record, synthesizer_class},
};
#define STRUCTURE_KIND_COUNT ((int)(sizeof STRUCTURE_KINDS / sizeof STRUCTURE_KINDS[0]))

typedef int (*item_match_t)(const entity_t *e, const char *name);

/* A stop-at label outlives the script line it came from. */
static char stop_label_copy[LINE_MAX_CHARS];
static p8_input_frame_t boot_frame;
static p8_input_t boot_input;

#define SCRATCH_BYTES (1 << 16)
BUMP_ZONE_STORAGE(scratch_cells, SCRATCH_BYTES);
static bump_zone_t scratch;
static allocator_t scratch_zone;

/* Valid until the next call, or the next emit. */
static entity_list_t live_candidates(void) {
    bump_zone_reset(&scratch);
    return pad_jump_candidates(&scratch_zone, entities_by_tag(TAG_NODE));
}

static int fail(pad_author_t *author, const script_t *script, const char *format, ...) {
    char reason[PAD_AUTHOR_ERROR_MAX / 2];
    va_list args;
    va_start(args, format);
    vsnprintf(reason, sizeof reason, format, args);
    va_end(args);
    int line = script != NULL && script->current != NULL ? script->current->number : 0;
    snprintf(author->error, sizeof author->error, "%.200s:%d: frame %d: %s", script != NULL ? script->path : "-", line,
             author->frame_count, reason);
    return 0;
}

static pad_nav_t *nav_of(pad_author_t *author) {
    return &author->step.pad_nav;
}

static pad_metrics_mode_t current_mode(pad_author_t *author) {
    return pad_metrics_mode(&author->step);
}

static int camera_point(int *x, int *y) {
    const camera_t *camera = main_camera_typed();
    if (camera == NULL) {
        *x = 0;
        *y = 0;
        return 0;
    }
    vec_t at = camera_pos_before_shake(camera);
    *x = p8_fixed_flr_int(at.x);
    *y = p8_fixed_flr_int(at.y);
    return 1;
}

static void centre_world_point(int *x, int *y) {
    camera_point(x, y);
    *x += P8_SCREEN_WIDTH / 2;
    *y += P8_SCREEN_HEIGHT / 2;
}

static int count_of(const entity_list_t *list) {
    return list != NULL ? list->count : 0;
}

static int invented_count(void) {
    const economy_t *economy = main_economy_typed();
    return economy != NULL ? economy->invented.count : 0;
}

void pad_author_init(pad_author_t *author) {
    memset(author, 0, sizeof *author);
    input_step_init(&author->step);
    bump_zone_init(&scratch, "pad_author scratch", scratch_cells, SCRATCH_BYTES);
    scratch_zone = allocator_from_bump_zone(&scratch);
    pad_metrics_init(&author->metrics, &scratch);
    memset(&boot_frame, 0, sizeof boot_frame);
    boot_input.frames = &boot_frame;
    boot_input.frame_count = 1;
    boot_input.current_frame = 0;
    dispatch_context_set_input(&boot_input);
}

static const char *button_name(int buttons) {
    assert(buttons != 0 && "button_name: no button set, the scan would never end");
    int b = 0;
    while (!(buttons & (1 << b))) b++;
    assert(b < PAD_METRICS_BUTTON_COUNT);
    return pad_metrics_button_name(b);
}

static void write_buttons(FILE *out, int buttons) {
    const char *separator = "";
    fputc('[', out);
    for (int b = 0; b < PAD_METRICS_BUTTON_COUNT; b++) {
        if (buttons & (1 << b)) {
            fprintf(out, "%s%s", separator, pad_metrics_button_name(b));
            separator = "+";
        }
    }
    fputc(']', out);
}

static int settle_circle(pad_author_t *author, const script_t *script, const pad_metrics_view_t *now, int final) {
    if (author->circle_pending == 0) return 1;
    if (pad_metrics_navigation_changed(&author->circle_before, now)) {
        author->circle_pending = 0;
        return 1;
    }
    author->circle_pending--;
    if (author->circle_pending > 0 && !final) return 1;
    author->circle_pending = 0;
    if (author->circle_required) {
        pad_metrics_count_required_noop(&author->metrics);
        return 1;
    }
    return fail(author, script, "dead circle: it changed no mode, object, camera, box or panel (mark it `back --required`)");
}

/* Keep in step with src/cmd/replay.c's loop, or the self-check fails. */
static int emit(pad_author_t *author, const script_t *script, int buttons) {
    if (author->frame_count == PAD_AUTHOR_FRAME_BUDGET) {
        return fail(author, script, "over the frame budget of %d", PAD_AUTHOR_FRAME_BUDGET);
    }
    const int face = P8_PAD_CROSS | P8_PAD_CIRCLE;
    int pressed = pad_metrics_pressed(&author->metrics, buttons);
    if ((pressed & face) == face) return fail(author, script, "chord: cross and circle pressed on the same frame");
    pad_metrics_mode_t mode = current_mode(author);
    int direction = pad_jump_first_direction(pressed);
    if (mode == PAD_METRICS_MODE_JUMP && direction != 0 &&
        pad_metrics_cone_empty(&author->metrics, pad_nav_hovered(nav_of(author)), direction)) {
        return fail(author, script, "empty cone: no candidate %s of the hovered object in jump mode",
                    button_name(direction));
    }
    pad_metrics_observe(&author->metrics, &author->step);
    pad_metrics_view_t before = *pad_metrics_view(&author->metrics);
    if (pressed & P8_PAD_CIRCLE) {
        if (!settle_circle(author, script, &before, 1)) return 0;
        author->circle_before = before;
        author->circle_pending = CIRCLE_EFFECT_FRAMES;
        author->circle_required = author->next_circle_required;
    }
    author->next_circle_required = 0;

    p8_input_frame_t *frame = &author->frames[author->frame_count];
    memset(frame, 0, sizeof *frame);
    frame->pad_buttons = buttons;
    int cheat = input_step_run(&author->step, frame);
    cart_update60();
    cart_draw();

    pad_metrics_record(&author->metrics, &author->step, buttons, cheat);
    const pad_metrics_view_t *after = pad_metrics_view(&author->metrics);
    if (author->mode_log != NULL) {
        fprintf(author->mode_log, "f%d %s ", author->frame_count, pad_metrics_mode_name(mode));
        write_buttons(author->mode_log, buttons);
        if (pressed != 0) {
            fputs(" pressed ", author->mode_log);
            write_buttons(author->mode_log, pressed);
            fprintf(author->mode_log, " effect %d", pad_metrics_view_changed(&before, after));
        }
        fputc('\n', author->mode_log);
    }
    author->frame_count++;
    return settle_circle(author, script, after, 0);
}

static int emit_repeated(pad_author_t *author, const script_t *script, int buttons, int count) {
    for (int i = 0; i < count; i++) {
        if (!emit(author, script, buttons)) return 0;
    }
    return 1;
}

static int tap(pad_author_t *author, const script_t *script, int buttons) {
    return emit(author, script, buttons) && emit(author, script, 0);
}

typedef int (*condition_t)(pad_author_t *author, const void *context);

static int wait_until(pad_author_t *author, const script_t *script, condition_t condition, const void *context,
                      int max_frames, const char *what) {
    for (int i = 0; !condition(author, context); i++) {
        if (i == max_frames) return fail(author, script, "%s: not within %d frames", what, max_frames);
        if (!emit(author, script, 0)) return 0;
    }
    return 1;
}

static int parse_buttons(const char *text) {
    int mask = 0;
    const char *p = text;
    while (*p != '\0') {
        size_t length = strcspn(p, "+");
        int bit = -1;
        for (int b = 0; b < PAD_METRICS_BUTTON_COUNT; b++) {
            const char *name = pad_metrics_button_name(b);
            if (strlen(name) == length && strncmp(name, p, length) == 0) bit = b;
        }
        if (bit < 0) return 0;
        mask |= 1 << bit;
        p += length;
        if (*p == '+') p++;
    }
    return mask;
}

static int parse_count(const char *text) {
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (text[0] == '\0' || *end != '\0' || value < 0 || value > PAD_AUTHOR_FRAME_BUDGET) return -1;
    return (int)value;
}

static int parse_coordinate(const char *text, int *out) {
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (text[0] == '\0' || *end != '\0' || value < -100000 || value > 100000) return 0;
    *out = (int)value;
    return 1;
}

static int difficulty_index(const char *name) {
    for (int d = 0; d < DIFFICULTY_OPTIONS_COUNT; d++) {
        if (strcmp(difficulty_options_table[d].name, name) == 0) return d;
    }
    return -1;
}

static int run_title(pad_author_t *author, const script_t *script, int cross_frame, int difficulty) {
    int first_down = cross_frame - 2 * difficulty;
    if (first_down < author->frame_count || cross_frame < author->frame_count) {
        return fail(author, script, "title: the cross at f%d needs its first Down at f%d", cross_frame, first_down);
    }
    if (main_economy_typed() != NULL) return fail(author, script, "title: a game is already running");
    if (!emit_repeated(author, script, 0, first_down - author->frame_count)) return 0;
    for (int d = 0; d < difficulty; d++) {
        if (!tap(author, script, P8_PAD_DOWN)) return 0;
    }
    if (!emit(author, script, P8_PAD_CROSS)) return 0;
    const button_t *chosen = economy_score_difficulty();
    if (main_economy_typed() == NULL || chosen == NULL || chosen->name == NULL ||
        strcmp(chosen->name, difficulty_options_table[difficulty].name) != 0) {
        return fail(author, script, "title: no %s game started on the cross", difficulty_options_table[difficulty].name);
    }
    return 1;
}

static entity_t *object_named(const char *id) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    for (int i = 0; i < count_of(nodes); i++) {
        entity_t *e = nodes->items[i];
        if (e->is_dead || !planet_is_family(e)) continue;
        const char *name = planet_id_or_null((const planet_t *)e);
        if (name != NULL && strcmp(name, id) == 0) return e;
    }
    return NULL;
}

static const char *name_of(const entity_t *e) {
    const char *name = e != NULL && planet_is_family(e) ? planet_id_or_null((const planet_t *)e) : NULL;
    return name != NULL ? name : "-";
}

static int is_mode(pad_author_t *author, const void *context) {
    return current_mode(author) == *(const pad_metrics_mode_t *)context;
}

static int on_the_map(pad_author_t *author, const void *context) {
    (void)context;
    pad_metrics_mode_t mode = current_mode(author);
    return mode == PAD_METRICS_MODE_JUMP || mode == PAD_METRICS_MODE_AIM;
}

static int require_map(pad_author_t *author, const script_t *script, const char *command) {
    if (on_the_map(author, NULL)) return 1;
    return fail(author, script, "%s: needs jump or aim mode, not %s", command,
                pad_metrics_mode_name(current_mode(author)));
}

static void rest_screen_point(const entity_t *e, int *x, int *y) {
    int camera_x, camera_y;
    camera_point(&camera_x, &camera_y);
    pad_jump_world_point(e, x, y);
    *x -= camera_x;
    *y -= camera_y;
}

static int inside(pad_author_t *author, const void *context) {
    (void)author;
    int x, y;
    rest_screen_point((const entity_t *)context, &x, &y);
    return pad_jump_inside(x, y);
}

static int pick_path(const entity_t *origin, const entity_t *target, const entity_list_t *candidates, int *directions) {
    int n = candidates->count;
    if (origin == target) return 0;
    int *previous = malloc((size_t)(n + 1) * sizeof *previous);
    int *via = malloc((size_t)(n + 1) * sizeof *via);
    int *queue = malloc((size_t)(n + 1) * sizeof *queue);
    assert(previous != NULL && via != NULL && queue != NULL);
    for (int i = 0; i <= n; i++) previous[i] = -2;
    int head = 0, tail = 0, found = -1;
    previous[n] = -1;
    queue[tail++] = n;
    while (head < tail && found < 0) {
        int at = queue[head++];
        const entity_t *from = at == n ? origin : candidates->items[at];
        for (int d = 0; d < PAD_JUMP_DIRECTIONS && found < 0; d++) {
            const entity_t *pick = pad_target_pick(from, pad_jump_direction(d), candidates);
            int index = -1;
            for (int i = 0; i < n && pick != NULL; i++) {
                if (candidates->items[i] == pick) index = i;
            }
            if (index < 0 || previous[index] != -2) continue;
            previous[index] = at;
            via[index] = pad_jump_direction(d);
            queue[tail++] = index;
            if (pick == target) found = index;
        }
    }
    int length = -1;
    if (found >= 0) {
        length = 0;
        for (int at = found; at != n; at = previous[at]) length++;
        if (length > PATH_STEPS_MAX) {
            length = -1;
        } else {
            int k = length;
            for (int at = found; at != n; at = previous[at]) directions[--k] = via[at];
        }
    }
    free(previous);
    free(via);
    free(queue);
    return length;
}

static int is_candidate_named(pad_author_t *author, const void *context) {
    (void)author;
    return pad_target_is_candidate(object_named(context));
}

static int leave_aim(pad_author_t *author, const script_t *script);

static int run_jump(pad_author_t *author, const script_t *script, const char *id) {
    if (!wait_until(author, script, is_candidate_named, id, REVEAL_FRAMES_MAX, "jump: the target a candidate")) return 0;
    entity_t *target = object_named(id);
    if (current_mode(author) == PAD_METRICS_MODE_AIM && !leave_aim(author, script)) return 0;
    pad_metrics_mode_t jump = PAD_METRICS_MODE_JUMP;
    if (!wait_until(author, script, is_mode, &jump, 2, "jump: jump mode")) return 0;
    pad_nav_t *nav = nav_of(author);
    if (pad_nav_hovered(nav) == NULL && !emit(author, script, 0)) return 0;
    entity_t *hovered = pad_nav_hovered(nav);
    if (hovered == NULL) return fail(author, script, "jump: nothing hovered");
    int directions[PATH_STEPS_MAX];
    entity_list_t candidates = live_candidates();
    int length = pick_path(hovered, target, &candidates, directions);
    if (length < 0) return fail(author, script, "jump: no D-pad path from %s to %s", name_of(hovered), id);
    for (int i = 0; i < length; i++) {
        candidates = live_candidates();
        entity_t *expected = pad_target_pick(pad_nav_hovered(nav), directions[i], &candidates);
        if (!tap(author, script, directions[i])) return 0;
        if (pad_nav_hovered(nav) != expected) {
            return fail(author, script, "jump: the tap hovered %s, not %s", name_of(pad_nav_hovered(nav)),
                        name_of(expected));
        }
    }
    if (pad_nav_hovered(nav) != target) return fail(author, script, "jump: hovering %s, not %s", name_of(pad_nav_hovered(nav)), id);
    return wait_until(author, script, inside, target, 40, "jump: the target inside the inner rectangle");
}

static entity_t *nearest_candidate(int x, int y) {
    entity_list_t candidates = live_candidates();
    return pad_jump_nearest(x, y, &candidates);
}

static int pan_distance(int n) {
    int d = 0;
    for (int k = 1; k <= n; k++) d += k <= 10 ? 1 : (k <= 40 ? 2 : 4);
    return d;
}

static int pan_frames(int distance) {
    int n = 0;
    while (pan_distance(n + 1) <= distance) n++;
    return n;
}

static int abs_int(int v) {
    return v < 0 ? -v : v;
}

static int pan_to(pad_author_t *author, const script_t *script, int x, int y) {
    for (int round = 0; round < PAN_ROUNDS_MAX; round++) {
        int cx, cy;
        centre_world_point(&cx, &cy);
        int dx = x - cx, dy = y - cy;
        if (dx == 0 && dy == 0) return 1;
        int nx = pan_frames(abs_int(dx)), ny = pan_frames(abs_int(dy));
        int bx = dx > 0 ? P8_PAD_RIGHT : P8_PAD_LEFT, by = dy > 0 ? P8_PAD_DOWN : P8_PAD_UP;
        for (int f = 0; f < (nx > ny ? nx : ny); f++) {
            if (!emit(author, script, (f < nx ? bx : 0) | (f < ny ? by : 0))) return 0;
        }
        centre_world_point(&cx, &cy);
        if (cx == x && cy == y) return 1;
        if (!emit(author, script, 0)) return 0;
    }
    int cx, cy;
    centre_world_point(&cx, &cy);
    return fail(author, script, "pan: the centre stays at %d %d, not %d %d", cx, cy, x, y);
}

static int pan_cost(int distance) {
    int frames = 0;
    while (distance > 0) {
        int n = pan_frames(distance);
        frames += n + 1;
        distance -= pan_distance(n);
    }
    return frames;
}

static int scroll_shift(int value, int low, int high) {
    return value < low ? value - low : value > high ? value - high : 0;
}

static void camera_after_jump(const entity_t *e, int *cx, int *cy) {
    int wx, wy;
    pad_jump_world_point(e, &wx, &wy);
    *cx += scroll_shift(wx - *cx, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_WIDTH - 1 - PAD_JUMP_SCROLL_MARGIN);
    *cy += scroll_shift(wy - *cy, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_HEIGHT - 1 - PAD_JUMP_SCROLL_MARGIN_BOTTOM);
}

static int pan_cost_to(int x, int y, int cx, int cy) {
    int px = pan_cost(abs_int(x - cx - P8_SCREEN_WIDTH / 2)), py = pan_cost(abs_int(y - cy - P8_SCREEN_HEIGHT / 2));
    return px > py ? px : py;
}

static void best_jumps_before_aim(pad_author_t *author, int x, int y, entity_t **first, entity_t **second) {
    enum { CANDIDATES_MAX = 128 };
    static int length_from[CANDIDATES_MAX];
    entity_t *hovered = pad_nav_hovered(nav_of(author));
    int cx, cy;
    camera_point(&cx, &cy);
    int best_cost = AIM_FRAME_WEIGHT * pan_cost_to(x, y, cx, cy);
    *first = NULL;
    *second = NULL;
    if (hovered == NULL) return;
    entity_list_t candidates = live_candidates();
    int n = candidates.count < CANDIDATES_MAX ? candidates.count : CANDIDATES_MAX;
    int directions[PATH_STEPS_MAX];
    for (int i = 0; i < n; i++) {
        entity_t *a = candidates.items[i];
        length_from[i] = name_of(a)[0] == '-' ? -1 : pick_path(hovered, a, &candidates, directions);
        if (length_from[i] <= 0) continue;
        int ax = cx, ay = cy;
        camera_after_jump(a, &ax, &ay);
        int cost = AIM_FRAME_WEIGHT * pan_cost_to(x, y, ax, ay) + length_from[i];
        if (cost < best_cost) {
            best_cost = cost;
            *first = a;
            *second = NULL;
        }
        for (int j = 0; j < n; j++) {
            entity_t *b = candidates.items[j];
            if (j == i || name_of(b)[0] == '-') continue;
            int bx = ax, by = ay;
            camera_after_jump(b, &bx, &by);
            int lower = AIM_FRAME_WEIGHT * pan_cost_to(x, y, bx, by) + length_from[i] + 1;
            if (lower >= best_cost) continue;
            int length = pick_path(a, b, &candidates, directions);
            if (length <= 0 || lower - 1 + length >= best_cost) continue;
            best_cost = lower - 1 + length;
            *first = a;
            *second = b;
        }
    }
}

static int enter_aim(pad_author_t *author, const script_t *script, int x, int y, int has_point) {
    if (current_mode(author) == PAD_METRICS_MODE_AIM) return 1;
    if (!require_map(author, script, "aim")) return 0;
    entity_t *jumps[2] = {NULL, NULL};
    if (has_point) best_jumps_before_aim(author, x, y, &jumps[0], &jumps[1]);
    char ids[2][LINE_MAX_CHARS];
    for (int k = 0; k < 2; k++) snprintf(ids[k], sizeof ids[k], "%s", jumps[k] != NULL ? name_of(jumps[k]) : "");
    for (int k = 0; k < 2; k++) {
        if (jumps[k] != NULL && !run_jump(author, script, ids[k])) return 0;
    }
    if (!emit(author, script, P8_PAD_SQUARE)) return 0;
    if (current_mode(author) != PAD_METRICS_MODE_AIM) return fail(author, script, "square: no aim mode");
    return 1;
}

static int any_candidate(void) {
    return pad_jump_any_candidate(entities_by_tag(TAG_NODE));
}

static int has_candidate(pad_author_t *author, const void *context) {
    (void)author;
    (void)context;
    return any_candidate();
}

static int leave_aim(pad_author_t *author, const script_t *script) {
    if (!wait_until(author, script, has_candidate, NULL, REVEAL_FRAMES_MAX, "square: a known object to jump back to")) {
        return 0;
    }
    if (!tap(author, script, P8_PAD_SQUARE)) return 0;
    if (current_mode(author) != PAD_METRICS_MODE_JUMP) return fail(author, script, "square: no jump mode after aim mode");
    return 1;
}

/* With nothing known, □ in aim mode is a dead press: stay in aim mode. */
static int leave_aim_if_able(pad_author_t *author, const script_t *script) {
    return current_mode(author) == PAD_METRICS_MODE_AIM && any_candidate() ? leave_aim(author, script) : 1;
}

/* A refused probe is marked dead on its release frame but stays tagged until it vanishes. */
static int live_probes(void) {
    const entity_list_t *probes = entities_by_tag(TAG_PROBE);
    int live = 0;
    for (int i = 0; i < count_of(probes); i++) live += !probes->items[i]->is_dead;
    return live;
}

static int run_probe(pad_author_t *author, const script_t *script, int x, int y) {
    if (!enter_aim(author, script, x, y, 1) || !pan_to(author, script, x, y)) return 0;
    int probes = live_probes();
    if (!tap(author, script, P8_PAD_CROSS)) return 0;
    if (live_probes() != probes + 1) return fail(author, script, "probe: refused at %d %d", x, y);
    return leave_aim_if_able(author, script);
}

static int run_fold(pad_author_t *author, const script_t *script, int x, int y, int has_point) {
    if (!enter_aim(author, script, x, y, has_point)) return 0;
    if (has_point && !pan_to(author, script, x, y)) return 0;
    if (count_of(entities_by_tag(TAG_PROBE)) == 0) return fail(author, script, "fold: no probe yet");
    p8_fixed_t credits = main_economy_typed()->credits;
    if (!emit_repeated(author, script, P8_PAD_CROSS, FOLD_HOLD_FRAMES) ||
        !emit(author, script, any_candidate() ? P8_PAD_CIRCLE : 0) || !emit(author, script, 0)) {
        return 0;
    }
    if (main_economy_typed()->credits >= credits) return fail(author, script, "fold: not confirmed, nothing charged");
    return leave_aim_if_able(author, script);
}

static int has_focus(pad_author_t *author, const void *context) {
    (void)context;
    return pad_nav_focus(nav_of(author)) != NULL;
}

enum { SETTLE_ITEMS_MAX = 64, SETTLE_FRAMES_MAX = 40 };

typedef struct {
    const entity_t *items[SETTLE_ITEMS_MAX];
    int x[SETTLE_ITEMS_MAX];
    int y[SETTLE_ITEMS_MAX];
    int count;
} panel_layout_t;

static panel_layout_t panel_layout(pad_author_t *author) {
    panel_layout_t layout;
    layout.count = 0;
    const entity_t *focus = pad_nav_focus(nav_of(author));
    if (focus == NULL) return layout;
    layout.items[layout.count++] = focus;
    for (int head = 0; head < layout.count; head++) {
        pad_panel_focus_point(layout.items[head], &layout.x[head], &layout.y[head]);
        for (int d = 0; d < PAD_JUMP_DIRECTIONS && layout.count < SETTLE_ITEMS_MAX; d++) {
            const entity_t *next = pad_nav_neighbor(layout.items[head], pad_jump_direction(d));
            int seen = next == NULL;
            for (int i = 0; i < layout.count && !seen; i++) seen = layout.items[i] == next;
            if (!seen) layout.items[layout.count++] = next;
        }
    }
    return layout;
}

static int wait_panel_settled(pad_author_t *author, const script_t *script, const char *what) {
    panel_layout_t before = panel_layout(author);
    for (int i = 0; i < SETTLE_FRAMES_MAX; i++) {
        if (!emit(author, script, 0)) return 0;
        panel_layout_t now = panel_layout(author);
        if (now.count > 0 && now.count == before.count && memcmp(now.items, before.items, sizeof now.items[0] * (size_t)now.count) == 0 &&
            memcmp(now.x, before.x, sizeof now.x[0] * (size_t)now.count) == 0 &&
            memcmp(now.y, before.y, sizeof now.y[0] * (size_t)now.count) == 0) {
            return 1;
        }
        before = now;
    }
    return fail(author, script, "%s: the panel still moves after %d frames", what, SETTLE_FRAMES_MAX);
}

static int panel_closed_on_map(pad_author_t *author, const void *context) {
    return !pad_nav_panel_open() && on_the_map(author, context);
}

static int move_down_rows(pad_author_t *author, const script_t *script, int rows) {
    pad_nav_t *nav = nav_of(author);
    for (int r = 0; r < rows; r++) {
        entity_t *expected = pad_nav_neighbor(pad_nav_focus(nav), P8_PAD_DOWN);
        if (expected == NULL) return fail(author, script, "menu: no row %d", r + 1);
        if (!tap(author, script, P8_PAD_DOWN)) return 0;
        if (pad_nav_focus(nav) != expected) return fail(author, script, "menu: Down missed row %d", r + 1);
    }
    return 1;
}

/* Looked up by id each time: a collapse replaces the planet with a new protostar entity. */
static int is_settled(pad_author_t *author, const void *context) {
    (void)author;
    const entity_t *planet = object_named(context);
    return planet != NULL && planet->state == ENTITY_STATE_SETTLED;
}

static int move_to_item(pad_author_t *author, const script_t *script, item_match_t match, const char *name,
                        const char *command);
static int is_project_button(const entity_t *e, const char *project);

static int run_colonise(pad_author_t *author, const script_t *script, const char *id, int row, const char *project) {
    if (!require_map(author, script, "colonise")) return 0;
    if (current_mode(author) == PAD_METRICS_MODE_AIM && !leave_aim(author, script)) return 0;
    if (!run_jump(author, script, id)) return 0;
    entity_t *planet = object_named(id);
    if (planet->state != ENTITY_STATE_KNOWN) return fail(author, script, "colonise: %s is not a known planet", id);
    if (!tap(author, script, P8_PAD_CROSS)) return 0;
    if (!wait_until(author, script, has_focus, NULL, 20, "colonise: the menu's header focused") ||
        !wait_panel_settled(author, script, "colonise")) {
        return 0;
    }
    int moved = project != NULL ? move_to_item(author, script, is_project_button, project, "colonise")
                                : move_down_rows(author, script, row);
    if (!moved || !tap(author, script, P8_PAD_CROSS)) return 0;
    if (!wait_until(author, script, is_settled, id, 5, "colonise: the planet settled")) return 0;
    return wait_until(author, script, panel_closed_on_map, NULL, 10, "colonise: the menu closed");
}

static entity_t *slipway_from_to(const entity_t *from, const entity_t *to) {
    const entity_list_t *slipways = entities_by_tag(TAG_SLIPWAY);
    for (int i = 0; i < count_of(slipways); i++) {
        const slipway_t *s = (const slipway_t *)slipways->items[i];
        if (s->header.state != ENTITY_STATE_PLACING && slipway_start(s) == from && slipway_end(s) == to) {
            return slipways->items[i];
        }
    }
    return NULL;
}

typedef struct {
    const entity_t *from;
    const entity_t *to;
} slipway_pair_t;

static int slipway_rebased(pad_author_t *author, const void *context) {
    const slipway_pair_t *pair = context;
    return slipway_from_to(pair->from, pair->to) != NULL &&
           (pad_nav_slipway_start(nav_of(author)) == pair->to || current_mode(author) == PAD_METRICS_MODE_JUMP);
}

static int aim_slipway_at(pad_author_t *author, const script_t *script, entity_t *start, entity_t *target) {
    pad_nav_t *nav = nav_of(author);
    entity_list_t candidates = live_candidates();
    entity_list_t targets = pad_slipway_candidates(&scratch_zone, &candidates, start);
    entity_t *aimed = pad_nav_slipway_target(nav) != NULL ? pad_nav_slipway_target(nav) : start;
    int directions[PATH_STEPS_MAX];
    int length = pick_path(aimed, target, &targets, directions);
    if (length < 0) return fail(author, script, "slipway: no D-pad path from %s to %s", name_of(aimed), name_of(target));
    for (int i = 0; i < length; i++) {
        candidates = live_candidates();
        targets = pad_slipway_candidates(&scratch_zone, &candidates, start);
        aimed = pad_nav_slipway_target(nav) != NULL ? pad_nav_slipway_target(nav) : start;
        entity_t *expected = pad_target_pick(aimed, directions[i], &targets);
        if (!tap(author, script, directions[i])) return 0;
        if (pad_nav_slipway_target(nav) != expected) {
            return fail(author, script, "slipway: the tap aimed at %s, not %s", name_of(pad_nav_slipway_target(nav)),
                        name_of(expected));
        }
    }
    return wait_until(author, script, inside, target, 40, "slipway: the target inside the inner rectangle");
}

static int run_slipway(pad_author_t *author, const script_t *script, char *const *ids, int count) {
    if (!require_map(author, script, "slipway")) return 0;
    if (current_mode(author) == PAD_METRICS_MODE_AIM && !leave_aim(author, script)) return 0;
    if (!run_jump(author, script, ids[0])) return 0;
    entity_t *start = object_named(ids[0]);
    if (!pad_slipway_can_start(start)) return fail(author, script, "slipway: %s takes no slipway", ids[0]);
    if (!tap(author, script, P8_PAD_CROSS)) return 0;
    if (pad_nav_slipway_start(nav_of(author)) != start) return fail(author, script, "slipway: no slipway out of %s", ids[0]);
    for (int i = 1; i < count; i++) {
        entity_t *target = object_named(ids[i]);
        if (target == NULL) return fail(author, script, "slipway: no object %s", ids[i]);
        if (pad_nav_slipway_start(nav_of(author)) != start) {
            return fail(author, script, "slipway: %s takes no further slipway", name_of(start));
        }
        if (!aim_slipway_at(author, script, start, target) || !emit(author, script, P8_PAD_CROSS)) return 0;
        slipway_pair_t pair = {start, target};
        if (!wait_until(author, script, slipway_rebased, &pair, 6, "slipway: built")) return 0;
        if (!emit(author, script, 0) || !emit(author, script, 0)) return 0;
        start = target;
    }
    for (int i = 0; i < 3 && current_mode(author) == PAD_METRICS_MODE_SLIPWAY; i++) {
        if (!tap(author, script, P8_PAD_CIRCLE)) return 0;
    }
    pad_metrics_mode_t jump = PAD_METRICS_MODE_JUMP;
    return wait_until(author, script, is_mode, &jump, 4, "slipway: back in jump mode");
}

static const structure_kind_t *structure_kind_named(const char *name) {
    for (int k = 0; k < STRUCTURE_KIND_COUNT; k++) {
        if (strcmp(STRUCTURE_KINDS[k].name, name) == 0) return &STRUCTURE_KINDS[k];
    }
    return NULL;
}

static int icon_x(const entity_t *icon) {
    int x, y;
    pad_panel_focus_point(icon, &x, &y);
    return x;
}

static entity_t *icon_of(const structure_kind_t *kind) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < count_of(bucket); i++) {
        entity_t *e = bucket->items[i];
        if (structure_icon_is_revealed(e) && ((structure_icon_t *)e)->kind->record == kind->record) return e;
    }
    return NULL;
}

typedef struct {
    int x;
    int y;
    const structure_kind_t *kind;
} placed_t;

static entity_t *structure_at(const placed_t *placed) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    for (int i = 0; i < count_of(nodes); i++) {
        entity_t *e = nodes->items[i];
        int x, y;
        if (e->is_dead || e->class != placed->kind->class_of() || e->state != ENTITY_STATE_SETTLED) continue;
        pad_jump_world_point(e, &x, &y);
        if (x == placed->x && y == placed->y) return e;
    }
    return NULL;
}

static int is_carrying(pad_author_t *author, const void *context) {
    (void)context;
    return pad_nav_placing(nav_of(author)) != NULL;
}

static int is_placed(pad_author_t *author, const void *context) {
    return pad_nav_placing(nav_of(author)) == NULL && structure_at(context) != NULL;
}

static int run_build(pad_author_t *author, const script_t *script, const char *kind_name, int x, int y) {
    const structure_kind_t *kind = structure_kind_named(kind_name);
    if (kind == NULL) return fail(author, script, "build: unknown kind '%s'", kind_name);
    if (!require_map(author, script, "build")) return 0;
    if (current_mode(author) == PAD_METRICS_MODE_JUMP) {
        entity_t *nearest = nearest_candidate(x, y);
        if (nearest != NULL && nearest != pad_nav_hovered(nav_of(author)) && !run_jump(author, script, name_of(nearest))) {
            return 0;
        }
    }
    if (!tap(author, script, P8_PAD_L1)) return 0;
    pad_nav_t *nav = nav_of(author);
    entity_t *focused = pad_nav_build_icon(nav);
    entity_t *wanted = icon_of(kind);
    if (focused == NULL) return fail(author, script, "build: the bar is not open");
    if (wanted == NULL) return fail(author, script, "build: no %s icon on the bar", kind_name);
    while (focused != wanted) {
        int step = icon_x(wanted) < icon_x(focused) ? P8_PAD_LEFT : P8_PAD_RIGHT;
        if (!tap(author, script, step)) return 0;
        if (pad_nav_build_icon(nav) == focused) return fail(author, script, "build: the bar's focus did not move");
        focused = pad_nav_build_icon(nav);
    }
    if (!emit(author, script, P8_PAD_CROSS)) return 0;
    if (!wait_until(author, script, is_carrying, NULL, 4, "build: the structure carried")) return 0;
    if (!emit(author, script, 0) || !pan_to(author, script, x, y)) return 0;
    placed_t placed = {x, y, kind};
    if (!emit(author, script, P8_PAD_CROSS)) return 0;
    if (!wait_until(author, script, is_placed, &placed, 6, "build: the structure built on the point")) return 0;
    return leave_aim_if_able(author, script);
}

static int is_tech_button(const entity_t *e, const char *tech) {
    if (e == NULL || e->class != button_class()) return 0;
    const button_t *button = (const button_t *)e;
    return button->technology != NULL && button->technology->name != NULL && strcmp(button->technology->name, tech) == 0;
}

typedef struct {
    int count;
} techs_t;

static int techs_reached(pad_author_t *author, const void *context) {
    (void)author;
    return invented_count() >= ((const techs_t *)context)->count;
}

static int tech_panel_up(pad_author_t *author, const void *context) {
    (void)context;
    return pad_nav_tech_open(nav_of(author)) && pad_nav_panel_open() && pad_nav_focus(nav_of(author)) != NULL;
}

static int item_path(const entity_t *from, item_match_t match, const char *name, int *directions) {
    enum { ITEMS_MAX = 64 };
    const entity_t *items[ITEMS_MAX];
    int previous[ITEMS_MAX], via[ITEMS_MAX];
    int count = 1, found = -1;
    items[0] = from;
    previous[0] = -1;
    for (int head = 0; head < count && found < 0; head++) {
        if (match(items[head], name)) {
            found = head;
            break;
        }
        for (int d = 0; d < PAD_JUMP_DIRECTIONS && count < ITEMS_MAX; d++) {
            const entity_t *next = pad_nav_neighbor(items[head], pad_jump_direction(d));
            int seen = next == NULL;
            for (int i = 0; i < count && !seen; i++) seen = items[i] == next;
            if (seen) continue;
            items[count] = next;
            previous[count] = head;
            via[count] = pad_jump_direction(d);
            count++;
        }
    }
    if (found < 0) return -1;
    int length = 0;
    for (int at = found; at != 0; at = previous[at]) length++;
    if (length > PATH_STEPS_MAX) return -1;
    int k = length;
    for (int at = found; at != 0; at = previous[at]) directions[--k] = via[at];
    return length;
}

static int move_to_item(pad_author_t *author, const script_t *script, item_match_t match, const char *name,
                        const char *command) {
    pad_nav_t *nav = nav_of(author);
    int directions[PATH_STEPS_MAX];
    int length = item_path(pad_nav_focus(nav), match, name, directions);
    if (length < 0) return fail(author, script, "%s: no row '%s' it can press", command, name);
    for (int i = 0; i < length; i++) {
        entity_t *expected = pad_nav_neighbor(pad_nav_focus(nav), directions[i]);
        if (!tap(author, script, directions[i])) return 0;
        if (pad_nav_focus(nav) != expected) return fail(author, script, "%s: a tap missed its row", command);
    }
    return 1;
}

static int is_project_button(const entity_t *e, const char *project) {
    if (e == NULL || e->class != button_class()) return 0;
    const research_project_t *chosen = research_project_get(((const button_t *)e)->project);
    return chosen != NULL && chosen->name != NULL && strcmp(chosen->name, project) == 0;
}

static void join_words(char *out, size_t size, char *const *words, int count) {
    out[0] = '\0';
    for (int i = 0; i < count; i++) {
        size_t used = strlen(out);
        snprintf(out + used, size - used, "%s%s", i > 0 ? " " : "", words[i]);
    }
}

static int run_research(pad_author_t *author, const script_t *script, const char *tech) {
    if (!require_map(author, script, "research")) return 0;
    techs_t wanted = {invented_count() + 1};
    if (!tap(author, script, P8_PAD_R1)) return 0;
    if (!wait_until(author, script, tech_panel_up, NULL, 10, "research: the tech panel") ||
        !wait_panel_settled(author, script, "research")) {
        return 0;
    }
    if (!move_to_item(author, script, is_tech_button, tech, "research") || !tap(author, script, P8_PAD_CROSS)) return 0;
    if (!wait_until(author, script, techs_reached, &wanted, 4, "research: invented")) return 0;
    for (int i = 0; i < 4 && !on_the_map(author, NULL); i++) {
        if (!emit(author, script, 0)) return 0;
    }
    if (current_mode(author) == PAD_METRICS_MODE_TECH && !tap(author, script, P8_PAD_CIRCLE)) return 0;
    return wait_until(author, script, on_the_map, NULL, 4, "research: back on the map");
}

static int score_panel_up(pad_author_t *author, const void *context) {
    (void)context;
    return pad_nav_score_open(nav_of(author)) && pad_nav_panel_open();
}

static int tech_drawn(pad_author_t *author, const void *context) {
    (void)context;
    return pad_nav_tech_open(nav_of(author)) && pad_nav_panel_open();
}

static int run_score(pad_author_t *author, const script_t *script) {
    if (!require_map(author, script, "score")) return 0;
    return tap(author, script, P8_PAD_R1) && wait_until(author, script, tech_drawn, NULL, 10, "score: the tech panel") &&
           tap(author, script, P8_PAD_R1) && wait_until(author, script, score_panel_up, NULL, 10, "score: the score panel") &&
           tap(author, script, P8_PAD_R1) && wait_until(author, script, panel_closed_on_map, NULL, 10, "score: back on the map");
}

typedef struct {
    p8_fixed_t year;
} year_t;

static int year_reached(pad_author_t *author, const void *context) {
    (void)author;
    const economy_t *economy = main_economy_typed();
    return economy != NULL && economy->year >= ((const year_t *)context)->year;
}

static int chain_closed(pad_author_t *author, const void *context) {
    (void)context;
    return !pad_nav_year_chain_open(nav_of(author));
}

static int run_year(pad_author_t *author, const script_t *script) {
    if (!require_map(author, script, "year")) return 0;
    year_t next = {p8_fixed_add(main_economy_typed()->year, p8_fixed_from_int(1))};
    if (!emit_repeated(author, script, P8_PAD_SELECT, PAD_SELECT_HOLD_FRAMES)) return 0;
    if (!wait_until(author, script, year_reached, &next, 4, "year: the year ended")) return 0;
    if (main_economy_typed()->year != next.year) return fail(author, script, "year: more than one year ended");
    return wait_until(author, script, chain_closed, NULL, PAD_SELECT_CHAIN_FRAMES + 4, "year: the chain window closed");
}

static int final_score_up(pad_author_t *author, const void *context) {
    (void)context;
    return main_economy_typed() != NULL && current_mode(author) == PAD_METRICS_MODE_MENU && !pad_panel_box_open() &&
           pad_nav_first_item() != NULL;
}

static int run_until_game_end(pad_author_t *author, const script_t *script) {
    for (int years = 0; !final_score_up(author, NULL); years++) {
        if (years == YEARS_TO_END_MAX) return fail(author, script, "until-game-end: no final score after %d years", years);
        if (!run_year(author, script)) return 0;
    }
    return 1;
}

static int hovered_moved(pad_author_t *author, const void *context) {
    entity_t *hovered = pad_nav_hovered(nav_of(author));
    return hovered != NULL && hovered != context;
}

static int run_menu(pad_author_t *author, const script_t *script, const char *id, int dwell) {
    if (!require_map(author, script, "menu")) return 0;
    if (current_mode(author) == PAD_METRICS_MODE_AIM && !leave_aim(author, script)) return 0;
    if (!run_jump(author, script, id) || !tap(author, script, P8_PAD_TRIANGLE)) return 0;
    if (!wait_until(author, script, has_focus, NULL, 20, "menu: a row focused") ||
        !wait_panel_settled(author, script, "menu")) {
        return 0;
    }
    if (pad_nav_neighbor(pad_nav_focus(nav_of(author)), P8_PAD_DOWN) != NULL && !move_down_rows(author, script, 1)) return 0;
    if (dwell > 0 && !emit_repeated(author, script, 0, dwell)) return 0;
    return tap(author, script, P8_PAD_CIRCLE) &&
           wait_until(author, script, panel_closed_on_map, NULL, 6, "menu: closed by circle");
}

static int run_bar(pad_author_t *author, const script_t *script) {
    if (!require_map(author, script, "bar") || !tap(author, script, P8_PAD_L1)) return 0;
    pad_nav_t *nav = nav_of(author);
    if (pad_nav_build_icon(nav) == NULL) return fail(author, script, "bar: not open");
    for (int i = 0; i < STRUCTURE_KIND_COUNT; i++) {
        entity_t *focused = pad_nav_build_icon(nav);
        int x = icon_x(focused), left = 0;
        const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
        for (int k = 0; k < count_of(bucket); k++) {
            left |= structure_icon_is_revealed(bucket->items[k]) && icon_x(bucket->items[k]) < x;
        }
        if (!left) break;
        if (!tap(author, script, P8_PAD_LEFT)) return 0;
        if (pad_nav_build_icon(nav) == focused) return fail(author, script, "bar: Left did not move");
    }
    return tap(author, script, P8_PAD_CIRCLE) && wait_until(author, script, on_the_map, NULL, 4, "bar: closed by circle");
}

static int run_attention(pad_author_t *author, const script_t *script) {
    if (!require_map(author, script, "attention")) return 0;
    const entity_t *before = pad_nav_hovered(nav_of(author));
    return tap(author, script, P8_PAD_SELECT) &&
           wait_until(author, script, hovered_moved, before, 2, "attention: a jump to another object");
}

static int run_back(pad_author_t *author, const script_t *script, int required) {
    author->next_circle_required = required;
    return emit(author, script, P8_PAD_CIRCLE) && emit(author, script, 0);
}

static int pause_shown(pad_author_t *author, const void *context) {
    (void)author;
    (void)context;
    return pause_menu_shown();
}

static int no_game(pad_author_t *author, const void *context) {
    (void)author;
    (void)context;
    return main_economy_typed() == NULL;
}

static int run_pause(pad_author_t *author, const script_t *script) {
    if (main_economy_typed() == NULL || pause_menu_shown()) return fail(author, script, "pause: no game to pause");
    return tap(author, script, P8_PAD_START) && wait_until(author, script, pause_shown, NULL, 4, "pause: the menu");
}

static int run_leave_game(pad_author_t *author, const script_t *script, const char *command, condition_t shown) {
    if (!shown(author, NULL)) return fail(author, script, "%s: not on its menu", command);
    if (!wait_until(author, script, has_focus, NULL, 20, command) || !wait_panel_settled(author, script, command)) return 0;
    return tap(author, script, P8_PAD_CROSS) && wait_until(author, script, no_game, NULL, 4, command);
}

static int run_file(pad_author_t *author, const char *path, const char *until_label, int depth);

static void include_path(char *out, size_t size, const char *including, const char *included) {
    const char *slash = strrchr(including, '/');
    if (included[0] == '/' || slash == NULL) {
        snprintf(out, size, "%s", included);
    } else {
        snprintf(out, size, "%.*s/%s", (int)(slash - including), including, included);
    }
}

static int expect_tokens(pad_author_t *author, const script_t *script, int low, int high) {
    int count = script->current->token_count;
    if (count < low || count > high) {
        return fail(author, script, "%s: %d argument(s), expected %d to %d", script->current->tokens[0], count - 1,
                    low - 1, high - 1);
    }
    return 1;
}

static int parse_point(pad_author_t *author, const script_t *script, int at, int *x, int *y) {
    char *const *tokens = script->current->tokens;
    if (!parse_coordinate(tokens[at], x) || !parse_coordinate(tokens[at + 1], y)) {
        return fail(author, script, "%s: bad world point '%s %s'", tokens[0], tokens[at], tokens[at + 1]);
    }
    return 1;
}

static int run_goal(pad_author_t *author, const script_t *script) {
    char *const *tokens = script->current->tokens;
    int count = script->current->token_count;
    const char *command = tokens[0];
    int x = 0, y = 0;
    if (strcmp(command, "jump") == 0) return expect_tokens(author, script, 2, 2) && run_jump(author, script, tokens[1]);
    if (strcmp(command, "probe") == 0) {
        return expect_tokens(author, script, 3, 3) && parse_point(author, script, 1, &x, &y) && run_probe(author, script, x, y);
    }
    if (strcmp(command, "fold") == 0) {
        if (!expect_tokens(author, script, 1, 3) || count == 2) return count == 2 ? fail(author, script, "fold: x without y") : 0;
        return (count == 1 || parse_point(author, script, 1, &x, &y)) && run_fold(author, script, x, y, count == 3);
    }
    if (strcmp(command, "colonise") == 0) {
        if (!expect_tokens(author, script, 2, 3)) return 0;
        int row = count == 3 ? parse_count(tokens[2]) : 1;
        const char *project = count == 3 && row < 0 ? tokens[2] : NULL;
        if (row == 0) return fail(author, script, "colonise: row 0 is the header");
        return run_colonise(author, script, tokens[1], row, project);
    }
    if (strcmp(command, "slipway") == 0) {
        if (count < 3) return fail(author, script, "slipway: needs a start and at least one target");
        return run_slipway(author, script, tokens + 1, count - 1);
    }
    if (strcmp(command, "build") == 0) {
        return expect_tokens(author, script, 4, 4) && parse_point(author, script, 2, &x, &y) &&
               run_build(author, script, tokens[1], x, y);
    }
    if (strcmp(command, "research") == 0) {
        if (count < 2) return fail(author, script, "research: needs a tech name");
        char tech[LINE_MAX_CHARS];
        join_words(tech, sizeof tech, tokens + 1, count - 1);
        return run_research(author, script, tech);
    }
    if (strcmp(command, "score") == 0) return expect_tokens(author, script, 1, 1) && run_score(author, script);
    if (strcmp(command, "year") == 0) return expect_tokens(author, script, 1, 1) && run_year(author, script);
    if (strcmp(command, "until-game-end") == 0) return expect_tokens(author, script, 1, 1) && run_until_game_end(author, script);
    if (strcmp(command, "menu") == 0) {
        if (!expect_tokens(author, script, 2, 3)) return 0;
        int dwell = count == 3 ? parse_count(tokens[2]) : 0;
        if (dwell < 0) return fail(author, script, "menu: bad dwell '%s'", tokens[2]);
        if (dwell > MENU_DWELL_MAX) return fail(author, script, "menu: dwell at most %d frames", MENU_DWELL_MAX);
        return run_menu(author, script, tokens[1], dwell);
    }
    if (strcmp(command, "bar") == 0) return expect_tokens(author, script, 1, 1) && run_bar(author, script);
    if (strcmp(command, "attention") == 0) return expect_tokens(author, script, 1, 1) && run_attention(author, script);
    if (strcmp(command, "back") == 0) {
        if (!expect_tokens(author, script, 1, 2)) return 0;
        if (count == 2 && strcmp(tokens[1], "--required") != 0) return fail(author, script, "back: unknown option '%s'", tokens[1]);
        return run_back(author, script, count == 2);
    }
    if (strcmp(command, "pause") == 0) return expect_tokens(author, script, 1, 1) && run_pause(author, script);
    if (strcmp(command, "restart") == 0) {
        return expect_tokens(author, script, 1, 1) && run_leave_game(author, script, "restart", pause_shown);
    }
    if (strcmp(command, "again") == 0) {
        return expect_tokens(author, script, 1, 1) && run_leave_game(author, script, "again", final_score_up);
    }
    if (strcmp(command, "new-game") == 0) {
        if (!expect_tokens(author, script, 2, 2)) return 0;
        int difficulty = difficulty_index(tokens[1]);
        if (difficulty < 0) return fail(author, script, "new-game: unknown difficulty '%s'", tokens[1]);
        return run_title(author, script, author->frame_count + TITLE_ROWS_FRAMES + 2 * difficulty, difficulty);
    }
    return fail(author, script, "unknown command '%s'", command);
}

static int run_line(pad_author_t *author, const script_t *script, int depth) {
    char *const *tokens = script->current->tokens;
    const char *command = tokens[0];
    if (strcmp(command, "idle") == 0) {
        if (!expect_tokens(author, script, 2, 2)) return 0;
        int count = parse_count(tokens[1]);
        if (count < 1) return fail(author, script, "idle: bad frame count '%s'", tokens[1]);
        return emit_repeated(author, script, 0, count);
    }
    if (strcmp(command, "tap") == 0 || strcmp(command, "hold") == 0) {
        int holds = command[0] == 'h';
        if (!expect_tokens(author, script, holds ? 3 : 2, holds ? 3 : 2)) return 0;
        int buttons = parse_buttons(tokens[1]);
        if (buttons == 0) return fail(author, script, "%s: unknown buttons '%s'", command, tokens[1]);
        int count = holds ? parse_count(tokens[2]) : 1;
        if (count < 1) return fail(author, script, "hold: bad frame count '%s'", tokens[2]);
        return emit_repeated(author, script, buttons, count);
    }
    if (strcmp(command, "title") == 0) {
        if (!expect_tokens(author, script, 3, 3)) return 0;
        int frame = parse_count(tokens[1]);
        int difficulty = difficulty_index(tokens[2]);
        if (frame < 0) return fail(author, script, "title: bad frame '%s'", tokens[1]);
        if (difficulty < 0) return fail(author, script, "title: unknown difficulty '%s'", tokens[2]);
        return run_title(author, script, frame, difficulty);
    }
    if (strcmp(command, "include") == 0) {
        if (!expect_tokens(author, script, 2, 3)) return 0;
        if (depth + 1 == INCLUDE_DEPTH_MAX) return fail(author, script, "include: nested deeper than %d", INCLUDE_DEPTH_MAX);
        char path[PATH_MAX_CHARS];
        include_path(path, sizeof path, script->path, tokens[1]);
        return run_file(author, path, script->current->token_count == 3 ? tokens[2] : NULL, depth + 1);
    }
    if (strcmp(command, "stop-at") == 0) {
        if (!expect_tokens(author, script, 2, 2)) return 0;
        if (author->stop_label != NULL) return fail(author, script, "stop-at: already stopping at '%s'", author->stop_label);
        snprintf(stop_label_copy, sizeof stop_label_copy, "%s", tokens[1]);
        author->stop_label = stop_label_copy;
        return 1;
    }
    if (strcmp(command, "label") == 0) {
        return expect_tokens(author, script, 2, 2);
    }
    return run_goal(author, script);
}

static int tokenize(pad_author_t *author, const script_t *script, script_line_t *line) {
    line->token_count = 0;
    for (char *token = strtok(line->text, " \t"); token != NULL; token = strtok(NULL, " \t")) {
        if (line->token_count == TOKENS_MAX) {
            snprintf(author->error, sizeof author->error, "%s:%d: more than %d words", script->path, line->number,
                     TOKENS_MAX);
            return 0;
        }
        line->tokens[line->token_count++] = token;
    }
    return 1;
}

static int load_lines(pad_author_t *author, script_t *script) {
    FILE *in = fopen(script->path, "r");
    if (in == NULL) return fail(author, NULL, "cannot read the script '%s'", script->path);
    int capacity = 0;
    char text[LINE_MAX_CHARS + 1];
    int number = 0;
    while (fgets(text, sizeof text, in) != NULL) {
        number++;
        size_t length = strlen(text);
        if (length == LINE_MAX_CHARS && text[length - 1] != '\n') {
            fclose(in);
            snprintf(author->error, sizeof author->error, "%s:%d: line longer than %d characters", script->path, number,
                     LINE_MAX_CHARS - 1);
            return 0;
        }
        text[strcspn(text, "#\r\n")] = '\0';
        if (strspn(text, " \t") == strlen(text)) continue;
        if (script->line_count == capacity) {
            capacity = capacity == 0 ? 64 : 2 * capacity;
            script_line_t *grown = realloc(script->lines, (size_t)capacity * sizeof *grown);
            assert(grown != NULL);
            script->lines = grown;
        }
        script_line_t *line = &script->lines[script->line_count++];
        memcpy(line->text, text, sizeof line->text);
        line->number = number;
    }
    fclose(in);
    /* Tokens point into each line's text: split only once realloc is done. */
    for (int i = 0; i < script->line_count; i++) {
        if (!tokenize(author, script, &script->lines[i])) return 0;
    }
    return 1;
}

static int has_label(const script_t *script, const char *label) {
    for (int i = 0; i < script->line_count; i++) {
        const script_line_t *line = &script->lines[i];
        if (line->token_count == 2 && strcmp(line->tokens[0], "label") == 0 && strcmp(line->tokens[1], label) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_label(const script_line_t *line, const char *label) {
    return label != NULL && line->token_count == 2 && strcmp(line->tokens[0], "label") == 0 &&
           strcmp(line->tokens[1], label) == 0;
}

static int run_file(pad_author_t *author, const char *path, const char *until_label, int depth) {
    script_t script = {.path = path};
    int ok = load_lines(author, &script);
    if (ok && until_label != NULL && !has_label(&script, until_label)) {
        ok = fail(author, &script, "include: no label '%s'", until_label);
    }
    for (int i = 0; ok && i < script.line_count && !author->stopped; i++) {
        script.current = &script.lines[i];
        if (is_label(script.current, author->stop_label)) {
            author->stopped = 1;
            break;
        }
        if (is_label(script.current, until_label)) break;
        ok = run_line(author, &script, depth);
    }
    if (ok && depth == 0) {
        pad_metrics_view_t now = pad_metrics_view_read(&author->step);
        ok = settle_circle(author, &script, &now, 1);
    }
    free(script.lines);
    return ok;
}

int pad_author_run(pad_author_t *author, const char *script_path, const char *stop_label) {
    assert(author != NULL && script_path != NULL);
    author->stop_label = stop_label;
    author->stopped = 0;
    author->error[0] = '\0';
    if (!run_file(author, script_path, NULL, 0)) return 0;
    if (author->stop_label != NULL && !author->stopped) {
        snprintf(author->error, sizeof author->error, "%s: frame %d: the run never reached label '%s'", script_path,
                 author->frame_count, author->stop_label);
        return 0;
    }
    return 1;
}

int pad_author_run_line(pad_author_t *author, const char *commands) {
    assert(author != NULL && commands != NULL);
    script_line_t line;
    script_t script = {.path = "try", .lines = &line, .line_count = 1, .current = &line};
    const char *at = commands;
    line.number = 0;
    while (*at != '\0') {
        size_t length = strcspn(at, ";");
        snprintf(line.text, sizeof line.text, "%.*s", (int)length, at);
        line.text[strcspn(line.text, "#\r\n")] = '\0';
        line.number++;
        at += length + (at[length] == ';');
        if (!tokenize(author, &script, &line)) return 0;
        if (line.token_count > 0 && !run_line(author, &script, 0)) return 0;
    }
    pad_metrics_view_t now = pad_metrics_view_read(&author->step);
    return settle_circle(author, &script, &now, 1);
}
