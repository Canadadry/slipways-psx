#include "t.h"
#include "../audio_sequencer.h"

#include <stdint.h>

/*
 * Covers audio_resolve_instrument's FULL 0-15 domain explicitly (this
 * chunk's own task description: "test all 16 values explicitly, not just a
 * spot check"), against an independently hand-typed expected array (identity
 * for 0-7, (n-8) for 8-15 -- docs/prd/triage/psx-spu-audio.md's own
 * Implementation Chunks item 3 wording, re-checked directly against the
 * PRD's exact text rather than assumed), not derived from the module's own
 * formula -- matching this codebase's established "derive expected values
 * independently, don't copy them from the implementation" convention
 * (audio_pitch_test.c's hand_computed_register).
 */
static void test_resolve_instrument_full_0_to_15_domain(void) {
    int expected[16] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
    for (int i = 0; i < 16; i++) {
        T_CHECK(audio_resolve_instrument(i) == expected[i]);
    }
}

/* audio_channel_init's own documented startup contract (audio_sequencer.h):
 * music-driven from note 0/tick 0, no sfx-stop pending. */
static void test_channel_init_starts_music_driven_at_row_zero(void) {
    audio_channel_state_t ch;
    audio_channel_init(&ch);
    T_CHECK(ch.is_music == 1);
    T_CHECK(ch.music_note_index == 0);
    T_CHECK(ch.music_tick == 0);
    T_CHECK(ch.sfx_pending_stop == 0);
}

/*
 * Constructs this test file's own core scenario (this chunk's own task
 * description: "construct a test scenario where a channel is playing a
 * music pattern's note sequence, a one-shot sfx trigger preempts it"),
 * derived independently against the real traced rn()/music() interaction
 * (docs/prd/triage/psx-spu-audio.md's own Channel model subsection,
 * re-read directly rather than copied verbatim from the PRD's own prose):
 * pico8/lua.lua's rn() always plays on channel 3, and any sfx() trigger on a
 * channel currently driven by a music() pattern preempts that channel's
 * note for the sfx's own duration. A channel mid-pattern (arbitrary,
 * non-zero music_note_index=5/music_tick=3, standing in for "somewhere in
 * the middle of a real g_music[] pattern's own note sequence", not 0/0 --
 * zero values would not actually prove the fields survive untouched, since
 * an accidental reset-to-zero bug would be indistinguishable from correct
 * behavior at those values) gets a real rn()-style trigger: this is part (a)
 * of the required assertion -- the sfx's own parameters ARE what's
 * "currently active" during preemption, which this struct expresses as
 * is_music==0 (audio_sequencer.h's own doc comment: 0 means a one-shot sfx,
 * not the music pattern, is what SHOULD be active on the real SPU voice).
 */
static void test_sfx_trigger_preempts_and_leaves_music_bookkeeping_untouched(void) {
    audio_channel_state_t ch;
    audio_channel_init(&ch);
    ch.music_note_index = 5;
    ch.music_tick = 3;

    audio_channel_sfx_trigger(&ch);

    T_CHECK(ch.is_music == 0);
    T_CHECK(ch.music_note_index == 5);
    T_CHECK(ch.music_tick == 3);
    T_CHECK(ch.sfx_pending_stop == 0);
}

/*
 * Continues the same scenario through the sfx's own key-off: this is part
 * (b) of the required assertion -- once the sfx ends, the music channel's
 * own note-index/tick state is UNCHANGED from before the preemption started
 * (still exactly 5/3, the same values test_sfx_trigger_preempts_and_leaves_
 * music_bookkeeping_untouched above set before preempting), i.e. the music
 * resumes exactly where it would have been had the preemption never
 * happened -- not reset, not restarted, not skipped ahead. is_music flipping
 * back to 1 and sfx_pending_stop flipping to 1 are the arbitration's own
 * observable "resume" signal (audio_sequencer.h's own doc comment on
 * audio_channel_sfx_finished) -- the real proof this test needs is that
 * music_note_index/music_tick are the SAME two numbers as before the whole
 * preempt-then-resume cycle, not merely "some" numbers.
 */
