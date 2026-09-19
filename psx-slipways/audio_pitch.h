#ifndef AUDIO_PITCH_H
#define AUDIO_PITCH_H

#include <stdint.h>

/*
 * AUDIO_WAVEFORM_REFERENCE_HZ -- the authoring frequency chunk 2's waveform
 * generator (tools/gen_psx_waveforms.py, psx-slipways/audio_waveforms.h/.c,
 * NOT this chunk's concern -- see docs/prd/triage/psx-spu-audio.md,
 * Implementation Decisions / "Pitch: PICO-8 note index to SPU pitch
 * register") bakes its 8 single-cycle basic-instrument waveforms at. This
 * constant lives here (audio_pitch.h), not audio_waveforms.h, because THIS
 * module's own table (audio_pitch.c) is generated against it -- chunk 2's
 * implementer should #include "audio_pitch.h" and reuse this constant
 * rather than picking an independent, conflicting value.
 *
 * Chosen: 220 Hz (the PRD's own suggested example value). At the SPU's fixed
 * 44100 Hz playback engine, one authored cycle at 220 Hz would hold
 * round(44100/220) == 200.4545... samples -- NOT the exact whole number the
 * PRD's own prose floats as a possible outcome ("220 Hz, chosen only for a
 * convenient whole-cycle sample count... like 220 Hz giving exactly 200
 * samples"); re-checked directly here, 44100 is not evenly divisible by 220
 * (44100 = 2^2*3^2*5^2*7^2, 220 = 2^2*5*11, and 11 does not divide 44100).
 * 220 Hz is kept anyway, because:
 *   - It is still the PRD's own explicitly named, well-reasoned default --
 *     the reasoning that matters is "an arbitrary, documented, shared
 *     constant both chunks agree on," not "the sample count is a
 *     mathematically exact integer".
 *   - It sits comfortably inside PICO-8's own real per-note pitch range
 *     (~65 Hz to ~2489 Hz, Implementation Decisions above) with headroom on
 *     both sides, and gives a round(...)-to-200-samples-per-cycle waveform
 *     buffer chunk 2 can size sensibly (200 samples, one rounded off from
 *     the true 200.4545...).
 *   - It reproduces this PRD's own sanity check exactly: audio_pitch_
 *     register's own conversion formula, evaluated directly at
 *     hz==AUDIO_WAVEFORM_REFERENCE_HZ (220), gives EXACTLY 4096 -- see that
 *     function's own doc comment below, and psx-slipways/host_tests/
 *     audio_pitch_test.c's test_direct_formula_at_reference_hz_gives_4096,
 *     which checks this directly against the formula, not through this
 *     module's 64-entry pico8_pitch table (no discrete pico8_pitch=0..63
 *     value is required to land exactly on 220 Hz for this property to hold
 *     -- see that test's own comment for why -- though, as it happens,
 *     pico8_pitch=21 DOES: 440*2^((21-33)/12) == 440*2^-1 == 220.0 exactly,
 *     confirmed independently during this chunk's own generation-time
 *     computation).
 * A different value is a real option for a future implementer who wants an
 * EXACT integer sample count (e.g. 245 Hz -> 44100/245 == 180 exactly, or
 * 441 Hz -> 100 exactly) -- but that tradeoff is chunk 2's own call to make
 * against its own waveform-quality concerns, not something this chunk
 * should second-guess. If chunk 2 ever changes this constant,
 * AUDIO_PITCH_REGISTER_TABLE (audio_pitch.c) must be regenerated against the
 * new value -- see audio_pitch.c's own header comment for exactly how.
 */
#define AUDIO_WAVEFORM_REFERENCE_HZ 220

