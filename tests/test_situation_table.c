/* psx/perf/full-run-situations.txt, the committed situation table of worst-case/full_game.json:
 * its shape, and hand-picked frames checked against worst-case/full_game.pad. Reads the file only. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "devtools/situation.h"

#define TABLE "psx/perf/full-run-situations.txt"

enum { FULL_GAME_FRAMES = 6061, NAME_CAP = 32 };

typedef struct {
    int frame;
    const char *situation;
} picked_frame_t;

static const picked_frame_t picked_frames[] = {
    {0, "title"},
    {12, "title"},                     /* the loading screen: press frame of title 12 reasonable */
    {13, "title"},                     /* the loading screen */
    {14, "galaxy-generation"},         /* title 12 reasonable: button 5 on f12, generation f14 */
    {32, "map-move"},                  /* probe 170 250: the cursor travels to the target */
    {59, "probe"},                     /* probe 170 250: the launch */
    {77, "map-rest"},                  /* fold 168 238: button 5 held on an empty spot */
    {90, "screen-change"},             /* score: the score panel opens */
    {93, "score-panel"},               /* score */
    {114, "menu"},                     /* colonise p33 strip: p33's menu */
    {115, "colonise"},                 /* colonise p33 strip */
    {160, "slipway-aim"},              /* slipway p33 p78: target not chosen yet */
    {161, "slipway-built"},            /* slipway p33 p78 */
    {231, "new-year"},                 /* year */
    {793, "build"},                    /* bar */
    {901, "tech-panel"},               /* research space folding: the tech panel */
    {903, "research"},                 /* research space folding */
    {2230, "structure-built"},         /* build processor 134 231 */
    {6045, "game-end"},
};

static char situations[FULL_GAME_FRAMES][NAME_CAP];
static char families[FULL_GAME_FRAMES][NAME_CAP];

static int read_table(void) {
    FILE *in = fopen(TABLE, "r");
    assert(in != NULL && "run make situations-check to regenerate " TABLE);
    char line[128];
    assert(fgets(line, sizeof line, in) != NULL);
    assert(strcmp(line, "frame situation family\n") == 0);
    int count = 0;
    while (fgets(line, sizeof line, in) != NULL) {
        assert(count < FULL_GAME_FRAMES);
        int frame;
        char tail[2];
        assert(sscanf(line, "%d %31s %31s %1s", &frame, situations[count], families[count], tail) == 3);
        assert(frame == count);
        count++;
    }
    fclose(in);
    return count;
}

static int situation_of(const char *name, situation_t *out) {
    for (int s = 0; s < SITUATION_COUNT; s++) {
        if (strcmp(situation_name((situation_t)s), name) == 0) {
            *out = (situation_t)s;
            return 1;
        }
    }
    return 0;
}

static void test_one_line_per_frame_of_the_full_game(void) {
    assert(read_table() == FULL_GAME_FRAMES);
}

static void test_every_family_is_its_situations(void) {
    for (int frame = 0; frame < FULL_GAME_FRAMES; frame++) {
        situation_t situation;
        assert(situation_of(situations[frame], &situation));
        assert(strcmp(families[frame], situation_family_name(situation_family(situation))) == 0);
    }
}

static void test_hand_picked_frames(void) {
    for (size_t i = 0; i < sizeof picked_frames / sizeof picked_frames[0]; i++) {
        const picked_frame_t *picked = &picked_frames[i];
        if (strcmp(situations[picked->frame], picked->situation) != 0) {
            fprintf(stderr, "test_situation_table: frame %d is %s, want %s\n", picked->frame,
                    situations[picked->frame], picked->situation);
            assert(0);
        }
    }
}

/* Every situation the table holds has a hand-picked frame. */
static void test_every_situation_present_is_picked(void) {
    for (int frame = 0; frame < FULL_GAME_FRAMES; frame++) {
        int picked = 0;
        for (size_t i = 0; i < sizeof picked_frames / sizeof picked_frames[0]; i++) {
            picked |= strcmp(picked_frames[i].situation, situations[frame]) == 0;
        }
        if (!picked) {
            fprintf(stderr, "test_situation_table: %s (frame %d) has no hand-picked frame\n", situations[frame], frame);
            assert(0);
        }
    }
}

int main(void) {
    test_one_line_per_frame_of_the_full_game();
    test_every_family_is_its_situations();
    test_hand_picked_frames();
    test_every_situation_present_is_picked();
    return 0;
}
