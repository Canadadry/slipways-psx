#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/demo_check.sh"
#define DIR "tmp/test_demo_check_work"
#define OUT DIR "/out.txt"
#define DEMO DIR "/demo.json"

#define RECORD                                                                             \
    "clock year 3412 month 1\n"                                                            \
    "resources credits 86\n"                                                               \
    "techs space folding\n"                                                                \
    "techs geneseeds\n"                                                                    \
    "planet p32 known planet x world 1 2 screen 3 4 visible 1\n"                           \
    "count planets 74\n"                                                                   \
    "count colonies 16\n"                                                                  \
    "count slipways 18\n"                                                                  \
    "count techs 2\n"                                                                      \
    "zone game 10 500 1000\n"                                                              \
    "cheat activations 0\n"

#define END                                                                                \
    "clock year 3413 month 1\n"                                                            \
    "resources credits 105\n"                                                              \
    "techs space folding\n"                                                                \
    "techs geneseeds\n"                                                                    \
    "techs xenofoods\n"                                                                    \
    "planet p32 colonized planet x world 1 2 screen 3 4 visible 1\n"                       \
    "count planets 74\n"                                                                   \
    "count colonies 17\n"                                                                  \
    "count slipways 19\n"                                                                  \
    "count techs 3\n"                                                                      \
    "zone game 10 500 1000\n"                                                              \
    "cheat activations 0\n"

static const char *const TEXTS[] = {
    "build a trading empire\\nacross the stars",
    "probe the dark\\nto find new planets",
    "colonize planets:\\neach one needs and makes goods",
    "link colonies with slipways\\nso goods can flow",
    "end the year:\\ntrade earns money and science",
    "spend science on new tech",
    "slipways\\npress any button",
};

#define CAPTIONS (sizeof TEXTS / sizeof TEXTS[0])

typedef struct {
    int from, to;
    const char *text;
} caption_t;

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call. */
static const char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int run(const char *arguments) {
    char command[1024];
    snprintf(command, sizeof command, SCRIPT " %s > " OUT " 2>&1", arguments);
    int status = system(command);
    return status == 0 ? 0 : 1;
}

static int output_has(const char *text) {
    return strstr(read_file(OUT), text) != NULL;
}

/* The layout pad_author writes: header, captions one per line, frames one per line. */
static void write_demo(const char *path, int frames, const caption_t *captions, size_t count) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs("{\n  \"input\": \"pad\",\n  \"start\": \"resume\",\n  \"initial_save\": \"534c\",\n", file);
    fputs("  \"captions\": [\n", file);
    for (size_t i = 0; i < count; i++)
        fprintf(file, "    { \"from\": %d, \"to\": %d, \"text\": \"%s\" }%s\n", captions[i].from, captions[i].to,
                captions[i].text, i + 1 < count ? "," : "");
    fputs("  ],\n  \"frames\": [\n", file);
    for (int i = 0; i < frames; i++) fprintf(file, "    { \"pad\": [%s] }%s\n", i == 3 ? "4" : "", i + 1 < frames ? "," : "");
    fputs("  ]\n}\n", file);
    fclose(file);
}

/* Six one-frame captions then the 180-frame closing card: 186 frames. */
static int good_captions(caption_t *captions) {
    for (size_t i = 0; i < CAPTIONS; i++) {
        captions[i].from = (int)i;
        captions[i].to = (int)i + 1;
        captions[i].text = TEXTS[i];
    }
    captions[CAPTIONS - 1].to = (int)CAPTIONS - 1 + 180;
    return captions[CAPTIONS - 1].to;
}

static void write_end_with(const char *path, const char *from, const char *to) {
    static char text[1 << 12];
    const char *at = strstr(END, from);
    assert(at != NULL && "the replaced text is in the end state");
    snprintf(text, sizeof text, "%.*s%s%s", (int)(at - END), END, to, at + strlen(from));
    write_file(path, text);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    caption_t captions[CAPTIONS];
    int frames = good_captions(captions);
    write_demo(DEMO, frames, captions, CAPTIONS);
    write_file(DIR "/record.txt", RECORD);
    write_file(DIR "/end.txt", END);
}

