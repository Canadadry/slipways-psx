#ifndef AUDIO_SEQUENCER_H
#define AUDIO_SEQUENCER_H

#include "audio_data.h"

/*
 * audio_sequencer.h/.c -- pure, no-PSX-headers module (mirrors rules_tech.h/
 * .c's own pure-core/stateful-wrapper split more than effects.h/.c's shape,
 * since this chunk's own second half is a genuine small state MACHINE with
 * real transitions to name and reason about, not a batch of independently-
 * traced formula/struct pairs -- effects.h's style is followed for the doc-
 * comment depth/citation discipline, rules_tech.h's for the actual
 * struct+pure-step-function shape).
 *
 * This module (docs/prd/triage/psx-spu-audio.md, Implementation Chunks
 * items 3-4, now BOTH complete) is:
 *   1. audio_resolve_instrument -- the 0-15 -> 0-7 instrument-fallback
 *      resolver (chunk 3).
 *   2. audio_channel_state_t + its pure transition functions -- the
 *      sfx-preempts-music channel-arbitration bookkeeping (chunk 3).
 *   3. audio_seq_state_t + its pure init/step/query functions -- the
 *      note-advance sequencer, stepping a sfx_def_t's own note rows per its
 *      speed/loop_start/loop_end fields (chunk 4, this addition).
 *
 * #include "audio_data.h" (chunk 3 needed no type from it at all -- pure int
 * arithmetic and a self-contained struct; chunk 4's audio_seq_step signature
 * genuinely needs sfx_def_t's real shape, so this include is new with this
 * chunk) is safe here: audio_data.h is itself "Pure data, no PSX/psxgpu.h
 * dependency anywhere" (its own header comment) -- pulling it in does not
 * compromise this module's own no-PSX-headers/host-testable posture, unlike
 * audio_waveforms.h (a GENERATED file audio_sequencer.c's own
 * AUDIO_SEQUENCER_BASIC_INSTRUMENT_COUNT comment deliberately avoids
 * depending on, for unrelated Makefile-coupling reasons that do not apply
 * here: audio_data.h is *also* generated/gitignored, but every test-audio-
 * sequencer run already has it on disk as a precondition of this whole
 * chunk's own testing, and Implementation Chunks item 4 explicitly requires
 * "Host-tested against g_sfx[] entries directly").
 *
 * No function/struct below issues any SPU register write, or any I/O of any
 * kind: this whole module stays pure and host-testable with plain gcc, no
 * PSn00bSDK toolchain, per this PRD's Testing Decisions ("Fully
 * host-testable... audio_sequencer.c's instrument-fallback/arbitration/
 * note-advance logic").
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

/*
 * audio_seq_state_t -- the note-advance sequencer's own state, per the
 * PICO-8 tracker model traced in docs/prd/triage/psx-spu-audio.md's own
 * "Sequencing: PICO-8's tracker model, and one explicit 'Incertain'"
 * subsection, quoted here verbatim since this whole struct/function trio
 * ports it directly: "Each sfx_def_t is a 32-row note sequence; speed gives
 * how long each row (note) holds before advancing... loop_start/loop_end...
 * give the row range to repeat once the sequence reaches the end -- ported
 * as: play rows 0..note_count-1 in order; upon completing the last row, if
 * loop_end > loop_start, jump back to loop_start and continue; otherwise
 * stop."
 *
 * One instance of this struct tracks ONE sfx_def_t currently being played on
 * ONE logical audio channel -- matching audio_channel_state_t's own "a
 * caller owns one instance per channel, not one shared struct" convention
 * above, for the identical reason (this port reserves one real SPU voice per
 * PICO-8 logical channel, 4 total, and a preempting sfx on a channel plays
 * against its OWN separate sfx_def_t/state, entirely independent of that
 * channel's own music_note_index/music_tick bookkeeping -- see
 * audio_channel_state_t's own doc comment above, "the sfx's OWN note/voice
 * parameters... are sequenced by that same future chunk-4 note-advance
 * step, just against the sfx's own separate sfx_def_t and its own separate,
 * caller-owned note-index/tick state, entirely outside this struct").
 *
 * Field shape (this chunk's own choice, documented here, mirroring
 * audio_channel_state_t's own "exact field shape decided at implementation
 * time" precedent):
 *
 *   current_row  -- which of the sfx_def_t's own notes[] rows (0..
 *                   note_count-1) is currently active/held. The literal
 *                   index a caller reads notes[current_row] at (or passes to
 *                   audio_seq_current_note below) to know what pitch/
 *                   instrument/volume should be sounding right now.
 *   row_tick     -- how many audio_seq_step calls have already elapsed on
 *                   the CURRENT row, counting up from 0 each time a row
 *                   starts. Compared against speed*tick_period (audio_seq_
 *                   step's own doc comment) to know when the row has held
 *                   long enough to advance.
 *   active       -- 1 while the sequence is still playing (row-by-row or
 *                   looping); 0 once it has permanently stopped (the
 *                   loop_end<=loop_start non-looping case, after completing
 *                   the last row -- see audio_seq_step's own doc comment).
 *                   Once 0, audio_seq_step is a no-op forever (matching
 *                   sfx_pending_stop's own "caller-consumed, never
 *                   auto-cleared" spirit from the opposite direction: here
 *                   it is the STATE itself, not a one-shot flag, that latches
 *                   permanently once stopped).
 */
