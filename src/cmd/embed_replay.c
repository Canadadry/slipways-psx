#include "devtools/replay_source.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void write_string_literal(FILE *out, const char *text) {
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        if (isalnum(*p) || strchr("._/-", *p)) {
            fputc(*p, out);
        } else {
            fprintf(out, "\\%03o", *p);
        }
    }
    fputc('"', out);
}

/* Returns 0 when a write failed; the caller removes the file. */
static int write_source(FILE *out, replay_source_t *source, const char *name) {
    int frame_count = replay_source_frame_count(source);

    fprintf(out, "#include \"devtools/embedded_replay.h\"\n\n");
    fprintf(out, "const char embedded_replay_name[] = ");
    write_string_literal(out, name);
    fprintf(out, ";\n\nconst int embedded_replay_frame_count = %d;\n\n", frame_count);

    fprintf(out, "const p8_input_frame_t embedded_replay_frames[%d] = {\n", frame_count);
    p8_input_frame_t frame;
    while (replay_source_next_frame(source, &frame)) {
        fprintf(out, "    { %d, %d, %d, %d },\n", frame.mouse_x, frame.mouse_y, frame.mouse_buttons, frame.pad_buttons);
    }
    fprintf(out, "};\n\n");

    /* A zero-length array is not C: an empty save keeps one unused byte. */
    fprintf(out, "const int embedded_replay_initial_save_count = %d;\n\n", source->initial_save_count);
    fprintf(out, "const uint8_t embedded_replay_initial_save[] = {");
    for (int i = 0; i < source->initial_save_count; i++) {
        fprintf(out, "%s0x%02x,", i % 12 == 0 ? "\n    " : " ", source->initial_save[i]);
    }
    fprintf(out, "%s\n};\n", source->initial_save_count == 0 ? "\n    0" : "");

    fprintf(out, "\nconst int embedded_replay_attract_after = %d;\n", replay_source_attract_after(source));

    return !ferror(out);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: embed_replay <events.json> <name> <output.c>\n");
        return 2;
    }
    const char *recording = argv[1];
    const char *name = argv[2];
    const char *output = argv[3];

    replay_source_t source;
    if (replay_source_open(&source, recording) != REPLAY_SOURCE_LOADED) {
        fprintf(stderr, "embed_replay: %s: unreadable, malformed or not a pad recording\n", recording);
        replay_source_close(&source);
        return 1;
    }
    if (replay_source_frame_count(&source) == 0) {
        fprintf(stderr, "embed_replay: %s: no frames\n", recording);
        replay_source_close(&source);
        return 1;
    }
    if (replay_source_start(&source) == REPLAY_SOURCE_START_RESUME) {
        fprintf(stderr, "embed_replay: %s: \"start\": \"resume\", the PSX replays start on the title\n", recording);
        replay_source_close(&source);
        return 1;
    }
    static bindings_t unused_fallback;
    static bindings_t read_bindings;
    static replay_source_bindings_report_t bindings_report;
    if (replay_source_read_bindings(&source, &unused_fallback, &read_bindings, &bindings_report)
        != REPLAY_SOURCE_BINDINGS_DEFAULT) {
        fprintf(stderr, "embed_replay: %s: a \"bindings\" member, the PSX replays play the default bindings\n",
                recording);
        replay_source_close(&source);
        return 1;
    }

    FILE *out = fopen(output, "w");
    if (!out) {
        fprintf(stderr, "embed_replay: %s: cannot write %s\n", recording, output);
        replay_source_close(&source);
        return 1;
    }
    int written = write_source(out, &source, name);
    replay_source_close(&source);
    if (fclose(out) != 0 || !written) {
        fprintf(stderr, "embed_replay: %s: write to %s failed\n", recording, output);
        remove(output);
        return 1;
    }
    return 0;
}
