#define _POSIX_C_SOURCE 199309L

#include "devtools/call_profile.h"
#include "devtools/capture_list.h"
#include "devtools/draw_filter.h"
#include "devtools/record_start.h"
#include "devtools/replay_source.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "game/bindings.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { SYMBOL_CAP = 16384, SYMBOL_NAME_CAP = 128, NAME_CAP = 160, PATH_CAP = 1024, MEASURED_PAIRS = 1000000 };

static const char *const ROOT_NAMES[CALL_PROFILE_ROOT_COUNT] = {"update", "draw"};

#define NO_INSTRUMENT __attribute__((no_instrument_function))

static int hooks_on;

void __cyg_profile_func_enter(void *function, void *call_site) NO_INSTRUMENT;
void __cyg_profile_func_exit(void *function, void *call_site) NO_INSTRUMENT;

void __cyg_profile_func_enter(void *function, void *call_site) {
    if (hooks_on) call_profile_enter((uintptr_t)function, (uintptr_t)call_site);
}

void __cyg_profile_func_exit(void *function, void *call_site) {
    if (hooks_on) call_profile_exit((uintptr_t)function, (uintptr_t)call_site);
}

/* Runs before state.c's exit writer, which calls instrumented code from wherever p8_fatal stopped. */
static void hooks_off(void) {
    hooks_on = 0;
}

static uint64_t monotonic_ns(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000u + (uint64_t)now.tv_nsec;
}

typedef struct {
    uintptr_t address;
    char name[SYMBOL_NAME_CAP];
} symbol_t;

static symbol_t symbols[SYMBOL_CAP];
static int symbol_count;
static uintptr_t load_offset;

/* Returns 0 after naming the failure. */
static int symbols_read(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "frame_profile: cannot read the symbol table '%s'\n", path);
        return 0;
    }
    char line[512];
    while (fgets(line, sizeof line, file) != NULL) {
        unsigned long long address;
        char type;
        char name[SYMBOL_NAME_CAP];
        if (sscanf(line, "%llx %c %127s", &address, &type, name) != 3) continue;
        if (strchr("TtWw", type) == NULL) continue;
        if (symbol_count == SYMBOL_CAP) {
            fprintf(stderr, "frame_profile: '%s' has more than %d text symbols\n", path, SYMBOL_CAP);
            fclose(file);
            return 0;
        }
        symbol_t *symbol = &symbols[symbol_count++];
        symbol->address = (uintptr_t)address;
#ifdef __APPLE__
        snprintf(symbol->name, sizeof symbol->name, "%s", name[0] == '_' ? name + 1 : name);
#else
        snprintf(symbol->name, sizeof symbol->name, "%s", name);
#endif
    }
    fclose(file);
    for (int s = 0; s < symbol_count; s++) {
        if (strcmp(symbols[s].name, "cart_draw") == 0) {
            load_offset = (uintptr_t)cart_draw - symbols[s].address;
            return 1;
        }
    }
    fprintf(stderr, "frame_profile: '%s' has no cart_draw text symbol\n", path);
    return 0;
}

/* Returns 0 after naming the failure. */
static int symbol_resolve(const char *path, const char *name, uintptr_t *address) {
    int found = 0;
    for (int s = 0; s < symbol_count; s++) {
        if (strcmp(symbols[s].name, name) != 0) continue;
        if (found && symbols[s].address + load_offset != *address) {
            fprintf(stderr, "frame_profile: '%s' has several text symbols '%s'\n", path, name);
            return 0;
        }
        *address = symbols[s].address + load_offset;
        found = 1;
    }
    if (!found) fprintf(stderr, "frame_profile: '%s' has no text symbol '%s'\n", path, name);
    return found;
}

/* Returns 0 after naming the failure. */
static int roots_resolve(const char *path, const char *draw_name, const char *update_name, uintptr_t *draw,
                         uintptr_t *update) {
    if (strcmp(draw_name, "cart_update60") == 0) {
        fprintf(stderr, "frame_profile: the draw root cannot be cart_update60\n");
        return 0;
    }
    if (strcmp(update_name, "cart_draw") == 0 || strcmp(update_name, draw_name) == 0) {
        fprintf(stderr, "frame_profile: the update root cannot be %s\n", update_name);
        return 0;
    }
    return symbol_resolve(path, draw_name, draw) && symbol_resolve(path, update_name, update);
}

