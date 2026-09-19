#include "t.h"
#include "../audio_pitch.h"
#include "../audio_waveforms.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * gen_psx_waveforms_test.c -- host test for tools/gen_psx_waveforms.py's
 * generated psx-slipways/audio_waveforms.c (chunk 2, docs/prd/triage/
 * psx-spu-audio.md, Implementation Chunks item 2).
 *
 * Everything below (the 8 waveform formulas, the ADPCM decoder, the
 * quantization-tolerance formula) is an INDEPENDENT re-derivation, not
 * copied from tools/gen_psx_waveforms.py -- matching this codebase's
 * established "derive expected values independently, don't just re-test
 * the tool against itself" convention (audio_pitch_test.c's
 * hand_computed_register, audio_data_test.c's hand-typed expected note
 * arrays). A bug shared between the generator and this test (e.g. both
 * getting a formula wrong the same way) is the one class of error this
 * discipline cannot catch -- unavoidable for any test, noted for honesty,
 * not a gap unique to this file.
 *
 * Waveform-shape citations: all 8 *_wave() functions below are ported from
 * zepto8 (https://github.com/samhocevar/zepto8, WTFPL), src/synth.cpp's
 * synth::waveform() function -- the same public, open-source,
 * reverse-engineered-from-real-PICO-8-output reimplementation
 * audio_pitch.h's own doc comment already cites for the equal-temperament
 * tuning convention. Only the base (non-"buzz") case is ported (buzz is a
 * per-note custom-instrument detune modifier, out of scope -- see
 * gen_psx_waveforms.py's own header comment for the full reasoning, which
 * this file's formulas mirror exactly).
 *
 * ADPCM format citation: the decoder below implements the real, standard,
 * publicly documented Sony PS1 SPU-ADPCM ("VAG") block format: 16-byte
 * blocks, each a 1-byte shift/filter header + 1-byte loop-flag byte + 14
 * payload bytes (28 4-bit samples, low nibble = earlier sample), decoded
 * via `sample = (int16_t)(nibble << 12) >> shift; decoded = sample +
 * ((k0*hist1 + k1*hist2) >> 6)`, against the same standard 5-entry
 * predictor table gen_psx_waveforms.py's own encoder uses (well-known and
 * reused across essentially every public PS-ADPCM encoder/decoder
 * implementation, per docs/prd/triage/psx-spu-audio.md's own "ADPCM..."
 * subsection).
 *
 * Quantization tolerance: derived directly from each block's own stored
 * shift (the format's real quantization granularity), not an arbitrary
 * flat number -- see tolerance_for_shift()'s own doc comment.
 */

/* Constants mirrored from gen_psx_waveforms.py (that script has no
 * C-header-parsing precedent to instead read these from -- see this
 * codebase's existing AUDIO_WAVEFORM_REFERENCE_HZ duplication precedent,
 * audio_pitch.h/gen_psx_waveforms.py, for the same reasoning).
 * Cross-checked below (test_reference_hz_and_sample_count_match_audio_
 * pitch_h) against the real AUDIO_WAVEFORM_REFERENCE_HZ this test
 * #includes from audio_pitch.h, so any future drift between the three
 * copies (audio_pitch.h, gen_psx_waveforms.py, this file) is caught by
 * `make test-audio-waveforms`, not silently possible. */
#define SAMPLES_PER_CYCLE 200
#define PADDED_SAMPLE_COUNT 224
#define PCM_PEAK_AMPLITUDE 30000
#define ADPCM_BLOCK_SAMPLES 28

/* case INST_TRIANGLE: ret = 1.0f - fabs(4.f * t - 2.0f); return ret * 0.5f;
 * -- the trailing *0.5f is dropped here too, matching gen_psx_waveforms.py's
 * own triangle_wave(), since this test's decoded output is compared against
 * the generator's independently-peak-normalized waveform, not the source's
 * raw [-0.5,0.5] range. */
static double triangle_wave(double t) {
    return 1.0 - fabs(4.0 * t - 2.0);
}

/* case INST_TILTED_SAW (a=0.875f):
 * ret = t<a ? 2*t/a-1 : 2*(1-t)/(1-a)-1; return ret*0.5; */
static double tilted_saw_wave(double t) {
    double a = 0.875;
    double ret = (t < a) ? (2.0 * t / a - 1.0) : (2.0 * (1.0 - t) / (1.0 - a) - 1.0);
    return ret * 0.5;
}

/* case INST_SAW: ret = (t<0.5f ? t : t-1.f); return 0.653f*ret; */
static double saw_wave(double t) {
    double ret = (t < 0.5) ? t : (t - 1.0);
    return 0.653 * ret;
}

/* case INST_SQUARE: return t < 0.5f ? 0.25f : -0.25f; */
static double square_wave(double t) {
    return (t < 0.5) ? 0.25 : -0.25;
}

/* case INST_PULSE: return t < 0.316f ? 0.25f : -0.25f; */
static double pulse_wave(double t) {
    return (t < 0.316) ? 0.25 : -0.25;
}

/* case INST_ORGAN: ret = t<0.5f ? 3.f-fabs(24.f*t-6.f) : 1.f-fabs(16.f*t-12.f);
 * return ret/9.f; */
static double organ_wave(double t) {
    double ret = (t < 0.5) ? (3.0 - fabs(24.0 * t - 6.0)) : (1.0 - fabs(16.0 * t - 12.0));
    return ret / 9.0;
}

/* case INST_PHASER: ret = 2.f-fabs(8.f*t-4.f);
 * ret += 1.f-fabs(4.f*fmod(advance*109.f/110.f,1.f)-2.f); return ret/6.f;
 * -- advance == t here (see gen_psx_waveforms.py's phaser_wave() for the
 * full reasoning: `advance` is the voice's raw unwrapped phase, which
 * equals wrapped phase `t` throughout a single fresh-key-on'd cycle). */
static double phaser_wave(double t) {
    double ret = 2.0 - fabs(8.0 * t - 4.0);
    double advance = t;
    ret += 1.0 - fabs(4.0 * fmod(advance * 109.0 / 110.0, 1.0) - 2.0);
    return ret / 6.0;
}

/* Same documented 32-bit LCG as gen_psx_waveforms.py's noise_samples()
 * (classic ANSI C/glibc rand() constants, chosen specifically for being
 * trivial to reproduce bit-for-bit across Python and C -- see that
 * function's own doc comment). Re-typed independently here, not copied. */
static uint32_t g_noise_rng_state;

static double noise_next_sample(void) {
    g_noise_rng_state = (uint32_t) (g_noise_rng_state * 1103515245u + 12345u);
    return ((double) g_noise_rng_state / 4294967295.0) * 2.0 - 1.0;
}


static int16_t clamp_int16(long v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t) v;
}

/* Fills expected_padded[PADDED_SAMPLE_COUNT] with the independently
 * recomputed, peak-normalized, block-padded reference PCM for one
 * instrument id (0-7) -- mirrors gen_psx_waveforms.py's own
 * generate_waveform_floats() -> quantize_to_pcm16() ->
 * pad_to_block_multiple() pipeline exactly (peak-normalize each waveform
 * independently to PCM_PEAK_AMPLITUDE, then flat-hold-pad the last real
 * sample out to a whole number of ADPCM blocks -- see that script's
 * pad_to_block_multiple() for the full reasoning on why flat-hold, not
 * zero/wrap padding), using this file's own independently re-derived
 * formulas/constants above. */
static void generate_expected_padded_pcm(int instrument, int16_t *expected_padded) {
    double raw[SAMPLES_PER_CYCLE];

    if (instrument == AUDIO_INSTRUMENT_NOISE) {
        g_noise_rng_state = 1;
        for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
            raw[i] = noise_next_sample();
        }
    } else {
        for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
            double t = (double) i / (double) SAMPLES_PER_CYCLE;
            double v;
            switch (instrument) {
                case AUDIO_INSTRUMENT_TRIANGLE: v = triangle_wave(t); break;
                case AUDIO_INSTRUMENT_TILTED_SAW: v = tilted_saw_wave(t); break;
                case AUDIO_INSTRUMENT_SAW: v = saw_wave(t); break;
                case AUDIO_INSTRUMENT_SQUARE: v = square_wave(t); break;
                case AUDIO_INSTRUMENT_PULSE: v = pulse_wave(t); break;
                case AUDIO_INSTRUMENT_ORGAN: v = organ_wave(t); break;
                case AUDIO_INSTRUMENT_PHASER: v = phaser_wave(t); break;
                default: v = 0.0; break;
            }
            raw[i] = v;
        }
    }

    double max_abs = 0.0;
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        double a = fabs(raw[i]);
        if (a > max_abs) max_abs = a;
    }
    double scale = (max_abs > 0.0) ? (PCM_PEAK_AMPLITUDE / max_abs) : 0.0;

    int16_t pcm16[SAMPLES_PER_CYCLE];
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        pcm16[i] = clamp_int16((long) lround(raw[i] * scale));
    }

    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        expected_padded[i] = pcm16[i];
    }
    for (int i = SAMPLES_PER_CYCLE; i < PADDED_SAMPLE_COUNT; i++) {
        expected_padded[i] = pcm16[SAMPLES_PER_CYCLE - 1];
    }
}