static void test_sfx_finished_resumes_music_from_exact_pre_preemption_state(void) {
    audio_channel_state_t ch;
    audio_channel_init(&ch);
    ch.music_note_index = 5;
    ch.music_tick = 3;

    audio_channel_sfx_trigger(&ch);
    T_CHECK(ch.is_music == 0);

    audio_channel_sfx_finished(&ch);

    T_CHECK(ch.is_music == 1);
    T_CHECK(ch.sfx_pending_stop == 1);
    T_CHECK(ch.music_note_index == 5);
    T_CHECK(ch.music_tick == 3);
}

/* audio_channel_sfx_finished's own documented defensive-no-op contract
 * (audio_sequencer.h): a stray key-off report on a channel that was never
 * actually preempted must not fabricate a pending stop, and must not touch
 * the music bookkeeping either. */
static void test_sfx_finished_while_not_preempted_is_a_defensive_no_op(void) {
    audio_channel_state_t ch;
    audio_channel_init(&ch);
    ch.music_note_index = 9;
    ch.music_tick = 2;

    audio_channel_sfx_finished(&ch);

    T_CHECK(ch.is_music == 1);
    T_CHECK(ch.sfx_pending_stop == 0);
    T_CHECK(ch.music_note_index == 9);
    T_CHECK(ch.music_tick == 2);
}

/* audio_channel_sfx_trigger's own documented re-trigger contract
 * (audio_sequencer.h): a fresh sfx trigger arriving while the channel is
 * already preempted (e.g. a second rn() call before the first preempting
 * sfx finished) cuts off whatever was sounding and clears any stale
 * sfx_pending_stop from a PRIOR resume this test's own caller has not yet
 * consumed -- music_note_index/music_tick stay untouched throughout, same
 * as every other transition here. */
static void test_re_trigger_while_already_preempted_clears_pending_stop(void) {
    audio_channel_state_t ch;
    audio_channel_init(&ch);
    ch.music_note_index = 7;
    ch.music_tick = 1;

    audio_channel_sfx_trigger(&ch);
    audio_channel_sfx_finished(&ch);
    T_CHECK(ch.sfx_pending_stop == 1);

    audio_channel_sfx_trigger(&ch);

    T_CHECK(ch.is_music == 0);
    T_CHECK(ch.sfx_pending_stop == 0);
    T_CHECK(ch.music_note_index == 7);
    T_CHECK(ch.music_tick == 1);
}

/*
 * Chunk 4 tests (Implementation Chunks item 4): the note-advance sequencer
 * (audio_seq_state_t/audio_seq_init/audio_seq_step/audio_seq_current_note).
 * Expected values below are derived independently against the stepping rule
 * QUOTED verbatim on audio_seq_state_t's own doc comment (audio_sequencer.h)
 * -- re-read directly from docs/prd/triage/psx-spu-audio.md's own
 * "Sequencing" subsection for this task, not copied from audio_sequencer.c's
 * own implementation -- matching this file's own established
 * derive-independently convention (see test_resolve_instrument_full_0_to_15_
 * domain's own comment above).
 */

/*
 * Implementation Chunks item 4's first required case: "a sequence with
 * loop_end==loop_start==0 plays rows 0..note_count-1 once and stops". A
 * small (note_count=4) synthetic fixture, speed=2, tick_period=1 (hold = 2
 * steps/row). Steps through every row explicitly, checking current_row
 * after EVERY single step call (not just at row boundaries) so an
 * off-by-one in either the hold-duration or the stop condition would show
 * up immediately, then confirms audio_seq_step keeps reporting inactive
 * (never resurrects) on further calls past the stop.
 */