static void test_a_demo_one_of_each_past_the_record_passes(void) {
    assert(run(DEMO " " DIR "/record.txt " DIR "/end.txt") == 0);
    assert(output_has("demo-check: " DEMO ": 186 frames"));
    assert(run(DEMO " " DIR "/record.txt " DIR "/end.txt " DIR "/end.txt") == 0);
    assert(strcmp(read_file(OUT), "demo-check: " DEMO ": 186 frames, 7 captions, 2 end states one year, colony, "
                                  "slipway and tech past the record, no cheat\n") == 0);
}

static void write_demo_of(const char *path, int frames) {
    caption_t captions[CAPTIONS];
    good_captions(captions);
    captions[CAPTIONS - 2].to = frames - 180;
    captions[CAPTIONS - 1].from = frames - 180;
    captions[CAPTIONS - 1].to = frames;
    write_demo(path, frames, captions, CAPTIONS);
}

static void test_no_frame_fails(void) {
    caption_t captions[CAPTIONS];
    good_captions(captions);
    write_demo(DIR "/empty.json", 0, captions, CAPTIONS);
    assert(run(DIR "/empty.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/empty.json: no frame\n"));
}

static void test_more_than_3600_frames_fails(void) {
    write_demo_of(DIR "/3600.json", 3600);
    assert(run(DIR "/3600.json " DIR "/record.txt " DIR "/end.txt") == 0);
    write_demo_of(DIR "/3601.json", 3601);
    assert(run(DIR "/3601.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/3601.json: 3601 frames, more than 3600\n"));
}

static void test_a_missing_misspelt_or_swapped_caption_fails(void) {
    caption_t captions[CAPTIONS];
    int frames = good_captions(captions);
    captions[1].text = "probe the dark\\nto find new planet";
    write_demo(DIR "/misspelt.json", frames, captions, CAPTIONS);
    assert(run(DIR "/misspelt.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/misspelt.json: caption 2 reads \"probe the dark\\nto find new planet\", "
                      "expected \"probe the dark\\nto find new planets\"\n"));

    good_captions(captions);
    captions[2].text = TEXTS[3];
    captions[3].text = TEXTS[2];
    write_demo(DIR "/swapped.json", frames, captions, CAPTIONS);
    assert(run(DIR "/swapped.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/swapped.json: caption 3 reads \"link colonies"));
    assert(output_has("demo-check: " DIR "/swapped.json: caption 4 reads \"colonize planets"));

    good_captions(captions);
    captions[4].to = captions[5].to;
    captions[5] = captions[6];
    write_demo(DIR "/missing.json", frames, captions, CAPTIONS - 1);
    assert(run(DIR "/missing.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/missing.json: 6 captions, expected 7\n"));
}

static void test_captions_that_do_not_tile_the_demo_fail(void) {
    caption_t captions[CAPTIONS];
    int frames = good_captions(captions);
    captions[0].from = 1;
    write_demo(DIR "/late.json", frames, captions, CAPTIONS);
    assert(run(DIR "/late.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/late.json: caption 1 starts at 1, expected 0\n"));

    good_captions(captions);
    captions[2].to = 2;
    write_demo(DIR "/gap.json", frames, captions, CAPTIONS);
    assert(run(DIR "/gap.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/gap.json: caption 4 starts at 3, expected 2\n"));
}

static void test_a_closing_card_not_180_frames_to_the_end_fails(void) {
    caption_t captions[CAPTIONS];
    int frames = good_captions(captions);
    captions[CAPTIONS - 1].to = frames - 1;
    write_demo(DIR "/short.json", frames - 1, captions, CAPTIONS);
    assert(run(DIR "/short.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/short.json: the closing caption lasts 179 frames, expected 180\n"));

    good_captions(captions);
    write_demo(DIR "/before.json", frames + 1, captions, CAPTIONS);
    assert(run(DIR "/before.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/before.json: the closing caption ends at 186, expected 187 (the frame count)\n"));
}

