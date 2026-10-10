#include "devtools/pad_caption.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void assert_parses_to(const char *script_text, const char *expected) {
    char out[PAD_CAPTION_TEXT_SIZE];
    char error[128] = "";
    int ok = pad_caption_parse(script_text, out, sizeof out, error, sizeof error);
    if (!ok || strcmp(out, expected) != 0) {
        fprintf(stderr, "parse '%s': ok %d, error '%s'\n", script_text, ok, error);
    }
    assert(ok);
    assert(strcmp(out, expected) == 0);
}

static void assert_refused(const char *script_text, const char *expected_error) {
    char out[PAD_CAPTION_TEXT_SIZE];
    char error[128] = "";
    int ok = pad_caption_parse(script_text, out, sizeof out, error, sizeof error);
    if (ok || strcmp(error, expected_error) != 0) {
        fprintf(stderr, "parse '%s': ok %d, error '%s', expected '%s'\n", script_text, ok, error, expected_error);
    }
    assert(!ok);
    assert(strcmp(error, expected_error) == 0);
}

static void test_a_slash_breaks_the_line_and_spaces_around_it_are_dropped(void) {
    assert_parses_to("a/b", "a\nb");
    assert_parses_to("a / b", "a\nb");
    assert_parses_to("  build a trading empire / across the stars ", "build a trading empire\nacross the stars");
}

static void test_a_line_holds_32_characters_not_33(void) {
    const char *line_32 = "abcdefghijklmnopqrstuwxyz0123456";
    assert(strlen(line_32) == 32);
    assert_parses_to(line_32, line_32);
    assert_parses_to("a/abcdefghijklmnopqrstuwxyz0123456", "a\nabcdefghijklmnopqrstuwxyz0123456");

    assert_refused("abcdefghijklmnopqrstuwxyz01234567", "caption: line 1 has 33 characters, at most 32");
    assert_refused("a / abcdefghijklmnopqrstuwxyz01234567 ", "caption: line 2 has 33 characters, at most 32");
}

static void test_two_lines_are_accepted_not_three(void) {
    assert_parses_to("colonize planets:/each one needs and makes goods",
                     "colonize planets:\neach one needs and makes goods");
    assert_refused("a/b/c", "caption: 3 lines, at most 2");
    assert_refused("a/b/c/d", "caption: 4 lines, at most 2");
}

static void test_the_letter_v_is_refused_in_both_cases(void) {
    assert_refused("discover new worlds", "caption: the letter v draws as u");
    assert_refused("a/VAST", "caption: the letter v draws as u");
}

static void test_a_character_without_a_glyph_is_refused(void) {
    assert_refused("trade & grow", "caption: no font glyph for '&'");
    assert_refused("say \"hi\"", "caption: no font glyph for '\"'");
    assert_refused("a\tb", "caption: no font glyph for byte 0x09");
    assert_refused("caf\xc3\xa9", "caption: no font glyph for byte 0xc3");
}

static void test_the_letter_v_is_checked_before_glyphs(void) {
    assert_refused("& v", "caption: the letter v draws as u");
}

static void test_an_empty_line_is_refused_in_each_position(void) {
    assert_refused("a//b", "caption: line 2 is empty");
    assert_refused("a/", "caption: line 2 is empty");
    assert_refused(" / b", "caption: line 1 is empty");
    assert_refused("", "caption: line 1 is empty");
}

static void test_uppercase_is_accepted(void) {
    assert_parses_to("END THE YEAR:/Score", "END THE YEAR:\nScore");
}

enum { CAPACITY = 8 };

typedef struct {
    pad_caption_t storage[CAPACITY];
    pad_caption_track_t track;
    char error[128];
} tracker_t;

static void tracker_init(tracker_t *tracker) {
    pad_caption_track_init(&tracker->track, tracker->storage, CAPACITY);
    tracker->error[0] = '\0';
}

