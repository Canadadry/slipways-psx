"""Extract pico8/sfx.json and pico8/music.json into portable, PSX-independent
procedural-synth data tables.

Reads pico8/sfx.json/pico8/music.json directly -- both are already
fully-decoded, structured JSON (small integers/enum-strings, no PCM sample
data anywhere; see docs/prd/triage/psx-sprites-audio-extraction.md,
Implementation Decisions). Emits psx-slipways/audio_data.h/.c:

  - sfx_effect_t: an enum of the note-effect values that actually appear in
    pico8/sfx.json's note data (see EFFECT_NAMES below).
  - sfx_note_t/sfx_def_t and g_sfx[64]: all 64 SFX slots' note sequences.
  - music_channel_t/music_pattern_t and g_music[64]: all 64 music patterns'
    per-channel sfx references.

Pure data, no synthesis, no SPU code, no playback function of any kind --
that's docs/plan.md Phase 5 step 19's job, not this one's. Mirrors
tools/gen_psx_sprite_atlas.py's convention of plain, readable Python with no
third-party dependencies (json module only).
"""
import argparse
import json

SFX_COUNT = 64
NOTES_PER_SFX = 32
MUSIC_PATTERN_COUNT = 64
CHANNELS_PER_PATTERN = 4

# The distinct `effect` string values that actually appear in
# pico8/sfx.json's note data, exhaustively confirmed by direct inspection
# (grep -o '"effect":"[a-zA-Z_ ]*"' pico8/sfx.json | sort -u), not guessed
# from PICO-8's general effect vocabulary -- per docs/prd/triage/
# psx-sprites-audio-extraction.md's own instruction to enumerate only
# values genuinely present. Listed here in PICO-8's canonical effect-index
# order (0=none,1=slide,2=vibrato,3=drop,4=fade_in,5=fade_out,6=arp_fast)
# for readability; PICO-8's 8th effect, arp_slow (would be index 7), never
# appears anywhere in this cart's sfx.json (also exhaustively confirmed by
# the same grep), so it is deliberately not enumerated below.
EFFECT_NAMES = [
    "none",
    "slide",
    "vibrato",
    "drop",
    "fade_in",
    "fade_out",
    "arp_fast",
]


def effect_enum_name(effect):
    """Maps a pico8/sfx.json effect string (e.g. "fade_in") to its
    generated sfx_effect_t enumerator name (e.g. SFX_EFFECT_FADE_IN)."""
    return "SFX_EFFECT_" + effect.upper()


def load_sfx_json(path):
    """Reads and validates pico8/sfx.json: exactly 64 sfx slots, each with
    exactly 32 notes (confirmed directly: pico8/sfx.json has exactly 2048
    "pitch" fields total, 2048/64=32 for every slot), each note's
    pitch/instrument/volume/effect within PICO-8's real, confirmed ranges
    (pitch 0-63, instrument 0-15, volume 0-7, effect one of EFFECT_NAMES)."""
    with open(path) as f:
        data = json.load(f)

    if len(data) != SFX_COUNT:
        raise ValueError(
            "%s: expected %d sfx entries, got %d" % (path, SFX_COUNT, len(data))
        )

    for i, sfx in enumerate(data):
        notes = sfx["notes"]
        if len(notes) != NOTES_PER_SFX:
            raise ValueError(
                "%s: sfx %d: expected %d notes, got %d"
                % (path, i, NOTES_PER_SFX, len(notes))
            )
        for j, note in enumerate(notes):
            pitch = note["pitch"]
            instrument = note["instrument"]
            volume = note["volume"]
            effect = note["effect"]
            if not (0 <= pitch <= 63):
                raise ValueError(
                    "%s: sfx %d note %d: pitch %r out of range [0,63]"
                    % (path, i, j, pitch)
                )
            if not (0 <= instrument <= 15):
                raise ValueError(
                    "%s: sfx %d note %d: instrument %r out of range [0,15]"
                    % (path, i, j, instrument)
                )
            if not (0 <= volume <= 7):
                raise ValueError(
                    "%s: sfx %d note %d: volume %r out of range [0,7]"
                    % (path, i, j, volume)
                )
            if effect not in EFFECT_NAMES:
                raise ValueError(
                    "%s: sfx %d note %d: unrecognized effect %r"
                    % (path, i, j, effect)
                )

    return data