static void test_seq_plays_all_rows_once_then_stops_when_not_looping(void) {
    sfx_def_t sfx;
    sfx.note_count = 4;
    sfx.speed = 2;
    sfx.loop_start = 0;
    sfx.loop_end = 0;

    audio_seq_state_t seq;
    audio_seq_init(&seq);
    T_CHECK(seq.current_row == 0);
    T_CHECK(seq.active == 1);

    /* Row 0 holds for 2 steps (speed=2 * tick_period=1): row unchanged
     * after step 1, advances to row 1 exactly on step 2. Same 2-step
     * cadence for every row up to the last (row 3 = note_count-1). */
    int expected_row_after_step[8] = {0, 1, 1, 2, 2, 3, 3, 3};
    int expected_active_after_step[8] = {1, 1, 1, 1, 1, 1, 1, 0};
    for (int i = 0; i < 8; i++) {
        int active = audio_seq_step(&seq, &sfx, 1);
        T_CHECK(seq.current_row == expected_row_after_step[i]);
        T_CHECK(active == expected_active_after_step[i]);
        T_CHECK(seq.active == expected_active_after_step[i]);
    }

    /* Stopped for good: row 3 (note_count-1) is where it stayed, and
     * further step calls are a no-op, not a crash/resurrection. */
    T_CHECK(seq.current_row == 3);
    for (int i = 0; i < 5; i++) {
        int active = audio_seq_step(&seq, &sfx, 1);
        T_CHECK(active == 0);
        T_CHECK(seq.current_row == 3);
    }
}

/*
 * Implementation Chunks item 4's second required case: "a sequence with
 * loop_end>loop_start repeats that sub-range indefinitely". A small
 * (note_count=5) synthetic fixture with a GENUINE sub-range (loop_start=2,
 * loop_end=4, neither 0 nor note_count-1), speed=1, tick_period=1 (hold = 1
 * step/row, so every single step call advances exactly one row -- the
 * simplest possible cadence for tracing many loop iterations without a
 * per-row inner loop).
 *
 * Per the stepping rule's own literal wording (audio_seq_state_t's doc
 * comment, audio_sequencer.h): row 4 (note_count-1) is ALWAYS visited on
 * every pass, looped or not -- loop_end's value (4, which happens to equal
 * note_count-1 here) is used only as the ">loop_start" gate, and the jump-
 * back destination is loop_start (2). This test therefore does not by
 * itself distinguish "loop back to loop_start, replay through note_count-1
 * again" from "loop_end bounds a shorter repeating window" (they coincide
 * when loop_end==note_count-1); test_seq_real_g_sfx0_loop_replays_full_
 * range_not_just_up_to_loop_end below uses a real g_sfx[] entry where
 * loop_end != note_count-1 specifically to rule out that other reading.
 * What THIS test proves, over many genuine repeats (not just one jump): the
 * loop truly keeps cycling row 2..4 forever, never re-visiting row 0 or 1
 * again after the very first pass.
 */
static void test_seq_loops_subrange_indefinitely(void) {
    sfx_def_t sfx;
    sfx.note_count = 5;
    sfx.speed = 1;
    sfx.loop_start = 2;
    sfx.loop_end = 4;

    audio_seq_state_t seq;
    audio_seq_init(&seq);

    /* First pass: 0 -> 1 -> 2 -> 3 -> 4 (every row visited once). */
    int first_pass[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        int active = audio_seq_step(&seq, &sfx, 1);
        T_CHECK(active == 1);
        T_CHECK(seq.current_row == first_pass[i]);
    }

    /* Three full loop iterations of the sub-range [loop_start=2,
     * note_count-1=4]: jump back to 2, then 2->3->4, repeated. Never 0 or 1
     * again. */
    int loop_pass[4] = {2, 3, 4};
    for (int iter = 0; iter < 3; iter++) {
        for (int i = 0; i < 3; i++) {
            int active = audio_seq_step(&seq, &sfx, 1);
            T_CHECK(active == 1);
            T_CHECK(seq.current_row == loop_pass[i]);
            T_CHECK(seq.current_row != 0);
            T_CHECK(seq.current_row != 1);
        }
    }
}

/*
 * Implementation Chunks item 4's third required case: "row-hold duration
 * matches speed * tick_period exactly for a parameterized tick period" --
 * uses g_sfx[0]'s own real speed value (16, cross-checked directly against
 * psx-slipways/audio_data.c's own g_sfx[0].speed field, not assumed) with a
 * concrete test tick_period=3, so hold = 16*3 = 48. note_count=2 is enough
 * to observe exactly one row transition (row 0 -> row 1); loop_start/
 * loop_end are irrelevant to this case and left at 0/0.
 *
 * Checks the EXACT boundary from both sides: after 47 steps (hold-1), still
 * on row 0 (proves no early/off-by-one advance); the 48th step (exactly
 * speed*tick_period) is the one that advances -- proving no late/off-by-one
 * advance either.
 */