static int show(tracker_t *tracker, int frame, const char *text) {
    return pad_caption_track_show(&tracker->track, frame, text, tracker->error, sizeof tracker->error);
}

static int frame(tracker_t *tracker, int index, int buttons) {
    return pad_caption_track_frame(&tracker->track, index, buttons, tracker->error, sizeof tracker->error);
}

static int end(tracker_t *tracker, int frame, int checked) {
    return pad_caption_track_end(&tracker->track, frame, checked, tracker->error, sizeof tracker->error);
}

static int clear(tracker_t *tracker, int frame) {
    return pad_caption_track_clear(&tracker->track, frame, tracker->error, sizeof tracker->error);
}

static void assert_caption(const tracker_t *tracker, int index, int from, int to, const char *text) {
    assert(index < tracker->track.count);
    const pad_caption_t *caption = &tracker->track.captions[index];
    assert(caption->from == from);
    assert(caption->to == to);
    assert(strcmp(caption->text, text) == 0);
}

static void test_a_new_caption_closes_the_shown_one_and_end_closes_the_last(void) {
    tracker_t tracker;
    tracker_init(&tracker);

    assert(show(&tracker, 0, "build a trading empire\nacross the stars"));
    assert(show(&tracker, 150, "probe the dark"));
    assert(end(&tracker, 300, 1));

    assert(tracker.track.count == 2);
    assert(!tracker.track.shown);
    assert_caption(&tracker, 0, 0, 150, "build a trading empire\nacross the stars");
    assert_caption(&tracker, 1, 150, 300, "probe the dark");
}

static void test_clear_leaves_a_gap_before_the_next_caption(void) {
    tracker_t tracker;
    tracker_init(&tracker);

    assert(show(&tracker, 0, "a"));
    assert(clear(&tracker, 150));
    assert(!tracker.track.shown);
    assert(show(&tracker, 200, "b"));
    assert(end(&tracker, 360, 1));

    assert(tracker.track.count == 2);
    assert_caption(&tracker, 0, 0, 150, "a");
    assert_caption(&tracker, 1, 200, 360, "b");
}

static void test_clear_with_nothing_shown_fails(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(!clear(&tracker, 0));
    assert(strcmp(tracker.error, "caption -: no caption shown") == 0);

    assert(show(&tracker, 0, "a"));
    assert(clear(&tracker, 150));
    assert(!clear(&tracker, 160));
    assert(strcmp(tracker.error, "caption -: no caption shown") == 0);
}

static void assert_error(const tracker_t *tracker, const char *expected) {
    if (strcmp(tracker->error, expected) != 0) {
        fprintf(stderr, "error '%s', expected '%s'\n", tracker->error, expected);
    }
    assert(strcmp(tracker->error, expected) == 0);
}

static void test_a_caption_shown_149_frames_fails_and_150_passes(void) {
    for (int closer = 0; closer < 3; closer++) {
        tracker_t tracker;
        tracker_init(&tracker);
        assert(show(&tracker, 10, "a"));
        int closed = closer == 0 ? show(&tracker, 159, "b") : closer == 1 ? clear(&tracker, 159) : end(&tracker, 159, 1);
        assert(!closed);
        assert_error(&tracker, "caption: shown 149 frames, at least 150");

        tracker_init(&tracker);
        assert(show(&tracker, 10, "a"));
        closed = closer == 0 ? show(&tracker, 160, "b") : closer == 1 ? clear(&tracker, 160) : end(&tracker, 160, 1);
        assert(closed);
        assert_caption(&tracker, 0, 10, 160, "a");
    }
}

static void test_a_caption_opened_and_closed_on_one_frame_fails(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(show(&tracker, 40, "a"));
    assert(!show(&tracker, 40, "b"));
    assert_error(&tracker, "caption: shown 0 frames, at least 150");
}

