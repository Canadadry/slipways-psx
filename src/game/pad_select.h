#ifndef GAME_PAD_SELECT_H
#define GAME_PAD_SELECT_H

#include "engine/entity_class.h"
#include "engine/entity_list.h"

#define PAD_SELECT_HOLD_FRAMES 30
#define PAD_SELECT_TAP_FRAMES 10
#define PAD_SELECT_CHAIN_FRAMES 15

/* Select's state: the current press, and the window after a year in which a fresh press ends one more. */
typedef struct {
    int held_frames;
    int fired;
    int chain_frames;
} pad_select_t;

typedef enum {
    PAD_SELECT_NONE,
    PAD_SELECT_TAP,
    PAD_SELECT_YEAR
} pad_select_event_t;

void pad_select_init(pad_select_t *s);

/* One frame. A frame with allowed 0 returns NONE and makes the rest of a press under way inert until it is released;
 * the chain window still counts down. */
pad_select_event_t pad_select_step(pad_select_t *s, int down, int allowed);

/* Opens the chain window: called only when the year press went out, never on a refused one. */
void pad_select_year_ended(pad_select_t *s);

/* 1 from pad_select_year_ended until the next step: that step is the year press's release frame. */
int pad_select_year_just_ended(const pad_select_t *s);

/* Reads only; 0 for NULL. 1 for a settled pad_target_is_candidate with a need its panel lists under "needs:". */
int pad_select_needs_attention(const entity_t *e);

/* The first entry after current, in list order and wrapping round, that needs attention; the search starts from the
 * first entry when current is NULL or not listed. May return current itself; NULL when none needs attention. */
entity_t *pad_select_next(const entity_list_t *candidates, const entity_t *current);

#endif