/* The standard, publicly documented 5-entry PS-ADPCM predictor-coefficient
 * table (see this file's own header comment for the citation) -- re-typed
 * independently here, not copied from gen_psx_waveforms.py. */
static const int ADPCM_FILTER_K0[5] = {0, 60, 115, 98, 122};
static const int ADPCM_FILTER_K1[5] = {0, 0, -52, -55, -60};

/* Decodes AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT 16-byte blocks
 * (AUDIO_WAVEFORM_ADPCM_BYTES total) into PADDED_SAMPLE_COUNT int16 PCM
 * samples, plus out_shift[block] (each block's own stored shift, read back
 * out for tolerance_for_shift() below) and out_loop_flags[block] (each
 * block's raw loop-flag byte, for the loop self-consistency checks).
 *
 * Per-nibble decode formula: a 4-bit signed code (nibble, sign-extended)
 * is first widened to a 16-bit-range value by shifting it up to the top of
 * an int16 (nibble << 12, i.e. one of 16 evenly spaced codes spanning
 * roughly the full int16 range), then scaled DOWN by this block's own
 * shift (0-12) to the residual magnitude actually intended for this block
 * -- `raw = (int16_t)(nibble << 12) >> shift`. The final sample is that
 * residual plus this filter's own 2-tap prediction from the two
 * previously DECODED samples, clamped to int16 range -- exactly mirroring
 * gen_psx_waveforms.py's own closed-loop encoder, so decode here is
 * bit-exact against what that script actually encoded. */