static void symbol_name(uintptr_t function, char *buffer, size_t size) {
    uintptr_t address = function - load_offset;
    const symbol_t *found = NULL;
    int shared = 0;
    for (int s = 0; s < symbol_count; s++) {
        if (symbols[s].address == address && found == NULL) found = &symbols[s];
    }
    if (found == NULL) {
        snprintf(buffer, size, "fn@%lx", (unsigned long)address);
        return;
    }
    for (int s = 0; s < symbol_count; s++) {
        if (strcmp(symbols[s].name, found->name) == 0 && symbols[s].address != address) shared = 1;
    }
    if (shared) snprintf(buffer, size, "%s@%lx", found->name, (unsigned long)address);
    else snprintf(buffer, size, "%s", found->name);
}

typedef struct {
    char names[CALL_PROFILE_ROOT_COUNT][CALL_PROFILE_CHILD_CAP][NAME_CAP];
    uint64_t pair_tenths_ns;
    const char *events;
    const char *draw_root;
    const char *update_root;
} report_t;

static void name_children(report_t *report) {
    for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
        for (int child = 0; child < call_profile_child_count((call_profile_root_t)root); child++) {
            call_profile_child_t info = call_profile_child_info((call_profile_root_t)root, child);
            char symbol[NAME_CAP];
            symbol_name(info.function, symbol, sizeof symbol);
            call_profile_child_name(info, symbol, report->names[root][child], NAME_CAP);
        }
    }
}

static void write_header(FILE *out, const report_t *report) {
    fprintf(out, "profile %s frames %d hook-pair-ns %llu.%llu clock monotonic-ns\n", report->events,
            call_profile_frame_count(), (unsigned long long)(report->pair_tenths_ns / 10),
            (unsigned long long)(report->pair_tenths_ns % 10));
    fprintf(out, "root draw %s\n", report->draw_root);
    fprintf(out, "root update %s\n", report->update_root);
}

static void write_frames(FILE *out, const report_t *report) {
    write_header(out, report);
    for (int frame = 0; frame < call_profile_frame_count(); frame++) {
        fprintf(out, "frame %d update %llu draw %llu\n", frame,
                (unsigned long long)call_profile_root_ns(frame, CALL_PROFILE_UPDATE),
                (unsigned long long)call_profile_root_ns(frame, CALL_PROFILE_DRAW));
        for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
            for (int child = 0; child < call_profile_child_count((call_profile_root_t)root); child++) {
                call_profile_child_figures_t figures = call_profile_child_figures(frame, (call_profile_root_t)root, child);
                fprintf(out, "frame %d child %s %s %lu %llu %llu %lu\n", frame, ROOT_NAMES[root],
                        report->names[root][child], (unsigned long)figures.calls, (unsigned long long)figures.ns,
                        (unsigned long long)call_profile_net_ns(figures, report->pair_tenths_ns),
                        (unsigned long)figures.below);
            }
        }
        for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
            fprintf(out, "frame %d self %s %llu\n", frame, ROOT_NAMES[root],
                    (unsigned long long)call_profile_self_ns(frame, (call_profile_root_t)root));
        }
        for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
            call_profile_family_figures_t figures = call_profile_family_figures(frame, (draw_filter_family_t)family);
            fprintf(out, "frame %d family %s %lu %llu units %lu switches %lu\n", frame,
                    capture_list_family_name((draw_filter_family_t)family), (unsigned long)figures.calls,
                    (unsigned long long)figures.ns, (unsigned long)figures.units, (unsigned long)figures.switches);
        }
        fprintf(out, "frame %d in-call %llu\n", frame, (unsigned long long)call_profile_in_call_ns(frame));
    }
}

/* Writes " <pct>%" without ending the line. */
static void write_share_field(FILE *out, uint64_t part, uint64_t total) {
    uint64_t tenths = total == 0 ? 0 : (part * 1000 + total / 2) / total;
    fprintf(out, " %llu.%llu%%", (unsigned long long)(tenths / 10), (unsigned long long)(tenths % 10));
}

