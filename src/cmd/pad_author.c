/* usage: pad_author <script> <events.json> [--stop-at <label>] [--replay <replay binary>] [--no-aim-budget]
 *        [--try <file>]
 *
 * Plays the script headless and writes the pad frames it pressed to
 * <events.json>, or, when a command fails, to tmp/pad-author/<name>.failed.json
 * with the state in the default place. STATE=<file>: the state of the last frame, as the replay
 * tool's STATE writes it (default tmp/pad-author/<name>.state.txt). Then
 * replays <events.json> with the replay binary (default tmp/replay) and exits
 * 1 unless its state file is byte-identical.
 *
 * Writes the per-frame mode log and the run's totals to
 * tmp/pad-author/<name>.modes.txt, the totals also to stdout. A whole run
 * (no --stop-at) fails when 25% or more of its in-game frames (neither title
 * nor pause) are in aim mode, unless --no-aim-budget.
 *
 * --try <file>: after the run, each line of <file> (commands separated by `;`)
 * runs in its own fork from the run's last frame, writes its state to
 * tmp/pad-author/<name>.try<k>.txt and prints `try <k> ok <frames> science/year <n> aim <aim frames>` or
 * `try <k> failed <error>`; nothing it does reaches <events.json>. */

#define _POSIX_C_SOURCE 200809L

#include "devtools/pad_author.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/main_economy.h"
#include "game/map_tables.h"
#include "game/planet.h"
#include "game/research.h"
#include "engine/entity_registry.h"
#include "game/economy_formulas.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define WORK_DIR "tmp/pad-author"
#define PATH_CHARS 512
#define NAME_CHARS 128

static pad_author_t author;

static void usage(void) {
    fprintf(stderr, "usage: pad_author <script> <events.json> [--stop-at <label>] [--replay <replay binary>] "
                    "[--no-aim-budget] [--try <file>]\n");
}

static void base_name(char *out, size_t size, const char *path) {
    const char *slash = strrchr(path, '/');
    const char *name = slash != NULL ? slash + 1 : path;
    size_t length = strcspn(name, ".");
    snprintf(out, size, "%.*s", (int)length, name);
}

static int write_state(const char *path) {
    state_release();
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "pad_author: cannot open the state file '%s'\n", path);
        return 0;
    }
    state_write(out);
    fprintf(out, "cheat activations %d\n", pad_metrics_cheat_activations(&author.metrics));
    return fclose(out) == 0;
}

static int replay(const char *replay_binary, const char *events, const char *state_path, const char *dump_path) {
    remove(state_path);
    fflush(NULL);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (setenv("STATE", state_path, 1) != 0) _exit(127);
        if (freopen("/dev/null", "w", stdout) == NULL) _exit(127);
        execl(replay_binary, replay_binary, events, "/dev/null", dump_path, (char *)NULL);
        _exit(127);
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child) return 0;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static int same_file(const char *left, const char *right) {
    FILE *a = fopen(left, "rb");
    FILE *b = fopen(right, "rb");
    int same = a != NULL && b != NULL;
    while (same) {
        int ca = fgetc(a);
        int cb = fgetc(b);
        same = ca == cb;
        if (ca == EOF) break;
    }
    if (a != NULL) fclose(a);
    if (b != NULL) fclose(b);
    return same;
}

/* Keep in step with economy_new_year's science read, its +1 left out. */
static int science_per_year(void) {
    economy_t *economy = main_economy_typed();
    int k;
    p8_fixed_t science = 0;
    if (economy == NULL || !(economy->has & ECONOMY_HAS_PRODUCTION) || !resource_code_lookup("k", &k)) return 0;
    resource_map_get(&economy->production, p8_fixed_from_int(k), &science);
    return p8_fixed_flr_int(science);
}

static const char *resource_letter(p8_fixed_t number) {
    for (int i = 0; i < RESOURCE_CODE_COUNT; i++) {
        if (p8_fixed_from_int(resource_codes[i].resource) == number) return resource_codes[i].key;
    }
    return "-";
}

static void write_economy(const char *path) {
    FILE *out = fopen(path, "w");
    if (out == NULL) return;
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; planets != NULL && i < planets->count; i++) {
        const planet_t *p = (const planet_t *)planets->items[i];
        if (p->header.state != ENTITY_STATE_SETTLED) continue;
        const research_project_t *project = research_project_get(p->project);
        const production_t *made = planet_production_read(p);
        fprintf(out, "eco %s project %s tier %d makes %s %d out %d in %d needs", planet_id_or_null(p) ? planet_id_or_null(p) : "-",
                project != NULL && project->name != NULL && project->name[0] ? project->name : "-",
                p->has_development_tier ? p8_fixed_flr_int(p->development_tier) : -1,
                made->has_resource ? resource_letter(made->resource) : "-", p8_fixed_flr_int(made->amount),
                (p->has & PLANET_HAS_OUTGOING_ROUTES) ? trade_route_list_count(&p->outgoing_routes) : 0,
                (p->has & PLANET_HAS_INCOMING_ROUTES) ? trade_route_list_count(&p->incoming_routes) : 0);
        for (int n = 0; (p->has & PLANET_HAS_NEEDS) && n < p->needs.count; n++) {
            const need_t *want = p->needs.items[n];
            if (want == NULL) continue;
            resource_t r = need_resource(want);
            fprintf(out, " %s:%d", r.kind == RESOURCE_FIXED ? resource_letter(r.number) : "?",
                    want->has_satisfied ? p8_fixed_flr_int(want->satisfied) : -1);
        }
        fputc('\n', out);
    }
    fclose(out);
}