def load_music_json(path):
    """Reads and validates pico8/music.json: exactly 64 patterns, each a
    4-element array of {sfxId, mute, flag} channel entries (confirmed by
    direct read: pico8/music.json is `[[{...},{...},{...},{...}], ...]`,
    64 top-level entries of exactly 4 channels each)."""
    with open(path) as f:
        data = json.load(f)

    if len(data) != MUSIC_PATTERN_COUNT:
        raise ValueError(
            "%s: expected %d music patterns, got %d"
            % (path, MUSIC_PATTERN_COUNT, len(data))
        )

    for i, pattern in enumerate(data):
        if len(pattern) != CHANNELS_PER_PATTERN:
            raise ValueError(
                "%s: pattern %d: expected %d channels, got %d"
                % (path, i, CHANNELS_PER_PATTERN, len(pattern))
            )

    return data


def format_sfx_c_array(sfx_list, indent="    "):
    entries = []
    for i, sfx in enumerate(sfx_list):
        notes = sfx["notes"]
        note_strs = [
            "{ %d, %d, %d, %s }"
            % (
                note["pitch"],
                note["instrument"],
                note["volume"],
                effect_enum_name(note["effect"]),
            )
            for note in notes
        ]
        notes_block = (",\n" + indent * 3).join(note_strs)
        entries.append(
            "%s[%d] = {\n"
            "%s.notes = {\n%s%s\n%s},\n"
            "%s.note_count = %d,\n"
            "%s.speed = %d,\n"
            "%s.loop_start = %d,\n"
            "%s.loop_end = %d,\n"
            "%s},"
            % (
                indent, i,
                indent * 2, indent * 3, notes_block, indent * 2,
                indent * 2, len(notes),
                indent * 2, sfx["speed"],
                indent * 2, sfx["loopStart"],
                indent * 2, sfx["loopEnd"],
                indent,
            )
        )
    return "\n".join(entries)


def format_music_c_array(music_list, indent="    "):
    entries = []
    for i, pattern in enumerate(music_list):
        chan_strs = [
            "{ %d, %s, %s }"
            % (
                chan["sfxId"],
                "true" if chan["mute"] else "false",
                "true" if chan["flag"] else "false",
            )
            for chan in pattern
        ]
        entries.append(
            "%s[%d] = { .channels = { %s } },"
            % (indent, i, ", ".join(chan_strs))
        )
    return "\n".join(entries)