static void decode_adpcm(const uint8_t block_bytes[AUDIO_WAVEFORM_ADPCM_BYTES],
                          int16_t out_pcm[PADDED_SAMPLE_COUNT],
                          int out_shift[AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT],
                          uint8_t out_loop_flags[AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT]) {
    int hist1 = 0;
    int hist2 = 0;

    for (int b = 0; b < AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT; b++) {
        const uint8_t *block = block_bytes + b * AUDIO_WAVEFORM_ADPCM_BLOCK_BYTES;
        int shift = (block[0] >> 4) & 0x0F;
        int filter = block[0] & 0x0F;
        uint8_t loop_flags = block[1];
        out_shift[b] = shift;
        out_loop_flags[b] = loop_flags;

        int k0 = ADPCM_FILTER_K0[filter];
        int k1 = ADPCM_FILTER_K1[filter];

        for (int i = 0; i < ADPCM_BLOCK_SAMPLES; i++) {
            uint8_t byte_val = block[2 + i / 2];
            int nibble_raw = (i % 2 == 0) ? (byte_val & 0x0F) : ((byte_val >> 4) & 0x0F);
            int16_t nibble = (int16_t) (nibble_raw << 12);
            int32_t residual = ((int32_t) nibble) >> shift;

            int32_t pred = (k0 * hist1 + k1 * hist2) >> 6;
            int32_t decoded = residual + pred;
            int16_t decoded16 = clamp_int16(decoded);

            out_pcm[b * ADPCM_BLOCK_SAMPLES + i] = decoded16;
            hist2 = hist1;
            hist1 = decoded16;
        }
    }
}

