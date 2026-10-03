/* The screen capture made by the headless replay driver: as many frames
 * written as frames replayed, and the same pixels as the game drew.
 *
 * THIS TEST RUNS THE REAL DRIVER, NOT A COPY. tmp/replay is invoked by
 * system() on a short fixture. Redoing the driver's sequence here
 * (cart_load, cart_init, then per frame cheat_step, cart_update60,
 * cart_draw and the capture write) would add one more copy of a sequence
 * that src/cmd/game.c and src/cmd/replay.c already hold, so one more
 * subtly different replay: the test would prove that a replay writes the
 * right pixels, and say nothing of the one `make replay` runs.
 *
 * Each replay run here rewrites the driver's default dump with the short
 * fixture's; test_frames_tool and `make replay EVENTS=...` rewrite it
 * too. */

#include "devtools/dump.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "devtools/screencast.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIXTURE "tests/fixtures/screencast_capture_events.json"
#define DRIVER "./tmp/replay"
/* The default dump path of src/cmd/replay.c, when argv[3] is missing.
 * Fidelity AND neutrality are checked on this output. */
#define DRIVER_DUMP "tmp/replay_dump.txt"
#define CUSTOM_DUMP "tmp/test_headless_capture_custom_dump.txt"
#define DUMP_WITNESS "witness: this dump must not be rewritten\n"
#define CAPTURE "tmp/test_headless_capture.bin"
#define STDERR_LOG "tmp/test_headless_capture_stderr.txt"
/* A path whose parent directory does not exist: the invalid path case,
 * reached without depending on file system permissions or on a full
 * disk. */
#define BAD_CAPTURE "tmp/no_such_directory_for_capture/capture.bin"

/* The whole content of `path`, with one more NUL byte at the end so that
 * strstr() can be used on text outputs. `*out_len` is the real size,
 * without that NUL: dump comparisons go through memcmp on that length and
 * so do not depend on it. Allocated buffer: the caller frees it. */
static char *read_whole_file(const char *path, size_t *out_len) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size > 0);
    rewind(file);

    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    fclose(file);
    buf[size] = '\0';

    *out_len = (size_t)size;
    return buf;
}

/* The text that dump_screen produces for a screen whose pixels are those
 * of `pixels`. The comparison goes through THE REAL dumper and not a hex
 * decoder written here: a test that rebuilt the dump format itself would
 * compare its own idea of that format with the driver's -- two things
 * that can drift together with nothing turning red.
 *
 * Allocated buffer, freed by the caller. It is NOT NUL-terminated (unlike
 * read_whole_file above): only memcmp over `*out_len` is valid on it. */
static char *dump_text_of_pixels(const unsigned char *pixels, size_t *out_len) {
    p8_memory_t mem;
    p8_mem_init(&mem);
    memcpy(mem.screen, pixels, sizeof(mem.screen));

    FILE *tmp = tmpfile();
    assert(tmp != NULL);
    assert(dump_screen(&mem, tmp));
    long size = ftell(tmp);
    assert(size > 0);
    rewind(tmp);

    char *buf = malloc((size_t)size);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, tmp) == (size_t)size);
    fclose(tmp);

    *out_len = (size_t)size;
    return buf;
}

/* How many frames the fixture holds, read by the same loader as the
 * driver's rather than hard-coded here. The property aimed at is "as many
 * frames written as frames replayed"; a hard-coded constant would only
 * say "eight", and would stop speaking of the replay the day the fixture
 * changes. */
static int fixture_frame_count(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, FIXTURE));
    int count = input.frame_count;
    p8_input_free(&input);
    return count;
}

static void test_capture_holds_exactly_the_replayed_frames(void) {
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);

    screencast_reader_t reader;
    assert(screencast_reader_open(&reader, CAPTURE) == SCREENCAST_OK);
    assert(screencast_frame_count(&reader) == fixture_frame_count());
    screencast_reader_close(&reader);

    remove(CAPTURE);
}

/* CAPTURE FIDELITY. The last frame is the only possible point of
 * comparison: the driver's text dump describes that frame alone. If it
 * matches, the capture writes what the game drew, at the right moment --
 * neither the frame before, nor a half-drawn screen. */
