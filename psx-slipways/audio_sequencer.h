#ifndef AUDIO_SEQUENCER_H
#define AUDIO_SEQUENCER_H

/*
 * audio_sequencer.h/.c -- pure, no-PSX-headers module (mirrors rules_tech.h/
 * .c's own pure-core/stateful-wrapper split more than effects.h/.c's shape,
 * since this chunk's own second half is a genuine small state MACHINE with
 * real transitions to name and reason about, not a batch of independently-
 * traced formula/struct pairs -- effects.h's style is followed for the doc-
 * comment depth/citation discipline, rules_tech.h's for the actual
 * struct+pure-step-function shape).
 *
 * This chunk (docs/prd/triage/psx-spu-audio.md, Implementation Chunks item
 * 3) is ONLY:
 *   1. audio_resolve_instrument -- the 0-15 -> 0-7 instrument-fallback
 *      resolver.
 *   2. audio_channel_state_t + its pure transition functions -- the
 *      sfx-preempts-music channel-arbitration bookkeeping.
 *
 * The note-advance sequencer (speed/loop_start/loop_end stepping over a
 * sfx_def_t's own note rows) is explicitly chunk 4, a separate future chunk
 * that will also live in this same audio_sequencer.h/.c pair (Implementation
 * Chunks item 4) -- NOT implemented here. Neither function/struct below
 * issues any SPU register write, or any I/O of any kind: this whole module
 * stays pure and host-testable with plain gcc, no PSn00bSDK toolchain, per
 * this PRD's Testing Decisions ("Fully host-testable... audio_sequencer.c's
 * instrument-fallback/arbitration/note-advance logic").
 */

/*
 * audio_resolve_instrument -- ports the instrument-fallback resolver traced
 * in docs/prd/triage/psx-spu-audio.md, Implementation Decisions / "The
 * instrument-palette mismatch, confirmed from real data, not assumed", and
 * restated verbatim in Implementation Chunks item 3: "audio_resolve_
 * instrument(0..15) -> 0..7 (identity for 0-7, (n-8) for 8-15)".
 *
 * PICO-8's sfx_note_t.instrument field (psx-slipways/audio_data.h) ranges
 * 0-15:
 *   - 0-7 are PICO-8's 8 built-in basic-waveform instruments (Triangle,
 *     Tilted Saw, Saw, Square, Pulse, Organ, Noise, Phaser -- chunk 2's own
 *     AUDIO_INSTRUMENT_* constants, psx-slipways/audio_waveforms.h) --
 *     baked as real ADPCM waveforms by chunk 2's gen_psx_waveforms.py, so
 *     these map to themselves (identity): the real waveform is already
 *     available.
 *   - 8-15 are PICO-8's per-cart "custom instruments" -- 777 of this cart's
 *     2048 real sfx notes (37.9%) use one of these 8 slots (the PRD's own
 *     direct grep-count against pico8/sfx.json). Their real per-cart
 *     waveform definitions are NOT present in any file this project has
 *     extracted (pico8/ contains exactly gff.json, gfx.json, label.json,
 *     lua.lua, map.json/map.png, music.json, original.p8.png, sfx.json,
 *     sprite.png -- confirmed by direct listing, none of which is a
 *     custom-instrument definition table) -- a real, load-bearing,
 *     documented gap, not an oversight (see the PRD's own Further Notes,
 *     "The 37.9%-of-notes custom-instrument gap is this PRD's own most
 *     consequential open item"). This resolver's fallback for 8-15 is the
 *     PRD's own explicit, deliberate placeholder substitution: n-8, i.e.
 *     instrument 8 falls back to basic waveform 0 (Triangle), 9 to 1
 *     (Tilted Saw), ..., 15 to 7 (Phaser) -- a simple, non-silent,
 *     clearly-flagged substitution, never an attempt to invent or guess the
 *     real custom-instrument sound.
 *
 * pico8_instrument is expected in [0,15] (sfx_note_t.instrument's own valid
 * range, already range-checked by gen_psx_audio_data.py per the PRD's
 * Problem Statement) -- this function does not clamp or otherwise defend
 * against an out-of-range input, matching audio_pitch_register's own
 * identical "every real caller's input already comes from an
 * already-range-checked field" posture (audio_pitch.h).
 *
 * Returns a value in [0,7] -- a valid index into chunk 2's
 * g_instrument_waveform_adpcm[AUDIO_INSTRUMENT_COUNT] array
 * (psx-slipways/audio_waveforms.h), for whichever future chunk actually
 * uses this resolved index to select a baked waveform for upload/playback.
 */
