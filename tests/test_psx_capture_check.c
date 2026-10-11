/* tools/psx_capture_check.sh (the seven GPU capture conditions of make psx-check) and
 * tools/psx_capture_record.sh (the ceiling and acceptance writers), on crafted run directories:
 * no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_psx_capture_check_work"
#define RUN DIR "/run"
#define LOG DIR "/log.txt"
#define CHECK                                                                                          \
    "sh tools/psx_capture_check.sh " DIR "/list.txt " DIR "/pc " DIR "/gpu.tty.txt " RUN "/gpu " DIR \
    "/repeat/gpu " RUN "/gpu-captures " DIR "/ceilings.txt " DIR "/references.txt " RUN "/conditions.txt"
#define RECORD "sh tools/psx_capture_record.sh "

enum { CAPTURE_PIXEL_BYTES = 153600 };

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) return buf;
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void write_capture(const char *path, const char *header, unsigned char fill) {
    static unsigned char pixels[CAPTURE_PIXEL_BYTES];
    memset(pixels, fill, sizeof pixels);
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    fputs(header, file);
    assert(fwrite(pixels, 1, sizeof pixels, file) == sizeof pixels);
    fclose(file);
}

static void flip_byte(const char *path, long offset) {
    FILE *file = fopen(path, "r+b");
    assert(file != NULL);
    fseek(file, offset, SEEK_SET);
    int byte = fgetc(file);
    fseek(file, offset, SEEK_SET);
    fputc(byte ^ 0xFF, file);
    fclose(file);
}

#define REPORT_CLS "cls f1 mask cls calls 1 differ 0 (0.0\xe2\x80\xb0) near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform 0\n"
#define REPORT_RECT(differ, nonuniform)                                                                       \
    "rect f5 mask cls,rect calls 2 differ " differ " (0.1\xe2\x80\xb0) near1 1 near2 0 far 1 bbox 3,4-5,6 " \
    "offpalette 0 nonuniform " nonuniform "\n"

/* A run where every condition holds, the rect capture 3 pixels off with a ceiling of 3. */
static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/pc " RUN "/gpu-captures " DIR "/repeat") == 0);
    write_file(DIR "/list.txt", "# test\nreplay pad_slipway\n\ncls 1 cls\nrect 5 cls,rect\n");
    write_file(DIR "/pc/capture_script.txt", "1 0001\n5 0009\n");
    write_file(DIR "/pc/counts.txt", "1 cls 1\n1 rect 0\n5 cls 1\n5 rect 2\n");
    write_file(DIR "/gpu.tty.txt", "psx-replay: backend gpu\r\npsx-replay: capture 1 0001\r\n"
                                   "psx-replay: capture 5 0009\npsx-replay: done pad_slipway 10 vsyncs 50\n");
    write_capture(RUN "/gpu.f1.vram", "psx-vram 320 240 16\n", 0x00);
    write_capture(RUN "/gpu.f5.vram", "psx-vram 320 240 16\n", 0x80);
    write_capture(DIR "/repeat/gpu.f1.vram", "psx-vram 320 240 16\n", 0x00);
    write_capture(DIR "/repeat/gpu.f5.vram", "psx-vram 320 240 16\n", 0x80);
    write_file(RUN "/gpu-captures/report.txt", REPORT_CLS REPORT_RECT("3", "0"));
    const char *outputs[] = { "cls-f1.diff.txt", "cls-f1.ppm", "rect-f5.diff.txt", "rect-f5.ppm" };
    for (size_t i = 0; i < sizeof outputs / sizeof outputs[0]; i++) {
        char path[256];
        snprintf(path, sizeof path, RUN "/gpu-captures/%s", outputs[i]);
        write_file(path, "x");
    }
    write_file(DIR "/ceilings.txt", "cls 1 0\nrect 5 3\n");
    write_file(DIR "/references.txt", "");
}

static int check(void) {
    remove(RUN "/conditions.txt");
    return system(CHECK " > " LOG " 2>&1");
}

static void assert_logged(const char *needle) {
    const char *log = read_file(LOG);
    if (strstr(log, needle) == NULL) {
        fprintf(stderr, "log lacks '%s':\n%s", needle, log);
        assert(0);
    }
}

static void assert_status(const char *condition, const char *status) {
    char line[256];
    snprintf(line, sizeof line, "%s %s\n", condition, status);
    assert(strstr(read_file(RUN "/conditions.txt"), line) != NULL);
}

static void assert_failed(const char *condition, const char *needle) {
    assert_status(condition, "failed");
    char line[512];
    snprintf(line, sizeof line, "psx-check: %s: ", condition);
    const char *log = read_file(LOG);
    const char *at = strstr(log, line);
    assert(at != NULL);
    const char *end = strchr(at, '\n');
    if (strstr(at, needle) == NULL || (end != NULL && strstr(at, needle) > end)) {
        fprintf(stderr, "%s line lacks '%s':\n%s", condition, needle, log);
        assert(0);
    }
}