static void test_an_unchecked_end_closes_a_short_caption(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(show(&tracker, 0, "a"));
    assert(show(&tracker, 150, "b"));
    assert(end(&tracker, 151, 0));

    assert(!tracker.track.shown);
    assert_caption(&tracker, 1, 150, 151, "b");
}

static void test_an_end_with_nothing_shown_passes(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(end(&tracker, 0, 1));
    assert(tracker.track.count == 0);
}

static void test_a_button_down_89_frames_after_a_caption_fails_and_90_passes(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(show(&tracker, 200, "a"));

    assert(!frame(&tracker, 289, 0x10));
    assert_error(&tracker, "caption: a button down 89 frames after the caption appeared, at least 90");
    assert(!frame(&tracker, 200, 0x01));
    assert_error(&tracker, "caption: a button down 0 frames after the caption appeared, at least 90");
    assert(!frame(&tracker, 201, 0x01));
    assert_error(&tracker, "caption: a button down 1 frame after the caption appeared, at least 90");
    assert(frame(&tracker, 290, 0x10));
}

static void test_an_empty_frame_passes_at_any_index(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(show(&tracker, 200, "a"));
    for (int i = 200; i < 400; i++) {
        assert(frame(&tracker, i, 0));
    }
}

static void test_any_frame_passes_with_no_caption_shown(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(frame(&tracker, 0, 0x3f));

    assert(show(&tracker, 0, "a"));
    assert(clear(&tracker, 150));
    assert(frame(&tracker, 151, 0x3f));
}

static void test_the_member_has_one_caption_per_line_and_escapes_the_line_break(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    assert(show(&tracker, 0, "build a trading empire\nacross the stars"));
    assert(show(&tracker, 150, "probe the dark"));
    assert(end(&tracker, 330, 1));

    char member[512];
    int length = pad_caption_write_member(&tracker.track, member, sizeof member);

    const char *expected = "\"captions\": [\n"
                           "    { \"from\": 0, \"to\": 150, \"text\": \"build a trading empire\\nacross the stars\" },\n"
                           "    { \"from\": 150, \"to\": 330, \"text\": \"probe the dark\" }\n"
                           "  ]";
    if (strcmp(member, expected) != 0) {
        fprintf(stderr, "member:\n%s\nexpected:\n%s\n", member, expected);
    }
    assert(strcmp(member, expected) == 0);
    assert(length == (int)strlen(expected));
}

static void test_no_caption_writes_no_member(void) {
    tracker_t tracker;
    tracker_init(&tracker);
    char member[16] = "untouched";
    assert(pad_caption_write_member(&tracker.track, member, sizeof member) == 0);
    assert(member[0] == '\0');
}

int main(void) {
    test_a_slash_breaks_the_line_and_spaces_around_it_are_dropped();
    test_a_line_holds_32_characters_not_33();
    test_two_lines_are_accepted_not_three();
    test_the_letter_v_is_refused_in_both_cases();
    test_a_character_without_a_glyph_is_refused();
    test_the_letter_v_is_checked_before_glyphs();
    test_an_empty_line_is_refused_in_each_position();
    test_uppercase_is_accepted();
    test_a_new_caption_closes_the_shown_one_and_end_closes_the_last();
    test_clear_leaves_a_gap_before_the_next_caption();
    test_clear_with_nothing_shown_fails();
    test_a_caption_shown_149_frames_fails_and_150_passes();
    test_a_caption_opened_and_closed_on_one_frame_fails();
    test_an_unchecked_end_closes_a_short_caption();
    test_an_end_with_nothing_shown_passes();
    test_a_button_down_89_frames_after_a_caption_fails_and_90_passes();
    test_an_empty_frame_passes_at_any_index();
    test_any_frame_passes_with_no_caption_shown();
    test_the_member_has_one_caption_per_line_and_escapes_the_line_break();
    test_no_caption_writes_no_member();

    printf("test_pad_caption: all tests passed\n");
    return 0;
}