#define PREDICTOR_MARGIN 96

/* The real VAG/PS-ADPCM decode formula (see decode_adpcm()'s own doc
 * comment just above) is `sample = (int16_t)(nibble << 12) >> shift`: the
 * 4-bit code sits in a 16-bit word's TOP 4 bits (nibble*4096) and is then
 * shifted RIGHT by `shift`. So the actual quantization STEP size at a
 * given stored shift is `2^(12-shift)` -- a *larger* shift means a
 * *finer* step, not a larger one. The finest achievable rounding error is
 * +-half a step. */
static int tolerance_for_shift(int shift) {
    return (1 << (12 - shift)) / 2 + PREDICTOR_MARGIN;
}


static void check_round_trip_and_self_consistency(int instrument) {
    int16_t expected_padded[PADDED_SAMPLE_COUNT];
    generate_expected_padded_pcm(instrument, expected_padded);

    int16_t decoded[PADDED_SAMPLE_COUNT];
    int shifts[AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT];
    uint8_t loop_flags[AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT];
    decode_adpcm(g_instrument_waveform_adpcm[instrument], decoded, shifts, loop_flags);

    /* --- Decode-and-compare round trip, within a documented, shift-derived
     * quantization tolerance (see tolerance_for_shift()'s own doc comment). */
    for (int i = 0; i < PADDED_SAMPLE_COUNT; i++) {
        int block = i / ADPCM_BLOCK_SAMPLES;
        int tol = tolerance_for_shift(shifts[block]);
        int diff = decoded[i] - expected_padded[i];
        if (diff < 0) diff = -diff;
        T_CHECK(diff <= tol);
    }

    /* --- Block count self-consistency: ceil(SAMPLES_PER_CYCLE/28) blocks. */
    int expected_blocks = (SAMPLES_PER_CYCLE + ADPCM_BLOCK_SAMPLES - 1) / ADPCM_BLOCK_SAMPLES;
    T_CHECK(expected_blocks == AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT);
    T_CHECK(AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT * ADPCM_BLOCK_SAMPLES == PADDED_SAMPLE_COUNT);

    /* --- Loop-flag self-consistency: loop-start (bit 2, 0x04) on block 0;
     * loop-end (bit 0, 0x01) + loop-repeat (bit 1, 0x02) on the last block;
     * nothing set on any block in between. */
    T_CHECK((loop_flags[0] & 0x04) != 0);
    int last = AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT - 1;
    T_CHECK((loop_flags[last] & 0x01) != 0);
    T_CHECK((loop_flags[last] & 0x02) != 0);
    for (int b = 1; b < last; b++) {
        T_CHECK(loop_flags[b] == 0);
    }

    /* --- Loop-point metadata (audio_waveforms.h's own extern arrays)
     * matches the blocks the loop flags above actually mark. */
    T_CHECK(g_instrument_waveform_loop_start_block[instrument] == 0);
    T_CHECK(g_instrument_waveform_loop_end_block[instrument] == last);
}

static void test_triangle_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_TRIANGLE);
}
static void test_tilted_saw_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_TILTED_SAW);
}
static void test_saw_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_SAW);
}
static void test_square_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_SQUARE);
}
static void test_pulse_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_PULSE);
}
static void test_organ_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_ORGAN);
}
static void test_noise_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_NOISE);
}
static void test_phaser_round_trip_and_self_consistency(void) {
    check_round_trip_and_self_consistency(AUDIO_INSTRUMENT_PHASER);
}