HEADER_TEMPLATE = """#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include <stdbool.h>
#include <stdint.h>

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_audio_data.py from pico8/sfx.json/pico8/music.json. Re-run
 * `make audio-data` (or `make test-audio-data`/`make psx-slipways`, which
 * both depend on it) to regenerate. See docs/prd/triage/
 * psx-sprites-audio-extraction.md, "Extraction pipeline and output
 * format" / Implementation Chunks chunk 7.
 *
 * Pure data, no PSX/psxgpu.h dependency anywhere in this pair (unlike
 * sprite_atlas.c, this module needs no __mips__-gated half -- it's pure
 * data end to end, per the PRD's Testing Decisions section) and no SPU/
 * synthesis/playback code of any kind (docs/plan.md Phase 5 step 19's
 * job, not this one's). */

/* The distinct note-effect values that actually appear in pico8/sfx.json's
 * note data (see gen_psx_audio_data.py's EFFECT_NAMES for the exhaustive-
 * grep citation), in PICO-8's canonical effect-index order. PICO-8's 8th
 * effect, arp_slow, never appears anywhere in this cart's sfx.json and is
 * deliberately not enumerated here -- this enum lists only values
 * genuinely present in the real data, not PICO-8's generic vocabulary. */
typedef enum {
    SFX_EFFECT_NONE,
    SFX_EFFECT_SLIDE,
    SFX_EFFECT_VIBRATO,
    SFX_EFFECT_DROP,
    SFX_EFFECT_FADE_IN,
    SFX_EFFECT_FADE_OUT,
    SFX_EFFECT_ARP_FAST,
} sfx_effect_t;

/* One PICO-8 SFX note: pitch (0-63), instrument (0-15), volume (0-7), and
 * the note's effect (see sfx_effect_t above). Matches pico8/sfx.json's own
 * per-note shape exactly, field for field. */
typedef struct {
    uint8_t pitch;
    uint8_t instrument;
    uint8_t volume;
    sfx_effect_t effect;
} sfx_note_t;

/* One PICO-8 SFX slot: pico8/sfx.json's own "notes"/"speed"/"loopStart"/
 * "loopEnd" fields (its "editorMode" field is PICO-8-editor-only UI state
 * with no gameplay/playback meaning and is not ported). Every one of the
 * 64 real slots has exactly 32 notes (confirmed directly against
 * pico8/sfx.json), but note_count is still carried explicitly rather than
 * hardcoded, matching the struct shape a future SPU-playback consumer
 * (step 19) will actually want. */
typedef struct {
    sfx_note_t notes[32];
    int note_count;
    int speed;
    int loop_start;
    int loop_end;
} sfx_def_t;

/* All 64 SFX slots, in pico8/sfx.json's own id order -- the same numeric
 * id PICO-8's own sfx(no,...) call and this codebase's own already-traced
 * rn() call sites (docs/prd/psx-structures-and-transforms.md's Further
 * Notes) index into.
 *
 * Declared here (extern, non-static) rather than this codebase's usual
 * "static const table + accessor function" convention (see
 * rules_tech.c/rules_planets.c) for the same reason sprite_atlas.h's
 * g_sprite_atlas_4bpp/g_pico8_clut already established: host_tests/
 * audio_data_test.c needs direct read access to specific slots/notes to
 * assert against pico8/sfx.json's own values, not just an opaque accessor
 * return. */
extern const sfx_def_t g_sfx[64];

/* One channel slot within a PICO-8 music pattern: which SFX slot it plays
 * (sfx_id), whether it's silenced (mute), and PICO-8's own pattern-
 * transition marker (flag, its loop/stop-at-end semantics). Matches
 * pico8/music.json's own per-channel shape exactly, field for field. */
typedef struct {
    int sfx_id;
    bool mute;
    bool flag;
} music_channel_t;

/* One PICO-8 music pattern: up to 4 SFX slots played simultaneously, one
 * per PICO-8 audio channel -- pico8/music.json's own per-pattern shape. */
typedef struct {
    music_channel_t channels[4];
} music_pattern_t;

/* All 64 music patterns, in pico8/music.json's own id order. Same extern,
 * non-static reasoning as g_sfx above -- host_tests/audio_data_test.c
 * reads specific patterns' channel data directly. */
extern const music_pattern_t g_music[64];

#endif
"""

C_TEMPLATE = """#include "audio_data.h"

/* GENERATED FILE -- do not edit by hand. Produced by
 * tools/gen_psx_audio_data.py from pico8/sfx.json/pico8/music.json. See
 * audio_data.h for the struct-shape/enum decisions this data encodes. */

const sfx_def_t g_sfx[64] = {
%(sfx_entries)s
};

const music_pattern_t g_music[64] = {
%(music_entries)s
};
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "sfx_json", nargs="?", default="pico8/sfx.json",
        help="Path to pico8/sfx.json (default: pico8/sfx.json)",
    )
    parser.add_argument(
        "music_json", nargs="?", default="pico8/music.json",
        help="Path to pico8/music.json (default: pico8/music.json)",
    )
    parser.add_argument(
        "--out-header", default="psx-slipways/audio_data.h",
        help="Output header path (default: psx-slipways/audio_data.h)",
    )
    parser.add_argument(
        "--out-c", default="psx-slipways/audio_data.c",
        help="Output C source path (default: psx-slipways/audio_data.c)",
    )
    args = parser.parse_args()

    sfx_list = load_sfx_json(args.sfx_json)
    music_list = load_music_json(args.music_json)

    with open(args.out_header, "w") as f:
        f.write(HEADER_TEMPLATE)

    with open(args.out_c, "w") as f:
        f.write(C_TEMPLATE % {
            "sfx_entries": format_sfx_c_array(sfx_list),
            "music_entries": format_music_c_array(music_list),
        })

    print("Wrote %s (%d sfx) and %s (%d music patterns)" % (
        args.out_header, len(sfx_list), args.out_c, len(music_list),
    ))


if __name__ == "__main__":
    main()