static void assert_passed(const char *condition) {
    assert_status(condition, "ok");
}

static void test_a_good_run_passes_with_every_capture_pending(void) {
    setup();
    assert(check() == 0);
    assert_logged("gpu-capture-list: ok");
    assert_logged("gpu-captures: ok");
    assert_logged("gpu-capture-deterministic: ok");
    assert_logged("gpu-capture-clean: ok");
    assert_logged("gpu-capture-report: ok");
    assert_logged("gpu-capture-ceilings: ok");
    assert_logged("gpu-capture-references: ok");
    assert_logged("pending human acceptance: cls f1");
    assert_logged("pending human acceptance: rect f5");
    assert(strstr(read_file(LOG), "below ceiling") == NULL && "a count equal to its ceiling is silent");
    assert(strstr(read_file(RUN "/conditions.txt"), "failed") == NULL);
}

/* Each condition reads its own inputs: one crafted run can break several at once. */
static void test_faults_of_the_first_kind_each_fail_their_condition(void) {
    setup();
    write_file(DIR "/pc/counts.txt", "1 cls 1\n5 cls 1\n5 rect 0\n");
    write_file(DIR "/gpu.tty.txt", "psx-replay: capture 1 0001\npsx-replay: capture 5 0009\n"
                                   "psx-replay: capture 7 0001\npsx-replay: done pad_slipway 10 vsyncs 50\n");
    flip_byte(DIR "/repeat/gpu.f5.vram", 5000);
    remove(RUN "/gpu.f1.vram");
    remove(DIR "/pc/capture_script.txt");
    write_file(RUN "/gpu-captures/report.txt", REPORT_CLS REPORT_RECT("3", "1"));
    remove(RUN "/gpu-captures/rect-f5.ppm");
    write_file(DIR "/ceilings.txt", "cls 1 0\nrect 5 2\n");
    write_file(DIR "/references.txt", "line 9 0000000000000000000000000000000000000000000000000000000000000000\n");
    assert(check() != 0);
    assert_failed("gpu-capture-list", "rect f5 draws no rect call on PC");
    assert_failed("gpu-capture-list", "capture_script.txt does not hold the list's frames");
    assert_failed("gpu-captures", "missing " RUN "/gpu.f1.vram");
    assert_failed("gpu-captures", "TTY capture frames '1 5 7', expected '1 5'");
    assert_failed("gpu-capture-deterministic", "frame 1 differs");
    assert_failed("gpu-capture-clean", "rect f5: nonuniform 1 offpalette 0");
    assert_failed("gpu-capture-report", "rect-f5.ppm");
    assert_failed("gpu-capture-ceilings", "rect f5 differ 3 above its ceiling 2");
    assert_failed("gpu-capture-references", "reference for a capture no longer listed: line f9");
}

static void test_faults_of_the_second_kind_each_fail_their_condition(void) {
    setup();
    write_file(DIR "/list.txt", "replay pad_title\ncls 1 cls\nrect 5 cls,rect\n");
    write_capture(RUN "/gpu.f1.vram", "psx-vram 320 240 24\n", 0x00);
    write_capture(DIR "/repeat/gpu.f1.vram", "psx-vram 320 240 24\n", 0x00);
    write_file(RUN "/gpu-captures/report.txt", REPORT_CLS);
    write_file(DIR "/ceilings.txt", "cls 1 0\nline 9 4\n");
    write_file(DIR "/references.txt", "cls 1 0000000000000000000000000000000000000000000000000000000000000000\n");
    assert(check() != 0);
    assert_failed("gpu-capture-list", "replay pad_title, the PS-EXE replays pad_slipway");
    assert_failed("gpu-captures", "malformed " RUN "/gpu.f1.vram");
    assert_failed("gpu-capture-clean", "rect f5 has no report line");
    assert_failed("gpu-capture-report", "no report line for rect f5");
    assert_failed("gpu-capture-ceilings", "no ceiling for rect f5");
    assert_failed("gpu-capture-ceilings", "ceiling for a capture no longer listed: line f9");
    assert_failed("gpu-capture-references", "cls f1 differs from its reference");
    assert_passed("gpu-capture-deterministic");
}

static const char *sha256_of(const char *path) {
    static char hash[65];
    char command[256];
    snprintf(command, sizeof command, "(sha256sum %s 2>/dev/null || shasum -a 256 %s) | cut -c1-64 > " DIR "/hash.txt",
             path, path);
    assert(system(command) == 0);
    snprintf(hash, sizeof hash, "%s", read_file(DIR "/hash.txt"));
    assert(strlen(hash) == 64);
    return hash;
}