static void test_seq_row_hold_matches_speed_times_tick_period_exactly(void) {
    sfx_def_t sfx;
    sfx.note_count = 2;
    sfx.speed = 16; /* g_sfx[0].speed, psx-slipways/audio_data.c */
    sfx.loop_start = 0;
    sfx.loop_end = 0;
    int tick_period = 3;
    int expected_hold = sfx.speed * tick_period;
    T_CHECK(expected_hold == 48);

    audio_seq_state_t seq;
    audio_seq_init(&seq);

    for (int i = 0; i < expected_hold - 1; i++) {
        int active = audio_seq_step(&seq, &sfx, tick_period);
        T_CHECK(active == 1);
        T_CHECK(seq.current_row == 0);
    }

    int active = audio_seq_step(&seq, &sfx, tick_period);
    T_CHECK(active == 1);
    T_CHECK(seq.current_row == 1);
}

/*
 * Implementation Chunks item 4's own explicit ask ("Host-tested against
 * g_sfx[] entries directly") plus this codebase's established "test against
 * real extracted data where practical" discipline: a fixture built from
 * g_sfx[0]'s OWN real fields, hand-transcribed directly from
 * psx-slipways/audio_data.c (note_count=32, speed=16, loop_start=0,
 * loop_end=30, and every one of its 32 real notes) rather than a synthetic
 * stand-in. Not #include-d/linked against the generated audio_data.c's own
 * g_sfx[] symbol -- this chunk's own Makefile-touch constraint (chunk 4's
 * own task description) leaves test-audio-sequencer's build sources as
 * audio_sequencer.c + this file only, so the real EXTERN g_sfx array itself
 * is never actually linked in; this fixture's field VALUES are the real
 * g_sfx[0] data content, cross-checked directly against audio_data.c, not
 * invented.
 *
 * loop_end (30) is deliberately NOT note_count-1 (31) here -- the one real
 * g_sfx[] entry this task's own instructions point at for "interesting
 * loop_start/loop_end/note_count values" -- specifically to distinguish the
 * two plausible readings of the stepping rule the header comment
 * (audio_seq_step, audio_sequencer.h) already flags: under the "loop_end
 * bounds a shorter repeating window" reading, row 31 would never play again
 * after the first pass; under the literal rule actually implemented here
 * (loop_end used only as the ">loop_start" gate, loop destination is always
 * loop_start), row 31 (note_count-1) is visited on EVERY pass, looped or
 * not. This test asserts the latter, using tick_period=1/speed=16 (hold=16
 * steps/row) over two full passes.
 */