static void write_share(FILE *out, uint64_t part, uint64_t total) {
    write_share_field(out, part, total);
    fputc('\n', out);
}

static void write_summary(FILE *out, const report_t *report) {
    write_header(out, report);
    int frames = call_profile_frame_count();
    uint64_t roots[CALL_PROFILE_ROOT_COUNT] = {0, 0};
    uint64_t in_call = 0;
    for (int frame = 0; frame < frames; frame++) {
        for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
            roots[root] += call_profile_root_ns(frame, (call_profile_root_t)root);
        }
        in_call += call_profile_in_call_ns(frame);
    }
    fprintf(out, "update %llu draw %llu\n", (unsigned long long)roots[CALL_PROFILE_UPDATE],
            (unsigned long long)roots[CALL_PROFILE_DRAW]);
    uint64_t children[CALL_PROFILE_ROOT_COUNT] = {0, 0};
    for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
        for (int child = 0; child < call_profile_child_count((call_profile_root_t)root); child++) {
            uint64_t calls = 0, ns = 0, net = 0, below = 0;
            for (int frame = 0; frame < frames; frame++) {
                call_profile_child_figures_t figures = call_profile_child_figures(frame, (call_profile_root_t)root, child);
                calls += figures.calls;
                ns += figures.ns;
                net += call_profile_net_ns(figures, report->pair_tenths_ns);
                below += figures.below;
            }
            children[root] += ns;
            fprintf(out, "child %s %s %llu %llu %llu %llu", ROOT_NAMES[root], report->names[root][child],
                    (unsigned long long)calls, (unsigned long long)ns, (unsigned long long)net,
                    (unsigned long long)below);
            write_share(out, ns, roots[root]);
        }
    }
    for (int root = 0; root < CALL_PROFILE_ROOT_COUNT; root++) {
        fprintf(out, "self %s %llu", ROOT_NAMES[root], (unsigned long long)(roots[root] - children[root]));
        write_share(out, roots[root] - children[root], roots[root]);
    }
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        uint64_t calls = 0, ns = 0, units = 0, switches = 0;
        for (int frame = 0; frame < frames; frame++) {
            call_profile_family_figures_t figures = call_profile_family_figures(frame, (draw_filter_family_t)family);
            calls += figures.calls;
            ns += figures.ns;
            units += figures.units;
            switches += figures.switches;
        }
        fprintf(out, "family %s %llu %llu", capture_list_family_name((draw_filter_family_t)family),
                (unsigned long long)calls, (unsigned long long)ns);
        write_share_field(out, ns, roots[CALL_PROFILE_DRAW]);
        fprintf(out, " units %llu switches %llu\n", (unsigned long long)units, (unsigned long long)switches);
    }
    fprintf(out, "in-call %llu\n", (unsigned long long)in_call);
}

typedef void (*writer_t)(FILE *out, const report_t *report);

/* Returns 0 after naming the failure. */
static int write_output(const char *dir, const char *file_name, writer_t writer, const report_t *report) {
    char path[PATH_CAP];
    snprintf(path, sizeof path, "%s/%s", dir, file_name);
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "frame_profile: cannot open '%s'\n", path);
        return 0;
    }
    writer(out, report);
    if (fclose(out) != 0) {
        fprintf(stderr, "frame_profile: cannot write '%s'\n", path);
        return 0;
    }
    return 1;
}

/* Removes the outputs of an earlier run, so that a run stopped by p8_fatal leaves none. Returns 0
 * after naming the failure. */
static int prepare_out_dir(const char *dir) {
    static const char *const OUTPUTS[] = {"frames.txt", "summary.txt"};
    for (size_t o = 0; o < sizeof OUTPUTS / sizeof OUTPUTS[0]; o++) {
        char path[PATH_CAP];
        snprintf(path, sizeof path, "%s/%s", dir, OUTPUTS[o]);
        FILE *probe = fopen(path, "w");
        if (probe == NULL) {
            fprintf(stderr, "frame_profile: cannot write in the output directory '%s'\n", dir);
            return 0;
        }
        fclose(probe);
        remove(path);
    }
    return 1;
}

