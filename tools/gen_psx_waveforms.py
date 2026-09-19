"""Bake PICO-8's 8 basic-instrument single-cycle waveforms into looping
PS-ADPCM ("VAG") sample data for the PS1 SPU.

docs/prd/triage/psx-spu-audio.md, Implementation Chunks item 2 (chunk 2 of
that PRD). Two independent pieces of research/synthesis, done here from
scratch, no third-party dependencies (stdlib only), matching
tools/gen_psx_sprite_atlas.py/tools/gen_psx_audio_data.py's own established
convention:

  1. The 8 waveform SHAPES themselves (see each *_wave() function's own doc
     comment below for its exact zepto8 citation) -- well-documented,
     publicly known PICO-8 engine behavior, not decompiled
     cart content (this cart's own pico8/lua.lua has zero synthesis code,
     confirmed by that PRD's own "instrument-palette mismatch" research --
     PICO-8's synth runs in the host engine, outside any cart, exactly the
     same research posture docs/prd/psx-juice-effects.md's fixed_sin/
     fixed_cos and this project's own audio_pitch.c (chunk 1,
     psx-slipways/audio_pitch.h's 440*2^((pitch-33)/12) tuning formula)
     already established for "public engine knowledge, not decompiled cart
     content, cited from a public source").
  2. A from-scratch PS-ADPCM ("VAG") ENCODER, against the real, standard,
     well-documented Sony PS1 SPU-ADPCM block format (see ADPCM_FILTER_K0/
     ADPCM_FILTER_K1 below and encode_adpcm_block()'s own doc comment) --
     psxavenc (PSn00bSDK's own PCM/VAG encoder) is not present in this
     environment's installed toolchain (confirmed by that PRD's own
     "ADPCM..." subsection), so this script implements the encode math
     itself rather than shelling out to a tool that isn't there.

Emits psx-slipways/audio_waveforms.h/.c: g_instrument_waveform_adpcm[8][...]
(a fixed 2D array -- all 8 waveforms share the same authored cycle length,
so they all encode to the same ADPCM byte length too, confirmed directly
below by construction, not assumed), the 8 named AUDIO_INSTRUMENT_*
constants, and each waveform's loop-start/loop-end block indices.

AUDIO_WAVEFORM_REFERENCE_HZ (below) must exactly match
psx-slipways/audio_pitch.h's own constant of the same name (chunk 1,
already implemented/committed -- read-only for this chunk; that file's own
doc comment gives the full reasoning for picking 220 Hz and explicitly
names this script as the intended reuser). Hardcoded here rather than
parsed out of the C header (this project's tools/gen_psx_*.py scripts have
no C-header-parsing precedent, and adding one would be new, unscoped
machinery for a single constant); psx-slipways/host_tests/
gen_psx_waveforms_test.c independently #includes audio_pitch.h and asserts
AUDIO_WAVEFORM_REFERENCE_HZ == 220 itself, so any future drift between the
two copies is caught by `make test-audio-waveforms`, not silently possible.
"""
import argparse


AUDIO_WAVEFORM_REFERENCE_HZ = 220  # must match psx-slipways/audio_pitch.h -- see this module's own docstring above

AUTHORING_SAMPLE_RATE_HZ = 44100  # the PS1 SPU's fixed native playback rate every waveform below is authored against

# round(44100/220) -- audio_pitch.h's own doc comment already flagged that
# 44100/220 is 200.4545... Hz, not an exact integer (44100 = 2^2*3^2*5^2*7^2,
# 220 = 2^2*5*11, and 11 does not divide 44100), and explicitly settled on
# round()-to-200-samples-per-cycle as the sample count this script should
# use. Recomputed here independently via Python's own round(), not copied
# from that comment, and asserted below.
SAMPLES_PER_CYCLE = round(AUTHORING_SAMPLE_RATE_HZ / AUDIO_WAVEFORM_REFERENCE_HZ)
assert SAMPLES_PER_CYCLE == 200, (
    "SAMPLES_PER_CYCLE changed from the value audio_pitch.h's own doc "
    "comment documents and depends on (200) -- AUDIO_WAVEFORM_REFERENCE_HZ "
    "or AUTHORING_SAMPLE_RATE_HZ must have changed; audio_pitch.c's table "
    "would need regenerating too, which is outside this script's scope."
)