static void test_seq_real_g_sfx0_loop_replays_full_range_not_just_up_to_loop_end(void) {
    sfx_def_t sfx;
    sfx.notes[0] = (sfx_note_t){31, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[1] = (sfx_note_t){31, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[2] = (sfx_note_t){24, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[3] = (sfx_note_t){24, 15, 0, SFX_EFFECT_NONE};
    sfx.notes[4] = (sfx_note_t){26, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[5] = (sfx_note_t){26, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[6] = (sfx_note_t){31, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[7] = (sfx_note_t){31, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[8] = (sfx_note_t){24, 15, 4, SFX_EFFECT_NONE};
    sfx.notes[9] = (sfx_note_t){24, 7, 2, SFX_EFFECT_VIBRATO};
    sfx.notes[10] = (sfx_note_t){26, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[11] = (sfx_note_t){26, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[12] = (sfx_note_t){31, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[13] = (sfx_note_t){31, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[14] = (sfx_note_t){24, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[15] = (sfx_note_t){29, 8, 0, SFX_EFFECT_DROP};
    sfx.notes[16] = (sfx_note_t){31, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[17] = (sfx_note_t){31, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[18] = (sfx_note_t){26, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[19] = (sfx_note_t){26, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[20] = (sfx_note_t){24, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[21] = (sfx_note_t){24, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[22] = (sfx_note_t){26, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[23] = (sfx_note_t){26, 15, 0, SFX_EFFECT_NONE};
    sfx.notes[24] = (sfx_note_t){34, 15, 4, SFX_EFFECT_NONE};
    sfx.notes[25] = (sfx_note_t){31, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[26] = (sfx_note_t){24, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[27] = (sfx_note_t){24, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[28] = (sfx_note_t){26, 15, 3, SFX_EFFECT_VIBRATO};
    sfx.notes[29] = (sfx_note_t){26, 7, 6, SFX_EFFECT_VIBRATO};
    sfx.notes[30] = (sfx_note_t){31, 7, 0, SFX_EFFECT_VIBRATO};
    sfx.notes[31] = (sfx_note_t){31, 7, 4, SFX_EFFECT_VIBRATO};
    sfx.note_count = 32;
    sfx.speed = 16;
    sfx.loop_start = 0;
    sfx.loop_end = 30;

    audio_seq_state_t seq;
    audio_seq_init(&seq);

    /* audio_seq_current_note at row 0, before any stepping: matches
     * g_sfx[0].notes[0] exactly. */
    const sfx_note_t *note0 = audio_seq_current_note(&seq, &sfx);
    T_CHECK(note0->pitch == 31);
    T_CHECK(note0->instrument == 15);
    T_CHECK(note0->volume == 3);
    T_CHECK(note0->effect == SFX_EFFECT_VIBRATO);

    /* Two full passes (64 rows total): row 31 (note_count-1) must be
     * reached, and reached again on the second (looped) pass -- proving
     * loop_end=30 is NOT used as a live upper bound cutting the pass short. */
    int rows_hit_31_count = 0;
    for (int pass = 0; pass < 2; pass++) {
        for (int row = 0; row < 32; row++) {
            for (int t = 0; t < 16; t++) {
                int active = audio_seq_step(&seq, &sfx, 1);
                T_CHECK(active == 1);
            }
            int expected_row = (row < 31) ? row + 1 : 0; /* loop_start=0 */
            T_CHECK(seq.current_row == expected_row);
            if (row == 31 && expected_row == 0) {
                /* Just completed row 31 and looped back to loop_start. */
                rows_hit_31_count++;
            }
        }
    }
    T_CHECK(rows_hit_31_count == 2);

    /* Row 15's real note (29, 8, 0, DROP) is reachable via the same
     * accessor after stepping there -- spot-checks audio_seq_current_note
     * against a real mid-sequence row, not just row 0. */
    seq.current_row = 15;
    const sfx_note_t *note15 = audio_seq_current_note(&seq, &sfx);
    T_CHECK(note15->pitch == 29);
    T_CHECK(note15->instrument == 8);
    T_CHECK(note15->volume == 0);
    T_CHECK(note15->effect == SFX_EFFECT_DROP);
}

int main(void) {
    t_run("resolve_instrument_full_0_to_15_domain", test_resolve_instrument_full_0_to_15_domain);
    t_run("channel_init_starts_music_driven_at_row_zero", test_channel_init_starts_music_driven_at_row_zero);
    t_run("sfx_trigger_preempts_and_leaves_music_bookkeeping_untouched", test_sfx_trigger_preempts_and_leaves_music_bookkeeping_untouched);
    t_run("sfx_finished_resumes_music_from_exact_pre_preemption_state", test_sfx_finished_resumes_music_from_exact_pre_preemption_state);
    t_run("sfx_finished_while_not_preempted_is_a_defensive_no_op", test_sfx_finished_while_not_preempted_is_a_defensive_no_op);
    t_run("re_trigger_while_already_preempted_clears_pending_stop", test_re_trigger_while_already_preempted_clears_pending_stop);
    t_run("seq_plays_all_rows_once_then_stops_when_not_looping", test_seq_plays_all_rows_once_then_stops_when_not_looping);
    t_run("seq_loops_subrange_indefinitely", test_seq_loops_subrange_indefinitely);
    t_run("seq_row_hold_matches_speed_times_tick_period_exactly", test_seq_row_hold_matches_speed_times_tick_period_exactly);
    t_run("seq_real_g_sfx0_loop_replays_full_range_not_just_up_to_loop_end", test_seq_real_g_sfx0_loop_replays_full_range_not_just_up_to_loop_end);
    return t_report();
}
