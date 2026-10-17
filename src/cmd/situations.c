#include "devtools/record_start.h"
#include "devtools/replay_source.h"
#include "devtools/situation.h"
#include "engine/dispatch_context.h"
#include "game/bindings.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: situations <events.json> <table>\n");
        return 1;
    }
    const char *events = argv[1];
    static replay_source_t source;
    if (replay_source_open(&source, events) != REPLAY_SOURCE_LOADED) {
        fprintf(stderr, "situations: cannot load the recording '%s'\n", events);
        return 1;
    }
    if (!replay_source_seed_storage(&source)) {
        fprintf(stderr, "situations: %s: cannot seed the save storage\n", events);
        return 1;
    }
    int resume_start = replay_source_start(&source) == REPLAY_SOURCE_START_RESUME;
    static bindings_t bindings;
    static replay_source_bindings_report_t bindings_report;
    replay_source_bindings_status_t bindings_status =
        replay_source_read_bindings(&source, &bindings_compiled, &bindings, &bindings_report);
    if (resume_start && bindings_status != REPLAY_SOURCE_BINDINGS_DEFAULT) {
        fprintf(stderr, "situations: %s: a \"start\": \"resume\" file plays on the default bindings\n", events);
        return 1;
    }
    if (bindings_status != REPLAY_SOURCE_BINDINGS_DEFAULT && bindings_status != REPLAY_SOURCE_BINDINGS_FIXTURE) {
        fprintf(stderr, "situations: %s\n", bindings_report.message);
        return 1;
    }
    FILE *out = fopen(argv[2], "w");
    if (out == NULL) {
        fprintf(stderr, "situations: cannot open '%s'\n", argv[2]);
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
    cart_load();
    cart_init();

    input_step_t step;
    input_step_init(&step, &bindings);
    input_step_set_attract_after(&step, replay_source_attract_after(&source));
    if (resume_start && !record_start_resume(&step, source.initial_save, source.initial_save_count)) {
        fprintf(stderr, "situations: %s: the initial_save does not decode\n", events);
        fclose(out);
        return 1;
    }

    situation_snapshot_t before = situation_snapshot_read(&step);
    situation_t previous = situation_start(&before);
    fprintf(out, "frame situation family\n");
    for (int frame = 0; frame < input->frame_count; frame++) {
        input_step_run(&step, &input->frames[input->current_frame]);
        cart_update60();
        situation_snapshot_t after = situation_snapshot_read(&step);
        previous = situation_classify(&before, &after, previous);
        fprintf(out, "%d %s %s\n", frame, situation_name(previous), situation_family_name(situation_family(previous)));
        before = after;
        cart_draw();
        if (frame < input->frame_count - 1) p8_input_step(input);
    }
    replay_source_close(&source);
    if (fclose(out) != 0) {
        fprintf(stderr, "situations: cannot write '%s'\n", argv[2]);
        return 1;
    }
    return 0;
}
