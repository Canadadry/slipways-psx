#include "t.h"
#include "../audio_data.h"

#include <stdint.h>

/* Independently-derived expected values for chunk 7
 * (docs/prd/triage/psx-sprites-audio-extraction.md, Implementation Chunks
 * item 7), hand-typed here from pico8/sfx.json/pico8/music.json re-read
 * directly at test-authorship time -- not copied from the generated
 * psx-slipways/audio_data.c or from this PRD's own prose -- so this test
 * independently checks gen_psx_audio_data.py's output rather than
 * tautologically re-testing the tool against itself.
 *
 * sfx[0]'s full 32-note sequence (byte offset 1 in pico8/sfx.json, the
 * file's first sfx entry) -- covers the none/vibrato/drop effects. */
static const sfx_note_t g_expected_sfx0_notes[32] = {
    { 31, 15, 3, SFX_EFFECT_VIBRATO },
    { 31, 7, 6, SFX_EFFECT_VIBRATO },
    { 24, 15, 3, SFX_EFFECT_VIBRATO },
    { 24, 15, 0, SFX_EFFECT_NONE },
    { 26, 15, 3, SFX_EFFECT_VIBRATO },
    { 26, 7, 6, SFX_EFFECT_VIBRATO },
    { 31, 15, 3, SFX_EFFECT_VIBRATO },
    { 31, 7, 6, SFX_EFFECT_VIBRATO },
    { 24, 15, 4, SFX_EFFECT_NONE },
    { 24, 7, 2, SFX_EFFECT_VIBRATO },
    { 26, 15, 3, SFX_EFFECT_VIBRATO },
    { 26, 7, 6, SFX_EFFECT_VIBRATO },
    { 31, 15, 3, SFX_EFFECT_VIBRATO },
    { 31, 7, 6, SFX_EFFECT_VIBRATO },
    { 24, 15, 3, SFX_EFFECT_VIBRATO },
    { 29, 8, 0, SFX_EFFECT_DROP },
    { 31, 15, 3, SFX_EFFECT_VIBRATO },
    { 31, 7, 6, SFX_EFFECT_VIBRATO },
    { 26, 15, 3, SFX_EFFECT_VIBRATO },
    { 26, 7, 6, SFX_EFFECT_VIBRATO },
    { 24, 15, 3, SFX_EFFECT_VIBRATO },
    { 24, 7, 6, SFX_EFFECT_VIBRATO },
    { 26, 15, 3, SFX_EFFECT_VIBRATO },
    { 26, 15, 0, SFX_EFFECT_NONE },
    { 34, 15, 4, SFX_EFFECT_NONE },
    { 31, 15, 3, SFX_EFFECT_VIBRATO },
    { 24, 15, 3, SFX_EFFECT_VIBRATO },
    { 24, 7, 6, SFX_EFFECT_VIBRATO },
    { 26, 15, 3, SFX_EFFECT_VIBRATO },
    { 26, 7, 6, SFX_EFFECT_VIBRATO },
    { 31, 7, 0, SFX_EFFECT_VIBRATO },
    { 31, 7, 4, SFX_EFFECT_VIBRATO },
};

/* sfx[61]'s full 32-note sequence (byte offset 116650 in pico8/sfx.json,
 * the 62nd of 64 top-level sfx entries) -- covers the fade_in/arp_fast/
 * vibrato effects. */
static const sfx_note_t g_expected_sfx61_notes[32] = {
    { 15, 9, 0, SFX_EFFECT_FADE_IN },
    { 15, 1, 5, SFX_EFFECT_ARP_FAST },
    { 3, 8, 4, SFX_EFFECT_VIBRATO },
    { 19, 9, 0, SFX_EFFECT_FADE_IN },
    { 19, 1, 5, SFX_EFFECT_ARP_FAST },
    { 7, 8, 4, SFX_EFFECT_VIBRATO },
    { 15, 9, 0, SFX_EFFECT_FADE_IN },
    { 15, 1, 5, SFX_EFFECT_ARP_FAST },
    { 3, 8, 4, SFX_EFFECT_VIBRATO },
    { 19, 9, 0, SFX_EFFECT_FADE_IN },
    { 19, 1, 5, SFX_EFFECT_ARP_FAST },
    { 7, 8, 4, SFX_EFFECT_VIBRATO },
    { 15, 9, 0, SFX_EFFECT_FADE_IN },
    { 15, 9, 0, SFX_EFFECT_FADE_IN },
    { 15, 1, 5, SFX_EFFECT_ARP_FAST },
    { 3, 8, 4, SFX_EFFECT_VIBRATO },
    { 16, 9, 0, SFX_EFFECT_FADE_IN },
    { 16, 1, 5, SFX_EFFECT_ARP_FAST },
    { 4, 8, 4, SFX_EFFECT_VIBRATO },
    { 20, 9, 0, SFX_EFFECT_FADE_IN },
    { 20, 1, 5, SFX_EFFECT_ARP_FAST },
    { 8, 8, 4, SFX_EFFECT_VIBRATO },
    { 16, 9, 0, SFX_EFFECT_FADE_IN },
    { 16, 1, 5, SFX_EFFECT_ARP_FAST },
    { 4, 8, 4, SFX_EFFECT_VIBRATO },
    { 20, 9, 0, SFX_EFFECT_FADE_IN },
    { 20, 1, 5, SFX_EFFECT_ARP_FAST },
    { 8, 8, 4, SFX_EFFECT_VIBRATO },
    { 16, 9, 0, SFX_EFFECT_FADE_IN },
    { 16, 9, 0, SFX_EFFECT_FADE_IN },
    { 16, 1, 5, SFX_EFFECT_ARP_FAST },
    { 4, 8, 4, SFX_EFFECT_VIBRATO },
};