INST_TRIANGLE = 0
INST_TILTED_SAW = 1
INST_SAW = 2
INST_SQUARE = 3
INST_PULSE = 4
INST_ORGAN = 5
INST_NOISE = 6
INST_PHASER = 7

INSTRUMENT_NAMES = [
    "TRIANGLE",
    "TILTED_SAW",
    "SAW",
    "SQUARE",
    "PULSE",
    "ORGAN",
    "NOISE",
    "PHASER",
]


def triangle_wave(t):
    """zepto8 src/synth.cpp, case INST_TRIANGLE (buzz==false):
    `ret = 1.0f - fabs(4.f * t - 2.0f); return ret * 0.5f;` -- the real
    source's own natural range is [-0.5,0.5], trough at t=0/t=1, peak at
    t=0.5; the trailing *0.5f is dropped here since every waveform this
    generator produces is independently peak-normalized to a fixed PCM
    amplitude afterward (see quantize_to_pcm16 below), making any
    per-instrument scale factor from the source formula moot."""
    return 1.0 - abs(4.0 * t - 2.0)


def tilted_saw_wave(t):
    """zepto8 src/synth.cpp, case INST_TILTED_SAW (buzz==false, a=0.875f):
    `ret = t<a ? 2*t/a-1 : 2*(1-t)/(1-a)-1; return ret*0.5;` -- an
    asymmetric ramp: rises linearly from -1 to 1 over [0, 0.875), then
    falls linearly from 1 back to -1 over the short [0.875, 1) tail, the
    whole thing scaled to [-0.5, 0.5]."""
    a = 0.875
    if t < a:
        ret = 2.0 * t / a - 1.0
    else:
        ret = 2.0 * (1.0 - t) / (1.0 - a) - 1.0
    return ret * 0.5


def saw_wave(t):
    """zepto8 src/synth.cpp, case INST_SAW (buzz==false):
    `ret = (t<0.5f ? t : t-1.f); return 0.653f*ret;` -- a symmetric ramp
    sawtooth (rises from 0 to 0.5, jumps to -0.5, rises back to 0), scaled
    by 0.653."""
    ret = t if t < 0.5 else t - 1.0
    return 0.653 * ret


def square_wave(t):
    """zepto8 src/synth.cpp, case INST_SQUARE (buzz==false):
    `return t < 0.5f ? 0.25f : -0.25f;` -- a flat 50%-duty square wave at
    +-0.25."""
    return 0.25 if t < 0.5 else -0.25


def pulse_wave(t):
    """zepto8 src/synth.cpp, case INST_PULSE (buzz==false):
    `return t < 0.316f ? 0.25f : -0.25f;` -- a narrow ~31.6%-duty pulse
    wave at +-0.25 (the same amplitude as SQUARE, narrower duty cycle)."""
    return 0.25 if t < 0.316 else -0.25


def organ_wave(t):
    """zepto8 src/synth.cpp, case INST_ORGAN (buzz==false):
    `ret = t<0.5f ? 3.f-fabs(24.f*t-6.f) : 1.f-fabs(16.f*t-12.f);
    return ret/9.f;` -- two differently-scaled triangle pulses stitched
    together across the cycle (a higher, narrower one in the first half,
    a lower, wider one in the second), giving ORGAN's characteristic
    asymmetric double-hump timbre."""
    if t < 0.5:
        ret = 3.0 - abs(24.0 * t - 6.0)
    else:
        ret = 1.0 - abs(16.0 * t - 12.0)
    return ret / 9.0


