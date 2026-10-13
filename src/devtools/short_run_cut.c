#include "devtools/short_run_cut.h"

#include <stdio.h>
#include <string.h>

static int refuse(char error[SHORT_RUN_CUT_ERROR_CAP], const char *line, const char *reason) {
    snprintf(error, SHORT_RUN_CUT_ERROR_CAP, "'%.*s': %s", SHORT_RUN_CUT_ERROR_CAP / 2, line, reason);
    return 0;
}

int short_run_cut_parse_entry(const char *line, short_run_cut_entry_t *entry, char error[SHORT_RUN_CUT_ERROR_CAP]) {
    char kind[16];
    char extra[2];
    int consumed = 0;
    memset(entry, 0, sizeof *entry);
    if (sscanf(line, "%31s %15s %d %d%n", entry->name, kind, &entry->window_first, &entry->window_last, &consumed) < 4) {
        return refuse(error, line, "not <name> <kind> <first> <last>");
    }
    const char *rest = line + consumed;
    if (strcmp(kind, "prefix") == 0) {
        int end_consumed = 0;
        entry->kind = SHORT_RUN_CUT_PREFIX;
        if (sscanf(rest, "%d%n", &entry->end, &end_consumed) < 1) return refuse(error, line, "a prefix run needs its end");
        rest += end_consumed;
    } else if (strcmp(kind, "resume") == 0) {
        entry->kind = SHORT_RUN_CUT_RESUME;
    } else {
        char reason[48];
        snprintf(reason, sizeof reason, "unknown kind '%s'", kind);
        return refuse(error, line, reason);
    }
    if (sscanf(rest, "%1s", extra) == 1) return refuse(error, line, "extra text");
    if (entry->window_first < 0) return refuse(error, line, "negative frame");
    if (entry->window_last < entry->window_first) return refuse(error, line, "last frame before the first");
    if (entry->kind == SHORT_RUN_CUT_PREFIX && entry->end < entry->window_last) {
        return refuse(error, line, "end before the window's last frame");
    }
    return 1;
}

/* The first 13 frames of tests/e2e/pad_resume_at_start.json: the title navigation that resumes a stored record. */
static const int TITLE_PADS[SHORT_RUN_CUT_TITLE_FRAMES] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P8_PAD_BUTTON_1, 0, P8_PAD_BUTTON_5
};

static int fits(const short_run_cut_entry_t *entry, int last, int frame_count, int full_count,
                char error[SHORT_RUN_CUT_ERROR_CAP]) {
    if (last >= full_count) {
        snprintf(error, SHORT_RUN_CUT_ERROR_CAP, "%s: ends at f%d, past the recording's %d frames", entry->name, last,
                 full_count);
        return 0;
    }
    if (frame_count > SHORT_RUN_CUT_FRAMES_CAP) {
        snprintf(error, SHORT_RUN_CUT_ERROR_CAP, "%s: %d frames, more than the breakdown table's %d", entry->name,
                 frame_count, SHORT_RUN_CUT_FRAMES_CAP);
        return 0;
    }
    return 1;
}

static int plan_prefix(const p8_input_frame_t *full, int full_count, const short_run_cut_entry_t *entry,
                       short_run_cut_plan_t *plan, char error[SHORT_RUN_CUT_ERROR_CAP]) {
    if (!fits(entry, entry->end, entry->end + 1, full_count, error)) return 0;
    plan->frame_count = entry->end + 1;
    memcpy(plan->frames, full, sizeof full[0] * (size_t)plan->frame_count);
    plan->save_index = -1;
    plan->cut = -1;
    plan->first_game_frame = 0;
    plan->source_first = 0;
    plan->reference_last = entry->end;
    return 1;
}

static int plan_resume(const p8_input_frame_t *full, int full_count, const short_run_cut_save_t *saves, int save_count,
                       const short_run_cut_entry_t *entry, short_run_cut_plan_t *plan, char error[SHORT_RUN_CUT_ERROR_CAP]) {
    int chosen = -1;
    for (int i = 0; i < save_count; i++) {
        int next = saves[i].frame + 1;
        if (saves[i].frame <= entry->window_first - SHORT_RUN_CUT_LEAD && next < full_count && full[next].pad_buttons == 0) {
            chosen = i;
        }
    }
    if (chosen < 0) {
        snprintf(error, SHORT_RUN_CUT_ERROR_CAP, "%s: no save at or before f%d with an idle next frame", entry->name,
                 entry->window_first - SHORT_RUN_CUT_LEAD);
        return 0;
    }
    int cut = saves[chosen].frame;
    int source_count = entry->window_last - cut;
    if (!fits(entry, entry->window_last, SHORT_RUN_CUT_TITLE_FRAMES + source_count, full_count, error)) return 0;
    memset(plan->frames, 0, sizeof plan->frames[0] * SHORT_RUN_CUT_TITLE_FRAMES);
    for (int i = 0; i < SHORT_RUN_CUT_TITLE_FRAMES; i++) plan->frames[i].pad_buttons = TITLE_PADS[i];
    memcpy(&plan->frames[SHORT_RUN_CUT_TITLE_FRAMES], &full[cut + 1], sizeof full[0] * (size_t)source_count);
    plan->frame_count = SHORT_RUN_CUT_TITLE_FRAMES + source_count;
    plan->save_index = chosen;
    plan->cut = cut;
    plan->first_game_frame = SHORT_RUN_CUT_TITLE_FRAMES;
    plan->source_first = cut + 1;
    plan->reference_last = entry->window_last;
    return 1;
}

int short_run_cut_plan(const p8_input_frame_t *full, int full_count, const short_run_cut_save_t *saves,
                       int save_count, const short_run_cut_entry_t *entry, short_run_cut_plan_t *plan,
                       char error[SHORT_RUN_CUT_ERROR_CAP]) {
    if (entry->kind == SHORT_RUN_CUT_PREFIX) return plan_prefix(full, full_count, entry, plan, error);
    return plan_resume(full, full_count, saves, save_count, entry, plan, error);
}

void short_run_cut_format_map(const short_run_cut_entry_t *entry, const short_run_cut_plan_t *plan,
                              char line[SHORT_RUN_CUT_MAP_CAP]) {
    char cut[16] = "none";
    if (plan->cut >= 0) snprintf(cut, sizeof cut, "%d", plan->cut);
    snprintf(line, SHORT_RUN_CUT_MAP_CAP, "cut %s first-game-frame %d source-first %d window %d %d\n", cut,
             plan->first_game_frame, plan->source_first, entry->window_first, entry->window_last);
}