static int run_trials(const char *try_path, const char *name) {
    FILE *in = fopen(try_path, "r");
    if (in == NULL) {
        fprintf(stderr, "pad_author: cannot read '%s'\n", try_path);
        return 0;
    }
    char line[1024];
    for (int k = 0; fgets(line, sizeof line, in) != NULL; k++) {
        line[strcspn(line, "\r\n")] = '\0';
        fflush(NULL);
        pid_t child = fork();
        assert(child >= 0);
        if (child == 0) {
            int ok = pad_author_run_line(&author, line);
            char path[PATH_CHARS];
            snprintf(path, sizeof path, "%s/%s.try%d.txt", WORK_DIR, name, k);
            write_state(path);
            snprintf(path, sizeof path, "%s/%s.try%d.eco.txt", WORK_DIR, name, k);
            write_economy(path);
            if (ok) {
                printf("try %d ok %d science/year %d aim %d\n", k, author.frame_count, science_per_year(),
                       pad_metrics_mode_frames(&author.metrics, PAD_METRICS_MODE_AIM));
            } else {
                printf("try %d failed %s\n", k, author.error);
            }
            fflush(NULL);
            _exit(0);
        }
        int status = 0;
        waitpid(child, &status, 0);
        if (!WIFEXITED(status)) printf("try %d failed: the fork stopped\n", k);
    }
    fclose(in);
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 3 || argv[1][0] == '-' || argv[2][0] == '-') {
        usage();
        return 2;
    }
    const char *script_path = argv[1];
    const char *events_path = argv[2];
    const char *stop_label = NULL;
    const char *replay_binary = "tmp/replay";
    int aim_budget = 1;
    const char *try_path = NULL;
    for (int a = 3; a < argc; a++) {
        if (strcmp(argv[a], "--stop-at") == 0 && a + 1 < argc) {
            stop_label = argv[++a];
        } else if (strcmp(argv[a], "--try") == 0 && a + 1 < argc) {
            try_path = argv[++a];
        } else if (strcmp(argv[a], "--no-aim-budget") == 0) {
            aim_budget = 0;
        } else if (strcmp(argv[a], "--replay") == 0 && a + 1 < argc) {
            replay_binary = argv[++a];
        } else {
            usage();
            return 2;
        }
    }

    static char name[NAME_CHARS];
    static char own_state[PATH_CHARS];
    static char replay_state[PATH_CHARS];
    static char replay_dump[PATH_CHARS];
    static char mode_log_path[PATH_CHARS];
    base_name(name, sizeof name, events_path);
    mkdir("tmp", 0777);
    mkdir(WORK_DIR, 0777);
    const char *state_path = getenv("STATE");
    snprintf(own_state, sizeof own_state, "%s/%s.state.txt", WORK_DIR, name);
    if (state_path == NULL) state_path = own_state;
    snprintf(replay_state, sizeof replay_state, "%s/%s.replay-state.txt", WORK_DIR, name);
    snprintf(replay_dump, sizeof replay_dump, "%s/%s.replay-dump.txt", WORK_DIR, name);
    snprintf(mode_log_path, sizeof mode_log_path, "%s/%s.modes.txt", WORK_DIR, name);

    static p8_vram_t memory;
    static p8_graphics_t graphics;
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    pad_author_init(&author);
    author.mode_log = fopen(mode_log_path, "w");
    if (author.mode_log == NULL) {
        fprintf(stderr, "pad_author: cannot open the mode log '%s'\n", mode_log_path);
        return 1;
    }
    cart_load();
    cart_init();

    state_write_on_exit(state_path, &author.frame_count);
    int played = pad_author_run(&author, script_path, stop_label);
    static char failed_events[PATH_CHARS];
    if (!played) {
        fprintf(stderr, "pad_author: %s\n", author.error);
        snprintf(failed_events, sizeof failed_events, "%s/%s.failed.json", WORK_DIR, name);
        events_path = failed_events;
        state_path = own_state;
    }
    pad_metrics_write_totals(&author.metrics, author.mode_log, "total ");
    fclose(author.mode_log);
    author.mode_log = NULL;
    if (author.frame_count == 0) {
        state_release();
        fprintf(stderr, "pad_author: %s pressed no frame, '%s' not written\n", script_path, events_path);
        return 1;
    }
    if (!p8_input_save(author.frames, author.frame_count, events_path, NULL)) {
        fprintf(stderr, "pad_author: cannot write '%s'\n", events_path);
        return 1;
    }
    if (!write_state(state_path)) return 1;
    printf("pad_author: %s -> %s, %d frame(s), cheat activations %d\n", script_path, events_path, author.frame_count,
           pad_metrics_cheat_activations(&author.metrics));
    pad_metrics_write_totals(&author.metrics, stdout, "pad_author: ");
    if (!played) return 1;
    if (aim_budget && stop_label == NULL && !pad_metrics_aim_ok(&author.metrics)) {
        fprintf(stderr, "pad_author: %d of %d in-game frames in aim mode, 25%% or more\n",
                pad_metrics_mode_frames(&author.metrics, PAD_METRICS_MODE_AIM),
                pad_metrics_in_game_frames(&author.metrics));
        return 1;
    }

    if (!replay(replay_binary, events_path, replay_state, replay_dump)) {
        fprintf(stderr, "pad_author: replaying '%s' with %s failed\n", events_path, replay_binary);
        return 1;
    }
    if (try_path != NULL && !run_trials(try_path, name)) return 1;
    if (!same_file(state_path, replay_state)) {
        fprintf(stderr, "pad_author: generator bug: %s and the replay's %s differ\n", state_path, replay_state);
        return 1;
    }
    printf("pad_author: self-check passed, %s replays to the same state\n", events_path);
    return 0;
}