static void test_an_end_state_not_exactly_one_of_each_past_the_record_fails(void) {
    static const struct {
        const char *from, *to, *reason;
    } cases[] = {
        { "year 3413", "year 3412", "year 3412, expected 3413 (the record's 3412 + 1)" },
        { "year 3413", "year 3414", "year 3414, expected 3413 (the record's 3412 + 1)" },
        { "colonies 17", "colonies 16", "colonies 16, expected 17 (the record's 16 + 1)" },
        { "colonies 17", "colonies 18", "colonies 18, expected 17 (the record's 16 + 1)" },
        { "slipways 19", "slipways 18", "slipways 18, expected 19 (the record's 18 + 1)" },
        { "slipways 19", "slipways 20", "slipways 20, expected 19 (the record's 18 + 1)" },
        { "techs 3", "techs 2", "techs 2, expected 3 (the record's 2 + 1)" },
        { "techs 3", "techs 4", "techs 4, expected 3 (the record's 2 + 1)" },
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        write_end_with(DIR "/wrong.txt", cases[i].from, cases[i].to);
        assert(run(DEMO " " DIR "/record.txt " DIR "/wrong.txt") != 0);
        char expected[256];
        snprintf(expected, sizeof expected, "demo-check: " DIR "/wrong.txt: %s\n", cases[i].reason);
        assert(output_has(expected));
    }
}

static void test_a_cheat_fails(void) {
    write_end_with(DIR "/cheat.txt", "cheat activations 0", "cheat activations 1");
    assert(run(DEMO " " DIR "/record.txt " DIR "/cheat.txt") != 0);
    assert(output_has("demo-check: " DIR "/cheat.txt: cheat activations 1, expected 0\n"));

    write_file(DIR "/cheat_record.txt", RECORD "cheat activations 2\n");
    assert(run(DEMO " " DIR "/cheat_record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/cheat_record.txt: cheat activations 2, expected 0\n"));
}

static void test_every_end_state_is_checked(void) {
    write_end_with(DIR "/second.txt", "colonies 17", "colonies 18");
    assert(run(DEMO " " DIR "/record.txt " DIR "/end.txt " DIR "/second.txt") != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "demo-check: " DIR "/second.txt: colonies 18, expected 17") != NULL);
    assert(strstr(out, DIR "/end.txt") == NULL);
}

static void test_an_unreadable_input_fails(void) {
    write_end_with(DIR "/no_slipways.txt", "count slipways 19\n", "");
    write_file(DIR "/stopped.txt", "stopped p8_fatal frame 42\n" END);
    assert(run(DEMO " " DIR "/record.txt " DIR "/no_slipways.txt " DIR "/stopped.txt " DIR "/absent.txt") != 0);
    assert(output_has("demo-check: " DIR "/no_slipways.txt: no count slipways line\n"));
    assert(output_has("demo-check: " DIR "/stopped.txt: stopped\n"));
    assert(output_has("demo-check: " DIR "/absent.txt: no state\n"));

    assert(run(DEMO " " DIR "/absent.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/absent.txt: no state\n"));

    assert(run(DIR "/absent.json " DIR "/record.txt " DIR "/end.txt") != 0);
    assert(output_has("demo-check: " DIR "/absent.json: no file\n"));
}

int main(void) {
    reset();
    test_a_demo_one_of_each_past_the_record_passes();
    test_no_frame_fails();
    test_more_than_3600_frames_fails();
    test_a_missing_misspelt_or_swapped_caption_fails();
    test_captions_that_do_not_tile_the_demo_fail();
    test_a_closing_card_not_180_frames_to_the_end_fails();
    test_an_end_state_not_exactly_one_of_each_past_the_record_fails();
    test_a_cheat_fails();
    test_every_end_state_is_checked();
    test_an_unreadable_input_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_demo_check: all tests passed\n");
    return 0;
}