/* sfx[63]'s full 32-note sequence (byte offset 120385 in pico8/sfx.json,
 * the file's last sfx entry) -- covers the none/vibrato effects, and a
 * speed/loopStart/loopEnd combination (1/0/0) distinct from sfx[0]'s
 * (16/0/30), proving the per-slot fields aren't accidentally shared. */
static const sfx_note_t g_expected_sfx63_notes[32] = {
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 12, 7, 0, SFX_EFFECT_NONE },
    { 12, 6, 0, SFX_EFFECT_NONE },
    { 19, 7, 0, SFX_EFFECT_NONE },
    { 24, 6, 0, SFX_EFFECT_NONE },
    { 24, 7, 0, SFX_EFFECT_NONE },
    { 48, 6, 0, SFX_EFFECT_NONE },
    { 31, 7, 0, SFX_EFFECT_NONE },
    { 60, 6, 0, SFX_EFFECT_NONE },
    { 43, 7, 4, SFX_EFFECT_VIBRATO },
    { 24, 6, 0, SFX_EFFECT_NONE },
    { 24, 7, 0, SFX_EFFECT_NONE },
    { 12, 6, 0, SFX_EFFECT_NONE },
    { 19, 7, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 12, 7, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
    { 0, 6, 0, SFX_EFFECT_NONE },
};

static void test_sfx0_full_notes_and_fields_match_sfx_json(void) {
    T_CHECK(g_sfx[0].note_count == 32);
    for (int i = 0; i < 32; i++) {
        T_CHECK(g_sfx[0].notes[i].pitch == g_expected_sfx0_notes[i].pitch);
        T_CHECK(g_sfx[0].notes[i].instrument == g_expected_sfx0_notes[i].instrument);
        T_CHECK(g_sfx[0].notes[i].volume == g_expected_sfx0_notes[i].volume);
        T_CHECK(g_sfx[0].notes[i].effect == g_expected_sfx0_notes[i].effect);
    }
    T_CHECK(g_sfx[0].speed == 16);
    T_CHECK(g_sfx[0].loop_start == 0);
    T_CHECK(g_sfx[0].loop_end == 30);
}

static void test_sfx61_full_notes_and_fields_match_sfx_json(void) {
    T_CHECK(g_sfx[61].note_count == 32);
    for (int i = 0; i < 32; i++) {
        T_CHECK(g_sfx[61].notes[i].pitch == g_expected_sfx61_notes[i].pitch);
        T_CHECK(g_sfx[61].notes[i].instrument == g_expected_sfx61_notes[i].instrument);
        T_CHECK(g_sfx[61].notes[i].volume == g_expected_sfx61_notes[i].volume);
        T_CHECK(g_sfx[61].notes[i].effect == g_expected_sfx61_notes[i].effect);
    }
    T_CHECK(g_sfx[61].speed == 64);
    T_CHECK(g_sfx[61].loop_start == 0);
    T_CHECK(g_sfx[61].loop_end == 32);
}

static void test_sfx63_full_notes_and_fields_match_sfx_json(void) {
    T_CHECK(g_sfx[63].note_count == 32);
    for (int i = 0; i < 32; i++) {
        T_CHECK(g_sfx[63].notes[i].pitch == g_expected_sfx63_notes[i].pitch);
        T_CHECK(g_sfx[63].notes[i].instrument == g_expected_sfx63_notes[i].instrument);
        T_CHECK(g_sfx[63].notes[i].volume == g_expected_sfx63_notes[i].volume);
        T_CHECK(g_sfx[63].notes[i].effect == g_expected_sfx63_notes[i].effect);
    }
    T_CHECK(g_sfx[63].speed == 1);
    T_CHECK(g_sfx[63].loop_start == 0);
    T_CHECK(g_sfx[63].loop_end == 0);
}