static void test_last_captured_frame_matches_the_text_dump(void) {
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);

    size_t dump_len;
    char *dump = read_whole_file(DRIVER_DUMP, &dump_len);

    screencast_reader_t reader;
    assert(screencast_reader_open(&reader, CAPTURE) == SCREENCAST_OK);
    int last = screencast_frame_count(&reader) - 1;
    assert(last > 0);

    unsigned char last_pixels[SCREENCAST_FRAME_PIXELS];
    assert(screencast_read_frame(&reader, last, last_pixels));

    /* The second-to-last frame is a witness, not the subject: it shows
     * that this fixture draws two DIFFERENT images at the end. Without
     * that, a capture shifted by one frame would pass the comparison
     * below by accident, and this test would no longer prove anything
     * about the moment the write happens. */
    unsigned char previous_pixels[SCREENCAST_FRAME_PIXELS];
    assert(screencast_read_frame(&reader, last - 1, previous_pixels));
    assert(memcmp(previous_pixels, last_pixels, sizeof(last_pixels)) != 0);
    screencast_reader_close(&reader);

    size_t captured_len;
    char *captured = dump_text_of_pixels(last_pixels, &captured_len);

    assert(captured_len == dump_len);
    assert(memcmp(captured, dump, dump_len) == 0);

    free(captured);
    free(dump);
    remove(CAPTURE);
}

/* CAPTURE NEUTRALITY: a replay with a capture and a replay without one
 * produce the same final text dump, the guarantee that the instrument
 * does not alter what it measures. The final dump is the only observation
 * this test compares; if it is identical on both sides, the capture
 * changed nothing of the final screen.
 *
 * The same test covers the other half of the requirement -- the
 * behaviour without a capture argument is unchanged -- since the replay
 * without a capture runs here as is and must produce that dump. */
static void test_capturing_changes_nothing_the_replay_produces(void) {
    remove(DRIVER_DUMP);
    assert(system(DRIVER " " FIXTURE " > /dev/null") == 0);
    size_t without_len;
    char *without = read_whole_file(DRIVER_DUMP, &without_len);

    remove(DRIVER_DUMP);
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);
    size_t with_len;
    char *with = read_whole_file(DRIVER_DUMP, &with_len);

    assert(with_len == without_len);
    assert(memcmp(with, without, without_len) == 0);

    free(with);
    free(without);
    remove(CAPTURE);
}

/* A capture that cannot start makes the driver print its failure message
 * and return 1. This is the opposite policy to
 * session_recorder_record_frame, which drops a frame it cannot append,
 * and it flips here because nobody is playing: a truncated file would
 * silently yield a wrong sha256, that is a false red indistinguishable
 * from a regression. A driver that went on without capturing would have
 * returned 0, and `make e2e` could have hashed the file of an earlier
 * replay.
 *
 * The failure message is asserted, not only the exit status: it is the
 * only human interface of this failure, and it names the path, since a
 * mistyped path is only fixed by reading it again. */
static void test_unopenable_capture_path_is_fatal_and_names_the_path(void) {
    remove(STDERR_LOG);
    assert(system(DRIVER " " FIXTURE " " BAD_CAPTURE " > /dev/null 2> " STDERR_LOG) != 0);

    size_t log_len;
    char *log = read_whole_file(STDERR_LOG, &log_len);
    assert(strstr(log, BAD_CAPTURE) != NULL);

    free(log);
    remove(STDERR_LOG);
}

/* Variant replays run side by side must not overwrite each other's dump:
 * with argv[3] the dump goes there, and the default path -- read by this
 * test and by test_frames_tool -- keeps its witness. */
static void test_third_argument_redirects_the_text_dump(void) {
    remove(DRIVER_DUMP);
    assert(system(DRIVER " " FIXTURE " > /dev/null") == 0);
    size_t default_len;
    char *default_dump = read_whole_file(DRIVER_DUMP, &default_len);

    FILE *witness = fopen(DRIVER_DUMP, "w");
    assert(witness != NULL);
    assert(fputs(DUMP_WITNESS, witness) >= 0);
    assert(fclose(witness) == 0);

    remove(CUSTOM_DUMP);
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " " CUSTOM_DUMP " > /dev/null") == 0);

    size_t custom_len;
    char *custom = read_whole_file(CUSTOM_DUMP, &custom_len);
    assert(custom_len == default_len);
    assert(memcmp(custom, default_dump, default_len) == 0);

    size_t untouched_len;
    char *untouched = read_whole_file(DRIVER_DUMP, &untouched_len);
    assert(untouched_len == strlen(DUMP_WITNESS));
    assert(memcmp(untouched, DUMP_WITNESS, untouched_len) == 0);

    free(untouched);
    free(custom);
    free(default_dump);
    remove(CUSTOM_DUMP);
    remove(CAPTURE);
    remove(DRIVER_DUMP);
}

int main(void) {
    test_capture_holds_exactly_the_replayed_frames();
    test_last_captured_frame_matches_the_text_dump();
    test_capturing_changes_nothing_the_replay_produces();
    test_unopenable_capture_path_is_fatal_and_names_the_path();
    test_third_argument_redirects_the_text_dump();
    printf("test_headless_capture: all tests passed\n");
    return 0;
}