/* AUDIO_WAVEFORM_REFERENCE_HZ (audio_pitch.h, chunk 1) and this test's own
 * SAMPLES_PER_CYCLE constant (mirrored from gen_psx_waveforms.py, which
 * itself mirrors AUDIO_WAVEFORM_REFERENCE_HZ -- see this file's own header
 * comment) must all agree -- this is the "catch future drift" cross-check
 * that duplication convention depends on. */
static void test_reference_hz_and_sample_count_match_audio_pitch_h(void) {
    T_CHECK(AUDIO_WAVEFORM_REFERENCE_HZ == 220);
    int recomputed = (int) lround(44100.0 / (double) AUDIO_WAVEFORM_REFERENCE_HZ);
    T_CHECK(recomputed == SAMPLES_PER_CYCLE);
    T_CHECK(SAMPLES_PER_CYCLE == 200);
}

/* AUDIO_INSTRUMENT_* ids match PICO-8's own canonical instrument-index
 * order (this PRD's own "instrument-palette mismatch" citation: 0
 * Triangle, 1 Tilted Saw, 2 Saw, 3 Square, 4 Pulse, 5 Organ, 6 Noise,
 * 7 Phaser). */
static void test_instrument_ids_match_pico8_canonical_order(void) {
    T_CHECK(AUDIO_INSTRUMENT_TRIANGLE == 0);
    T_CHECK(AUDIO_INSTRUMENT_TILTED_SAW == 1);
    T_CHECK(AUDIO_INSTRUMENT_SAW == 2);
    T_CHECK(AUDIO_INSTRUMENT_SQUARE == 3);
    T_CHECK(AUDIO_INSTRUMENT_PULSE == 4);
    T_CHECK(AUDIO_INSTRUMENT_ORGAN == 5);
    T_CHECK(AUDIO_INSTRUMENT_NOISE == 6);
    T_CHECK(AUDIO_INSTRUMENT_PHASER == 7);
    T_CHECK(AUDIO_INSTRUMENT_COUNT == 8);
}

/* All 8 waveforms share the same encoded byte length (Data Model's own
 * "confirm this is so, or document why not" instruction) -- checked here
 * directly against the real array dimensions, not just assumed from the
 * fixed 2D array type compiling. */
static void test_all_instruments_share_same_adpcm_byte_length(void) {
    T_CHECK(AUDIO_WAVEFORM_ADPCM_BYTES == 128);
    T_CHECK(sizeof(g_instrument_waveform_adpcm[0]) == 128);
    T_CHECK(sizeof(g_instrument_waveform_adpcm) == 8 * 128);
}

int main(void) {
    t_run("reference_hz_and_sample_count_match_audio_pitch_h", test_reference_hz_and_sample_count_match_audio_pitch_h);
    t_run("instrument_ids_match_pico8_canonical_order", test_instrument_ids_match_pico8_canonical_order);
    t_run("all_instruments_share_same_adpcm_byte_length", test_all_instruments_share_same_adpcm_byte_length);
    t_run("triangle_round_trip_and_self_consistency", test_triangle_round_trip_and_self_consistency);
    t_run("tilted_saw_round_trip_and_self_consistency", test_tilted_saw_round_trip_and_self_consistency);
    t_run("saw_round_trip_and_self_consistency", test_saw_round_trip_and_self_consistency);
    t_run("square_round_trip_and_self_consistency", test_square_round_trip_and_self_consistency);
    t_run("pulse_round_trip_and_self_consistency", test_pulse_round_trip_and_self_consistency);
    t_run("organ_round_trip_and_self_consistency", test_organ_round_trip_and_self_consistency);
    t_run("noise_round_trip_and_self_consistency", test_noise_round_trip_and_self_consistency);
    t_run("phaser_round_trip_and_self_consistency", test_phaser_round_trip_and_self_consistency);
    return t_report();
}
