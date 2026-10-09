#include "devtools/pad_author.h"

#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/game_data.h"
#include "game/main_economy.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_CHARS 256
#define TOKENS_MAX 8
#define PATH_MAX_CHARS 256
#define INCLUDE_DEPTH_MAX 4

typedef struct {
    char text[LINE_MAX_CHARS];
    char *tokens[TOKENS_MAX];
    int token_count;
    int number;
} script_line_t;

/* One script file, owned by its run_file call; lines freed at its end. */
typedef struct {
    const char *path;
    script_line_t *lines;
    int line_count;
    const script_line_t *current;
} script_t;

static const char *const BUTTON_NAMES[] = {
    "up", "down", "left", "right", "cross", "circle", "square", "triangle", "l1", "r1", "start", "select",
};
#define BUTTON_NAME_COUNT ((int)(sizeof BUTTON_NAMES / sizeof BUTTON_NAMES[0]))

/* A stop-at label outlives the script line it came from. */
static char stop_label_copy[LINE_MAX_CHARS];
static p8_input_frame_t boot_frame;
static p8_input_t boot_input;

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

void pad_author_init(pad_author_t *author) {
    memset(author, 0, sizeof *author);
    input_step_init(&author->step);
    memset(&boot_frame, 0, sizeof boot_frame);
    boot_input.frames = &boot_frame;
    boot_input.frame_count = 1;
    boot_input.current_frame = 0;
    dispatch_context_set_input(&boot_input);
}

/* Keep in step with src/cmd/replay.c's loop, or the self-check fails. */
static int emit(pad_author_t *author, const script_t *script, int buttons) {
    if (author->frame_count == PAD_AUTHOR_FRAME_BUDGET) {
        return fail(author, script, "over the frame budget of %d", PAD_AUTHOR_FRAME_BUDGET);
    }
    p8_input_frame_t *frame = &author->frames[author->frame_count];
    memset(frame, 0, sizeof *frame);
    frame->pad_buttons = buttons;
    author->cheat_activations += input_step_run(&author->step, frame);
    cart_update60();
    cart_draw();
    author->frame_count++;
    return 1;
}

static int emit_repeated(pad_author_t *author, const script_t *script, int buttons, int count) {
    for (int i = 0; i < count; i++) {
        if (!emit(author, script, buttons)) return 0;
    }
    return 1;
}

/* Returns the button mask of "<name>[+<name>...]", 0 when a name is unknown. */
static int parse_buttons(const char *text) {
    int mask = 0;
    const char *p = text;
    while (*p != '\0') {
        size_t length = strcspn(p, "+");
        int bit = -1;
        for (int b = 0; b < BUTTON_NAME_COUNT; b++) {
            if (strlen(BUTTON_NAMES[b]) == length && strncmp(BUTTON_NAMES[b], p, length) == 0) bit = b;
        }
        if (bit < 0) return 0;
        mask |= 1 << bit;
        p += length;
        if (*p == '+') p++;
    }
    return mask;
}

/* Returns -1 unless text is a whole decimal number >= 0. */
static int parse_count(const char *text) {
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (text[0] == '\0' || *end != '\0' || value < 0 || value > PAD_AUTHOR_FRAME_BUDGET) return -1;
    return (int)value;
}

static int difficulty_index(const char *name) {
    for (int d = 0; d < DIFFICULTY_OPTIONS_COUNT; d++) {
        if (strcmp(difficulty_options_table[d].name, name) == 0) return d;
    }
    return -1;
}

/* The title's focus starts on the first difficulty: one Down tap per row,
 * every other frame, the last two frames before the cross. */
static int run_title(pad_author_t *author, const script_t *script, int cross_frame, int difficulty) {
    int first_down = cross_frame - 2 * difficulty;
    if (first_down < author->frame_count || cross_frame < author->frame_count) {
        return fail(author, script, "title: the cross at f%d needs its first Down at f%d", cross_frame, first_down);
    }
    if (main_economy_typed() != NULL) return fail(author, script, "title: a game is already running");
    if (!emit_repeated(author, script, 0, first_down - author->frame_count)) return 0;
    for (int d = 0; d < difficulty; d++) {
        if (!emit(author, script, P8_PAD_DOWN) || !emit(author, script, 0)) return 0;
    }
    if (!emit(author, script, P8_PAD_CROSS)) return 0;
    const button_t *chosen = economy_score_difficulty();
    if (main_economy_typed() == NULL || chosen == NULL || chosen->name == NULL ||
        strcmp(chosen->name, difficulty_options_table[difficulty].name) != 0) {
        return fail(author, script, "title: no %s game started on the cross", difficulty_options_table[difficulty].name);
    }
    return 1;
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
    return fail(author, script, "unknown command '%s'", command);
}

/* Returns 0 when the file cannot be read or holds an overlong line; the
 * caller frees script->lines. */
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
        script_line_t *line = &script->lines[i];
        line->token_count = 0;
        for (char *token = strtok(line->text, " \t"); token != NULL; token = strtok(NULL, " \t")) {
            if (line->token_count == TOKENS_MAX) {
                snprintf(author->error, sizeof author->error, "%s:%d: more than %d words", script->path,
                         line->number, TOKENS_MAX);
                return 0;
            }
            line->tokens[line->token_count++] = token;
        }
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
