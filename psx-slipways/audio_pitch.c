#include "audio_pitch.h"

/*
 * AUDIO_PITCH_REGISTER_TABLE -- the 64-entry precomputed SPU pitch-register
 * table audio_pitch_register indexes into, one entry per PICO-8 `pitch`
 * value (0-63).
 *
 * Implementation choice: this chunk uses option (b) from this PRD's two
 * documented alternatives -- a hand-written table computed once via a
 * throwaway host script and pasted here as a literal array with provenance
 * comments -- rather than option (a), a checked-in tools/gen_psx_*.py
 * generator that would regenerate this file on every build (like
 * tools/gen_psx_sprite_atlas.py/tools/gen_psx_audio_data.py do for their own
 * modules). Reasoning: this PRD's own Data Model section (docs/prd/triage/
 * psx-spu-audio.md) explicitly names a new tools/gen_psx_waveforms.py script
 * for chunk 2's waveform data, but does NOT name any equivalent generator
 * for this chunk's pitch table -- unlike sprite_atlas.h/.c or audio_data.h/.c
 * (each fed by real, evolving source data -- pico8/gfx.json, pico8/sfx.json/
 * music.json -- that could change and would need re-extraction), this
 * table's entire input is two fixed constants (the pitch=33-at-440Hz
 * equal-temperament formula and AUDIO_WAVEFORM_REFERENCE_HZ, audio_pitch.h)
 * that never change at build time -- there is no "regenerate on every build"
 * need a checked-in script would serve, matching fixed.h's own
 * FIXED_QUARTER_SIN_TABLE precedent exactly (a throwaway host program run
 * once, offline, its output pasted into the header as a static const array
 * -- fixed.h's own block comment on FIXED_QUARTER_SIN_TABLE cites this same
 * precedent explicitly, "generated once, offline, by a throwaway host-side
 * program (not shipped, not a runtime dependency)").
 *
 * Provenance: every value below was computed by a temporary Python script
 * (tools/tmp_gen_pitch_table.py, run once via a temporary Makefile target,
 * both deleted before this chunk was reported done -- the exact
 * "temporary Makefile target, deleted before finishing" workaround this
 * chunk's own task description calls for) implementing exactly the two-step
 * formula audio_pitch_register's own doc comment (audio_pitch.h) describes,
 * in real host-side double-precision floating point (never shipped -- this
 * restriction only applies to PSX C code, not the offline generation
 * script):
 *
 *   hz(pitch)  = 440.0 * 2.0**((pitch-33)/12.0)
 *   register   = round(hz(pitch) * 4096.0 / AUDIO_WAVEFORM_REFERENCE_HZ)
 *
 * where AUDIO_WAVEFORM_REFERENCE_HZ is 220 (audio_pitch.h) and round(x) is
 * round-half-up (int(x+0.5) -- every computed value here is positive, so
 * round-half-up and round-half-to-even never disagree on any entry in this
 * table; no value below lands exactly on a .5 boundary in practice, so the
 * distinction is moot regardless).
 *
 * Independently re-derivable spot checks (see audio_pitch_test.c for the
 * actual host-test assertions, each hand-computed there separately from
 * this table, not copied from it):
 *   pico8_pitch=0:  hz=65.406391...  Hz -> register 1218
 *   pico8_pitch=33: hz=440.0         Hz -> register 8192
 *   pico8_pitch=63: hz=2489.015870.. Hz -> register 46341
 *   pico8_pitch=21: hz=220.0         Hz -> register 4096 (exactly
 *                   AUDIO_WAVEFORM_REFERENCE_HZ -- this PRD's own sanity
 *                   check lands here, not at pitch=33, because
 *                   AUDIO_WAVEFORM_REFERENCE_HZ is 220 here, not 440 --
 *                   see audio_pitch.h's own doc comment on
 *                   AUDIO_WAVEFORM_REFERENCE_HZ and on audio_pitch_register)
 */
static const uint16_t AUDIO_PITCH_REGISTER_TABLE[64] = {
    1218,  1290,  1367,  1448,  1534,  1625,  1722,  1825,  1933,  2048,  2170,  2299,  2435,  2580,  2734,  2896,
    3069,  3251,  3444,  3649,  3866,  4096,  4340,  4598,  4871,  5161,  5468,  5793,  6137,  6502,  6889,  7298,
    7732,  8192,  8679,  9195,  9742,  10321, 10935, 11585, 12274, 13004, 13777, 14596, 15464, 16384, 17358, 18390,
    19484, 20643, 21870, 23170, 24548, 26008, 27554, 29193, 30929, 32768, 34716, 36781, 38968, 41285, 43740, 46341,
};

uint16_t audio_pitch_register(int pico8_pitch) {
    return AUDIO_PITCH_REGISTER_TABLE[pico8_pitch];
}