/*
 * audio_pitch_register -- ports PICO-8's `pitch` field (0-63, a semitone
 * index into PICO-8's own internal equal-temperament tuning table) to the
 * PS1 SPU's real per-voice 4.12 fixed-point pitch register value
 * (SPU_CH_FREQ, psxspu.h), per docs/prd/triage/psx-spu-audio.md,
 * Implementation Decisions / "Pitch: PICO-8 note index to SPU pitch
 * register" (the PRD's own reviewer-corrected formula -- NOT a bare
 * getSPUSampleRate(hz) call, see below).
 *
 * Two steps, both cited in the PRD, combined algebraically there into one
 * formula this function implements as a precomputed table (no runtime
 * float/pow -- this codebase's hard no-FPU/transcendental-math-on-the-
 * R3000A rule, docs/plan.md, fixed.h's own FIXED_QUARTER_SIN_TABLE
 * precedent):
 *
 *   1. hz(pitch) = 440 * 2^((pitch-33)/12) -- equal-temperament, pitch=33
 *      anchored at A4 (440 Hz). Well-documented, publicly cited PICO-8
 *      tuning convention (matching zepto8 and community SFX-format
 *      write-ups) -- there is no cart-side code to decompile for this (same
 *      host-engine-only reasoning as the instrument palette, Implementation
 *      Decisions).
 *
 *   2. spu_pitch_register(hz) = (hz * 4096) / AUDIO_WAVEFORM_REFERENCE_HZ --
 *      NOT getSPUSampleRate(hz) directly (psxspu.h's own
 *      `((rate*4096)/44100)`, which would only be correct in the degenerate
 *      case AUDIO_WAVEFORM_REFERENCE_HZ==44100, never used here). This is
 *      the PRD's own corrected formula: each of the 8 baked single-cycle
 *      waveforms (chunk 2, NOT this chunk) is authored at
 *      AUDIO_WAVEFORM_REFERENCE_HZ assuming a 44100 Hz sampling rate, so
 *      getSPUSampleRate()'s own 44100 divisor cancels algebraically against
 *      the waveform's own sample count, leaving AUDIO_WAVEFORM_REFERENCE_HZ
 *      as the real divisor. Sanity check (independently re-verified during
 *      this chunk's own table generation, not just restated from the PRD):
 *      at hz == AUDIO_WAVEFORM_REFERENCE_HZ, this formula gives EXACTLY 4096
 *      -- the SPU's native/unshifted playback rate, matching the fact that a
 *      waveform buffer authored at its own reference frequency already IS
 *      that pitch when stepped through unmodified. Confirmed directly at
 *      table-generation time: (220.0 * 4096) / 220.0 == 4096.0 exactly, no
 *      rounding involved (see audio_pitch.c's own generation-provenance
 *      comment). Because AUDIO_WAVEFORM_REFERENCE_HZ is 220 here (not 440),
 *      pico8_pitch=33 (hz=440) does NOT land on register 4096 -- it lands on
 *      exactly 8192 (440 is exactly 2x this reference, and the formula is
 *      linear in hz) -- see audio_pitch_test.c for both this module's own
 *      pico8_pitch=33 check (register 8192) and a SEPARATE, direct check of
 *      the hz==AUDIO_WAVEFORM_REFERENCE_HZ==220 sanity property itself (via
 *      pico8_pitch=21, which happens to be exactly 220 Hz, AND via the raw
 *      formula independent of any discrete pico8_pitch value).
 *
 * The resulting 64-entry table (AUDIO_PITCH_REGISTER_TABLE, audio_pitch.c)
 * is a hand-computed literal array, not generated by a checked-in
 * tools/gen_psx_*.py script -- see audio_pitch.c's own header comment for
 * why (option (b) of this chunk's two documented implementation choices),
 * and for the exact throwaway generation-time provenance of every value in
 * it.
 *
 * pico8_pitch is expected in [0,63] (PICO-8's own valid range, matching
 * sfx_note_t's own `pitch` field, psx-slipways/audio_data.h) -- this
 * function does not clamp or otherwise defend against an out-of-range
 * input; every real caller's pico8_pitch value comes from an already-range-
 * checked sfx_note_t (gen_psx_audio_data.py's own range check, cited in this
 * PRD's Problem Statement), so an out-of-range table index here would
 * indicate a real caller bug upstream, not a case this pure function needs
 * to paper over silently.
 */
uint16_t audio_pitch_register(int pico8_pitch);

#endif