int audio_resolve_instrument(int pico8_instrument);

/*
 * audio_channel_state_t -- the sfx-preempts-music channel-arbitration
 * bookkeeping traced in docs/prd/triage/psx-spu-audio.md, Implementation
 * Decisions / "Channel model: PICO-8's own hardcoded channel 3, and SPU
 * voice reservation":
 *
 *   pico8/lua.lua:330-332
 *   function rn(no)
 *   if (no and not ih)sfx(no,3)
 *   end
 *
 * Every real SFX trigger in this cart goes through rn() and always plays on
 * PICO-8's hardcoded channel 3 (ih is a mute flag, out of scope here --
 * Out of Scope in this PRD, unwired options menu). music(...) is called
 * directly (not through rn()) and drives all 4 channels (0-3) simultaneously
 * via g_music[pattern].channels[0..3] (psx-slipways/audio_data.h). This port
 * reserves one real SPU voice per PICO-8 logical channel (0-3), so each
 * channel's own arbitration state is independent of every other channel's --
 * a caller owns one audio_channel_state_t per channel (4 total), not one
 * shared struct.
 *
 * Per real, documented PICO-8 engine behavior (public engine knowledge, not
 * decompiled -- same citation class as the pitch table, audio_pitch.h): a
 * one-shot sfx() trigger on a channel currently playing as part of a music()
 * pattern PREEMPTS that channel's music note for the sfx's own duration.
 * Once the sfx's own key-off happens, the pattern's own sequencing simply
 * CONTINUES from wherever its own per-tick step logic already has it -- an
 * EMERGENT resume, not a separate "restore" mechanism: the music channel's
 * own note-index/tick counters are never touched by the preemption in either
 * direction (neither frozen-and-restored, nor reset/skipped) -- they are
 * simply never read or written by anything except chunk 4's future
 * note-advance step, which this arbitration layer never calls while a
 * channel is preempted (Implementation Chunks item 3 vs. item 4 -- item 4,
 * the actual note-advance function, is NOT implemented by this chunk; the
 * caller/future audio_update() is the one that would skip calling it for a
 * preempted channel, driven by this struct's own is_music field).
 *
 * Field shape (this PRD's own suggested shape, "exact field shape decided at
 * implementation time" -- this chunk's own choice, documented here):
 *
 *   is_music          -- 1: this channel's currently active source is the
 *                        music pattern's own current note (chunk 4's future
 *                        note-advance step is the one driving
 *                        music_note_index/music_tick forward, tick by tick).
 *                        0: a one-shot sfx has preempted this channel; the
 *                        sfx's OWN note/voice parameters (not modeled by
 *                        this struct at all -- a preempting sfx is
 *                        sequenced by that same future chunk-4 note-advance
 *                        step, just against the sfx's own separate sfx_def_t
 *                        and its own separate, caller-owned note-index/tick
 *                        state, entirely outside this struct) are what
 *                        SHOULD be active on the real SPU voice instead.
 *   music_note_index  -- the underlying music pattern's own current note-row
 *                        index for this channel. Untouched by
 *                        audio_channel_sfx_trigger/audio_channel_sfx_
 *                        finished below -- this field's value is exactly the
 *                        same immediately after a full preempt-then-resume
 *                        cycle as it was the instant before the preempting
 *                        sfx_trigger call, proving the "emergent resume, not
 *                        reset/restored" behavior traced above. Only chunk
 *                        4's future note-advance step (not this chunk) ever
 *                        writes a new value here, and only while is_music is
 *                        1 (a preempted channel's music state is frozen in
 *                        place simply because nothing calls that step for
 *                        it, not because this struct enforces the freeze
 *                        itself).
 *   music_tick        -- the underlying music pattern's own per-row
 *                        hold-tick counter for this channel (chunk 4's
 *                        speed/loop_start/loop_end stepping needs this to
 *                        know how long the current row has already held).
 *                        Same untouched-by-preemption guarantee as
 *                        music_note_index above, for the identical reason.
 *   sfx_pending_stop  -- 1 for exactly the one step call in which
 *                        audio_channel_sfx_finished just fired (a preempting
 *                        sfx's own key-off just happened this tick): tells
 *                        the impure caller "issue audio_voice_stop for the
 *                        sfx's own SPU voice now, then let this channel's
 *                        music resume" -- this pure module makes no SPU
 *                        register write itself (Implementation Chunks item
 *                        3: "returns 'what should now be playing on this
 *                        channel' for the PSX-only half to act on"), so this
 *                        flag is the one explicit signal that a real stop
 *                        action is due. The caller is expected to consume
 *                        this flag and clear it itself once handled (this
 *                        module never auto-clears it on a later call --
 *                        keeping this struct's own transition functions free
 *                        of any implicit "did the caller already handle the
 *                        last stop?" bookkeeping of their own).
 */
