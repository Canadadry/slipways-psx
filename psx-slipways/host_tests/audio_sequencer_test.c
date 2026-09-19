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

int main(void) {
    t_run("resolve_instrument_full_0_to_15_domain", test_resolve_instrument_full_0_to_15_domain);
    t_run("channel_init_starts_music_driven_at_row_zero", test_channel_init_starts_music_driven_at_row_zero);
    t_run("sfx_trigger_preempts_and_leaves_music_bookkeeping_untouched", test_sfx_trigger_preempts_and_leaves_music_bookkeeping_untouched);
    t_run("sfx_finished_resumes_music_from_exact_pre_preemption_state", test_sfx_finished_resumes_music_from_exact_pre_preemption_state);
    t_run("sfx_finished_while_not_preempted_is_a_defensive_no_op", test_sfx_finished_while_not_preempted_is_a_defensive_no_op);
    t_run("re_trigger_while_already_preempted_clears_pending_stop", test_re_trigger_while_already_preempted_clears_pending_stop);
    return t_report();
}