/* sfx[7]'s note 2 (byte offset 13148+~34 in pico8/sfx.json) is one of only
 * 54 "slide"-effect notes anywhere in the file (confirmed by exhaustive
 * grep) -- a targeted spot check for that effect value, rather than
 * decoding sfx[7]'s whole 32-note sequence, since a single distinguishing
 * note is enough to prove SFX_EFFECT_SLIDE decodes correctly. */
static void test_sfx7_note2_is_slide(void) {
    T_CHECK(g_sfx[7].notes[2].pitch == 31);
    T_CHECK(g_sfx[7].notes[2].instrument == 15);
    T_CHECK(g_sfx[7].notes[2].volume == 0);
    T_CHECK(g_sfx[7].notes[2].effect == SFX_EFFECT_SLIDE);
}

/* sfx[9]'s note 9 (byte offset 17580 in pico8/sfx.json) is the file's
 * first "fade_out"-effect note (one of only 23 total, confirmed by
 * exhaustive grep) -- a targeted spot check for that effect value, same
 * reasoning as sfx[7]'s slide check above. Also checks sfx[9]'s own
 * speed/loopStart/loopEnd (129/0/32), a third distinct combination from
 * sfx[0]'s and sfx[63]'s. */
static void test_sfx9_note9_is_fade_out_and_fields_match(void) {
    T_CHECK(g_sfx[9].notes[9].pitch == 20);
    T_CHECK(g_sfx[9].notes[9].instrument == 4);
    T_CHECK(g_sfx[9].notes[9].volume == 1);
    T_CHECK(g_sfx[9].notes[9].effect == SFX_EFFECT_FADE_OUT);
    T_CHECK(g_sfx[9].speed == 129);
    T_CHECK(g_sfx[9].loop_start == 0);
    T_CHECK(g_sfx[9].loop_end == 32);
}

/* pico8/music.json's pattern 0 (the file's first top-level entry):
 * [{"sfxId":13,"mute":false,"flag":false},{"sfxId":14,...},{"sfxId":15,
 * ...},{"sfxId":4,"mute":true,"flag":false}] -- covers a mixed mute state
 * (channel 3 muted, the other three not). */
static void test_music_pattern0_matches_music_json(void) {
    T_CHECK(g_music[0].channels[0].sfx_id == 13);
    T_CHECK(g_music[0].channels[0].mute == false);
    T_CHECK(g_music[0].channels[0].flag == false);

    T_CHECK(g_music[0].channels[1].sfx_id == 14);
    T_CHECK(g_music[0].channels[1].mute == false);
    T_CHECK(g_music[0].channels[1].flag == false);

    T_CHECK(g_music[0].channels[2].sfx_id == 15);
    T_CHECK(g_music[0].channels[2].mute == false);
    T_CHECK(g_music[0].channels[2].flag == false);

    T_CHECK(g_music[0].channels[3].sfx_id == 4);
    T_CHECK(g_music[0].channels[3].mute == true);
    T_CHECK(g_music[0].channels[3].flag == false);
}

/* pico8/music.json's pattern 11 (the file's 12th top-level entry, chosen
 * for a different sfxId set/no-mute combination than pattern 0 above):
 * [{"sfxId":33,"mute":false,"flag":false},{"sfxId":17,...},{"sfxId":11,
 * ...},{"sfxId":34,"mute":false,"flag":false}]. */
static void test_music_pattern11_matches_music_json(void) {
    T_CHECK(g_music[11].channels[0].sfx_id == 33);
    T_CHECK(g_music[11].channels[0].mute == false);
    T_CHECK(g_music[11].channels[0].flag == false);

    T_CHECK(g_music[11].channels[1].sfx_id == 17);
    T_CHECK(g_music[11].channels[1].mute == false);
    T_CHECK(g_music[11].channels[1].flag == false);

    T_CHECK(g_music[11].channels[2].sfx_id == 11);
    T_CHECK(g_music[11].channels[2].mute == false);
    T_CHECK(g_music[11].channels[2].flag == false);

    T_CHECK(g_music[11].channels[3].sfx_id == 34);
    T_CHECK(g_music[11].channels[3].mute == false);
    T_CHECK(g_music[11].channels[3].flag == false);
}

int main(void) {
    t_run("sfx0_full_notes_and_fields_match_sfx_json", test_sfx0_full_notes_and_fields_match_sfx_json);
    t_run("sfx61_full_notes_and_fields_match_sfx_json", test_sfx61_full_notes_and_fields_match_sfx_json);
    t_run("sfx63_full_notes_and_fields_match_sfx_json", test_sfx63_full_notes_and_fields_match_sfx_json);
    t_run("sfx7_note2_is_slide", test_sfx7_note2_is_slide);
    t_run("sfx9_note9_is_fade_out_and_fields_match", test_sfx9_note9_is_fade_out_and_fields_match);
    t_run("music_pattern0_matches_music_json", test_music_pattern0_matches_music_json);
    t_run("music_pattern11_matches_music_json", test_music_pattern11_matches_music_json);
    return t_report();
}