typedef struct {
    int is_music;
    int music_note_index;
    int music_tick;
    int sfx_pending_stop;
} audio_channel_state_t;

/*
 * audio_channel_init -- resets *ch to its startup state: driven by music
 * from the very first note/tick (is_music = 1, music_note_index = 0,
 * music_tick = 0), with no sfx-stop pending (sfx_pending_stop = 0). Mirrors
 * rules_tech_init's own plain-reset-to-known-state shape (rules_tech.c).
 */
void audio_channel_init(audio_channel_state_t *ch);

/*
 * audio_channel_sfx_trigger -- ports the arbitration half of a real rn()/
 * sfx() call landing on this channel (audio_channel_state_t's own doc
 * comment above has the full trace): this channel is now preempted by a
 * one-shot sfx. Sets is_music to 0 and sfx_pending_stop to 0 (a freshly
 * triggered sfx has nothing pending yet); leaves music_note_index and
 * music_tick COMPLETELY UNCHANGED -- the load-bearing property this whole
 * struct exists to guarantee (see audio_channel_state_t's doc comment).
 *
 * Calling this while the channel is already preempted (is_music already 0 --
 * a real, if not traced-as-currently-live, case: e.g. a second rn() call
 * before the first preempting sfx has finished) is not defended against
 * specially: it simply re-triggers, matching real PICO-8's own sfx()
 * semantics of a fresh trigger cutting off whatever was previously sounding
 * on that channel, sfx or music alike. music_note_index/music_tick are
 * unaffected either way, since this function never touches them.
 */
void audio_channel_sfx_trigger(audio_channel_state_t *ch);

/*
 * audio_channel_sfx_finished -- ports the arbitration half of a preempting
 * sfx's own key-off (the end of its own note sequence, chunk 4's future
 * job to detect and report -- this function is simply told it already
 * happened): this channel's music resumes.
 *
 * Sets is_music back to 1 and sfx_pending_stop to 1 (signaling the impure
 * caller to issue the real SPU stop for the sfx voice this tick, per
 * audio_channel_state_t's own doc comment on sfx_pending_stop); leaves
 * music_note_index and music_tick COMPLETELY UNCHANGED, so the very next
 * time chunk 4's future note-advance step runs for this channel, it resumes
 * from EXACTLY the row/tick it would have been at had the preemption never
 * happened -- the "emergent resume, not a separate restore mechanism"
 * property this whole module is built to prove (docs/prd/triage/
 * psx-spu-audio.md's own Channel model subsection).
 *
 * Calling this while the channel is NOT currently preempted (is_music
 * already 1 -- e.g. a stray/duplicate key-off report) is a defensive no-op:
 * is_music is left at 1 and sfx_pending_stop is left UNCHANGED (not forced
 * to 0 -- a stray duplicate call must not silently erase a real, still-
 * unconsumed pending stop from an earlier, legitimate finish; there is no
 * real sfx voice to stop on THIS call, so no new stop is signaled) --
 * source has no real path that would call this twice for the same
 * preemption, but this guard keeps the function's own contract
 * ("sfx_pending_stop means a real stop is due") honest under a caller bug,
 * the same defensive-but-untraced posture effects_flow_try_spawn's own
 * from==to guard already establishes for this codebase (effects.h).
 */
void audio_channel_sfx_finished(audio_channel_state_t *ch);

#endif