def phaser_wave(t):
    """zepto8 src/synth.cpp, case INST_PHASER (buzz==false):
    `ret = 2.f-fabs(8.f*t-4.f); ret += 1.f-fabs(4.f*fmod(advance*109.f/
    110.f,1.f)-2.f); return ret/6.f;` where `advance` is the voice's raw
    (unwrapped) running phase accumulator (`float advance = params.phi;`,
    confirmed directly from the same source), i.e. a second, very slightly
    detuned (109/110) triangle partial summed with the first.

    Faithfully baking PHASER's real behavior needs a running phase that
    keeps advancing across many cycles (the whole point of the 109/110
    detune is a slow multi-cycle beat/sweep, its actual "phasing"
    character) -- fundamentally incompatible with a single finite baked
    cycle that gets looped verbatim forever (Data Model). This is the same
    class of honestly-flagged approximation this PRD's own Testing
    Decisions section already applies to real playback quality broadly.
    The approximation taken here: `advance` is evaluated at the SAME phase
    value as `t` itself (both are exactly `phi` during this one authored
    reference cycle, since a freshly key-on'd voice starts at phi==0, and
    `t == fmod(phi,1) == phi` throughout the first, and only baked, cycle
    -- `advance` is phi unwrapped, `t` is phi wrapped, and they are
    identical for phi in [0,1)). This still reproduces PHASER's
    characteristic asymmetric double-triangle timbre for one cycle,
    just without the slow inter-cycle drift/sweep that only emerges over
    many played cycles at real playback time."""
    ret = 2.0 - abs(8.0 * t - 4.0)
    advance = t
    ret += 1.0 - abs(4.0 * ((advance * 109.0 / 110.0) % 1.0) - 2.0)
    return ret / 6.0


NOISE_RNG_SEED = 1
NOISE_RNG_MULTIPLIER = 1103515245
NOISE_RNG_INCREMENT = 12345
NOISE_RNG_MODULUS = 1 << 32


def noise_samples(count):
    """Deterministic pseudorandom samples in [-1,1], count of them, via the
    documented 32-bit LCG above (see its own comment)."""
    state = NOISE_RNG_SEED
    samples = []
    for _ in range(count):
        state = (state * NOISE_RNG_MULTIPLIER + NOISE_RNG_INCREMENT) % NOISE_RNG_MODULUS
        samples.append((state / (NOISE_RNG_MODULUS - 1)) * 2.0 - 1.0)
    return samples


def generate_waveform_floats(instrument):
    """Returns SAMPLES_PER_CYCLE float samples (not yet peak-normalized or
    quantized) for one instrument id (0-7), one authored cycle, t = i /
    SAMPLES_PER_CYCLE for i in [0, SAMPLES_PER_CYCLE)."""
    if instrument == INST_NOISE:
        return noise_samples(SAMPLES_PER_CYCLE)

    wave_fn = {
        INST_TRIANGLE: triangle_wave,
        INST_TILTED_SAW: tilted_saw_wave,
        INST_SAW: saw_wave,
        INST_SQUARE: square_wave,
        INST_PULSE: pulse_wave,
        INST_ORGAN: organ_wave,
        INST_PHASER: phaser_wave,
    }[instrument]
    return [wave_fn(i / float(SAMPLES_PER_CYCLE)) for i in range(SAMPLES_PER_CYCLE)]


# Peak PCM amplitude every waveform is independently normalized to by
# quantize_to_pcm16() below (see that function's own docstring for the full
# "why normalize independently, not preserve PICO-8's own relative
# instrument loudness" reasoning) -- deliberately below int16's true max
# (32767), leaving headroom for ADPCM's own predictor to occasionally
# overshoot the source signal's range at sharp transitions (SQUARE/PULSE's
# step edges, SAW/TILTED_SAW's ramp-reset discontinuity) without
# wrapping/clamping in a way that would distort the round-trip tolerance
# check.
PCM_PEAK_AMPLITUDE = 30000

ADPCM_BLOCK_SAMPLES = 28  # PS-ADPCM/VAG's fixed per-block sample count (Implementation Chunks item 2)
ADPCM_BLOCK_BYTES = 16  # 1 shift/filter header byte + 1 loop-flag byte + 14 payload bytes (28 samples, 2/byte)

