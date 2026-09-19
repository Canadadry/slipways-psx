#include "t.h"
#include "../audio_pitch.h"

#include <math.h>
#include <stdint.h>

/*
 * hand_computed_register -- an INDEPENDENT re-derivation of
 * audio_pitch_register's own two-step formula (audio_pitch.h's doc
 * comment), using real host-side libm (pow), not the module-under-test's
 * own precomputed table. Every test below that asserts against this
 * function's output is checking audio_pitch_register's TABLE against the
 * FORMULA the table was supposed to encode -- not re-testing the table
 * against itself, matching this codebase's established "derive expected
 * values independently, don't copy them from the implementation"
 * convention (effects_test.c's test_idle_bob_delta_matches_formula_at_
 * several_t_values, audio_data_test.c's hand-typed expected note arrays).
 * Rounding matches audio_pitch.c's own documented round-half-up convention
 * exactly ((uint16_t)(reg + 0.5), reg always positive here).
 */
static uint16_t hand_computed_register(int pico8_pitch) {
    double hz = 440.0 * pow(2.0, (pico8_pitch - 33) / 12.0);
    double reg = hz * 4096.0 / (double)AUDIO_WAVEFORM_REFERENCE_HZ;
    return (uint16_t)(reg + 0.5);
}

/*
 * This PRD's own sanity check (docs/prd/triage/psx-spu-audio.md,
 * Implementation Decisions / "Pitch: PICO-8 note index to SPU pitch
 * register"): "at hz == AUDIO_WAVEFORM_REFERENCE_HZ the corrected formula
 * gives exactly 4096". Checked here as a DIRECT evaluation of the raw
 * spu_pitch_register(hz) = (hz*4096)/AUDIO_WAVEFORM_REFERENCE_HZ formula at
 * hz == AUDIO_WAVEFORM_REFERENCE_HZ itself -- independent of whether any of
 * the 64 discrete pico8_pitch values happens to land exactly on that Hz (per
 * this chunk's own task description: "if none of the 64 pico8_pitch values
 * land exactly on your reference Hz, that's fine... verify the sanity check
 * using a DIRECT test of audio_pitch_register-equivalent math at
 * hz=AUDIO_WAVEFORM_REFERENCE_HZ itself"). Exact equality is used, not a
 * tolerance -- (REF*4096)/REF is an exact algebraic identity for any nonzero
 * REF, with no floating-point rounding risk at this specific ratio (REF
 * cancels exactly; the division is REF*4096/REF, not some unrelated
 * irrational quantity). */
static void test_direct_formula_at_reference_hz_gives_4096(void) {
    double hz = (double)AUDIO_WAVEFORM_REFERENCE_HZ;
    double reg = hz * 4096.0 / (double)AUDIO_WAVEFORM_REFERENCE_HZ;
    T_CHECK(reg == 4096.0);
}

/* AUDIO_WAVEFORM_REFERENCE_HZ is 220, not 440, in this implementation (see
 * audio_pitch.h's own doc comment on why), so no pico8_pitch=33 shortcut
 * applies -- but pico8_pitch=21 happens to land on EXACTLY 220 Hz (440 *
 * 2^((21-33)/12) == 440 * 2^-1 == 220.0 exactly, a whole octave-and-a-bit
 * relationship, not a coincidence of floating-point rounding), so this is a
 * SECOND, independent confirmation of the same sanity-check property as
 * test_direct_formula_at_reference_hz_gives_4096 above, this time routed
 * through the real audio_pitch_register table lookup rather than the bare
 * formula -- both checked as exact equality/a tight epsilon, not a
 * tolerance band, since the underlying math is exact here. */
static void test_pitch_21_is_exactly_reference_hz_and_register_4096(void) {
    double hz21 = 440.0 * pow(2.0, (21 - 33) / 12.0);
    T_CHECK(fabs(hz21 - (double)AUDIO_WAVEFORM_REFERENCE_HZ) < 0.0000001);
    T_CHECK(audio_pitch_register(21) == 4096);
}