static void test_a_count_below_its_ceiling_and_a_matching_reference_pass(void) {
    char text[256];
    setup();
    snprintf(text, sizeof text, "cls 1 %s\n", sha256_of(RUN "/gpu.f1.vram"));
    write_file(DIR "/references.txt", text);
    write_file(DIR "/ceilings.txt", "cls 1 -1\nrect 5 4\n");
    assert(check() == 0 && "a capture matching its reference ignores its ceiling");
    assert_logged("below ceiling: rect f5 4 -> 3");
    assert_logged("pending human acceptance: rect f5");
    assert(strstr(read_file(LOG), "pending human acceptance: cls f1") == NULL);
}

static int record(const char *kind, const char *label, const char *file) {
    char command[512];
    snprintf(command, sizeof command, RECORD "%s " RUN " %s %s > " LOG " 2>&1", kind, label, file);
    return system(command);
}

#define ALL_OK                                                                                         \
    "magic ok\ngpu-capture-list ok\ngpu-captures ok\ngpu-capture-deterministic ok\ngpu-capture-clean ok\n" \
    "gpu-capture-report ok\ngpu-capture-references ok\n"

static void test_the_ceiling_writer_touches_its_label_only(void) {
    setup();
    write_file(RUN "/conditions.txt", ALL_OK "gpu-capture-ceilings failed\n");
    write_file(DIR "/ceilings.txt", "cls 1 0\nline 3 7\n");
    assert(record("ceilings", "rect", DIR "/ceilings.txt") == 0);
    assert(strcmp(read_file(DIR "/ceilings.txt"), "cls 1 0\nline 3 7\nrect 5 3\n") == 0);

    write_file(RUN "/conditions.txt", ALL_OK "gpu-capture-ceilings ok\n");
    write_file(DIR "/ceilings.txt", "cls 1 0\nrect 5 9\n");
    assert(record("ceilings", "rect", DIR "/ceilings.txt") == 0);
    assert(strcmp(read_file(DIR "/ceilings.txt"), "cls 1 0\nrect 5 3\n") == 0 && "lowered to the count");
}

static void test_the_ceiling_writer_refuses_a_run_failing_elsewhere_or_missing(void) {
    setup();
    write_file(RUN "/conditions.txt", ALL_OK "gpu-capture-ceilings failed\ngpu-capture-deterministic failed\n");
    write_file(DIR "/ceilings.txt", "cls 1 0\n");
    assert(record("ceilings", "rect", DIR "/ceilings.txt") != 0);
    assert_logged("gpu-capture-deterministic");
    assert(strcmp(read_file(DIR "/ceilings.txt"), "cls 1 0\n") == 0);

    assert(system("rm -rf " RUN) == 0);
    assert(record("ceilings", "rect", DIR "/ceilings.txt") != 0);
    assert_logged("no make psx-check run");
}

static void test_the_acceptance_writer_needs_a_green_run(void) {
    setup();
    write_file(RUN "/conditions.txt", ALL_OK "gpu-capture-ceilings failed\n");
    assert(record("references", "rect", DIR "/references.txt") != 0);
    assert(strcmp(read_file(DIR "/references.txt"), "") == 0);

    write_file(RUN "/conditions.txt", ALL_OK "gpu-capture-ceilings ok\n");
    write_file(DIR "/references.txt", "cls 1 abc\n");
    assert(record("references", "rect", DIR "/references.txt") == 0);
    char expected[160];
    snprintf(expected, sizeof expected, "cls 1 abc\nrect 5 %s\n", sha256_of(RUN "/gpu.f5.vram"));
    assert(strcmp(read_file(DIR "/references.txt"), expected) == 0);

    snprintf(expected, sizeof expected, "rect 5 %s\n", sha256_of(RUN "/gpu.f5.vram"));
    write_file(DIR "/references.txt", expected);
    assert(check() == 0);
    assert(strstr(read_file(LOG), "pending human acceptance: rect f5") == NULL);
    assert_logged("pending human acceptance: cls f1");
}

int main(void) {
    test_a_good_run_passes_with_every_capture_pending();
    test_faults_of_the_first_kind_each_fail_their_condition();
    test_faults_of_the_second_kind_each_fail_their_condition();
    test_a_count_below_its_ceiling_and_a_matching_reference_pass();
    test_the_ceiling_writer_touches_its_label_only();
    test_the_ceiling_writer_refuses_a_run_failing_elsewhere_or_missing();
    test_the_acceptance_writer_needs_a_green_run();
    printf("test_psx_capture_check: ok\n");
    return 0;
}