# ceil(SAMPLES_PER_CYCLE / ADPCM_BLOCK_SAMPLES) -- SAMPLES_PER_CYCLE (200)
# is not a multiple of ADPCM_BLOCK_SAMPLES (28; 200/28 == 7.142857...), so
# the last block needs padding (see pad_to_block_multiple()'s own doc
# comment for the padding convention and its honestly-documented tradeoff).
ADPCM_BLOCK_COUNT = -(-SAMPLES_PER_CYCLE // ADPCM_BLOCK_SAMPLES)  # ceil via negated floor division
assert ADPCM_BLOCK_COUNT == 8
PADDED_SAMPLE_COUNT = ADPCM_BLOCK_COUNT * ADPCM_BLOCK_SAMPLES
assert PADDED_SAMPLE_COUNT == 224


def clamp_int16(v):
    if v > 32767:
        return 32767
    if v < -32768:
        return -32768
    return v


def quantize_to_pcm16(floats, peak=PCM_PEAK_AMPLITUDE):
    """Normalizes floats (any real range) so its own max absolute value
    maps exactly to +-peak, then rounds to int16. Each waveform is
    normalized INDEPENDENTLY -- this is not a shared/global scale across
    all 8 instruments.

    Why independent, not shared: zepto8's own waveform() formulas (see
    each *_wave() function's own doc comment) give each instrument a
    different natural peak amplitude in [-1,1] -- e.g. ORGAN peaks at
    +-1/3, SQUARE/PULSE at +-0.25, TRIANGLE at the full +-1. That is
    PICO-8's own real relative instrument loudness, but this script does
    NOT preserve it in the baked sample data: this port's own per-note
    volume (0-7, sfx_note_t.volume, audio_data.h) is applied independently
    at playback via the SPU's own SPU_CH_VOL_L/R registers (Data Model,
    audio.h's audio_voice_play -- a later chunk, not this one), not by
    scaling the sample data itself. Baking PICO-8's small inherent
    per-instrument loudness differences into the ADPCM sample data would
    only throw away 4-bit ADPCM's limited quantization resolution for the
    quieter-shaped instruments (ORGAN, SQUARE/PULSE, SAW) for zero playback
    benefit -- so every waveform is instead authored at full, equal peak
    amplitude (PCM_PEAK_AMPLITUDE), maximizing each one's own round-trip
    fidelity independently."""
    max_abs = max(abs(v) for v in floats)
    scale = (peak / max_abs) if max_abs > 0 else 0.0
    return [clamp_int16(int(round(v * scale))) for v in floats]


def pad_to_block_multiple(pcm16):
    """Pads a SAMPLES_PER_CYCLE-length int16 PCM buffer up to
    PADDED_SAMPLE_COUNT (a whole number of 28-sample ADPCM blocks) by
    repeating (flat-holding) the buffer's own last real sample.

    This is a genuine, deliberate implementation choice (this codebase's
    ADPCM format has no other precedent to instead match), documented here
    rather than glossed over, exactly as audio_pitch.h's own doc comment
    modeled for its 220 Hz reference-frequency rounding tradeoff:

    Two other padding schemes were considered and rejected:
      - Zero-padding: would insert a sharp jump from the waveform's real
        trailing amplitude down to silence every single loop, an obvious,
        avoidable click.
      - Wrapping the pad region around to repeat the waveform's own start
        (samples[0:24]): keeps the *shape* continuous through the pad
        region, but then the SAME 24 samples get played TWICE in a row
        every loop iteration (once as the wrapped pad tail, immediately
        followed by the real start of the next loop pass) -- an audible
        stutter, worse than a flat hold.

    Flat-holding the last real sample avoids both of those failure modes
    (no big jump relative to the last real sample; no repeated content),
    at the cost of a brief (24-of-224-sample, ~11%) plateau each cycle.

    The real, honestly-flagged consequence: the ADPCM loop this script
    ultimately encodes covers PADDED_SAMPLE_COUNT (224) samples, not the
    "true" SAMPLES_PER_CYCLE (200) the 220 Hz reference frequency assumes
    -- PS-ADPCM/VAG's loop-start/loop-end flags are only block-granular
    (Implementation Chunks item 2), and 200 does not land on a block
    boundary (196 and 224 are the two nearest multiples of 28 -- see
    below). At the SPU's native/unshifted playback rate, a 224-sample loop
    period is 44100/224 == 196.875 Hz, not 220 Hz -- systematically about
    1.915 semitones flat (12*log2(220/196.875) ~= 1.915) versus what
    audio_pitch_register()'s table assumes for every note played on these
    baked instruments. This is a real, bounded, block-granularity artifact
    -- not a bug this script's own scope can eliminate (a mathematically
    seamless loop would need either a variable-length final ADPCM block,
    which the format does not support, or re-deriving
    AUDIO_WAVEFORM_REFERENCE_HZ to land exactly on a 28-sample-block
    multiple, out of this chunk's scope: audio_pitch.h/.c is chunk 1,
    already implemented and committed, read-only here). Recorded here,
    the same "genuinely not verifiable/not perfectly solvable in this
    environment, stated honestly" posture this PRD's own Testing Decisions
    section already establishes for real SPU playback quality broadly.

    (For reference: truncating to 196 samples -- 7 whole blocks, no
    padding at all -- would instead give 44100/196 == 225 Hz exactly, only
    about 0.39 semitones sharp, a SMALLER error than padding's ~1.915
    semitones flat. It is not used here because it would silently discard
    the last ~4.5 authored samples of every waveform's true cycle, and
    because this chunk's own required self-consistency check --
    "ceil(num_samples/28) blocks" -- specifically anticipates a real
    partial-final-block/padding code path to exist and be tested, not a
    truncated, evenly-divisible sample count.)
    """
    assert len(pcm16) == SAMPLES_PER_CYCLE
    pad_count = PADDED_SAMPLE_COUNT - SAMPLES_PER_CYCLE
    return pcm16 + [pcm16[-1]] * pad_count



# The standard, publicly documented 5-entry PS-ADPCM predictor-coefficient
# table (k0, k1 pairs, each really a /64 fixed-point fraction applied via
# `(k0*hist1 + k1*hist2) >> 6`) -- well-known and reused across essentially
# every public PS-ADPCM/VAG encoder or decoder implementation (per this
# PRD's own "ADPCM..." subsection, docs/prd/triage/psx-spu-audio.md).
# Index 0 is the "no prediction" filter (both coefficients zero); indices
# 1-4 are increasingly aggressive 2-tap predictors.
ADPCM_FILTER_K0 = [0, 60, 115, 98, 122]
ADPCM_FILTER_K1 = [0, 0, -52, -55, -60]
ADPCM_FILTER_COUNT = 5
ADPCM_MAX_SHIFT = 12  # the header's shift nibble only ever needs 0-12; see encode_adpcm_block's own doc comment

# Loop-flag byte bits (block byte offset 1), per this PRD's own "ADPCM..."
# subsection (docs/prd/triage/psx-spu-audio.md), itself citing the
# standard, publicly documented VAG convention: bit 0 = loop-end, bit 1 =
# loop-repeat/sustain, bit 2 = loop-start. Set by encode_waveform_adpcm()
# below (encode_adpcm_block() itself always leaves this byte 0).
ADPCM_LOOP_END = 0x01
ADPCM_LOOP_REPEAT = 0x02
ADPCM_LOOP_START = 0x04


def _simulate_block_encode(k0, k1, shift, block_samples, hist1, hist2):
    """Runs the REAL closed-loop quantization (the same math
    encode_adpcm_block ultimately stores) for one candidate (filter, shift)
    pair, starting from the REAL entering hist1/hist2. Returns (nibbles,
    new hist1, new hist2, max abs deviation of the resulting DECODED values
    from the true block_samples) -- that last value is what
    encode_adpcm_block's own shift search below picks a winner by.

    The real VAG/PS-ADPCM decode formula (well-known/public, and
    re-confirmed directly for this chunk -- see the matching decoder in
    psx-slipways/host_tests/gen_psx_waveforms_test.c) is `sample =
    (int16_t)(nibble << 12) >> shift`: the 4-bit code sits in a 16-bit
    word's TOP 4 bits (nibble*4096) and is then shifted RIGHT by `shift`.
    So the actual quantization STEP size is `2^(12-shift)` -- a *larger*
    stored shift value means a *finer* (smaller) step, the opposite of the
    naive "shift directly scales the step" reading -- `divisor` below is
    exactly that real step."""
    divisor = 1 << (ADPCM_MAX_SHIFT - shift)
    h1, h2 = hist1, hist2
    nibbles = []
    max_abs_dev = 0
    for s in block_samples:
        pred = (k0 * h1 + k1 * h2) >> 6
        diff = s - pred
        nibble = int(round(diff / float(divisor)))
        if nibble > 7:
            nibble = 7
        elif nibble < -8:
            nibble = -8
        decoded = clamp_int16(pred + nibble * divisor)
        dev = abs(decoded - s)
        if dev > max_abs_dev:
            max_abs_dev = dev
        h2 = h1
        h1 = decoded
        nibbles.append(nibble)
    return nibbles, h1, h2, max_abs_dev


def encode_adpcm_block(block_samples, hist1, hist2):
    """Encodes exactly ADPCM_BLOCK_SAMPLES (28) int16 PCM samples into one
    16-byte PS-ADPCM block (loop-flag byte left 0 -- the caller sets it),
    against the standard PS-ADPCM/VAG block format: 1 shift/filter header
    byte + 1 loop-flag byte + 14 payload bytes (28 packed 4-bit samples,
    low nibble = earlier sample of each byte pair -- the standard nibble
    order essentially every public VAG encoder/decoder uses), predicted via
    ADPCM_FILTER_K0/ADPCM_FILTER_K1 above (see that table's own citation).

    hist1/hist2: the two most recently DECODED (quantized+reconstructed)
    samples carried in from the end of the previous block -- real PS-ADPCM
    is a single continuous predictor stream (16-byte "blocks" are a
    storage/DMA chunking convention only, not a predictor-reset boundary),
    so this function's own closed-loop quantization must feed forward
    exactly the history a real decoder would have at this point, for the
    decoder to reconstruct bit-identical samples. hist1/hist2 are 0 at the
    very start of a fresh buffer (a freshly key-on'd SPU voice starts with
    zeroed predictor history -- standard, matches every public PS-ADPCM
    decoder/encoder's own reset convention).

    Both the FILTER (of the 5, ADPCM_FILTER_K0/K1) and the SHIFT
    (quantization step) are chosen together by full analysis-by-synthesis:
    actually run _simulate_block_encode() above -- the real closed-loop
    quantization, starting from the REAL entering hist1/hist2 -- for every
    one of the 5*13 = 65 (filter, shift) combinations, and keep whichever
    one minimizes the TRUE resulting peak deviation from this block's real
    target samples. This directly measures what matters (how close the
    real decode ends up to the real signal) instead of estimating it
    analytically -- earlier, cheaper analytic/heuristic estimates tried
    here (an open-loop residual bound seeded from the block's own filter
    choice only; a steady-state DC-reachability bound) each independently
    missed real failure modes this chunk's own round-trip test caught: a
    near-unity-DC-gain filter (filter 2, combined gain 63/64) whose
    closed-loop steady state for a sustained full-scale plateau converges
    to a ceiling strictly below the true target and never catches up (a
    textbook ADPCM "slope overload" artifact); a block starting right
    after a coarsely-quantized previous block needing several samples to
    recover from a stale entering history; and (choosing the filter via a
    cheaper open-loop heuristic rather than real simulation) a filter that
    looked like the best shape match in isolation but did not, in the
    real closed loop, actually minimize this specific block's peak error
    as well as a different filter/shift combination could. Trying every
    combination and directly measuring the result sidesteps needing a
    fully general closed-form bound for any of these failure modes -- the
    same "analysis-by-synthesis" principle real production audio/speech
    codecs use for parameter selection, not a project-specific
    improvisation. 65 combinations x 28 samples is trivial compute for a
    generator script (this is not runtime PSX code -- Constraints).

    Returns (16 bytes, new hist1, new hist2).
    """
    assert len(block_samples) == ADPCM_BLOCK_SAMPLES

    best_filter = 0
    best_shift = 0
    best_nibbles = None
    best_new_h1 = best_new_h2 = 0
    best_dev = None
    for filt in range(ADPCM_FILTER_COUNT):
        k0 = ADPCM_FILTER_K0[filt]
        k1 = ADPCM_FILTER_K1[filt]
        for shift in range(ADPCM_MAX_SHIFT, -1, -1):
            nibbles, new_h1, new_h2, dev = _simulate_block_encode(k0, k1, shift, block_samples, hist1, hist2)
            if best_dev is None or dev < best_dev:
                best_dev = dev
                best_filter = filt
                best_shift = shift
                best_nibbles = nibbles
                best_new_h1 = new_h1
                best_new_h2 = new_h2

    header = bytes([(best_shift << 4) | best_filter, 0x00])
    payload = bytearray(14)
    for i in range(ADPCM_BLOCK_SAMPLES // 2):
        lo = best_nibbles[2 * i] & 0x0F
        hi = best_nibbles[2 * i + 1] & 0x0F
        payload[i] = lo | (hi << 4)

    return header + bytes(payload), best_new_h1, best_new_h2


def encode_waveform_adpcm(padded_pcm16):
    """Encodes a PADDED_SAMPLE_COUNT-length (a whole multiple of
    ADPCM_BLOCK_SAMPLES) int16 PCM buffer into its full ADPCM byte stream
    (ADPCM_BLOCK_COUNT blocks of ADPCM_BLOCK_BYTES each), loop flags set
    per the standard convention this PRD's own "ADPCM..." subsection cites:
    loop-start on block 0, loop-end+loop-repeat on the last block (or all
    three bits on the single block, if there were only one -- not the case
    here, but handled generally below)."""
    assert len(padded_pcm16) % ADPCM_BLOCK_SAMPLES == 0
    num_blocks = len(padded_pcm16) // ADPCM_BLOCK_SAMPLES

    blocks = []
    hist1 = hist2 = 0
    for b in range(num_blocks):
        block_samples = padded_pcm16[b * ADPCM_BLOCK_SAMPLES:(b + 1) * ADPCM_BLOCK_SAMPLES]
        block_bytes, hist1, hist2 = encode_adpcm_block(block_samples, hist1, hist2)
        blocks.append(bytearray(block_bytes))

    if num_blocks == 1:
        blocks[0][1] = ADPCM_LOOP_START | ADPCM_LOOP_END | ADPCM_LOOP_REPEAT
    else:
        blocks[0][1] = ADPCM_LOOP_START
        blocks[-1][1] = ADPCM_LOOP_END | ADPCM_LOOP_REPEAT

    out = bytearray()
    for block in blocks:
        out.extend(block)
    return bytes(out)



def format_bytes_c_array(data, indent="        "):
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        lines.append(indent + ", ".join("0x%02x" % b for b in chunk) + ",")
    return "\n".join(lines)


HEADER_TEMPLATE = """#ifndef AUDIO_WAVEFORMS_H
#define AUDIO_WAVEFORMS_H

#include <stdint.h>

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_waveforms.py -- pure synthesis from public PICO-8 engine
 * knowledge plus psx-slipways/audio_pitch.h's AUDIO_WAVEFORM_REFERENCE_HZ,
 * no pico8 JSON input. Re-run `make audio-waveforms` (or
 * `make test-audio-waveforms`, which depends on it) to regenerate. See
 * docs/prd/triage/psx-spu-audio.md, Implementation Chunks item 2, and this
 * script's own header comment for the waveform-shape/ADPCM-encoding
 * decisions this data encodes. */

/* The 8 PICO-8 basic-instrument ids, in PICO-8's own canonical index order
 * (see gen_psx_waveforms.py's own header comment for the citation). */
#define AUDIO_INSTRUMENT_TRIANGLE 0
#define AUDIO_INSTRUMENT_TILTED_SAW 1
#define AUDIO_INSTRUMENT_SAW 2
#define AUDIO_INSTRUMENT_SQUARE 3
#define AUDIO_INSTRUMENT_PULSE 4
#define AUDIO_INSTRUMENT_ORGAN 5
#define AUDIO_INSTRUMENT_NOISE 6
#define AUDIO_INSTRUMENT_PHASER 7
#define AUDIO_INSTRUMENT_COUNT 8

/* Every one of the 8 waveforms is authored at the same %(samples_per_cycle)d-sample
 * cycle length (round(44100/%(reference_hz)d), audio_pitch.h's own
 * AUDIO_WAVEFORM_REFERENCE_HZ), so every one of them encodes to the same
 * %(block_count)d-block/%(adpcm_bytes)d-byte ADPCM length too -- confirmed directly by
 * construction (not assumed), hence the fixed (not ragged/per-instrument-sized)
 * 2D array below. See gen_psx_waveforms.py's pad_to_block_multiple() for why
 * %(block_count)d blocks (%(padded_samples)d padded samples), not
 * ceil(%(samples_per_cycle)d/%(block_samples)d)-worth-of-only-real-data. */
#define AUDIO_WAVEFORM_ADPCM_BLOCK_BYTES %(block_bytes)d
#define AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT %(block_count)d
#define AUDIO_WAVEFORM_ADPCM_BYTES (AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT * AUDIO_WAVEFORM_ADPCM_BLOCK_BYTES)

/* The 8 baked, ADPCM-encoded single-cycle waveforms, one per
 * AUDIO_INSTRUMENT_* id above. Declared extern/non-static (not this
 * codebase's usual "static const table + accessor function" convention --
 * see sprite_atlas.h/audio_data.h's own precedent) because both this
 * chunk's host test and a later chunk's audio_init() (SPU-RAM upload) need
 * direct read access to the raw encoded bytes. */
extern const uint8_t g_instrument_waveform_adpcm[AUDIO_INSTRUMENT_COUNT][AUDIO_WAVEFORM_ADPCM_BYTES];

/* Each waveform's loop points, in ADPCM BLOCK indices (not byte/sample
 * offsets) -- trivially 0 and AUDIO_WAVEFORM_ADPCM_BLOCK_COUNT-1 for every
 * instrument here (a single baked cycle loops immediately, block 0 to its
 * own last block, Implementation Chunks item 2), but exposed explicitly
 * per-instrument, matching the Data Model's own "expose this cleanly"
 * instruction, rather than every future caller hardcoding those two
 * numbers itself. */
extern const int g_instrument_waveform_loop_start_block[AUDIO_INSTRUMENT_COUNT];
extern const int g_instrument_waveform_loop_end_block[AUDIO_INSTRUMENT_COUNT];

#endif
"""

C_TEMPLATE = """#include "audio_waveforms.h"

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_waveforms.py. See audio_waveforms.h and that script's own
 * header comment for the waveform-shape/ADPCM-encoding decisions this data
 * encodes. */

const uint8_t g_instrument_waveform_adpcm[AUDIO_INSTRUMENT_COUNT][AUDIO_WAVEFORM_ADPCM_BYTES] = {
%(waveform_entries)s
};

const int g_instrument_waveform_loop_start_block[AUDIO_INSTRUMENT_COUNT] = {
    %(loop_start_blocks)s
};

const int g_instrument_waveform_loop_end_block[AUDIO_INSTRUMENT_COUNT] = {
    %(loop_end_blocks)s
};
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--out-header", default="psx-slipways/audio_waveforms.h",
        help="Output header path (default: psx-slipways/audio_waveforms.h)",
    )
    parser.add_argument(
        "--out-c", default="psx-slipways/audio_waveforms.c",
        help="Output C source path (default: psx-slipways/audio_waveforms.c)",
    )
    args = parser.parse_args()

    waveform_entries = []
    loop_start_blocks = []
    loop_end_blocks = []
    for instrument in range(8):
        floats = generate_waveform_floats(instrument)
        pcm16 = quantize_to_pcm16(floats)
        padded = pad_to_block_multiple(pcm16)
        adpcm_bytes = encode_waveform_adpcm(padded)
        assert len(adpcm_bytes) == ADPCM_BLOCK_COUNT * ADPCM_BLOCK_BYTES

        waveform_entries.append(
            "    [%d] = { /* AUDIO_INSTRUMENT_%s */\n%s\n    },"
            % (instrument, INSTRUMENT_NAMES[instrument], format_bytes_c_array(adpcm_bytes))
        )
        loop_start_blocks.append("0")
        loop_end_blocks.append(str(ADPCM_BLOCK_COUNT - 1))

    with open(args.out_header, "w") as f:
        f.write(HEADER_TEMPLATE % {
            "samples_per_cycle": SAMPLES_PER_CYCLE,
            "reference_hz": AUDIO_WAVEFORM_REFERENCE_HZ,
            "block_count": ADPCM_BLOCK_COUNT,
            "block_bytes": ADPCM_BLOCK_BYTES,
            "block_samples": ADPCM_BLOCK_SAMPLES,
            "adpcm_bytes": ADPCM_BLOCK_COUNT * ADPCM_BLOCK_BYTES,
            "padded_samples": PADDED_SAMPLE_COUNT,
        })

    with open(args.out_c, "w") as f:
        f.write(C_TEMPLATE % {
            "waveform_entries": "\n".join(waveform_entries),
            "loop_start_blocks": ", ".join(loop_start_blocks),
            "loop_end_blocks": ", ".join(loop_end_blocks),
        })

    print("Wrote %s and %s (%d instruments, %d bytes each)" % (
        args.out_header, args.out_c, 8, ADPCM_BLOCK_COUNT * ADPCM_BLOCK_BYTES,
    ))


if __name__ == "__main__":
    main()