/* pitch=33 (A4, 440 Hz) -- this chunk's own task description's second
 * requested checkpoint. hand_computed_register(33) is independently
 * re-derived via libm above, then cross-checked against the literal value
 * 8192 (hand-verified here too: hz=440 is exactly 2x AUDIO_WAVEFORM_
 * REFERENCE_HZ==220, and the formula is linear in hz, so register is
 * exactly 2x4096==8192, no rounding involved), before comparing against
 * audio_pitch_register's own table output. */
static void test_pitch_33_matches_hand_computed_value(void) {
    uint16_t expected = hand_computed_register(33);
    T_CHECK(expected == 8192);
    T_CHECK(audio_pitch_register(33) == expected);
}

/* pitch=0 -- this chunk's own task description's requested checkpoint
 * (PICO-8's lowest representable pitch, ~65.4 Hz). Value hand-derived
 * independently via hand_computed_register (libm), not copied from
 * audio_pitch.c's own table or its provenance comment. */
static void test_pitch_0_matches_hand_computed_value(void) {
    uint16_t expected = hand_computed_register(0);
    T_CHECK(expected == 1218);
    T_CHECK(audio_pitch_register(0) == expected);
}

/* pitch=63 -- this chunk's own task description's requested checkpoint
 * (PICO-8's highest representable pitch, ~2489 Hz). Also implicitly proves
 * the resulting register (46341) comfortably fits uint16_t's 65535 ceiling
 * -- the full 64-entry table never overflows the register's own real
 * hardware width. */
static void test_pitch_63_matches_hand_computed_value(void) {
    uint16_t expected = hand_computed_register(63);
    T_CHECK(expected == 46341);
    T_CHECK(audio_pitch_register(63) == expected);
}

/* Monotonicity across the full 0-63 range: every step strictly increases
 * the register value -- this chunk's own task description's third required
 * property, checked exactly as specified ("table[i+1] > table[i] for all i
 * in 0..62"), via the public audio_pitch_register accessor rather than
 * reaching into the module's internal table directly (matching this
 * codebase's "test through the public interface" convention). Equal-
 * temperament's hz(pitch) is itself strictly increasing in pitch, and the
 * linear hz-to-register conversion preserves that ordering, so this is a
 * real, exercised property of the formula, not an incidental one. */
static void test_register_strictly_increases_across_full_range(void) {
    for (int i = 0; i < 63; i++) {
        uint16_t lo = audio_pitch_register(i);
        uint16_t hi = audio_pitch_register(i + 1);
        T_CHECK(hi > lo);
    }
}

/* A broader spot-check across the table's midrange (not just the three
 * explicitly-requested checkpoints above), each independently re-derived
 * via hand_computed_register -- catches a table-generation regression that
 * happened to leave the three required checkpoints correct by chance while
 * corrupting other entries (e.g. an off-by-one in the generation script's
 * loop bounds). Pitches chosen to spread across the full range, including
 * both even and odd offsets from the pitch=33 anchor. */
static void test_several_midrange_pitches_match_hand_computed_formula(void) {
    int pitches[] = {5, 12, 17, 24, 40, 50, 58};
    for (size_t i = 0; i < sizeof(pitches) / sizeof(pitches[0]); i++) {
        int p = pitches[i];
        uint16_t expected = hand_computed_register(p);
        uint16_t actual = audio_pitch_register(p);
        T_CHECK(actual == expected);
    }
}

int main(void) {
    t_run("direct_formula_at_reference_hz_gives_4096", test_direct_formula_at_reference_hz_gives_4096);
    t_run("pitch_21_is_exactly_reference_hz_and_register_4096", test_pitch_21_is_exactly_reference_hz_and_register_4096);
    t_run("pitch_33_matches_hand_computed_value", test_pitch_33_matches_hand_computed_value);
    t_run("pitch_0_matches_hand_computed_value", test_pitch_0_matches_hand_computed_value);
    t_run("pitch_63_matches_hand_computed_value", test_pitch_63_matches_hand_computed_value);
    t_run("register_strictly_increases_across_full_range", test_register_strictly_increases_across_full_range);
    t_run("several_midrange_pitches_match_hand_computed_formula", test_several_midrange_pitches_match_hand_computed_formula);
    return t_report();
}