static int write_state(const char *path, int cheat_activations) {
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "frame_profile: cannot open the state file '%s'\n", path);
        return 0;
    }
    state_write(out);
    state_write_cheat_activations(out, cheat_activations);
    return fclose(out) == 0;
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 6) {
        fprintf(stderr, "usage: frame_profile <events.json> <out-dir> <symbols> [<draw-root> [<update-root>]]\n");
        return 1;
    }
    static report_t report;
    report.events = argv[1];
    report.draw_root = argc >= 5 ? argv[4] : "cart_draw";
    report.update_root = argc == 6 ? argv[5] : "cart_update60";
    const char *out_dir = argv[2];
    uintptr_t draw_root = 0, update_root = 0;
    if (!symbols_read(argv[3]) || !roots_resolve(argv[3], report.draw_root, report.update_root, &draw_root, &update_root) ||
        !prepare_out_dir(out_dir)) {
        return 1;
    }

    call_profile_init(monotonic_ns, update_root, draw_root);
    report.pair_tenths_ns = call_profile_measure_pair(MEASURED_PAIRS);

    static replay_source_t source;
    if (replay_source_open(&source, report.events) != REPLAY_SOURCE_LOADED) {
        fprintf(stderr, "frame_profile: cannot load the recording '%s'\n", report.events);
        return 1;
    }
    if (!replay_source_seed_storage(&source)) {
        fprintf(stderr, "frame_profile: %s: cannot seed the save storage\n", report.events);
        return 1;
    }
    int resume_start = replay_source_start(&source) == REPLAY_SOURCE_START_RESUME;
    static bindings_t bindings;
    static replay_source_bindings_report_t bindings_report;
    replay_source_bindings_status_t bindings_status =
        replay_source_read_bindings(&source, &bindings_compiled, &bindings, &bindings_report);
    if (resume_start && bindings_status != REPLAY_SOURCE_BINDINGS_DEFAULT) {
        fprintf(stderr, "frame_profile: %s: a \"start\": \"resume\" file plays on the default bindings\n",
                report.events);
        return 1;
    }
    if (bindings_status != REPLAY_SOURCE_BINDINGS_DEFAULT && bindings_status != REPLAY_SOURCE_BINDINGS_FIXTURE) {
        fprintf(stderr, "frame_profile: %s\n", bindings_report.message);
        return 1;
    }

    static p8_vram_t memory;
    p8_vram_init(&memory);
    static p8_graphics_t graphics;
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    p8_input_t *input = &source.recording;
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(input);

    draw_filter_install();
    p8_draw_meter_install();
    cart_load();
    cart_init();

    input_step_t step;
    input_step_init(&step, &bindings);
    input_step_set_attract_after(&step, replay_source_attract_after(&source));
    if (resume_start && !record_start_resume(&step, source.initial_save, source.initial_save_count)) {
        fprintf(stderr, "frame_profile: %s: the initial_save does not decode\n", report.events);
        return 1;
    }

    const char *state_path = getenv("STATE");
    static int frame;
    if (state_path != NULL) state_write_on_exit(state_path, &frame);
    atexit(hooks_off);
    call_profile_family_install();
    int cheat_activations = 0;
    hooks_on = 1;
    for (frame = 0; frame < input->frame_count; frame++) {
        cheat_activations += input_step_run(&step, &input->frames[input->current_frame]);
        cart_update60();
        cart_draw();
        p8_draw_meter_frame_end();
        draw_filter_frame_end();
        call_profile_frame_end();
        if (frame < input->frame_count - 1) p8_input_step(input);
    }
    hooks_on = 0;

    if (state_path != NULL) {
        state_release();
        if (!write_state(state_path, cheat_activations)) return 1;
    }
    name_children(&report);
    if (!write_output(out_dir, "frames.txt", write_frames, &report)) return 1;
    if (!write_output(out_dir, "summary.txt", write_summary, &report)) return 1;
    replay_source_close(&source);
    printf("frame_profile: %d frame(s) of %s profiled, hook-pair-ns %llu.%llu, outputs in %s\n",
           call_profile_frame_count(), report.events, (unsigned long long)(report.pair_tenths_ns / 10),
           (unsigned long long)(report.pair_tenths_ns % 10), out_dir);
    return 0;
}