typedef struct {
    int current_row;
    int row_tick;
    int active;
} audio_seq_state_t;

/*
 * audio_seq_init -- resets *seq to its startup state: row 0, tick 0, active.
 * Mirrors audio_channel_init's own plain-reset-to-known-state shape
 * (audio_sequencer.c) -- a freshly triggered sfx or a freshly (re)started
 * music pattern always begins at its own row 0, per the traced stepping
 * rule's own "play rows 0..note_count-1 in order" starting point.
 */
void audio_seq_init(audio_seq_state_t *seq);

/*
 * audio_seq_step -- advances *seq by exactly one caller-defined step unit
 * against sfx (the sfx_def_t currently being played), per the stepping rule
 * quoted verbatim on audio_seq_state_t's own doc comment above. Returns
 * *seq's own new active value (1: still playing after this step; 0: this
 * step just permanently stopped the sequence, or it was already stopped
 * before this call) -- Implementation Chunks item 4's own wording: "returns
 * whether the sequence is still active (not stopped)".
 *
 * tick_period is the Incertain-flagged parameter this whole function exists
 * to isolate (docs/prd/triage/psx-spu-audio.md's own Implementation
 * Decisions: "the exact real-time unit speed counts in (PICO-8's internal
 * audio tick rate) is... recorded as Incertain... the sequencer's own pure
 * step function takes the tick rate as a parameter rather than a hardcoded
 * constant, so the real figure can be plugged in once confirmed without
 * re-deriving the state machine itself"). A row holds for exactly
 * sfx->speed * tick_period consecutive audio_seq_step calls before
 * advancing -- i.e. tick_period is "how many real audio_seq_step calls make
 * up one of PICO-8's own single speed-units", left for a future chunk to
 * plug in the real confirmed figure; this function itself asserts nothing
 * about what real-world unit (SPU sample-block interrupts, a fixed
 * per-frame count, or anything else) a "step" corresponds to.
 *
 * Called once already stopped (seq->active already 0): a defensive no-op,
 * returning 0 immediately without touching current_row/row_tick -- mirrors
 * audio_channel_sfx_finished's own "stray call after the real transition
 * already happened must not fabricate new state" posture.
 *
 * Row-advance/loop/stop logic, exactly per the quoted stepping rule: once
 * the current row has held for sfx->speed * tick_period steps, if
 * current_row is not yet the last row (note_count-1), current_row simply
 * increments by 1 and row_tick resets to 0 (play rows 0..note_count-1 in
 * order). Once current_row IS the last row and it has held its own full
 * duration, "upon completing the last row" fires: if sfx->loop_end >
 * sfx->loop_start, current_row jumps back to sfx->loop_start (row_tick
 * resets to 0, active stays 1) and playback continues from there --
 * otherwise active is set to 0 and current_row is left at note_count-1 (the
 * last row it was legitimately holding), permanently stopping. Note this is
 * the rule EXACTLY as traced -- loop_end's own value is used only as the
 * ">loop_start" boolean gate deciding WHETHER to loop, never as a
 * mid-sequence upper bound that cuts row 0..note_count-1 short on any
 * iteration (every full pass, looped or not, always plays every row up to
 * note_count-1 first); only the jump-BACK destination is loop_start. This
 * reading is the PRD's own literal wording, re-checked directly against the
 * current PRD text rather than assumed, and audio_sequencer_test.c's own
 * loop test constructs a scenario that would fail under the OTHER plausible
 * reading (loop_end as a live upper bound cutting the pass short) to prove
 * this one is what is actually implemented.
 */
int audio_seq_step(audio_seq_state_t *seq, const sfx_def_t *sfx, int tick_period);

/*
 * audio_seq_current_note -- "what note is currently active" query
 * (Implementation Chunks item 4's own caller-facing need), returning a
 * pointer to sfx->notes[seq->current_row] -- the pitch/instrument/volume/
 * effect the impure caller should be driving the real SPU voice with right
 * now. A thin, obviously-correct accessor (not a dedicated struct of its
 * own -- the PRD's Data Model section names no such struct, and sfx_note_t
 * itself, audio_data.h, already carries exactly the 4 fields a caller would
 * ever want here), kept as a real function rather than inlined at every call
 * site purely so callers/tests never need to know this struct's own
 * current_row field name to read "the active note" -- the same "small
 * interface over the real field" spirit as audio_pitch_register's own
 * table-lookup wrapper (audio_pitch.c).
 *
 * seq->current_row is expected in [0, sfx->note_count-1] (always true after
 * any real audio_seq_init/audio_seq_step sequence against the SAME sfx --
 * audio_seq_step never advances current_row outside that range for a
 * sfx_def_t whose own note_count/loop_start/loop_end are internally
 * consistent, matching this module's own established "callers pass
 * already-valid data, no defensive clamping" posture, e.g. audio_resolve_
 * instrument's own identical stance on its input range).
 */
const sfx_note_t *audio_seq_current_note(const audio_seq_state_t *seq, const sfx_def_t *sfx);

#endif
